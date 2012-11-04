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

static struct sockaddr_un socksize;
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX sizeof(socksize.sun_path)
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
        strncpy(path, env, UNIX_PATH_MAX - 1);
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
        ERR("connect");
        return EINA_FALSE;
     }

   fcntl(s, F_SETFL, O_NONBLOCK);

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
typedef struct _Cache_Entry Cache_Entry;
typedef struct _Glyph_Entry Glyph_Entry;

struct _Font_Entry
{
   const char *file;
   const char *name;
   unsigned int rend_flags;
   unsigned int size;
   unsigned int dpi;
   unsigned int unused;
   Eina_List *caches;
};

struct _Cache_Entry
{
   const char *shmname;
   unsigned int size;
   unsigned int usage;
   Eina_List *glyphs;
};

struct _Glyph_Entry
{
   unsigned int index;
   unsigned int offset;
   unsigned int size;
   unsigned int rows;
   unsigned int width;
   unsigned int pitch;
   unsigned int num_grays;
   unsigned int pixel_mode;
};

#define READIT(_dst, _src) \
   do { \
      memcpy(&_dst, _src, sizeof(_dst)); \
      _src += sizeof(_dst); \
   } while(0)

static Glyph_Entry *
_parse_glyph_entry(char **msg)
{
   Glyph_Entry *ge;
   char *buf = *msg;

   ge = calloc(1, sizeof(*ge));

   READIT(ge->index, buf);
   READIT(ge->offset, buf);
   READIT(ge->size, buf);
   READIT(ge->rows, buf);
   READIT(ge->width, buf);
   READIT(ge->pitch, buf);
   READIT(ge->num_grays, buf);
   READIT(ge->pixel_mode, buf);

   *msg = buf;

   return ge;
}

static Cache_Entry *
_parse_cache_entry(char **msg)
{
   Cache_Entry *ce;
   char *buf = *msg;
   unsigned int n;

   ce = calloc(1, sizeof(*ce));

   READIT(n, buf);
   ce->shmname = eina_stringshare_add_length(buf, n);
   buf += n;

   READIT(ce->size, buf);
   READIT(ce->usage, buf);

   READIT(n, buf);
   while (n--)
     {
        Glyph_Entry *ge;
        ge = _parse_glyph_entry(&buf);
        ce->glyphs = eina_list_append(ce->glyphs, ge);
     }

   *msg = buf;

   return ce;
}

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

   READIT(n, buf);
   while (n--)
     {
        Cache_Entry *ce;
        ce = _parse_cache_entry(&buf);
        fe->caches = eina_list_append(fe->caches, ce);
     }

   *msg = buf;

   return fe;
}

static Eina_List *
_debug_msg_read(void)
{
   Msg_Base *msg = NULL;
   char *buf;
   int size;
   unsigned int nfonts;
   Eina_List *fonts = NULL;

   printf("Requesting server debug info.\n\n");
   while (!msg)
     msg = _server_read(&size);

   if (msg->type != CSERVE2_FONT_DEBUG)
     {
        ERR("Invalid message received from server."
            "Something went badly wrong.");
        return NULL;
     }

   buf = (char *)msg + sizeof(*msg);

   READIT(nfonts, buf);
   while (nfonts--)
     {
        Font_Entry *fe;
        fe = _parse_font_entry(&buf);
        fonts = eina_list_append(fonts, fe);
     }

   return fonts;
}

static void
_glyph_entry_free(Glyph_Entry *ge)
{
   free(ge);
}

static void
_cache_entry_free(Cache_Entry *ce)
{
   Glyph_Entry *ge;

   EINA_LIST_FREE(ce->glyphs, ge)
     _glyph_entry_free(ge);

   eina_stringshare_del(ce->shmname);
   free(ce);
}

static void
_font_entry_free(Font_Entry *fe)
{
   Cache_Entry *ce;

   EINA_LIST_FREE(fe->caches, ce)
     _cache_entry_free(ce);

   eina_stringshare_del(fe->name);
   eina_stringshare_del(fe->file);
   free(fe);
}

static void
_glyph_entry_print(Glyph_Entry *ge)
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
   printf("\t\tGLYPH %u offset: %u size: %u %ux%u pitch: %u grays: %u "
          "pixel mode: %s\n",
          ge->index, ge->offset, ge->size, ge->width, ge->rows, ge->pitch,
          ge->num_grays, pxmode[ge->pixel_mode]);
}

static void
_cache_entry_print(Cache_Entry *ce)
{
   Eina_List *l;
   Glyph_Entry *ge;

   printf("\tSHM %s used %u/%u\n", ce->shmname, ce->usage, ce->size);

   EINA_LIST_FOREACH(ce->glyphs, l, ge)
     _glyph_entry_print(ge);
}

static void
_font_entry_print(Font_Entry *fe)
{
   Eina_List *l;
   Cache_Entry *ce;

   printf("FONT %s:%s size: %u dpi: %u %s%s%s %s\n",
          fe->file, fe->name, fe->size, fe->dpi,
          fe->rend_flags == 0 ? "REGULAR " : "",
          fe->rend_flags & 1 ? "SLANT " : "",
          fe->rend_flags & 2 ? "WEIGHT" : "",
          fe->unused ? "(unused)" : "");

   EINA_LIST_FOREACH(fe->caches, l, ce)
     _cache_entry_print(ce);

   putchar('\n');
}

int
main(void)
{
   Eina_List *fonts;
   Font_Entry *fe;

   eina_init();

   _evas_cserve2_debug_log_dom = eina_log_domain_register
      ("evas_cserve2_debug", EINA_COLOR_BLUE);

   if (!_server_connect())
     {
        ERR("Could not connect to server.");
        return -1;
     }

   _debug_msg_send();

   fonts = _debug_msg_read();

   EINA_LIST_FREE(fonts, fe)
     {
        _font_entry_print(fe);
        _font_entry_free(fe);
     }

   _server_disconnect();

   eina_shutdown();
}
