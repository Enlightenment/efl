#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "evas_cserve2.h"

#ifdef CSERVE2_BIN_DEFAULT_COLOR
#undef CSERVE2_BIN_DEFAULT_COLOR
#endif
#define CSERVE2_BIN_DEFAULT_COLOR EINA_COLOR_BLUE

#define MAX_SLAVES 3

struct _Slave_Worker {
   EINA_INLIST;
   void *data;
   Slave *slave;
   Eina_Binbuf *ret;
   int ret_size;
   Eina_Bool done;
   Eina_Bool delete_me;
};

typedef struct _Slave_Worker Slave_Worker;

int _evas_cserve2_bin_log_dom = -1;
static unsigned int _client_id = 0;
static Eina_Hash *client_list = NULL;
static Eina_Inlist *slaves_idle = NULL;
static Eina_Inlist *slaves_working = NULL;

struct _Glyph_Request {
   unsigned int index;
   unsigned int offset;
};

typedef struct _Glyph_Request Glyph_Request;

void
cserve2_client_error_send(Client *client, unsigned int rid, int error_code)
{
   int size;
   Msg_Error msg;

    // clear the struct with possible paddings, since it is not aligned.
    memset(&msg, 0, sizeof(msg));
    msg.base.rid = rid;
    msg.base.type = CSERVE2_ERROR;
    msg.error = error_code;

    size = sizeof(msg);
    cserve2_client_send(client, &size, sizeof(size));
    cserve2_client_send(client, &msg, sizeof(msg));
}

static void
_cserve2_client_image_setoptsed(Client *client, unsigned int rid)
{
   int size;
   Msg_Setoptsed msg;

   memset(&msg, 0, sizeof(msg));
   msg.base.rid = rid;
   msg.base.type = CSERVE2_SETOPTSED;

   size = sizeof(msg);
   cserve2_client_send(client, &size, sizeof(size));
   cserve2_client_send(client, &msg, size);
}

static void
_slave_dead_cb(Slave *s __UNUSED__, void *data)
{
   Slave_Worker *sw = data;

   if (sw->delete_me)
     {
        DBG("Slave killed by cserve2. Restart routine.");
        free(sw);
        return;
     }

   if (!sw->data)
     {
        WRN("Slave died with no pending job, but not requested.");
        slaves_idle = eina_inlist_remove(slaves_idle, EINA_INLIST_GET(sw));
        free(sw);
        return;
     }

   slaves_working = eina_inlist_remove(slaves_working, EINA_INLIST_GET(sw));
   if (!sw->done)
     cserve2_cache_requests_response(ERROR, (Error_Type[]){ CSERVE2_LOADER_DIED }, sw->data);
   if (sw->ret)
     eina_binbuf_free(sw->ret);
   free(sw);
}

static void
_slave_read_cb(Slave *s __UNUSED__, Slave_Command cmd, void *msg, void *data)
{
   Slave_Worker *sw = data;

   DBG("Received reply command '%d' from slave '%p'", cmd, sw->slave);
   switch (cmd)
     {
      case IMAGE_OPEN:
      case IMAGE_LOAD:
         sw->done = EINA_TRUE;
         break;
      case ERROR:
         break;
      default:
         ERR("Unrecognized command received from slave: %d", cmd);
     }
   cserve2_cache_requests_response(cmd, msg, sw->data);
   free(msg);

   // slave finishes its work, put it back to idle list
   sw->data = NULL;
   slaves_working = eina_inlist_remove(slaves_working, EINA_INLIST_GET(sw));

   if (!sw->delete_me) // if it is being deleted, it shouldn't be in any list
     slaves_idle = eina_inlist_append(slaves_idle, EINA_INLIST_GET(sw));

   cserve2_cache_requests_process();
}

int
cserve2_slave_available_get(void)
{
    return MAX_SLAVES - eina_inlist_count(slaves_working);
}

Eina_Bool
cserve2_slave_cmd_dispatch(void *data, Slave_Command cmd, const void *msg, int size)
{
   Slave_Worker *sw;
   char *exe;

   DBG("Dispatching command to slave. %d idle slaves, %d working slaves.",
       eina_inlist_count(slaves_idle), eina_inlist_count(slaves_working));

   // first check if there's an available slave
   if (slaves_idle)
     {
        sw = EINA_INLIST_CONTAINER_GET(slaves_idle, Slave_Worker);
        slaves_idle = eina_inlist_remove(slaves_idle, slaves_idle);
        slaves_working = eina_inlist_append(slaves_working,
                                            EINA_INLIST_GET(sw));

        sw->data = data;
        DBG("Dispatching command '%d' to slave '%p'", cmd, sw->slave);
        cserve2_slave_send(sw->slave, cmd, msg, size);
        return EINA_TRUE;
     }

   // no available slave, start a new one
   sw = calloc(1, sizeof(Slave_Worker));
   if (!sw) return EINA_FALSE;

   sw->data = data;
   exe = getenv("EVAS_CSERVE2_SLAVE");
   if (!exe) exe = "evas_cserve2_slave";
   sw->slave = cserve2_slave_run(exe, _slave_read_cb, _slave_dead_cb, sw);
   if (!sw->slave)
     {
        ERR("Could not launch slave process");
        cserve2_cache_requests_response(ERROR, (Error_Type[]){ CSERVE2_LOADER_EXEC_ERR }, sw->data);
        free(sw);
        return EINA_FALSE;
     }
   DBG("Dispatching command '%d' to slave '%p'", cmd, sw->slave);
   cserve2_slave_send(sw->slave, cmd, msg, size);

   slaves_working = eina_inlist_append(slaves_working, EINA_INLIST_GET(sw));

   return EINA_TRUE;
}

