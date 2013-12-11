#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "evas_cs2.h"

static unsigned int _rid_count = 0;

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX sizeof(((struct sockaddr_un *)NULL)->sun_path)
#endif

static void
debug_msg(const void *buf, int size)
{
    const char *str = buf;
    int i;

    printf("message: ");
    for (i = 0; i < size; i++)
      printf("%x ", str[i]);

    printf("\n");
}

static int
_read_line(char *buf, int size)
{
   int len;
   char *c;

   if (!fgets(buf, size, stdin))
     {
        buf[0] = 0;
        return 0;
     }

   c = strchr(buf, '#');
   if (c)
     *c = '\0';
   else
     {
        c = strchr(buf, '\n');
        if (c)
          *c = '\0';
     }
   len = strlen(buf);

   return len + 1;
}

static void *
parse_input_open(int *size)
{
   char path[4096];
   char key[4096];
   char line[4096];
   int path_len, key_len;
   Msg_Open msg;
   char *buf;
   int file_id;

   // TODO: Add load opts

   _read_line(line, sizeof(line));
   path_len = _read_line(path, sizeof(path));
   key_len = _read_line(key, sizeof(key));

   sscanf(line, "%d", &file_id);

   buf = malloc(sizeof(msg) + path_len + key_len);

   msg.base.rid = _rid_count++;
   msg.base.type = CSERVE2_OPEN;
   msg.file_id = file_id;
   msg.path_offset = 0;
   msg.key_offset = path_len;
   msg.has_load_opts = EINA_FALSE;
   msg.image_id = 0;

   memcpy(buf, &msg, sizeof(msg));
   memcpy(buf + sizeof(msg), path, path_len);
   memcpy(buf + sizeof(msg) + path_len, key, key_len);

   *size = sizeof(msg) + path_len + key_len;

   return buf;

#if 0
   // TODO: Adapt the following code
   Msg_Setopts *msg;
   char line[4096];
   int file_id, image_id;
   double dpi;
   int w, h;
   int scale;
   int rx, ry, rw, rh;
   int scale_src_x, scale_src_y, scale_src_w, scale_src_h;
   int scale_dst_w, scale_dst_h;
   int scale_smooth;
   int scale_hint;
   int degree;
   int orientation;

   // reading file_id, image_id
   _read_line(line, sizeof(line));
   sscanf(line, "%d %d", &file_id, &image_id);

   // reading load dpi
   _read_line(line, sizeof(line));
   dpi = atof(line);

   // reading load size
   _read_line(line, sizeof(line));
   sscanf(line, "%d %d", &w, &h);

   // reading load scale down
   _read_line(line, sizeof(line));
   sscanf(line, "%d", &scale);

   // reading load region
   _read_line(line, sizeof(line));
   sscanf(line, "%d %d %d %d", &rx, &ry, &rw, &rh);

   // reading original image's source coord
   _read_line(line, sizeof(line));
   sscanf(line, "%d %d", &scale_src_x, &scale_src_y);

   // reading original size
   _read_line(line, sizeof(line));
   sscanf(line, "%d %d", &scale_src_w, &scale_src_h);

   // reading scale size
   _read_line(line, sizeof(line));
   sscanf(line, "%d %d", &scale_dst_w, &scale_dst_h);

   // reading scale smooth
   _read_line(line, sizeof(line));
   sscanf(line, "%d", &scale_smooth);

   // reading scale hint
   _read_line(line, sizeof(line));
   sscanf(line, "%d", &scale_hint);

   // reading degree
   _read_line(line, sizeof(line));
   sscanf(line, "%d", &degree);

   // reading orientation
   _read_line(line, sizeof(line));
   sscanf(line, "%d", &orientation);

   msg = calloc(1, sizeof(*msg));

   msg->base.rid = _rid_count++;
   msg->base.type = CSERVE2_SETOPTS;
   msg->file_id = file_id;
   msg->image_id = image_id;
   msg->opts.dpi = dpi;
   msg->opts.w = w;
   msg->opts.h = h;
   msg->opts.scale_down_by = scale;
   msg->opts.region.x = rx;
   msg->opts.region.y = ry;
   msg->opts.region.w = rw;
   msg->opts.region.h = rh;
   msg->opts.scale_load.src_x = scale_src_x;
   msg->opts.scale_load.src_y = scale_src_y;
   msg->opts.scale_load.src_w = scale_src_w;
   msg->opts.scale_load.src_h = scale_src_h;
   msg->opts.scale_load.dst_w = scale_dst_w;
   msg->opts.scale_load.dst_h = scale_dst_h;
   msg->opts.scale_load.smooth = scale_smooth;
   msg->opts.scale_load.scale_hint = scale_hint;
   msg->opts.degree = degree;
   msg->opts.orientation = !!orientation;

   *size = sizeof(*msg);

   return msg;
#endif
}

static void *
parse_input_load(int *size)
{
   Msg_Load *msg;
   char line[4096];
   int image_id;

   // read image_id
   _read_line(line, sizeof(line));
   sscanf(line, "%d", &image_id);

   msg = calloc(1, sizeof(*msg));

   msg->base.rid = _rid_count++;
   msg->base.type = CSERVE2_LOAD;
   msg->image_id = image_id;

   *size = sizeof(*msg);

   return msg;
}

static void *
parse_input_preload(int *size)
{
   Msg_Preload *msg;
   char line[4096];
   int image_id;

   // read image_id
   _read_line(line, sizeof(line));
   sscanf(line, "%d", &image_id);

   msg = calloc(1, sizeof(*msg));

   msg->base.rid = _rid_count++;
   msg->base.type = CSERVE2_PRELOAD;
   msg->image_id = image_id;

   *size = sizeof(*msg);

   return msg;
}

