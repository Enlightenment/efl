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

Eina_Prefix *_evas_cserve2_pfx = NULL;
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

void
cserve2_index_list_send(int generation_id,
                        const char *strings_index_path,
                        const char *strings_entries_path,
                        const char *files_index_path,
                        const char *images_index_path,
                        const char *fonts_index_path,
                        Client *client)
{
   Eina_Iterator *iter;
   Msg_Index_List msg;
   const int size = sizeof(msg);

   if (!client_list)
     return;

   INF("New shared index: strings: '%s':'%s' files: '%s' images: '%s', fonts: '%s'",
       strings_index_path, strings_entries_path,
       files_index_path, images_index_path, fonts_index_path);

   memset(&msg, 0, size);
   msg.base.type = CSERVE2_INDEX_LIST;
   msg.generation_id = generation_id;
   if (strings_index_path)
     eina_strlcpy(msg.strings_index_path, strings_index_path, 64);
   if (strings_entries_path)
     eina_strlcpy(msg.strings_entries_path, strings_entries_path, 64);
   if (files_index_path)
     eina_strlcpy(msg.files_index_path, files_index_path, 64);
   if (images_index_path)
     eina_strlcpy(msg.images_index_path, images_index_path, 64);
   if (fonts_index_path)
     eina_strlcpy(msg.fonts_index_path, fonts_index_path, 64);

   if (!client)
     {
        iter = eina_hash_iterator_data_new(client_list);
        EINA_ITERATOR_FOREACH(iter, client)
          {
             DBG("Sending updated list of indexes to client %d", client->id);
             cserve2_client_send(client, &size, sizeof(size));
             cserve2_client_send(client, &msg, sizeof(msg));
          }
        eina_iterator_free(iter);
     }
   else
     {
        DBG("Sending updated list of indexes to client %d", client->id);
        cserve2_client_send(client, &size, sizeof(size));
        cserve2_client_send(client, &msg, sizeof(msg));
     }
}

static void
_cserve2_client_close(Client *client)
{
   Msg_Close *msg = (Msg_Close *)client->msg.buf;

   INF("Received CLOSE command: RID=%d", msg->base.rid);
   INF("File_ID: %d", msg->file_id);

   cserve2_cache_file_close(client, msg->file_id);
}

static void
_cserve2_client_unload(Client *client)
{
   Msg_Unload *msg = (Msg_Unload *)client->msg.buf;

   INF("Received UNLOAD command: RID=%d", msg->base.rid);
   INF("Image_ID: %d", msg->image_id);

   cserve2_cache_image_unload(client, msg->image_id);
}

static void
_cserve2_client_preload(Client *client)
{
   Msg_Preload *msg = (Msg_Preload *)client->msg.buf;

   INF("Received PRELOAD command: RID=%d", msg->base.rid);
   INF("Image_ID: %d", msg->image_id);

   cserve2_cache_image_preload(client, msg->image_id, msg->base.rid);
}

static void
_cserve2_client_load(Client *client)
{
   Msg_Load *msg = (Msg_Load *)client->msg.buf;

   INF("Received LOAD command: RID=%d, Image_ID: %d",
       msg->base.rid, msg->image_id);
   cserve2_cache_image_load(client, msg->image_id, msg->base.rid);
}

static void
_cserve2_client_open(Client *client)
{
   Msg_Open *msg = (Msg_Open *)client->msg.buf;
   const char *path, *key, *end;
   Evas_Image_Load_Opts *opts = NULL;
   Evas_Image_Load_Opts opts_copy;

   path = ((const char *)msg) + sizeof(*msg) + msg->path_offset;
   key = ((const char *)msg) + sizeof(*msg) + msg->key_offset;
   end = key + strlen(key) + 1;

   INF("Received OPEN command: RID=%d", msg->base.rid);
   INF("File_ID: %d, Image_ID: %d, path=\"%s\", key=\"%s\", has_load_opts=%d",
       msg->file_id, msg->image_id, path, key, (int) msg->has_load_opts);

   if (!key[0]) key = NULL;
   if (msg->has_load_opts)
     {
        opts = &opts_copy;
        memcpy(&opts_copy, end, sizeof(opts_copy));
     }

   cserve2_cache_file_open(client, msg->file_id, path, key, msg->base.rid,
                           opts);

   if (!msg->has_load_opts)
     cserve2_cache_image_entry_create(client, msg->base.rid,
                                      msg->file_id, msg->image_id, NULL);
   else
     {
        // FIXME: Check message size first?

        DBG("Load Options:");
        DBG("\tdpi: %03.1f", opts->dpi);
        DBG("\tsize: %dx%d", opts->w, opts->h);
        DBG("\tscale down: %d", opts->scale_down_by);
        DBG("\tregion: %d,%d + %dx%d",
            opts->region.x, opts->region.y, opts->region.w, opts->region.h);
        DBG("\toriginal image's source coord: %d,%d",
            opts->scale_load.src_x, opts->scale_load.src_y);
        DBG("\toriginal image size: %dx%d",
            opts->scale_load.src_w, opts->scale_load.src_h);
        DBG("\tscale size: %dx%d",
            opts->scale_load.dst_w, opts->scale_load.dst_h);
        DBG("\tscale smooth: %d", opts->scale_load.smooth);
        DBG("\tscale hint: %d", opts->scale_load.scale_hint);
        DBG("\tdegree: %d", opts->degree);
        DBG("\torientation: %d", opts->orientation);

        cserve2_cache_image_entry_create(client, msg->base.rid,
                                         msg->file_id, msg->image_id,
                                         opts);
     }
}