static void
_cserve2_client_close(Client *client)
{
   Msg_Close *msg = (Msg_Close *)client->msg.buf;

   INF("Received CLOSE command: RID=%d", msg->base.rid);
   INF("File_ID: %d\n", msg->file_id);

   cserve2_cache_file_close(client, msg->file_id);
}

static void
_cserve2_client_unload(Client *client)
{
   Msg_Unload *msg = (Msg_Unload *)client->msg.buf;

   INF("Received UNLOAD command: RID=%d", msg->base.rid);
   INF("Image_ID: %d\n", msg->image_id);

   cserve2_cache_image_unload(client, msg->image_id);
}

static void
_cserve2_client_preload(Client *client)
{
   Msg_Preload *msg = (Msg_Preload *)client->msg.buf;

   INF("Received PRELOAD command: RID=%d", msg->base.rid);
   INF("Image_ID: %d\n", msg->image_id);

   cserve2_cache_image_preload(client, msg->image_id, msg->base.rid);
   cserve2_cache_requests_process();
}

static void
_cserve2_client_load(Client *client)
{
   Msg_Load *msg = (Msg_Load *)client->msg.buf;

   INF("Received LOAD command: RID=%d", msg->base.rid);
   INF("Image_ID: %d\n", msg->image_id);

   cserve2_cache_image_load(client, msg->image_id, msg->base.rid);
   cserve2_cache_requests_process();
}

static void
_cserve2_client_setopts(Client *client)
{
   Msg_Setopts *msg = (Msg_Setopts *)client->msg.buf;

   INF("Received SETOPTS command: RID=%d", msg->base.rid);
   INF("File_ID: %d, Image_ID: %d", msg->file_id, msg->image_id);
   INF("Load Options:");
   INF("\tdpi: %03.1f", msg->opts.dpi);
   INF("\tsize: %dx%d", msg->opts.w, msg->opts.h);
   INF("\tscale down: %d", msg->opts.scale_down);
   INF("\tregion: %d,%d + %dx%d",
          msg->opts.rx, msg->opts.ry, msg->opts.rw, msg->opts.rh);
   INF("\torientation: %d\n", msg->opts.orientation);

   if (cserve2_cache_image_opts_set(client, msg) != 0)
     return;

   _cserve2_client_image_setoptsed(client, msg->base.rid);
}

static void
_cserve2_client_open(Client *client)
{
   Msg_Open *msg = (Msg_Open *)client->msg.buf;
   const char *path, *key;

   path = ((const char *)msg) + sizeof(*msg) + msg->path_offset;
   key = ((const char *)msg) + sizeof(*msg) + msg->key_offset;

   INF("Received OPEN command: RID=%d", msg->base.rid);
   INF("File_ID: %d, path=\"%s\", key=\"%s\"\n",
          msg->file_id, path, key);

   cserve2_cache_file_open(client, msg->file_id, path, key, msg->base.rid);
   cserve2_cache_requests_process();
}

static void
_cserve2_client_font_load(Client *client)
{
   Msg_Font_Load *msg = (Msg_Font_Load *)client->msg.buf;
   char name[PATH_MAX];

   memcpy(name, msg + 1, msg->pathlen);

   INF("Received %s command: RID=%d",
       (msg->base.type == CSERVE2_FONT_LOAD) ? "FONT_LOAD" : "FONT_UNLOAD",
       msg->base.rid);
   INF("Font: %s, rend_flags: %d, hint: %d, size: %d, dpi: %d",
       name, msg->rend_flags, msg->hint, msg->size, msg->dpi);

   cserve2_cache_font_load(client, name, msg->pathlen, msg->rend_flags,
                           msg->hint, msg->size, msg->dpi, msg->base.rid);
}

static void
_cserve2_client_font_glyphs_request(Client *client)
{
   Msg_Font_Glyphs_Request *msg = (Msg_Font_Glyphs_Request *)client->msg.buf;
   char fontpath[PATH_MAX];
   Glyph_Request *glyphs;
   unsigned int i;
   const char *bufpos = client->msg.buf;

   memcpy(fontpath, msg + 1, msg->pathlen);

   bufpos = bufpos + sizeof(msg) + msg->pathlen;
   glyphs = malloc(sizeof(*glyphs) * msg->nglyphs);

   for (i = 0; i < msg->nglyphs; i++)
     {
        memcpy(&glyphs[i], bufpos, sizeof(*glyphs));
        bufpos += sizeof(*glyphs);
     }

   if (msg->base.type == CSERVE2_FONT_GLYPHS_LOAD)
     {
        INF("Received CSERVE2_FONT_GLYPHS_LOAD command: RID=%d",
            msg->base.rid);
     }
   else
     {
        INF("Received CSERVE2_FONT_GLYPHS_USED command: RID=%d",
            msg->base.rid);
     }
}

