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
static int _evas_cserve2_usage_log_dom = -1;

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX sizeof(((struct sockaddr_un *)NULL)->sun_path)
#endif

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_cserve2_usage_log_dom, __VA_ARGS__)
#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_cserve2_usage_log_dom, __VA_ARGS__)
#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_cserve2_usage_log_dom, __VA_ARGS__)
#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_cserve2_usage_log_dom, __VA_ARGS__)

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

   if (fcntl(s, F_SETFL, O_NONBLOCK) < 0) ERR("can't set non-blocking fd");

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
_usage_msg_send(void)
{
   Msg_Base msg;
   int size;

   memset(&msg, 0, sizeof(msg));
   msg.type = CSERVE2_STATS;
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

static void
_shared_index_print(Msg_Base *data, size_t size)
{
   Msg_Index_List *msg = (Msg_Index_List *) data;

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

static void
_usage_msg_read(void)
{
   Msg_Stats *stats = NULL;
   int size;

   printf("Requesting server statistics.\n\n");

   while (!stats)
     {
        Msg_Base *msg = _server_read(&size);
        if (!msg) continue;
        switch (msg->type)
          {
           case CSERVE2_INDEX_LIST:
             _shared_index_print(msg, size);
             break;
           case CSERVE2_STATS:
             stats = (Msg_Stats *) msg;
             break;
           default:
             ERR("Invalid message received from server. "
                 "Something went badly wrong.");
             return;
          }
     }

   printf("Printing server usage.\n");
   printf("======================\n\n");
   printf("\nImage Usage Statistics:\n");
   printf("----------------------\n\n");
   printf("Image headers usage: %d bytes\n", stats->images.files_size);
   printf("Image data requested: %d kbytes\n", stats->images.requested_size / 1024);
   printf("Image data usage: %d kbytes\n", stats->images.images_size / 1024);
   printf("Image data unused: %d kbytes\n", stats->images.unused_size / 1024);
   printf("Image headers load time: %dus\n", stats->images.files_load_time);
   printf("Image headers saved time: %dus\n", stats->images.files_saved_time);
   printf("Image data load time: %dus\n", stats->images.images_load_time);
   printf("Image data saved time: %dus\n", stats->images.images_saved_time);
   printf("\nFont Usage Statistics:\n");
   printf("----------------------\n\n");
   printf("Requested usage: %d bytes\n", stats->fonts.requested_size);
   printf("Real usage: %d bytes\n", stats->fonts.real_size);
   printf("Unused size: %d bytes\n", stats->fonts.unused_size);
   printf("Fonts load time: %dus\n", stats->fonts.fonts_load_time);
   printf("Fonts used load time: %dus\n", stats->fonts.fonts_used_load_time);
   printf("Fonts used saved time: %dus\n", stats->fonts.fonts_used_saved_time);
   printf("Glyphs load time: %dus\n", stats->fonts.glyphs_load_time);
   printf("Glyphs render time: %dus\n", stats->fonts.glyphs_render_time);
   printf("Glyphs saved time: %dus\n", stats->fonts.glyphs_saved_time);
   printf("Glyphs request time: %dus\n", stats->fonts.glyphs_request_time);
   printf("Glyphs slave time: %dus\n", stats->fonts.glyphs_slave_time);

   printf("\n");
}

int
main(void)
{
   eina_init();
   _evas_cserve2_usage_log_dom = eina_log_domain_register
      ("evas_cserve2_usage", EINA_COLOR_BLUE);
   if (!_server_connect())
     {
        ERR("Could not connect to server.");
        return -1;
     }
   _usage_msg_send();
   _usage_msg_read();
   _server_disconnect();
   eina_shutdown();
   return 0;
}
