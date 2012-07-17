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

int _evas_cserve2_bin_log_dom = -1;
static unsigned int _client_id = 0;
static Eina_Hash *client_list = NULL;

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
}

static void
_cserve2_client_load(Client *client)
{
   Msg_Load *msg = (Msg_Load *)client->msg.buf;

   INF("Received LOAD command: RID=%d", msg->base.rid);
   INF("Image_ID: %d\n", msg->image_id);

   cserve2_cache_image_load(client, msg->image_id, msg->base.rid);
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
}

static void
_cserve2_client_font_load(Client *client)
{
   Msg_Font_Load *msg = (Msg_Font_Load *)client->msg.buf;
   char name[PATH_MAX], source[PATH_MAX], *buf;

   buf = ((char *)msg) + sizeof(*msg);
   memcpy(source, buf, msg->sourcelen);
   buf += msg->sourcelen;
   memcpy(name, buf, msg->pathlen);

   INF("Received %s command: RID=%d",
       (msg->base.type == CSERVE2_FONT_LOAD) ? "FONT_LOAD" : "FONT_UNLOAD",
       msg->base.rid);
   INF("Font: %s, rend_flags: %d, size: %d, dpi: %d",
       name, msg->rend_flags, msg->size, msg->dpi);

   if (msg->base.type == CSERVE2_FONT_LOAD)
     cserve2_cache_font_load(client, source, msg->sourcelen, name,
                             msg->pathlen, msg->rend_flags, msg->size,
                             msg->dpi, msg->base.rid);
   else
     cserve2_cache_font_unload(client, source, msg->sourcelen, name,
                               msg->pathlen, msg->rend_flags, msg->size,
                               msg->dpi, msg->base.rid);
}

static void
_cserve2_client_font_glyphs_request(Client *client)
{
   Msg_Font_Glyphs_Request *msg = (Msg_Font_Glyphs_Request *)client->msg.buf;
   char source[PATH_MAX], fontpath[PATH_MAX], *buf;
   unsigned int *glyphs;

   buf = ((char *)msg) + sizeof(*msg);
   memcpy(source, buf, msg->sourcelen);
   buf += msg->sourcelen;
   memcpy(fontpath, buf, msg->pathlen);
   buf += msg->pathlen;

   glyphs = malloc(sizeof(*glyphs) * msg->nglyphs);
   memcpy(glyphs, buf, sizeof(*glyphs) * msg->nglyphs);

   if (msg->base.type == CSERVE2_FONT_GLYPHS_LOAD)
     {
        INF("Received CSERVE2_FONT_GLYPHS_LOAD command: RID=%d",
            msg->base.rid);
        cserve2_cache_font_glyphs_load(client, source, msg->sourcelen,
                                       fontpath, msg->pathlen,
                                       msg->hint, msg->rend_flags, msg->size,
                                       msg->dpi, glyphs, msg->nglyphs,
                                       msg->base.rid);
     }
   else
     {
        INF("Received CSERVE2_FONT_GLYPHS_USED command: RID=%d",
            msg->base.rid);
        cserve2_cache_font_glyphs_used(client, source, msg->sourcelen,
                                       fontpath, msg->pathlen,
                                       msg->hint, msg->rend_flags, msg->size,
                                       msg->dpi, glyphs, msg->nglyphs,
                                       msg->base.rid);
     }
}

static void
_cserve2_client_stats_request(Client *client)
{
   Msg_Base *msg = (Msg_Base *)client->msg.buf;
   cserve2_cache_stats_get(client, msg->rid);
}

static void
_cserve2_client_font_debug_request(Client *client)
{
   Msg_Base *msg = (Msg_Base *)client->msg.buf;
   cserve2_cache_font_debug(client, msg->rid);
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
      case CSERVE2_STATS:
         _cserve2_client_stats_request(client);
         break;
      case CSERVE2_FONT_DEBUG:
         _cserve2_client_font_debug_request(client);
         break;
      default:
         WRN("Unhandled message");
     }
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

   cserve2_requests_init();

   cserve2_font_init();

   cserve2_cache_init();

   _clients_setup();

   cserve2_main_loop_run();

   _clients_finish();

   cserve2_cache_shutdown();

   cserve2_font_shutdown();

   cserve2_requests_shutdown();

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