void
cserve2_command_run(Client *client, Message_Type type)
{
   switch (type)
     {
      case CSERVE2_OPEN:
         _cserve2_client_open(client);
         break;
      case CSERVE2_SETOPTS:
         _cserve2_client_setopts(client);
         break;
      case CSERVE2_LOAD:
         _cserve2_client_load(client);
         break;
      case CSERVE2_PRELOAD:
         _cserve2_client_preload(client);
         break;
      case CSERVE2_UNLOAD:
         _cserve2_client_unload(client);
         break;
      case CSERVE2_CLOSE:
         _cserve2_client_close(client);
         break;
      case CSERVE2_FONT_LOAD:
      case CSERVE2_FONT_UNLOAD:
         _cserve2_client_font_load(client);
         break;
      case CSERVE2_FONT_GLYPHS_LOAD:
      case CSERVE2_FONT_GLYPHS_USED:
         _cserve2_client_font_glyphs_request(client);
         break;
      default:
         WRN("Unhandled message");
     }
}

static void
_slave_quit_send(Slave_Worker *sw)
{
   cserve2_slave_send(sw->slave, SLAVE_QUIT, NULL, 0);
}

static void
_slaves_restart(void)
{
   Slave_Worker *list, *sw;

   while (slaves_idle) // remove idle workers from idle list
     {
        sw = EINA_INLIST_CONTAINER_GET(slaves_idle, Slave_Worker);
        slaves_idle = eina_inlist_remove(slaves_idle, slaves_idle);
        sw->delete_me = EINA_TRUE;
        _slave_quit_send(sw);
     }

   // working workers will be removed from the working list when they
   // finish processing their jobs
   list = EINA_INLIST_CONTAINER_GET(slaves_working, Slave_Worker);
   EINA_INLIST_FOREACH(list, sw)
     {
        sw->delete_me = EINA_TRUE;
        _slave_quit_send(sw);
     }
}

static void
_timeout_cb(void)
{
   static unsigned int slaves_restart = 0;

   slaves_restart++;

   if (slaves_restart == 10)
     {
        DBG("kill slaves");
        _slaves_restart();
        slaves_restart = 0;
     }

   cserve2_timeout_cb_set(3000, _timeout_cb);
}

void
cserve2_client_accept(int fd)
{
   Client *client = calloc(1, sizeof(*client));

   client->socket = fd;
   client->id = _client_id++;

   while (eina_hash_find(client_list, &client->id))
     client->id = _client_id++;

   if (!eina_hash_add(client_list, &client->id, client))
     {
        Eina_Error err = eina_error_get();
        ERR("Could not add client to the list: \"%s\"",
            eina_error_msg_get(err));
        free(client);
        close(fd);
     }

   cserve2_fd_watch_add(fd, FD_READ | FD_ERROR, cserve2_message_handler,
                        client);
   INF("Client %d connection accepted.", client->id);

   cserve2_cache_client_new(client);
}

void
cserve2_client_del(Client *client)
{
   eina_hash_del_by_key(client_list, &client->id);
}

static void
_client_free(void *data)
{
   Client *client = data;
   cserve2_cache_client_del(client);
   if (client->msg.pending)
       eina_binbuf_free(client->msg.pending);
   cserve2_fd_watch_del(client->socket);
   close(client->socket);
   free(data);
}

static void
_clients_setup(void)
{
   client_list = eina_hash_int32_new(_client_free);
}

static void
_clients_finish(void)
{
   eina_hash_free(client_list);
}

int
main(int argc __UNUSED__, const char *argv[] __UNUSED__)
{
   eina_init();

   _evas_cserve2_bin_log_dom = eina_log_domain_register
      ("evas_cserve2_bin", CSERVE2_BIN_DEFAULT_COLOR);
   if (_evas_cserve2_bin_log_dom < 0)
     {
        EINA_LOG_ERR("impossible to create a log domain.");
        eina_shutdown();
        exit(1);
     }

   if (!cserve2_main_loop_setup())
     {
        ERR("could not setup main loop.");
        goto error;
     }

   if (!cserve2_slaves_init())
     {
        ERR("Could not init slaves subsystem.");
        goto error;
     }

   cserve2_cache_init();

   _clients_setup();

   cserve2_timeout_cb_set(3000, _timeout_cb);

   cserve2_main_loop_run();

   _clients_finish();

   cserve2_cache_shutdown();

   _slaves_restart();
   cserve2_slaves_shutdown();

   cserve2_main_loop_finish();

   eina_log_domain_unregister(_evas_cserve2_bin_log_dom);
   eina_shutdown();
   return 0;

error:
   eina_log_domain_unregister(_evas_cserve2_bin_log_dom);
   eina_shutdown();
   exit(1);
}
