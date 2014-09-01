#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include <Eina.h>

#include "evas_cs2.h"

static int socketfd = -1;
static unsigned int _rid_count = 1;
static int _evas_cserve2_debug_log_dom = -1;

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX sizeof(((struct sockaddr_un *)NULL)->sun_path)
#endif

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_cserve2_debug_log_dom, __VA_ARGS__)
#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_cserve2_debug_log_dom, __VA_ARGS__)
#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_cserve2_debug_log_dom, __VA_ARGS__)
#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_cserve2_debug_log_dom, __VA_ARGS__)

static void
_socket_path_set(char *path)
{
   char *env;
   char buf[UNIX_PATH_MAX];

   env = getenv("EVAS_CSERVE2_SOCKET");
   if (env && env[0])
     {
        eina_strlcpy(path, env, UNIX_PATH_MAX);
        return;
     }

   snprintf(buf, sizeof(buf), "/tmp/.evas-cserve2-%x.socket", (int)getuid());
   /* FIXME: check we can actually create this socket */
   strcpy(path, buf);
}

static Eina_Bool
_server_connect(void)
{
   int s, len;
   struct sockaddr_un remote;

   if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
     {
        ERR("socket");
        return EINA_FALSE;
     }

   remote.sun_family = AF_UNIX;
   _socket_path_set(remote.sun_path);
   len = strlen(remote.sun_path) + sizeof(remote.sun_family);
   if (connect(s, (struct sockaddr *)&remote, len) == -1)
     {
        close(s);
        ERR("connect");
        return EINA_FALSE;
     }

   if (fcntl(s, F_SETFL, O_NONBLOCK) < 0) ERR("fcntl failed");

   socketfd = s;

   DBG("connected to cserve2 server.");
   return EINA_TRUE;
}

static void
_server_disconnect(void)
{
   close(socketfd);
   socketfd = -1;
}

static Eina_Bool
_server_send(const void *data, int size)
{
   int sent = 0;
   ssize_t ret;
   const char *msg = data;

   while (sent < size)
     {
        ret = send(socketfd, msg + sent, size - sent, MSG_NOSIGNAL);
        if (ret < 0)
          {
             if ((errno == EAGAIN) || (errno == EINTR))
               continue;
             return EINA_FALSE;
          }
        sent += ret;
     }

   return EINA_TRUE;
}

static int sr_size = 0;
static int sr_got = 0;
static char *sr_buf = NULL;

static void *
_server_read(int *size)
{
   int n;
   void *ret;

   if (sr_size)
     goto get_data;

   n = recv(socketfd, &sr_size, sizeof(sr_size), 0);
   if (n < 0)
     return NULL;

   sr_buf = malloc(sr_size);

get_data:
   n = recv(socketfd, sr_buf + sr_got, sr_size - sr_got, 0);
   if (n < 0)
     return NULL;

   sr_got += n;
   if (sr_got < sr_size)
     return NULL;

   *size = sr_size;
   sr_size = 0;
   sr_got = 0;
   ret = sr_buf;
   sr_buf = NULL;

   return ret;
}

static void
_debug_msg_send(void)
{
   Msg_Base msg;
   int size;

   memset(&msg, 0, sizeof(msg));
   msg.type = CSERVE2_FONT_DEBUG;
   msg.rid = _rid_count++;

   size = sizeof(msg);

   if (!_server_send(&size, sizeof(size)))
     {
        ERR("Could not send usage msg size to server.");
        return;
     }
   if (!_server_send(&msg, size))
     {
        ERR("Could not send usage msg body to server.");
        return;
     }
}

typedef struct _Font_Entry Font_Entry;

struct _Font_Entry
{
   const char *file;
   const char *name;
   unsigned int rend_flags;
   unsigned int size;
   unsigned int dpi;
   unsigned int unused;
   char glyph_data_shm[64];
   char glyph_mempool_shm[64];
   Eina_List *glyphs; // Glyph_Data
};

#define READIT(_dst, _src) \
   do { \
      memcpy(&_dst, _src, sizeof(_dst)); \
      _src += sizeof(_dst); \
   } while(0)

static Font_Entry *
_parse_font_entry(char **msg)
{
   Font_Entry *fe;
   char *buf = *msg;
   unsigned int n;

   fe = calloc(1, sizeof(*fe));

   READIT(n, buf);
   if (n)
     fe->file = eina_stringshare_add_length(buf, n);
   buf += n;
   READIT(n, buf);
   if (n)
     fe->name = eina_stringshare_add_length(buf, n);
   buf += n;

   READIT(fe->rend_flags, buf);
   READIT(fe->size, buf);
   READIT(fe->dpi, buf);
   READIT(fe->unused, buf);

   eina_strlcpy(fe->glyph_data_shm, buf, 64);
   buf += 64;
   eina_strlcpy(fe->glyph_mempool_shm, buf, 64);
   buf += 64;

   READIT(n, buf);
   while (n--)
     {
        Glyph_Data *gd = calloc(1, sizeof(Glyph_Data));
        memcpy(gd, buf, sizeof(Glyph_Data));
        buf += sizeof(Glyph_Data);
        fe->glyphs = eina_list_append(fe->glyphs, gd);
     }

   *msg = buf;

   return fe;
}