static void *
parse_input_unload(int *size)
{
   Msg_Unload *msg;
   char line[4096];
   int image_id;

   // read image_id
   _read_line(line, sizeof(line));
   sscanf(line, "%d", &image_id);

   msg = calloc(1, sizeof(*msg));

   msg->base.rid = _rid_count++;
   msg->base.type = CSERVE2_UNLOAD;
   msg->image_id = image_id;

   *size = sizeof(*msg);

   return msg;
}

static void *
parse_input_close(int *size)
{
   Msg_Close *msg;
   char line[4096];
   int file_id;

   // read file_id
   _read_line(line, sizeof(line));
   sscanf(line, "%d", &file_id);

   msg = calloc(1, sizeof(*msg));

   msg->base.rid = _rid_count++;
   msg->base.type = CSERVE2_CLOSE;
   msg->file_id = file_id;

   *size = sizeof(*msg);

   return msg;
}

static void
parse_answer_opened(const void *buf)
{
   const Msg_Opened *msg = buf;
   printf("OPENED rid = %d\n", msg->base.rid);
   printf("size: %dx%d, alpha: %d\n\n",
          msg->image.w, msg->image.h, msg->image.alpha);
}

static void
parse_answer_loaded(const void *buf)
{
   const Msg_Loaded *msg = buf;
   const char *path;

   path = ((const char *)msg) + sizeof(*msg);

   printf("LOADED rid = %d\n", msg->base.rid);
   printf("shm mmap_offset = 0x%x, use_offset = 0x%x, mmap size = %d bytes\n",
          msg->shm.mmap_offset, msg->shm.use_offset, msg->shm.mmap_size);
   printf("shm path: \"%s\"\n\n", path);
}

static void
parse_answer_error(const void *buf)
{
   const Msg_Error *msg = buf;

   printf("ERROR rid = %d, error = %d\n", msg->base.rid, msg->error);
}

static void
parse_answer(const void *buf)
{
   const Msg_Base *msg = buf;

   switch (msg->type)
     {
      case CSERVE2_OPENED:
         parse_answer_opened(buf);
         break;
      case CSERVE2_LOADED:
         parse_answer_loaded(buf);
         break;
      case CSERVE2_ERROR:
         parse_answer_error(buf);
         break;
      default:
         printf("unhandled answer: %d\n", msg->type);
     }
}

static struct {
   const char *name;
   Message_Type type;
   void *(*parse_func)(int *size);
} _msg_types[] = {
   { "OPEN", CSERVE2_OPEN, parse_input_open },
   { "OPENED", CSERVE2_OPENED, NULL },
   { "LOAD", CSERVE2_LOAD, parse_input_load },
   { "LOADED", CSERVE2_LOADED, NULL },
   { "PRELOAD", CSERVE2_PRELOAD, parse_input_preload },
   { "UNLOAD", CSERVE2_UNLOAD, parse_input_unload },
   { "CLOSE", CSERVE2_CLOSE, parse_input_close },
   { NULL, 0, NULL }
};

int main(void)
{
   int s, t, len, skip_cmd = 0;
   struct sockaddr_un remote;
   char msgbuf[4096], buf[UNIX_PATH_MAX], *env;

   if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
     {
        perror("socket");
        exit(1);
     }

   printf("Trying to connect...\n");

   remote.sun_family = AF_UNIX;
   env = getenv("EVAS_CSERVE2_SOCKET");
   if (!env)
     {
        snprintf(buf, sizeof(buf), "/tmp/.evas-cserve2-%x.socket",
                 (int)getuid());
        env = buf;
     }
   eina_strlcpy(remote.sun_path, env, UNIX_PATH_MAX);
   len = strlen(remote.sun_path) + sizeof(remote.sun_family);
   if (connect(s, (struct sockaddr *)&remote, len) == -1)
     {
        perror("connect");
        exit(1);
     }

   printf("Connected.\n");

   while(!feof(stdin))
     {
        char cmd[1024];
        int i;
        int size;
        void *msg;

        if (skip_cmd)
          skip_cmd = 0;
        else
          printf("\n> ");
        if (!fgets(cmd, sizeof(cmd), stdin))
          break;
        len = strlen(cmd) - 1;
        cmd[len] = '\0';

        if (!len)
          {
             skip_cmd = 1;
             continue;
          }

        for (i = 0; _msg_types[i].name; i++)
          {
             if (!strcmp(cmd, _msg_types[i].name))
               break;
          }

        // discards the end of the message if we can't parse it
        if (!_msg_types[i].name)
          {
             printf("Invalid command.\n");
             continue;
          }

        if (!_msg_types[i].parse_func)
          {
             printf("Command %s still unhandled.\n", _msg_types[i].name);
             continue;
          }

        msg = _msg_types[i].parse_func(&size);

        if (send(s, &size, sizeof(size), MSG_NOSIGNAL) == -1)
          {
             perror("send size");
             exit(1);
          }
        if (send(s, msg, size, MSG_NOSIGNAL) == -1)
          {
             perror("send");
             exit(1);
          }

        free(msg);

        usleep(100000);

        if ((t=recv(s, &size, sizeof(size), MSG_DONTWAIT)) > 0)
          {
             len = recv(s, msgbuf, size, 0);
             printf("size of received message: %d\n", len);
             if (len != size)
               {
                  printf("couldn't read entire message.\n");
                  continue;
               }
             debug_msg(&size, sizeof(size));
             debug_msg(msgbuf, size);
             parse_answer(msgbuf);
          }
        else
          {
             if (t < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
               continue;
             else fprintf(stderr, "Server closed connection\n");
             exit(1);
          }
     }

   close(s);

   return 0;
}
