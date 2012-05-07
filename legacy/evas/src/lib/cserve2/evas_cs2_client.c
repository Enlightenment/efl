#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Eina.h>

#include "evas_cs2.h"
#include "evas_cs2_private.h"

#ifdef EVAS_CSERVE2

typedef void (*Op_Callback)(void *data, const void *msg);

struct _File_Entry {
   unsigned int file_id;
};

struct _Client_Request {
   Message_Type type;
   unsigned int rid;
   Op_Callback cb;
   void *data;
};

typedef struct _File_Entry File_Entry;
typedef struct _Client_Request Client_Request;

static int cserve2_init = 0;
static int socketfd = -1;

static unsigned int _rid_count = 0;
static unsigned int _file_id = 0;
static unsigned int _data_id = 0;

static Eina_List *_requests = NULL;

static struct sockaddr_un socksize;
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX sizeof(socksize.sun_path)
#endif

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

   env = getenv("XDG_RUNTIME_DIR");
   if (!env || !env[0])
     {
        env = getenv("HOME");
        if (!env || !env[0])
          {
             env = getenv("TMPDIR");
             if (!env || !env[0])
               env = "/tmp";
          }
     }

   snprintf(buf, sizeof(buf), "%s/evas-cserve2-%x.socket", env, getuid());
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

static void
_request_answer_add(Message_Type type, unsigned int rid, Op_Callback cb, void *data)
{
   Client_Request *cr = calloc(1, sizeof(*cr));

   cr->type = type;
   cr->rid = rid;
   cr->cb = cb;
   cr->data = data;

   _requests = eina_list_append(_requests, cr);
}