static Eina_List *
_debug_msg_read(void)
{
   Msg_Font_Debug *msg = NULL;
   char *buf;
   int size;
   unsigned int nfonts;
   Eina_List *fonts = NULL;

   printf("Requesting server debug info.\n\n");
   while (!msg)
     msg = _server_read(&size);

   if (msg->base.type != CSERVE2_FONT_DEBUG)
     {
        ERR("Invalid message received from server. "
            "Something went badly wrong.");
        return NULL;
     }

   buf = (char *)msg + sizeof(*msg);

   nfonts = msg->nfonts;
   while (nfonts--)
     {
        Font_Entry *fe;
        fe = _parse_font_entry(&buf);
        fonts = eina_list_append(fonts, fe);
     }

   printf("Font index table: %s\n", msg->fonts_index_path);
   printf("Contains %u fonts\n\n", msg->nfonts);
   return fonts;
}

static void
_font_entry_free(Font_Entry *fe)
{
   Glyph_Data *gd;

   EINA_LIST_FREE(fe->glyphs, gd)
     free(gd);

   eina_stringshare_del(fe->name);
   eina_stringshare_del(fe->file);
   free(fe);
}

static void
_glyph_data_print(Glyph_Data *gd)
{
   const char *pxmode[] = {
        "FT_PIXEL_MODE_NONE",
        "FT_PIXEL_MODE_MONO",
        "FT_PIXEL_MODE_GRAY",
        "FT_PIXEL_MODE_GRAY2",
        "FT_PIXEL_MODE_GRAY4",
        "FT_PIXEL_MODE_LCD",
        "FT_PIXEL_MODE_LCD_V"
   };
   printf("  GLYPH id: %-4u refcount %-2u: index: %-6u offset: %-6u size: %-3u "
          "%2ux%-3u pitch: %-2u grays: %-3u pixel mode: %s hint: %d\n",
          gd->id, gd->refcount, gd->index, gd->offset, gd->size,
          gd->width, gd->rows, gd->pitch,
          gd->num_grays, pxmode[gd->pixel_mode], gd->hint);
}

static void
_font_entry_print(Font_Entry *fe)
{
   Eina_List *l;
   Glyph_Data *gd;

   printf("FONT %s:%s size: %u dpi: %u %s%s%s %s\n",
          fe->file, fe->name, fe->size, fe->dpi,
          fe->rend_flags == 0 ? "REGULAR " : "",
          fe->rend_flags & 1 ? "SLANT " : "",
          fe->rend_flags & 2 ? "WEIGHT" : "",
          fe->unused ? "(unused)" : "");
   printf("  Index:       %s\n"
          "  Mempool:     %s\n"
          "  Glyph count: %u\n",
          fe->glyph_data_shm, fe->glyph_mempool_shm,
          eina_list_count(fe->glyphs));

   EINA_LIST_FOREACH(fe->glyphs, l, gd)
     _glyph_data_print(gd);

   putchar('\n');
}

static void
_shared_index_print(Msg_Index_List *msg, size_t size)
{
   if (size < sizeof(*msg) || msg->base.type != CSERVE2_INDEX_LIST)
     {
        ERR("Invalid message received from server. "
            "Something went wrong.");
        return;
     }

   printf("Printing shared indexes status.\n");
   printf("===============================\n\n");
   printf("Generation ID:        %-4d\n", msg->generation_id);
   printf("Strings entries path: %s\n", msg->strings_entries_path);
   printf("Strings index path:   %s\n", msg->strings_index_path);
   printf("Files index path:     %s\n", msg->files_index_path);
   printf("Images index path:    %s\n", msg->images_index_path);
   printf("Fonts index path:     %s\n", msg->fonts_index_path);
   printf("\n\n\n");
}

int
main(void)
{
   Eina_List *fonts;
   Font_Entry *fe;
   Msg_Index_List *msg = NULL;
   int size;

   eina_init();
   _evas_cserve2_debug_log_dom = eina_log_domain_register
      ("evas_cserve2_debug", EINA_COLOR_BLUE);
   if (!_server_connect())
     {
        ERR("Could not connect to server.");
        return -1;
     }

   while (!msg)
     msg = _server_read(&size);
   _shared_index_print(msg, size);

   _debug_msg_send();
   fonts = _debug_msg_read();
   EINA_LIST_FREE(fonts, fe)
     {
        _font_entry_print(fe);
        _font_entry_free(fe);
     }
   _server_disconnect();
   eina_shutdown();
   return 0;
}