static void
_cserve2_client_font_load(Client *client)
{
   Msg_Font_Load *msg = (Msg_Font_Load *)client->msg.buf;
   char name[PATH_MAX], source[PATH_MAX], *buf;

   buf = ((char *)msg) + sizeof(*msg);
   memcpy(source, buf, msg->sourcelen);
   source[msg->sourcelen] = 0;
   buf += msg->sourcelen;
   memcpy(name, buf, msg->pathlen);
   name[msg->pathlen] = 0;

   INF("Received %s command: RID=%d",
       (msg->base.type == CSERVE2_FONT_LOAD) ? "FONT_LOAD" : "FONT_UNLOAD",
       msg->base.rid);
   INF("Font: '%s' '%s', rend_flags: %d, size: %d, dpi: %d",
       source, name, msg->rend_flags, msg->size, msg->dpi);

   if (msg->base.type == CSERVE2_FONT_LOAD)
     cserve2_cache_font_load(client, source, name, msg->rend_flags, msg->size,
                             msg->dpi, msg->base.rid);
   else
     cserve2_cache_font_unload(client, source, name, msg->rend_flags, msg->size,
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
   source[msg->sourcelen] = 0;
   buf += msg->sourcelen;
   memcpy(fontpath, buf, msg->pathlen);
   fontpath[msg->pathlen] = 0;
   buf += msg->pathlen;

   glyphs = malloc(sizeof(*glyphs) * msg->nglyphs);
   memcpy(glyphs, buf, sizeof(*glyphs) * msg->nglyphs);

   if (msg->base.type == CSERVE2_FONT_GLYPHS_LOAD)
     {
        INF("Received CSERVE2_FONT_GLYPHS_LOAD command: RID=%d (%d glyphs)",
            msg->base.rid, msg->nglyphs);
        cserve2_cache_font_glyphs_load(client, source, fontpath,
                                       msg->hint, msg->rend_flags, msg->size,
                                       msg->dpi, glyphs, msg->nglyphs,
                                       msg->base.rid);
     }
   else
     {
        INF("Received CSERVE2_FONT_GLYPHS_USED command: RID=%d (%d glyphs)",
            msg->base.rid, msg->nglyphs);
        cserve2_cache_font_glyphs_used(client, source, fontpath,
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

   eina_hash_add(client_list, &client->id, client);

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
   client_list = NULL;
}

int
main(int argc EINA_UNUSED, const char *argv[])
{
   eina_init();

   _evas_cserve2_pfx = eina_prefix_new(argv[0], main,
                                       "EVAS", "evas", "checkme",
                                       PACKAGE_BIN_DIR,
                                       PACKAGE_LIB_DIR,
                                       PACKAGE_DATA_DIR,
                                       PACKAGE_DATA_DIR);

   _evas_cserve2_bin_log_dom = eina_log_domain_register
      ("evas_cserve2_bin", CSERVE2_BIN_DEFAULT_COLOR);
   if (_evas_cserve2_bin_log_dom < 0)
     {
        EINA_LOG_ERR("impossible to create a log domain.");
        eina_prefix_free(_evas_cserve2_pfx);
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

   cserve2_shm_init();
   cserve2_shared_index_init();
   cserve2_requests_init();
   cserve2_scale_init();
   cserve2_font_init();
   cserve2_cache_init();
   _clients_setup();

   cserve2_main_loop_run();

   _clients_finish();
   cserve2_cache_shutdown();
   cserve2_font_shutdown();
   cserve2_scale_shutdown();
   cserve2_requests_shutdown();
   cserve2_slaves_shutdown();

   cserve2_main_loop_finish();

   cserve2_shared_index_shutdown();
   cserve2_shm_shutdown();

   eina_prefix_free(_evas_cserve2_pfx);

   eina_log_domain_unregister(_evas_cserve2_bin_log_dom);
   eina_shutdown();
   return 0;

error:
   eina_prefix_free(_evas_cserve2_pfx);
   eina_log_domain_unregister(_evas_cserve2_bin_log_dom);
   eina_shutdown();
   exit(1);
}