static Eina_Bool
_server_send(const void *buf, int size, Op_Callback cb, void *data)
{
   const Msg_Base *msg;
   if (send(socketfd, &size, sizeof(size), MSG_NOSIGNAL) == -1)
     {
        ERR("Couldn't send message size to server.");
        return EINA_FALSE;
     }
   if (send(socketfd, buf, size, MSG_NOSIGNAL) == -1)
     {
        ERR("Couldn't send message body to server.");
        return EINA_FALSE;
     }

   msg = buf;
   switch (msg->type)
     {
      case CSERVE2_OPEN:
      case CSERVE2_SETOPTS:
      case CSERVE2_LOAD:
      case CSERVE2_PRELOAD:
         _request_answer_add(msg->type, msg->rid, cb, data);
         break;
      default:
         break;
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

int
evas_cserve2_init(void)
{
   if (cserve2_init++)
     return cserve2_init;

   DBG("Connecting to cserve2.");
   if (!_server_connect())
     {
        cserve2_init = 0;
        return 0;
     }

   return cserve2_init;
}

int
evas_cserve2_shutdown(void)
{
   if ((--cserve2_init) > 0)
     return cserve2_init;

   DBG("Disconnecting from cserve2.");
   _server_disconnect();

   return cserve2_init;
}

int
evas_cserve2_use_get(void)
{
   return cserve2_init;
}

static unsigned int
_next_rid(void)
{
   if (!_rid_count)
     _rid_count++;

   return _rid_count++;
}

static unsigned int
_server_dispatch(void)
{
   int size;
   unsigned int rid;
   Eina_List *l, *l_next;
   Client_Request *cr;
   Msg_Base *msg;

   msg = _server_read(&size);
   if (!msg)
     return 0;

   EINA_LIST_FOREACH_SAFE(_requests, l, l_next, cr)
     {
        if (cr->rid != msg->rid) // dispatch this answer
          continue;

        _requests = eina_list_remove_list(_requests, l);
        if (cr->cb)
          cr->cb(cr->data, msg);
        free(cr);
     }

   rid = msg->rid;
   free(msg);

   return rid;
}

static void
_server_dispatch_until(unsigned int rid)
{
   Eina_Bool done = EINA_FALSE;

   while (!done)
     {
        if (_server_dispatch() == rid)
          done = EINA_TRUE;
     }
}

static void
_image_opened_cb(void *data, const void *msg_received)
{
   const Msg_Base *answer = msg_received;
   const Msg_Opened *msg = msg_received;
   Image_Entry *ie = data;

   ie->open_rid = 0;

   if (answer->type == CSERVE2_ERROR)
     {
        const Msg_Error *msg_error = msg_received;
        ERR("Couldn't open image: '%s':'%s'; error: %d",
            ie->file, ie->key, msg_error->error);
        free(ie->data1);
        ie->data1 = NULL;
        return;
     }

   ie->w = msg->image.w;
   ie->h = msg->image.h;
   ie->flags.alpha = msg->image.alpha;
   ie->loop_hint = msg->image.loop_hint;
   ie->loop_count = msg->image.loop_count;
   ie->frame_count = msg->image.frame_count;
}

static void
_image_loaded_cb(void *data, const void *msg_received)
{
   const Msg_Base *answer = msg_received;
   const Msg_Loaded *msg = msg_received;
   Image_Entry *ie = data;
   Data_Entry *dentry = ie->data2;
   const char *shmpath;
   int fd;

   ie->load_rid = 0;

   if (!ie->data2)
     return;

   if (answer->type == CSERVE2_ERROR)
     {
        const Msg_Error *msg_error = msg_received;
        ERR("Couldn't load image: '%s':'%s'; error: %d",
            ie->file, ie->key, msg_error->error);
        free(ie->data2);
        ie->data2 = NULL;
        return;
     }

   shmpath = ((const char *)msg) + sizeof(*msg);

   // dentry->shm.path = strdup(shmpath);
   dentry->shm.mmap_offset = msg->shm.mmap_offset;
   dentry->shm.use_offset = msg->shm.use_offset;
   dentry->shm.mmap_size = msg->shm.mmap_size;
   dentry->shm.image_size = msg->shm.image_size;

   fd = shm_open(shmpath, O_RDONLY, S_IRUSR);
   if (fd < 0)
     {
        free(dentry);
        ie->data2 = NULL;
        return;
     }

   dentry->shm.data = mmap(NULL, dentry->shm.mmap_size, PROT_READ,
                           MAP_SHARED, fd, dentry->shm.mmap_offset);

   if (dentry->shm.data == MAP_FAILED)
     {
        free(dentry);
        ie->data2 = NULL;
     }

   close(fd);

   if (ie->data2)
     {
        ie->flags.loaded = EINA_TRUE;
        ie->flags.alpha_sparse = msg->alpha_sparse;
     }
}

static void
_image_preloaded_cb(void *data, const void *msg_received)
{
   const Msg_Base *answer = msg_received;
   Image_Entry *ie = data;
   Data_Entry *dentry = ie->data2;

   DBG("Received PRELOADED for RID: %d", answer->rid);
   ie->preload_rid = 0;

   if (answer->type == CSERVE2_ERROR)
     {
        const Msg_Error *msg_error = msg_received;
        ERR("Couldn't preload image: '%s':'%s'; error: %d",
            ie->file, ie->key, msg_error->error);
        dentry->preloaded_cb(data, EINA_FALSE);
        dentry->preloaded_cb = NULL;
        return;
     }

   if (dentry && (dentry->preloaded_cb))
     {
        dentry->preloaded_cb(data, EINA_TRUE);
        dentry->preloaded_cb = NULL;
     }
}

static const char *
_build_absolute_path(const char *path, char buf[], int size)
{
   char *p;
   int len;

   if (!path)
     return NULL;

   p = buf;

   if (path[0] == '/')
     strncpy(p, path, size);
   else if (path[0] == '~')
     {
        const char *home = getenv("HOME");
        if (!home)
          return NULL;
        strncpy(p, home, size);
        len = strlen(p);
        size -= len + 1;
        p += len;
        p[0] = '/';
        p++;
        strncpy(p, path + 2, size);
     }
   else
     {
        if (!getcwd(p, size))
          return NULL;
        len = strlen(p);
        size -= len + 1;
        p += len;
        p[0] = '/';
        p++;
        strncpy(p, path, size);
     }

   return buf;
}

static unsigned int
_image_open_server_send(Image_Entry *ie, const char *file, const char *key, RGBA_Image_Loadopts *lopt __UNUSED__)
{
   int flen, klen;
   int size;
   char *buf;
   char filebuf[PATH_MAX];
   Msg_Open msg_open;
   File_Entry *fentry;

   if (cserve2_init == 0)
     {
        ERR("Server not initialized.");
        return 0;
     }

   if (!key) key = "";

   _build_absolute_path(file, filebuf, sizeof(filebuf));

   fentry = calloc(1, sizeof(*fentry));

   memset(&msg_open, 0, sizeof(msg_open));

   fentry->file_id = ++_file_id;
   if (fentry->file_id == 0)
     fentry->file_id = ++_file_id;

   flen = strlen(filebuf) + 1;
   klen = strlen(key) + 1;

   msg_open.base.rid = _next_rid();
   msg_open.base.type = CSERVE2_OPEN;
   msg_open.file_id = fentry->file_id;
   msg_open.path_offset = 0;
   msg_open.key_offset = flen;

   size = sizeof(msg_open) + flen + klen;
   buf = malloc(size);
   if (!buf) return EINA_FALSE;
   memcpy(buf, &msg_open, sizeof(msg_open));
   memcpy(buf + sizeof(msg_open), filebuf, flen);
   memcpy(buf + sizeof(msg_open) + flen, key, klen);

   if (!_server_send(buf, size, _image_opened_cb, ie))
     {
        ERR("Couldn't send message to server.");
        free(buf);
        return 0;
     }

   free(buf);
   ie->data1 = fentry;

   return msg_open.base.rid;
}

unsigned int
_image_setopts_server_send(Image_Entry *ie)
{
   File_Entry *fentry;
   Data_Entry *dentry;
   Msg_Setopts msg;

   if (cserve2_init == 0)
     return 0;

   fentry = ie->data1;

   dentry = calloc(1, sizeof(*dentry));

   memset(&msg, 0, sizeof(msg));
   dentry->image_id = ++_data_id;
   if (dentry->image_id == 0)
     dentry->image_id = ++_data_id;

   msg.base.rid = _next_rid();
   msg.base.type = CSERVE2_SETOPTS;
   msg.file_id = fentry->file_id;
   msg.image_id = dentry->image_id;

   if (!_server_send(&msg, sizeof(msg), 0, NULL))
     return 0;

   ie->data2 = dentry;

   return msg.base.rid;
}

unsigned int
_image_load_server_send(Image_Entry *ie)
{
   Data_Entry *dentry;
   Msg_Load msg;

   if (cserve2_init == 0)
     return 0;

   if (!ie->data1)
     {
        ERR("No data for opened file.");
        return 0;
     }

   dentry = ie->data2;

   memset(&msg, 0, sizeof(msg));

   msg.base.rid = _next_rid();
   msg.base.type = CSERVE2_LOAD;
   msg.image_id = dentry->image_id;

   if (!_server_send(&msg, sizeof(msg), _image_loaded_cb, ie))
     return 0;

   return msg.base.rid;
}

unsigned int
_image_preload_server_send(Image_Entry *ie, void (*preloaded_cb)(void *im, Eina_Bool success))
{
   Data_Entry *dentry;
   Msg_Preload msg;

   if (cserve2_init == 0)
     return 0;

   dentry = ie->data2;
   dentry->preloaded_cb = preloaded_cb;

   memset(&msg, 0, sizeof(msg));

   msg.base.rid = _next_rid();
   msg.base.type = CSERVE2_PRELOAD;
   msg.image_id = dentry->image_id;

   if (!_server_send(&msg, sizeof(msg), _image_preloaded_cb, ie))
     return 0;

   return msg.base.rid;
}

unsigned int
_image_close_server_send(Image_Entry *ie)
{
   Msg_Close msg;
   File_Entry *fentry;

   if (cserve2_init == 0)
     return 0;

   if (!ie->data1)
     return 0;

   fentry = ie->data1;

   if (ie->data2)
     {
        free(ie->data2);
        ie->data2 = NULL;
     }

   memset(&msg, 0, sizeof(msg));
   msg.base.rid = _next_rid();
   msg.base.type = CSERVE2_CLOSE;
   msg.file_id = fentry->file_id;

   free(fentry);
   ie->data1 = NULL;

   if (!_server_send(&msg, sizeof(msg), NULL, NULL))
     return 0;

   return msg.base.rid;
}

unsigned int
_image_unload_server_send(Image_Entry *ie)
{
   Msg_Unload msg;
   Data_Entry *dentry;

   if (cserve2_init == 0)
     return 0;

   if (!ie->data2)
     return 0;

   dentry = ie->data2;

   // if (dentry->shm.path)
   //   free(dentry->shm.path);
   memset(&msg, 0, sizeof(msg));
   msg.base.rid = _next_rid();
   msg.base.type = CSERVE2_UNLOAD;
   msg.image_id = dentry->image_id;

   free(dentry);
   ie->data2 = NULL;

   if (!_server_send(&msg, sizeof(msg), NULL, NULL))
     return 0;

   return msg.base.rid;
}

Eina_Bool
evas_cserve2_image_load(Image_Entry *ie, const char *file, const char *key, RGBA_Image_Loadopts *lopt)
{
   unsigned int rid;

   rid = _image_open_server_send(ie, file, key, lopt);
   if (!rid)
     return EINA_FALSE;

   ie->open_rid = rid;

   _image_setopts_server_send(ie);

   // _server_dispatch_until(rid);

   if (ie->data1)
     return EINA_TRUE;
   else
     return EINA_FALSE;
}

int
evas_cserve2_image_load_wait(Image_Entry *ie)
{
   if (ie->open_rid)
     {
        _server_dispatch_until(ie->open_rid);
        if (!ie->data1)
          return CSERVE2_GENERIC;
        return CSERVE2_NONE;
     }
   else
     return CSERVE2_GENERIC;
}

Eina_Bool
evas_cserve2_image_data_load(Image_Entry *ie)
{
   unsigned int rid;

   rid = _image_load_server_send(ie);
   if (!rid)
     return EINA_FALSE;

   ie->load_rid = rid;

   if (ie->data2)
     return EINA_TRUE;
   else
     return EINA_FALSE;
}

void
evas_cserve2_image_load_data_wait(Image_Entry *ie)
{
   if (ie->load_rid)
     _server_dispatch_until(ie->load_rid);
}

Eina_Bool
evas_cserve2_image_preload(Image_Entry *ie, void (*preloaded_cb)(void *im, Eina_Bool success))
{
   unsigned int rid;

   if (!ie->data1)
     return EINA_FALSE;

   rid = _image_preload_server_send(ie, preloaded_cb);
   if (!rid)
     return EINA_FALSE;

   ie->preload_rid = rid;

   return EINA_FALSE;
}

void
evas_cserve2_image_free(Image_Entry *ie)
{
   if (!ie->data1)
     return;

   if (!_image_close_server_send(ie))
     WRN("Couldn't send close message to cserve2.");
}

void
evas_cserve2_image_unload(Image_Entry *ie)
{
   if (!ie->data2)
     return;

   if (!_image_unload_server_send(ie))
     WRN("Couldn't send unload message to cserve2.");
}

void
evas_cserve2_dispatch(void)
{
   _server_dispatch_until(0);
}
#endif
