#include "evas_cs.h"
#include <signal.h>

#ifdef EVAS_CSERVE

static Server *cserve = NULL;
static int csrve_init = 0;

static void
pipe_handler(int x, siginfo_t *info, void *data)
{
}

static void
pipe_handle(int push)
{
   static struct sigaction old_action;
   struct sigaction action;

   if (push)
     {
        action.sa_handler = NULL;
        action.sa_sigaction = pipe_handler;
        action.sa_flags = SA_RESTART | SA_SIGINFO;
        sigemptyset(&action.sa_mask);
        sigaction(SIGPIPE, &action, &old_action);
     }
   else
     {
        sigaction(SIGPIPE, &old_action, &action);
     }
}

static Server *
server_connect(void)
{
   Server *s;
   char buf[PATH_MAX];
   int curstate = 0;
   struct sockaddr_un socket_unix;
   int socket_unix_len;
   
   s = calloc(1, sizeof(Server));
   if (!s) return NULL;
   s->fd = -1;
   snprintf(buf, sizeof(buf), "/tmp/.evas-cserve-%x", getuid());
   s->socket_path = strdup(buf);
   if (!s->socket_path)
     {
        free(s);
        return NULL;
     }
   s->fd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (s->fd < 0) goto error;
   if (fcntl(s->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
   if (setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &curstate, sizeof(curstate)) < 0)
     goto error;
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   if (connect(s->fd, (struct sockaddr *)&socket_unix, socket_unix_len) < 0) goto error;
   return s;
   error:
   if (s->fd >= 0) close(s->fd);
   free(s->socket_path);
   free(s);
   return NULL;
}

static void
server_disconnect(Server *s)
{
   close(s->fd);
   free(s->socket_path);
   free(s);
}

static int
server_send(Server *s, int opcode, int size, unsigned char *data)
{
   int ints[2];
   int num;
   
   pipe_handle(1);
   ints[0] = size;
   ints[1] = opcode;
   num = write(s->fd, ints, (sizeof(int) * 2));
   if (num < 0)
     {
        pipe_handle(0);
        if (cserve) server_disconnect(cserve);
        cserve = NULL;
        return 0;
     }
   num = write(s->fd, data, size);
   if (num < 0)
     {
        pipe_handle(0);
        if (cserve) server_disconnect(cserve);
        cserve = NULL;
        return 0;
     }
   pipe_handle(0);
   return 1;
}

static unsigned char *
server_read(Server *s, int *opcode, int *size)
{
   int ints[2], num, left;
   unsigned char *data;
   
   num = read(s->fd, ints, sizeof(int) * 2);
   if (num != (sizeof(int) * 2))
     {
        if (cserve) server_disconnect(cserve);
        cserve = NULL;
        return NULL;
     }
   *size = ints[0];
   *opcode = ints[1];
   if ((*size < 0) || (*size > (1024 * 1024))) return NULL;
   data = malloc(*size);
   if (!data) return NULL;
   num = read(s->fd, data, *size);
   if (num < 0)
     {
        free(data);
        return NULL;
     }
   left = *size - num;
   while (left > 0)
     {
        num = read(s->fd, data + (*size - left), left);
        if (num < 0)
          {
             free(data);
             return NULL;
          }
        left -= num;
     }
   return data;
}

static int
server_init(Server *s)
{
   Op_Init msg, *rep;
   int opcode;
   int size;
   
   msg.pid = getpid();
   msg.server_id = 0;
   if (!server_send(s, OP_INIT, sizeof(msg), (unsigned char *)(&msg)))
     return 0;
   rep = (Op_Init *)server_read(s, &opcode, &size);
   if ((rep) && (opcode == OP_INIT) && (size == sizeof(Op_Init)))
     {
        s->pid = rep->pid;
        s->server_id = rep->server_id;
        free(rep);
        return 1;
     }
   if (rep) free(rep);
   return 0;
}

EAPI Eina_Bool
evas_cserve_init(void)
{
   csrve_init++;
   if (cserve) return 1;
   cserve = server_connect();
   if (!cserve) return 0;
   if (!server_init(cserve))
     {
        if (cserve) server_disconnect(cserve);
        cserve = NULL;
        return 0;
     }
   return 1;
}

EAPI int
evas_cserve_use_get(void)
{
   return csrve_init;
}

EAPI void
evas_cserve_shutdown(void)
{
   csrve_init--;
   if (csrve_init > 0) return;
   if (!cserve) return;
   server_disconnect(cserve);
   cserve = NULL;
}

static void
server_reinit(void)
{
   if (cserve) return;
   cserve = server_connect();
   if (cserve)
     {
        if (!server_init(cserve))
          {
             if (cserve) server_disconnect(cserve);
             cserve = NULL;
          }
     }
}

EAPI Eina_Bool
evas_cserve_image_load(Image_Entry *ie, const char *file, const char *key, RGBA_Image_Loadopts *lopt)
{
   Op_Load msg;
   Op_Load_Reply *rep;
   unsigned char *buf;
   char fbuf[PATH_MAX], wdb[PATH_MAX];
   int flen, klen;
   int opcode;
   int size;
   
   if (csrve_init > 0) server_reinit();
   else return 0;
   if (!cserve) return 0;
   if (!key) key = "";
   memset(&msg, 0, sizeof(msg));
   msg.lopt.scale_down_by = lopt->scale_down_by;
   msg.lopt.dpi = lopt->dpi;
   msg.lopt.w = lopt->w;
   msg.lopt.h = lopt->h;
   if (file[0] != '/')
     {
        if (getcwd(wdb, sizeof(wdb)))
          {
             snprintf(fbuf, sizeof(buf), "%s/%s", wdb, file);
             file = fbuf;
          }
     }
   if (!realpath(file, wdb)) file = wdb;
   flen = strlen(file) + 1;
   klen = strlen(key) + 1;
   buf = malloc(sizeof(msg) + flen + klen);
   memcpy(buf, &msg, sizeof(msg));
   strcpy(buf + sizeof(msg), file);
   strcpy(buf + sizeof(msg) + flen, key);
   if (!buf) return 0;
   if (!server_send(cserve, OP_LOAD, 
                    sizeof(msg) + flen + klen,
                    buf))
     {
        free(buf);
        return 0;
     }
   free(buf);
   if (!cserve) return 0;
   rep = (Op_Load_Reply *)server_read(cserve, &opcode, &size);
   if ((rep) && (opcode == OP_LOAD) && (size == sizeof(Op_Load_Reply)))
     {
        ie->w = rep->image.w;
        ie->h = rep->image.h;
        ie->flags.alpha = rep->image.alpha;
        ie->data1 = rep->handle;
     }
   if (rep) free(rep);
   if (ie->data1 == NULL) return 0;
   if (cserve)
     ie->server_id = cserve->server_id;
   return 1;
}

EAPI Eina_Bool
evas_cserve_image_data_load(Image_Entry *ie)
{
   Op_Loaddata msg;
   Op_Loaddata_Reply *rep;
   int opcode;
   int size;
   if (csrve_init > 0) server_reinit();
   else return 0;
   if (!cserve) return 0;
   if (ie->data1 == NULL) return 0;
   if (cserve->server_id != ie->server_id)
     {
        ie->data1 = NULL;
        if (!evas_cserve_image_load(ie, ie->file, ie->key, &(ie->load_opts)))
          return 0;
     }
   memset(&msg, 0, sizeof(msg));
   msg.handle = ie->data1;
   msg.server_id = cserve->server_id;
   if (!server_send(cserve, OP_LOADDATA, sizeof(msg), (unsigned char *)(&msg)))
     return 0;
   if (!cserve) return 0;
   rep = (Op_Loaddata_Reply *)server_read(cserve, &opcode, &size);
   if ((rep) && (opcode == OP_LOADDATA) && (size == sizeof(Op_Loaddata_Reply)))
     {
        if (rep->mem.size <= 0)
          {
             free(rep);
             return 0;
          }
        ie->data2 = evas_cserve_mem_open(cserve->pid, rep->mem.id, NULL, rep->mem.size, 0);
        free(rep);
        return 1;
     }
   if (rep) free(rep);
   return 0;
}

EAPI void
evas_cserve_image_free(Image_Entry *ie)
{
   Op_Unload msg;
   
   if (csrve_init > 0) server_reinit();
   else return;
   if (!cserve) return;
   if (ie->data1 == NULL) return;
   memset(&msg, 0, sizeof(msg));
   msg.handle = ie->data1;
   msg.server_id = cserve->server_id;
   if (ie->data2) evas_cserve_image_unload(ie);
   if (cserve)
     {
        if (ie->server_id == cserve->server_id)
          server_send(cserve, OP_UNLOAD, sizeof(msg), (unsigned char *)(&msg));
     }
   ie->data1 = NULL;
}

EAPI void
evas_cserve_image_unload(Image_Entry *ie)
{
   Op_Unloaddata msg;
   
   if (csrve_init > 0) server_reinit();
   else return;
   if (!cserve) return;
   if (ie->data1 == NULL) return;
   memset(&msg, 0, sizeof(msg));
   msg.handle = ie->data1;
   msg.server_id = cserve->server_id;
   if (ie->data2) evas_cserve_mem_close(ie->data2);
   ie->data2 = NULL;
   if (ie->server_id == cserve->server_id)
     server_send(cserve, OP_UNLOADDATA, sizeof(msg), (unsigned char *)(&msg));
}

EAPI void
evas_cserve_image_useless(Image_Entry *ie)
{
   Op_Unloaddata msg;
   
   if (csrve_init > 0) server_reinit();
   else return;
   if (!cserve) return;
   if (ie->data1 == NULL) return;
   memset(&msg, 0, sizeof(msg));
   msg.handle = ie->data1;
   msg.server_id = cserve->server_id;
   if (ie->data2) evas_cserve_mem_close(ie->data2);
   ie->data2 = NULL;
   if (ie->server_id == cserve->server_id)
     server_send(cserve, OP_USELESSDATA, sizeof(msg), (unsigned char *)(&msg));
}

EAPI Eina_Bool
evas_cserve_config_get(Op_Getconfig_Reply *config)
{
   Op_Getconfig_Reply *rep;
   int opcode;
   int size;
   if (csrve_init > 0) server_reinit();
   else return 0;
   if (!cserve) return 0;
   if (!server_send(cserve, OP_GETCONFIG, 0, NULL)) return 0;
   rep = (Op_Getconfig_Reply *)server_read(cserve, &opcode, &size);
   if ((rep) && (opcode == OP_GETCONFIG) && (size == sizeof(Op_Getconfig_Reply)))
     {
        memcpy(config, rep, sizeof(Op_Getconfig_Reply));
        free(rep);
        return 1;
     }
   if (rep) free(rep);
   return 0;
}

EAPI Eina_Bool
evas_cserve_config_set(Op_Setconfig *config)
{
   if (csrve_init > 0) server_reinit();
   else return 0;
   if (!cserve) return 0;
   if (!server_send(cserve, OP_SETCONFIG, sizeof(Op_Setconfig), (unsigned char *)config)) return 0;
   return 1;
}

EAPI Eina_Bool
evas_cserve_stats_get(Op_Getstats_Reply *stats)
{
   Op_Getstats_Reply *rep;
   int opcode;
   int size;
   if (csrve_init > 0) server_reinit();
   else return 0;
   if (!cserve) return 0;
   if (!server_send(cserve, OP_GETSTATS, 0, NULL)) return 0;
   rep = (Op_Getstats_Reply *)server_read(cserve, &opcode, &size);
   if ((rep) && (opcode == OP_GETSTATS) && (size == sizeof(Op_Getstats_Reply)))
     {
        memcpy(stats, rep, sizeof(Op_Getstats_Reply));
        free(rep);
        return 1;
     }
   if (rep) free(rep);
   return 0;
}

EAPI Op_Getinfo_Reply *
evas_cserve_info_get(void)
{
   Op_Getinfo_Reply *rep;
   int opcode;
   int size;
   if (csrve_init > 0) server_reinit();
   else return NULL;
   if (!cserve) return NULL;
   if (!server_send(cserve, OP_GETINFO, 0, NULL)) return NULL;
   rep = (Op_Getinfo_Reply *)server_read(cserve, &opcode, &size);
   if ((rep) && (opcode == OP_GETINFO) && (size >= sizeof(Op_Getinfo_Reply)))
     {
        return rep;
     }
   if (rep) free(rep);
   return NULL;
}

#endif
