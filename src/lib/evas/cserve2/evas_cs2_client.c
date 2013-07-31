#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <Eina.h>

#include "evas_cs2.h"
#include "evas_cs2_private.h"
#include "evas_cs2_utils.h"

#ifdef EVAS_CSERVE2

typedef void (*Op_Callback)(void *data, const void *msg, int size);

struct _File_Entry {
   unsigned int file_id;
   unsigned int server_file_id;
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

// Shared index table
static Index_Table _index;
static const char *_shared_string_get(int id);
static int _server_index_list_set(Msg_Base *data, int size);
static const File_Data *_shared_image_entry_file_data_find(Image_Entry *ie);
static const Image_Data *_shared_image_entry_image_data_find(Image_Entry *ie);

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX sizeof(((struct sockaddr_un *)NULL)->sun_path)
#endif

static inline Eina_Bool
_memory_zero_cmp(void *data, size_t len)
{
   const int *idata = data;
   const char *cdata;
   int remain;

   if (!data || !len) return EINA_TRUE;

   for (remain = len / sizeof(idata); remain > 0; --remain)
     if (*idata++ != 0) return EINA_FALSE;

   cdata = (const char*) idata;
   for (remain = ((const char*) data + len) - cdata; remain > 0; --remain)
     if (*cdata++ != 0) return EINA_FALSE;

   return EINA_TRUE;
}

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
#if 0   
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
#endif   
}

static Eina_Bool
_server_connect(void)
{
   int s, len;
   struct sockaddr_un remote;
#ifdef HAVE_FCNTL
   int flags;
#endif

   if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
     {
        ERR("cserve2 socket creation failed with error [%d] %s", errno, strerror(errno));
        return EINA_FALSE;
     }

#ifdef HAVE_FCNTL
   flags = fcntl(s, F_GETFD);
   flags |= FD_CLOEXEC;
   fcntl(s, F_SETFD, flags);
#endif

   remote.sun_family = AF_UNIX;
   _socket_path_set(remote.sun_path);
   len = strlen(remote.sun_path) + sizeof(remote.sun_family);
   for (;;)
     {
        errno = 0;
        if (connect(s, (struct sockaddr *)&remote, len) != -1) break;
        if (errno == EACCES)
          {
             ERR("not authorized to connect to cserve2!");
             return EINA_FALSE;
          }
        ERR("cserve2 connect failed: [%d] %s. Retrying...", errno, strerror(errno));

        errno = 0;
        usleep(10000);
        if (errno == EINTR)
          {
             WRN("received interruption while trying to connect to cserve2!");
             return EINA_FALSE;
          }

        /* FIXME: Here we should identify the error, maybe signal the daemon manager
         * that we need cserve2 to [re]start or just quit and return false.
         * There probably should be a timeout of some sort also...
         * -- jpeg
         */
     }

#ifdef HAVE_FCNTL
   fcntl(s, F_SETFL, O_NONBLOCK);
#endif

   socketfd = s;

   DBG("connected to cserve2 server.");
   return EINA_TRUE;
}

static void
_server_disconnect(void)
{
   if (socketfd != -1)
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
_server_safe_send(int fd, const void *data, int size)
{
   int sent = 0;
   ssize_t ret;
   const char *msg = data;

   while (sent < size)
     {
        ret = send(fd, msg + sent, size - sent, MSG_NOSIGNAL);
        if (ret < 0)
          {
             if ((errno == EAGAIN) || (errno == EINTR))
               continue;
             DBG("send() failed with error [%d] %s", errno, strerror(errno));
             return EINA_FALSE;
          }
        sent += ret;
     }

   return EINA_TRUE;
}

static Eina_Bool
_server_send(const void *buf, int size, Op_Callback cb, void *data)
{
   const Msg_Base *msg;
   if (!_server_safe_send(socketfd, &size, sizeof(size)))
     {
        ERR("Couldn't send message size to server.");
        return EINA_FALSE;
     }
   if (!_server_safe_send(socketfd, buf, size))
     {
        ERR("Couldn't send message body to server.");
        return EINA_FALSE;
     }

   msg = buf;
   switch (msg->type)
     {
      case CSERVE2_OPEN:
      case CSERVE2_LOAD:
      case CSERVE2_PRELOAD:
      case CSERVE2_FONT_LOAD:
      case CSERVE2_FONT_GLYPHS_LOAD:
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

   memset(&_index, 0, sizeof(_index));

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
   const char zeros[sizeof(Msg_Index_List)] = {0};

   if ((--cserve2_init) > 0)
     return cserve2_init;

   DBG("Disconnecting from cserve2.");
   _server_index_list_set((Msg_Base *) zeros, sizeof(Msg_Index_List));
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
_server_dispatch(Eina_Bool *failed)
{
   int size;
   unsigned int rid;
   Eina_List *l, *l_next;
   Client_Request *cr;
   Msg_Base *msg;

   msg = _server_read(&size);
   if (!msg)
     {
        *failed = EINA_TRUE;
        return 0;
     }
   *failed = EINA_FALSE;

   // Special messages (no request)
   switch (msg->type)
     {
      case CSERVE2_INDEX_LIST:
        _server_index_list_set(msg, size);
        free(msg);
        return 0;
      default: break;
     }

   // Normal client to server requests
   EINA_LIST_FOREACH_SAFE(_requests, l, l_next, cr)
     {
        if (cr->rid != msg->rid) // dispatch this answer
          continue;

        _requests = eina_list_remove_list(_requests, l);
        if (cr->cb)
          cr->cb(cr->data, msg, size);
        free(cr);
     }

   rid = msg->rid;
   free(msg);

   return rid;
}

static Eina_Bool
_server_dispatch_until(unsigned int rid)
{
   Eina_Bool failed;
   fd_set rfds;
   unsigned int rrid;
   struct timeval tv;

   while (1)
     {
        rrid = _server_dispatch(&failed);
        if (rrid == rid) break;
        else if (failed)
          {
             int sel;

             //DBG("Waiting for request %d...", rid);
             FD_ZERO(&rfds);
             FD_SET(socketfd, &rfds);
             tv.tv_sec = 1;
             tv.tv_usec = 0;
             sel = select(socketfd + 1, &rfds, NULL, NULL, &tv);
             if (sel == -1)
               {
                  ERR("select() failed: [%d] %s", errno, strerror(errno));
                  /* FIXME: Depending on the error, we should probably try to reconnect to the server.
                   * Or even ask to [re]start the daemon.
                   * Or maybe just return EINA_FALSE after some timeout?
                   * -- jpeg
                   */
                  if (errno == EINTR)
                    {
                       DBG("giving up on request %d after interrupt", rid);
                       return EINA_FALSE;
                    }
               }
             else if (!sel)
               {
                  WRN("select() timed out. giving up on request %d", rid);
                  return EINA_FALSE;
               }
          }
     }
   return EINA_TRUE;
}

static void
_image_opened_cb(void *data, const void *msg_received, int size)
{
   const Msg_Base *answer = msg_received;
   const Msg_Opened *msg = msg_received;
   Image_Entry *ie = data;

   /* FIXME: Maybe we could have more asynchronous loading in the server side
    * and so we would have to check that open_rid is equal to answer->rid.
    * -- jpeg
    */
   //DBG("Received OPENED for RID: %d [open_rid: %d]", answer->rid, ie->open_rid);
   if (answer->rid != ie->open_rid)
     {
        WRN("Message rid (%d) differs from expected rid (open_rid: %d)", answer->rid, ie->open_rid);
        return;
     }
   ie->open_rid = 0;

   if (answer->type != CSERVE2_OPENED)
     {
        if (answer->type == CSERVE2_ERROR)
          {
             const Msg_Error *msg_error = msg_received;
             ERR("Couldn't open image: '%s':'%s'; error: %d",
                 ie->file, ie->key, msg_error->error);
          }
        else
          ERR("Invalid message type received: %d (%s)", answer->type, __FUNCTION__);
        free(ie->data1);
        ie->data1 = NULL;
        return;
     }
   else if (size < (int) sizeof(*msg))
     {
        ERR("Received message is too small");
        free(ie->data1);
        ie->data1 = NULL;
        return;
     }

   ie->w = msg->image.w;
   ie->h = msg->image.h;
   ie->flags.alpha = msg->image.alpha;
   ie->animated.loop_hint = msg->image.loop_hint;
   ie->animated.loop_count = msg->image.loop_count;
   ie->animated.frame_count = msg->image.frame_count;
}

static void
_loaded_handle(Image_Entry *ie, const Msg_Loaded *msg, int size)
{
   Data_Entry *dentry = ie->data2;
   RGBA_Image *im = (RGBA_Image *)ie;
   const char *shmpath;

   shmpath = ((const char *)msg) + sizeof(*msg);
   if ((size < (int) sizeof(*msg) + 1)
       || (strnlen(shmpath, size - sizeof(*msg)) >= (size - sizeof(*msg))))
     {
        DBG("invalid message size");
        goto fail;
     }

   // dentry->shm.path = strdup(shmpath);
   dentry->shm.mmap_offset = msg->shm.mmap_offset;
   dentry->shm.use_offset = msg->shm.use_offset;
   dentry->shm.mmap_size = msg->shm.mmap_size;
   dentry->shm.image_size = msg->shm.image_size;

   dentry->shm.f = eina_file_open(shmpath, EINA_TRUE);
   if (!dentry->shm.f)
     {
        DBG("could not open the shm file: %d %m", errno);
        goto fail;
     }

   dentry->shm.data = eina_file_map_new(dentry->shm.f, EINA_FILE_WILLNEED,
                                        dentry->shm.mmap_offset,
                                        dentry->shm.mmap_size);

   if (!dentry->shm.data)
     {
        DBG("could not mmap the shm file: %d %m", errno);
        eina_file_close(dentry->shm.f);
        goto fail;
     }

   im->image.data = evas_cserve2_image_data_get(ie);
   ie->flags.alpha_sparse = msg->alpha_sparse;
   ie->flags.loaded = EINA_TRUE;
   im->image.no_free = 1;
   return;

fail:
   ERR("Failed in %s", __FUNCTION__);
   free(ie->data2);
   ie->data2 = NULL;
}

static void
_image_loaded_cb(void *data, const void *msg_received, int size)
{
   const Msg_Base *answer = msg_received;
   const Msg_Loaded *msg = msg_received;
   Image_Entry *ie = data;

   //DBG("Received LOADED for RID: %d [load_rid: %d]", answer->rid, ie->load_rid);
   if (answer->rid != ie->load_rid)
     {
        WRN("Message rid (%d) differs from expected rid (load_rid: %d)", answer->rid, ie->load_rid);
        return;
     }
   ie->load_rid = 0;

   if (!ie->data2)
     {
        ERR("No data2 for loaded file");
        return;
     }

   if (answer->type != CSERVE2_LOADED)
     {
        if (answer->type == CSERVE2_ERROR)
          {
             const Msg_Error *msg_error = msg_received;
             ERR("Couldn't load image: '%s':'%s'; error: %d",
                 ie->file, ie->key, msg_error->error);
          }
        else
          ERR("Invalid message type received: %d (%s)", answer->type, __FUNCTION__);
        free(ie->data2);
        ie->data2 = NULL;
        return;
     }

   _loaded_handle(ie, msg, size);
}

static void
_image_preloaded_cb(void *data, const void *msg_received, int size)
{
   const Msg_Base *answer = msg_received;
   Image_Entry *ie = data;
   Data_Entry *dentry = ie->data2;

   //DBG("Received PRELOADED for RID: %d [preload_rid: %d]", answer->rid, ie->preload_rid);
   if (answer->rid != ie->preload_rid)
     WRN("Message rid (%d) differs from expected rid (preload_rid: %d)", answer->rid, ie->preload_rid);
   ie->preload_rid = 0;

   if (!ie->data2)
     {
        ERR("No data2 for preloaded file");
        return;
     }

   if (answer->type != CSERVE2_PRELOAD &&
       answer->type != CSERVE2_LOADED)
     {
        if (answer->type == CSERVE2_ERROR)
          {
             const Msg_Error *msg_error = msg_received;
             ERR("Couldn't preload image: '%s':'%s'; error: %d",
                 ie->file, ie->key, msg_error->error);
          }
        else
          ERR("Invalid message type received: %d (%s)", answer->type, __FUNCTION__);
        if (dentry->preloaded_cb)
          dentry->preloaded_cb(data, EINA_FALSE);
        dentry->preloaded_cb = NULL;
        return;
     }

   _loaded_handle(ie, msg_received, size);

   dentry = ie->data2;
   if (dentry && (dentry->preloaded_cb))
     {
        dentry->preloaded_cb(data, EINA_TRUE);
        dentry->preloaded_cb = NULL;
     }
}

static int
_build_absolute_path(const char *path, char buf[], int size)
{
   char *p;
   int len;

   if (!path)
     return 0;

   p = buf;

   if (path[0] == '/')
     len = eina_strlcpy(p, path, size);
   else if (path[0] == '~')
     {
        const char *home = getenv("HOME");
        if (!home)
          return 0;
        len = eina_strlcpy(p, home, size);
        size -= len + 1;
        p += len;
        p[0] = '/';
        p++;
        len++;
        len += eina_strlcpy(p, path + 2, size);
     }
   else
     {
        if (!getcwd(p, size))
          return 0;
        len = strlen(p);
        size -= len + 1;
        p += len;
        p[0] = '/';
        p++;
        len++;
        len += eina_strlcpy(p, path, size);
     }

   return len;
}

static unsigned int
_image_open_server_send(Image_Entry *ie, const char *file, const char *key,
                        Evas_Image_Load_Opts *opts)
{
   int flen, klen;
   int size;
   char *buf;
   char filebuf[PATH_MAX];
   Msg_Open msg_open;
   File_Entry *fentry;
   Data_Entry *dentry;

   if (cserve2_init == 0)
     {
        ERR("Server not initialized.");
        return 0;
     }

   ie->data1 = NULL;
   ie->data2 = NULL;

   flen = _build_absolute_path(file, filebuf, sizeof(filebuf));
   if (!flen)
     {
        ERR("Could not find absolute path for %s", file);
        return 0;
     }
   flen++;

   if (!key) key = "";

   fentry = calloc(1, sizeof(*fentry));
   if (!fentry)
     return 0;

   dentry = calloc(1, sizeof(*dentry));
   if (!dentry)
     {
        free(fentry);
        return 0;
     }

   memset(&msg_open, 0, sizeof(msg_open));

   fentry->file_id = ++_file_id;
   klen = strlen(key) + 1;

   msg_open.base.rid = _next_rid();
   msg_open.base.type = CSERVE2_OPEN;
   msg_open.file_id = fentry->file_id;
   msg_open.path_offset = 0;
   msg_open.key_offset = flen;
   msg_open.has_load_opts = (opts != NULL);
   msg_open.image_id = ++_data_id;

   size = sizeof(msg_open) + flen + klen;
   if (opts)
     size += sizeof(*opts);
   buf = malloc(size);
   if (!buf)
     {
        free(fentry);
        free(dentry);
        return 0;
     }
   memcpy(buf, &msg_open, sizeof(msg_open));
   memcpy(buf + sizeof(msg_open), filebuf, flen);
   memcpy(buf + sizeof(msg_open) + flen, key, klen);
   if (opts)
     memcpy(buf + sizeof(msg_open) + flen + klen, opts, sizeof(*opts));

   if (!_server_send(buf, size, _image_opened_cb, ie))
     {
        ERR("Couldn't send message to server.");
        free(buf);
        free(fentry);
        free(dentry);
        return 0;
     }

   free(buf);
   ie->data1 = fentry;

   dentry->image_id = msg_open.image_id;
   ie->data2 = dentry;

   return msg_open.base.rid;
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
        ERR("No data1 for opened file.");
        return 0;
     }

   dentry = ie->data2;
   if (!dentry)
     {
        ERR("No data2 for opened file.");
        return 0;
     }

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
   if (!dentry)
     {
        ERR("No data2 for opened file.");
        return 0;
     }
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
        Data_Entry *dentry = ie->data2;
        if (dentry->shm.data)
          eina_file_map_free(dentry->shm.f, dentry->shm.data);
        if (dentry->shm.f)
          eina_file_close(dentry->shm.f);
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

   if (dentry->shm.data)
     eina_file_map_free(dentry->shm.f, dentry->shm.data);
   if (dentry->shm.f)
     eina_file_close(dentry->shm.f);

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
evas_cserve2_image_load(Image_Entry *ie)
{
   unsigned int rid;
   const char *file, *key;
   Evas_Image_Load_Opts *opts = NULL;

   if (!ie)
     return EINA_FALSE;

   file = ie->file;
   key = ie->key;
   if (!_memory_zero_cmp(&ie->load_opts, sizeof(ie->load_opts)))
     opts = &ie->load_opts;
   rid = _image_open_server_send(ie, file, key, opts);
   if (!rid)
     return EINA_FALSE;

   ie->open_rid = rid;

   if (ie->data1)
     return EINA_TRUE;
   else
     return EINA_FALSE;
}

int
evas_cserve2_image_load_wait(Image_Entry *ie)
{
   const File_Data *fd;

   if (!ie)
     return CSERVE2_GENERIC;

   fd = _shared_image_entry_file_data_find(ie);
   if (fd)
     {
        INF("Bypassing socket wait (open_rid %d)", ie->open_rid);
        ie->w = fd->w;
        ie->h = fd->h;
        ie->flags.alpha = fd->alpha;
        ie->animated.loop_hint = fd->loop_hint;
        ie->animated.loop_count = fd->loop_count;
        ie->animated.frame_count = fd->frame_count;
        ie->open_rid = 0;
        return CSERVE2_NONE;
     }

   if (ie->open_rid)
     {
        if (!_server_dispatch_until(ie->open_rid))
          return CSERVE2_GENERIC;
        if (!ie->data1)
          return CSERVE2_GENERIC;
     }

   return CSERVE2_NONE;
}

Eina_Bool
evas_cserve2_image_data_load(Image_Entry *ie)
{
   unsigned int rid;

   if (!ie)
     return EINA_FALSE;

   rid = _image_load_server_send(ie);
   if (!rid)
     return EINA_FALSE;

   ie->load_rid = rid;

   if (ie->data2)
     return EINA_TRUE;
   else
     return EINA_FALSE;
}

int
evas_cserve2_image_load_data_wait(Image_Entry *ie)
{
   const Image_Data *idata;

   if (!ie)
     return CSERVE2_GENERIC;

   idata = _shared_image_entry_image_data_find(ie);
   if (idata)
     {
        // FIXME: Ugly copy & paste from _loaded_handle
        Data_Entry *dentry = ie->data2;
        RGBA_Image *im = (RGBA_Image *)ie;
        const char *shmpath;

        shmpath = _shared_string_get(idata->shm_id);
        if (!shmpath) goto load_wait;
        INF("Bypassing image load socket wait. Image found: %d in %s",
            idata->id, shmpath);

        dentry->shm.mmap_offset = 0;
        dentry->shm.use_offset = 0;
        dentry->shm.f = eina_file_open(shmpath, EINA_TRUE);
        dentry->shm.mmap_size = eina_file_size_get(dentry->shm.f);
        dentry->shm.image_size = dentry->shm.mmap_size;

        dentry->shm.data = eina_file_map_new(dentry->shm.f, EINA_FILE_WILLNEED,
                                             dentry->shm.mmap_offset,
                                             dentry->shm.mmap_size);
        if (!dentry->shm.data)
          {
             DBG("could not mmap the shm file: %d %m", errno);
             eina_file_close(dentry->shm.f);
             dentry->shm.f = NULL;
             goto load_wait;
          }

        im->image.data = dentry->shm.data;
        ie->flags.alpha_sparse = idata->alpha_sparse;
        ie->flags.loaded = EINA_TRUE;
        im->image.no_free = 1;

        ie->load_rid = 0;
        return CSERVE2_NONE;
     }

load_wait:
   if (ie->load_rid)
     {
        if (!_server_dispatch_until(ie->load_rid))
          return CSERVE2_GENERIC;
        if (!ie->data2)
          return CSERVE2_GENERIC;
     }

   return CSERVE2_NONE;
}

Eina_Bool
evas_cserve2_image_preload(Image_Entry *ie, void (*preloaded_cb)(void *im, Eina_Bool success))
{
   unsigned int rid;

   if (!ie || !ie->data1)
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
   if (!ie || !ie->data1)
     return;

   if (!_image_close_server_send(ie))
     WRN("Couldn't send close message to cserve2.");
}

void
evas_cserve2_image_unload(Image_Entry *ie)
{
   if (!ie || !ie->data2)
     return;

   if (!_image_unload_server_send(ie))
     WRN("Couldn't send unload message to cserve2.");
}

void
evas_cserve2_dispatch(void)
{
   _server_dispatch_until(0);
}

typedef struct _Glyph_Map Glyph_Map;
typedef struct _CS_Glyph_Out CS_Glyph_Out;

struct _Font_Entry
{
   const char *source;
   const char *name;
   unsigned int size;
   unsigned int dpi;
   Font_Rend_Flags wanted_rend;

   unsigned int rid; // open

   Eina_Hash *glyphs_maps;
   Fash_Glyph2 *fash[3]; // one per hinting value

   Eina_Clist glyphs_queue;
   int glyphs_queue_count;
   Eina_Clist glyphs_used;
   int glyphs_used_count;

   Eina_Bool failed : 1;
};

struct _Glyph_Map
{
   Font_Entry *fe;
   const char *name;
   unsigned int size;
   Eina_File *map;
   unsigned char *data;
   Eina_Clist glyphs;
};

struct _CS_Glyph_Out
{
   RGBA_Font_Glyph_Out base;
   Eina_Clist map_entry;
   Eina_Clist used_list;
   unsigned int idx;
   unsigned int rid;
   Glyph_Map *map;
   unsigned int offset;
   unsigned int size;
   Eina_Bool used;
};

static void
_glyphs_map_free(Glyph_Map *m)
{
   eina_file_map_free(m->map, m->data);
   eina_file_close(m->map);
   eina_stringshare_del(m->name);
   free(m);
}

static void
_glyph_out_free(void *gl)
{
   CS_Glyph_Out *glout = gl;

   if (glout->map)
     {
        // FIXME: Invalid write of size 8 here (64 bit machine)
        eina_clist_remove(&glout->map_entry);
        if (eina_clist_empty(&glout->map->glyphs))
          {
             eina_hash_del(glout->map->fe->glyphs_maps, &glout->map->name,
                           NULL);
             _glyphs_map_free(glout->map);
          }
     }

   free(glout);
}

static Eina_Bool
_glyphs_maps_foreach_free(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   Glyph_Map *m = data;

   _glyphs_map_free(m);
   return EINA_TRUE;
}

static void
_font_entry_free(Font_Entry *fe)
{
   int i;

   for (i = 0; i < 3; i++)
     if (fe->fash[i])
       fash_gl_free(fe->fash[i]);

   eina_stringshare_del(fe->source);
   eina_stringshare_del(fe->name);
   eina_hash_foreach(fe->glyphs_maps, _glyphs_maps_foreach_free, NULL);
   eina_hash_free(fe->glyphs_maps);
   free(fe);
}

static void
_font_loaded_cb(void *data, const void *msg, int size)
{
   const Msg_Base *m = msg;
   Font_Entry *fe = data;

   fe->rid = 0;

   if ((size < (int) sizeof(*m))
       || (m->type == CSERVE2_ERROR))
     fe->failed = EINA_TRUE;
}

static unsigned int
_font_load_server_send(Font_Entry *fe, Message_Type type)
{
   Msg_Font_Load *msg;
   int source_len, path_len, size;
   char *buf;
   unsigned int ret = 0;
   Op_Callback cb = NULL;

   if (!cserve2_init)
     return 0;

   source_len = fe->source ? eina_stringshare_strlen(fe->source) + 1 : 0;
   path_len = eina_stringshare_strlen(fe->name) + 1;

   size = sizeof(*msg) + path_len + source_len;
   msg = calloc(1, size);

   msg->base.rid = _next_rid();
   msg->base.type = type;

   msg->sourcelen = source_len;
   msg->pathlen = path_len;
   msg->rend_flags = fe->wanted_rend;
   msg->size = fe->size;
   msg->dpi = fe->dpi;

   buf = ((char *)msg) + sizeof(*msg);
   if (source_len > 0) memcpy(buf, fe->source, source_len);
   buf += source_len;
   memcpy(buf, fe->name, path_len);

   if (type == CSERVE2_FONT_LOAD)
     cb = _font_loaded_cb;

   if (_server_send(msg, size, cb, fe))
     ret = msg->base.rid;

   free(msg);

   return ret;
}

Font_Entry *
evas_cserve2_font_load(const char *source, const char *name, int size, int dpi, Font_Rend_Flags wanted_rend)
{
   Font_Entry *fe;

   fe = calloc(1, sizeof(Font_Entry));
   if (!fe) return NULL;

   fe->source = source ? eina_stringshare_add(source) : NULL;
   fe->name = eina_stringshare_add(name);
   fe->size = size;
   fe->dpi = dpi;
   fe->wanted_rend = wanted_rend;

   if (!(fe->rid = _font_load_server_send(fe, CSERVE2_FONT_LOAD)))
     {
        eina_stringshare_del(fe->source);
        eina_stringshare_del(fe->name);
        free(fe);
        return NULL;
     }

   fe->glyphs_maps = eina_hash_stringshared_new(NULL);
   eina_clist_init(&fe->glyphs_queue);
   eina_clist_init(&fe->glyphs_used);

   return fe;
}

int
evas_cserve2_font_load_wait(Font_Entry *fe)
{
   if (!_server_dispatch_until(fe->rid))
     return CSERVE2_GENERIC;

   if (fe->failed)
     return CSERVE2_GENERIC;

   return CSERVE2_NONE;
}

void
evas_cserve2_font_free(Font_Entry *fe)
{
   int ret;

   if (!fe) return;

   ret = evas_cserve2_font_load_wait(fe);
   if (ret != CSERVE2_NONE)
     {
        ERR("Failed to wait loading font '%s'.", fe->name);
        _font_entry_free(fe);
        return;
     }

   _font_load_server_send(fe, CSERVE2_FONT_UNLOAD);

   _font_entry_free(fe);
}

typedef struct
{
   Font_Entry *fe;
   Font_Hint_Flags hints;
   unsigned int rid;
} Glyph_Request_Data;

static void
_glyph_request_cb(void *data, const void *msg, int size)
{
   const Msg_Font_Glyphs_Loaded *resp = msg;
   Glyph_Request_Data *grd = data;
   Font_Entry *fe = grd->fe;
   unsigned int ncaches;
   const char *buf;

   if (resp->base.type == CSERVE2_ERROR)
     goto end;

   if (!fe->fash[grd->hints])
     goto end;

   if (size <= (int) sizeof(*resp)) goto end;

   buf = (const char *)resp + sizeof(*resp);
   for (ncaches = 0; ncaches < resp->ncaches; ncaches++)
     {
        int i, nglyphs;
        int namelen;
        const char *name;
        Glyph_Map *map;
        int pos = buf - (const char*) resp;

        pos += sizeof(int);
        if (pos > size) goto end;

        memcpy(&namelen, buf, sizeof(int));
        buf += sizeof(int);

        pos += namelen + sizeof(int);
        if (pos > size) goto end;

        name = eina_stringshare_add_length(buf, namelen);
        buf += namelen;

        memcpy(&nglyphs, buf, sizeof(int));
        buf += sizeof(int);

        map = eina_hash_find(fe->glyphs_maps, name);
        if (!map)
          {
             map = calloc(1, sizeof(*map));
             map->fe = fe;
             map->name = name;
             map->map = eina_file_open(name, EINA_TRUE);
             map->data = eina_file_map_all(map->map, EINA_FILE_WILLNEED);
             eina_clist_init(&map->glyphs);
             eina_hash_direct_add(fe->glyphs_maps, &map->name, map);
          }
        else
          eina_stringshare_del(name);

        for (i = 0; i < nglyphs; i++)
          {
             unsigned int idx, offset, glsize;
             int rows, width, pitch, num_grays, pixel_mode;
             CS_Glyph_Out *gl;

             pos = buf - (const char*) resp;
             pos += 8 * sizeof(int);
             if (pos > size) goto end;

             memcpy(&idx, buf, sizeof(int));
             buf += sizeof(int);
             memcpy(&offset, buf, sizeof(int));
             buf += sizeof(int);
             memcpy(&glsize, buf, sizeof(int));
             buf += sizeof(int);
             memcpy(&rows, buf, sizeof(int));
             buf += sizeof(int);
             memcpy(&width, buf, sizeof(int));
             buf += sizeof(int);
             memcpy(&pitch, buf, sizeof(int));
             buf += sizeof(int);
             memcpy(&num_grays, buf, sizeof(int));
             buf += sizeof(int);
             memcpy(&pixel_mode, buf, sizeof(int));
             buf += sizeof(int);

             gl = fash_gl_find(fe->fash[grd->hints], idx);
             if (gl)
               {
                  gl->map = map;
                  gl->offset = offset;
                  gl->size = glsize;
                  gl->base.bitmap.rows = rows;
                  gl->base.bitmap.width = width;
                  gl->base.bitmap.pitch = pitch;
                  gl->base.bitmap.buffer = map->data + gl->offset;
                  gl->base.bitmap.num_grays = num_grays;
                  gl->base.bitmap.pixel_mode = pixel_mode;
                  gl->rid = 0;

                  eina_clist_add_head(&map->glyphs, &gl->map_entry);
               }
          }
     }

end:
   free(grd);
}

static unsigned int
_glyph_request_server_send(Font_Entry *fe, Font_Hint_Flags hints, Eina_Bool used)
{
   Msg_Font_Glyphs_Request *msg;
   Glyph_Request_Data *grd = NULL;
   int source_len, name_len, size, nglyphs;
   char *buf;
   unsigned int *glyphs;
   unsigned int ret = 0;
   Op_Callback cb;
   Eina_Clist *queue, *itr, *itr_next;


   source_len = fe->source ? eina_stringshare_strlen(fe->source) + 1 : 0;
   name_len = eina_stringshare_strlen(fe->name) + 1;

   if (!used)
     {
        nglyphs = fe->glyphs_queue_count;
        queue = &fe->glyphs_queue;
     }
   else
     {
        nglyphs = fe->glyphs_used_count;
        queue = &fe->glyphs_used;
     }

   size = sizeof(*msg) + source_len + name_len + (nglyphs * sizeof(int));
   msg = calloc(1, size);

   msg->base.rid = _next_rid();
   if (!used)
     msg->base.type = CSERVE2_FONT_GLYPHS_LOAD;
   else
     msg->base.type = CSERVE2_FONT_GLYPHS_USED;

   msg->sourcelen = source_len;
   msg->pathlen = name_len;
   msg->rend_flags = fe->wanted_rend;
   msg->size = fe->size;
   msg->dpi = fe->dpi;
   msg->hint = hints;
   msg->nglyphs = nglyphs;

   buf = ((char *)msg) + sizeof(*msg);
   if (source_len > 0) memcpy(buf, fe->source, source_len);
   buf += source_len;
   memcpy(buf, fe->name, name_len);
   buf += name_len;
   glyphs = (unsigned int *)buf;
   nglyphs = 0;
   EINA_CLIST_FOR_EACH_SAFE(itr, itr_next, queue)
     {
        CS_Glyph_Out *gl;

        if (!used)
          {
             gl = EINA_CLIST_ENTRY(itr, CS_Glyph_Out, map_entry);
             gl->rid = msg->base.rid;
             eina_clist_remove(&gl->map_entry);
          }
        else
          {
             gl = EINA_CLIST_ENTRY(itr, CS_Glyph_Out, used_list);
             gl->used = EINA_FALSE;
             eina_clist_remove(&gl->used_list);
          }
        glyphs[nglyphs++] = gl->idx;
     }
   if (!used)
     fe->glyphs_queue_count = 0;
   else
     fe->glyphs_used_count = 0;

   if (!used)
     {
        cb = _glyph_request_cb;
        grd = malloc(sizeof(*grd));
        grd->fe = fe;
        grd->rid = msg->base.rid;
        grd->hints = hints;
     }
   else
     cb = NULL;

   if (_server_send(msg, size, cb, grd))
     ret = msg->base.rid;
   else
     free(grd);

   free(msg);

   return ret;
}

Eina_Bool
evas_cserve2_font_glyph_request(Font_Entry *fe, unsigned int idx, Font_Hint_Flags hints)
{
   Fash_Glyph2 *fash;
   CS_Glyph_Out *glyph;

   if (fe->rid)
     _server_dispatch_until(0); /* dispatch anything pending just to avoid
                                   requesting glyphs for a font we may already
                                   know it failed loading, but don't block */

   if (fe->failed)
     return EINA_FALSE;

   fash = fe->fash[hints];
   if (!fash)
     {
        fash = fash_gl_new(_glyph_out_free);
        fe->fash[hints] = fash;
     }

   glyph = fash_gl_find(fash, idx);
   if (!glyph)
     {
        glyph = calloc(1, sizeof(*glyph));

        glyph->idx = idx;

        fash_gl_add(fash, idx, glyph);

        eina_clist_add_head(&fe->glyphs_queue, &glyph->map_entry);
        fe->glyphs_queue_count++;
     }
   else if (!glyph->used)
     {
        eina_clist_add_head(&fe->glyphs_used, &glyph->used_list);
        fe->glyphs_used_count++;
        glyph->used = EINA_TRUE;
     }

   /* FIXME crude way to manage a queue, but it will work for now */
   if (fe->glyphs_queue_count == 50)
     _glyph_request_server_send(fe, hints, EINA_FALSE);

   return EINA_TRUE;
}

Eina_Bool
evas_cserve2_font_glyph_used(Font_Entry *fe, unsigned int idx, Font_Hint_Flags hints)
{
   Fash_Glyph2 *fash;
   CS_Glyph_Out *glyph;

   if (fe->rid)
     _server_dispatch_until(0); /* dispatch anything pending just to avoid
                                   requesting glyphs for a font we may already
                                   know it failed loading, but don't block */

   if (fe->failed)
     return EINA_FALSE;

   fash = fe->fash[hints];
   if (!fash)
     return EINA_FALSE;

   glyph = fash_gl_find(fash, idx);
   /* If we found the glyph on client cache, we should also have at least
    * its request done.
    */
   if (!glyph)
     return EINA_FALSE;

   if (!glyph->map)
     return EINA_TRUE;

   if (glyph->used)
     return EINA_TRUE;

   eina_clist_add_head(&fe->glyphs_used, &glyph->used_list);
   fe->glyphs_used_count++;
   glyph->used = EINA_TRUE;

   return EINA_TRUE;
}

RGBA_Font_Glyph_Out *
evas_cserve2_font_glyph_bitmap_get(Font_Entry *fe, unsigned int idx, Font_Hint_Flags hints)
{
   Fash_Glyph2 *fash;
   CS_Glyph_Out *out;

   if (fe->failed)
     return NULL;

   /* quick hack, flush pending queue when we are asked for a bitmap */
   if (fe->glyphs_queue_count)
     _glyph_request_server_send(fe, hints, EINA_FALSE);

   if (fe->glyphs_used_count)
     _glyph_request_server_send(fe, hints, EINA_TRUE);

   fash = fe->fash[hints];
   if (!fash)
     {
        // this should not happen really, so let the user know he fucked up
        ERR("%s was called with a hinting value that was not requested!", __FUNCTION__);
        return NULL;
     }

   out = fash_gl_find(fash, idx);
   if (!out)
     {
        // again, if we are asking for a bitmap we were supposed to already
        // have requested the glyph, it must be there
        ERR("%s was called with a glyph index that was not requested!", __FUNCTION__);
        return NULL;
     }
   if (out->rid)
     _server_dispatch_until(out->rid);

   // promote shm and font entry in lru or something

   return &(out->base);
}



// Fast access to shared index tables


// Returns the number of correctly opened index arrays
static int
_server_index_list_set(Msg_Base *data, int size)
{
   Msg_Index_List *msg = (Msg_Index_List *) data;
   unsigned sz;
   int ret = 0;

   // TODO #1: Check populate rule.
   // TODO #2: Protect memory for read-only access.
   // TODO #3: Optimize file reopen/remap (esp. strings table)

   if (size != sizeof(*msg) || msg->base.type != CSERVE2_INDEX_LIST)
     {
        CRIT("Invalid message! type: %d, size: %d (expected %d)",
             msg->base.type, size, (int) sizeof(*msg));
        return -1;
     }

   // Reset index table
   if (_index.strings.index_file)
     {
        if (_index.strings.index_header)
          eina_file_map_free(_index.strings.index_file,
                             (void *) _index.strings.index_header);
        eina_file_close(_index.strings.index_file);
     }
   if (_index.strings.entries_file)
     {
        if (_index.strings.index_header)
          eina_file_map_free(_index.strings.entries_file,
                             (void *) _index.strings.index_header);
        eina_file_close(_index.strings.entries_file);
     }
   if (_index.files.f)
     {
        if (_index.files.header)
          eina_file_map_free(_index.files.f, (void *) _index.files.header);
        eina_file_close(_index.files.f);
     }
   if (_index.images.f)
     {
        if (_index.images.header)
          eina_file_map_free(_index.images.f, (void *) _index.images.header);
        eina_file_close(_index.images.f);
     }
   if (_index.fonts.f)
     {
        if (_index.fonts.header)
          eina_file_map_free(_index.fonts.f, (void *) _index.fonts.header);
        eina_file_close(_index.fonts.f);
     }

   // Open new indexes
   eina_strlcpy(_index.strings.index_path, msg->strings_index_path, 64);
   eina_strlcpy(_index.strings.entries_path, msg->strings_entries_path, 64);
   eina_strlcpy(_index.files.path, msg->files_index_path, 64);
   eina_strlcpy(_index.images.path, msg->images_index_path, 64);
   eina_strlcpy(_index.fonts.path, msg->fonts_index_path, 64);

   if (_index.strings.index_path[0] && _index.strings.entries_path[0])
     {
        _index.strings.index_file = eina_file_open(_index.strings.index_path,
                                                   EINA_TRUE);
        sz = eina_file_size_get(_index.strings.index_file);
        _index.strings.index_header = eina_file_map_all(
                 _index.strings.index_file, EINA_FILE_POPULATE);
        if (_index.strings.index_header && sz > sizeof(Shared_Array_Header)
            && sz >= (_index.strings.index_header->count * sizeof(Index_Entry)
                      + sizeof(Shared_Array_Header)))
          {
             _index.strings.indexes = (Index_Entry *)
                   &(_index.strings.index_header[1]);
             _index.strings.entries_file = eina_file_open(
                      _index.strings.entries_path, EINA_TRUE);
             _index.strings.entries_size = eina_file_size_get(
                      _index.strings.entries_file);
             _index.strings.data = eina_file_map_all(
                      _index.strings.entries_file, EINA_FILE_RANDOM);
             if (!_index.strings.entries_size || !_index.strings.data)
               goto strings_map_failed;
             DBG("Mapped shared string table with indexes in %s and data in %s",
                 _index.strings.index_path, _index.strings.entries_path);
          }
        else
          {
strings_map_failed:
             eina_file_map_free(_index.strings.entries_file,
                                (void *) _index.strings.data);
             eina_file_close(_index.strings.entries_file);
             eina_file_map_free(_index.strings.index_file,
                                (void *) _index.strings.index_header);
             eina_file_close(_index.strings.index_file);
             memset(&_index.strings, 0, sizeof(_index.strings));
          }
     }

   if (_index.files.path[0])
     {
        _index.files.f = eina_file_open(_index.files.path, EINA_TRUE);
        sz = eina_file_size_get(_index.files.f);
        if (sz < sizeof(Shared_Array_Header))
          {
             ERR("Shared index for files is too small: %u", sz);
             eina_file_close(_index.files.f);
             _index.files.f = NULL;
          }
        else
          {
             _index.files.header = eina_file_map_all(_index.files.f,
                                                     EINA_FILE_POPULATE);
             if (sz < (_index.files.header->count * sizeof(File_Data)
                       + sizeof(Shared_Array_Header)))
               {
                  ERR("Shared index size does not match array size: %u / %u",
                      sz, _index.files.header->count);
                  eina_file_map_free(_index.files.f,
                                     (void *) _index.files.header);
                  eina_file_close(_index.files.f);
                  _index.files.f = NULL;
                  _index.files.header = NULL;
               }
             else
               {
                  _index.files.entries.fdata =
                        (File_Data *) &(_index.files.header[1]);
                  DBG("Mapped files shared index '%s' at %p: %u entries max",
                      _index.files.path, _index.files.header,
                      _index.files.header->count);
                  ret++;
               }
          }
     }

   if (_index.images.path[0])
     {
        _index.images.f = eina_file_open(_index.images.path, EINA_TRUE);
        sz = eina_file_size_get(_index.images.f);
        if (sz < sizeof(Shared_Array_Header))
          {
             ERR("Shared index for images is too small: %u", sz);
             eina_file_close(_index.images.f);
             _index.images.f = NULL;
          }
        else
          {
             int size = eina_file_size_get(_index.images.f);
             _index.images.header = eina_file_map_all(_index.images.f,
                                                     EINA_FILE_POPULATE);
             if (sz < (_index.images.header->count * sizeof(Image_Data)
                       + sizeof(Shared_Array_Header)))
               {
                  ERR("Shared index size does not match array size: %u / %u",
                      sz, _index.images.header->count);
                  eina_file_map_free(_index.images.f,
                                     (void *) _index.images.header);
                  eina_file_close(_index.images.f);
                  _index.images.f = NULL;
                  _index.images.header = NULL;
               }
             else
               {
                  _index.images.count = (size - sizeof(Shared_Array_Header))
                        / sizeof(Image_Data);
                  _index.images.entries.idata =
                        (Image_Data *) &(_index.images.header[1]);
                  DBG("Mapped images shared index '%s' at %p: %u entries max",
                      _index.images.path, _index.images.header,
                      _index.images.header->count);
                  ret++;
               }
          }
     }

   if (_index.fonts.path[0])
     ERR("Not implemented yet: fonts shared index");

   return ret;
}

// FIXME: Copy & paste from evas_cserve2_cache.c
static int
_shm_object_id_cmp_cb(const void *data1, const void *data2)
{
   const Shm_Object *obj;
   unsigned int key;

   if (data1 == data2) return 0;
   if (!data1) return 1;
   if (!data2) return -1;

   obj = data1;
   key = *((unsigned int *) data2);
   if (obj->id == key) return 0;
   if (obj->id < key)
     return -1;
   else
     return +1;
}

// FIXME: (almost) copy & paste from evas_cserve2_cache.c
static const char *
_shared_string_get(int id)
{
   const char *ret;
   const Index_Entry *ie = NULL;
   int k;

   if (id <= 0) return NULL;
   if (!_index.strings.data) return NULL;

   // Binary search
   if (_index.strings.index_header->sortedidx > 0)
     {
        int low = 0;
        int high = _index.strings.index_header->sortedidx;
        int prev = -1;
        int r;
        k = high / 2;
        while (prev != k)
          {
             ie = &(_index.strings.indexes[k]);
             r = _shm_object_id_cmp_cb(ie, &id);
             if (!r)
               goto found;
             else if (r > 0)
               high = k;
             else
               low = k;
             prev = k;
             k = low + (high - low) / 2;
          }
     }

   // Linear search O(n)
   k = _index.strings.index_header->sortedidx;
   for (; k < _index.strings.index_header->emptyidx; k++)
     {
        ie = &(_index.strings.indexes[k]);
        if (!_shm_object_id_cmp_cb(ie, &id))
          goto found;
     }

   return NULL;

found:
   if (!ie) return NULL;
   if (!ie->refcount) return NULL;
   if (ie->length + ie->offset > (int) _index.strings.entries_size)
     return NULL;

   ret = _index.strings.data + ie->offset;
   return ret;
}

static inline Eina_Bool
_shared_image_entry_file_data_match(Image_Entry *ie, const File_Data *fd)
{
   const char *path, *key, *loader;

   if (!fd || !ie) return EINA_FALSE;
   if (!ie->file && !ie->key)
     return EINA_FALSE;

   path = _shared_string_get(fd->path);
   key = _shared_string_get(fd->key);
   loader = _shared_string_get(fd->loader_data);

   if (!path && ie->file)
     return EINA_FALSE;
   if (ie->file && strcmp(path, ie->file))
     return EINA_FALSE;

   if (!key && ie->key)
     return EINA_FALSE;
   if (ie->key && strcmp(key, ie->key))
     return EINA_FALSE;

   /*
   if (!loader && ie->loader_data)
     return EINA_FALSE;
   if (strcmp(loader, ie->loader_data))
     return EINA_FALSE;
   */

   // Check w,h ?
   // Not sure which load opts should be checked here
   DBG("Found a match for %s:%s", ie->file, ie->key);
   return EINA_TRUE;
}

static const File_Data *
_shared_image_entry_file_data_find(Image_Entry *ie)
{
   const File_Data *fdata = NULL;
   File_Entry *fe;
   int k;

   DBG("Trying to find if image '%s:%s' is already opened by cserve2",
       ie->file, ie->key);

   if (!_index.files.entries.fdata)
     return NULL;

#warning FIXME Use safe count
   for (k = 0; k < _index.files.header->count; k++)
     {
        const File_Data *fd = &(_index.files.entries.fdata[k]);
        if (!fd->id) return NULL;
        if (!fd->refcount) continue;

        if (_shared_image_entry_file_data_match(ie, fd))
          {
             fdata = fd;
             break;
          }
     }

   DBG("Found file data for %s:%s: %d", ie->file, ie->key, fdata->id);
   fe = ie->data1;
   fe->server_file_id = fdata->id;
   return fdata;
}

static inline Eina_Bool
_shared_image_entry_image_data_match(Image_Entry *ie, const Image_Data *id)
{
   int cmp;
   cmp = memcmp(&ie->load_opts, &id->opts, sizeof(ie->load_opts));
   if (!cmp)
     {
        DBG("Found loaded image entry at %d", id->id);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

#define SHARED_INDEX_CHECK(si, typ) \
   if (!_shared_index_remap_check(&(si), sizeof(typ))) return NULL

static Eina_Bool
_shared_index_remap_check(Shared_Index *si, int elemsize)
{
   size_t filesize;
   Eina_Bool refresh = EINA_FALSE;

   // Note: all checks are unlikely to be true.

   if (!si || elemsize <= 0) return EINA_FALSE;
   if (si->generation_id != _index.generation_id)
     {
        DBG("Generation ID changed.");
        if (si->f && si->data)
          {
             if (eina_file_refresh(si->f))
               {
                  DBG("Remapping index.");
                  eina_file_map_free(si->f, si->data);
                  si->data = NULL;
               }
          }
        else if (si->f)
          {
             eina_file_close(si->f);
             si->f = NULL;
          }
        si->generation_id = _index.generation_id;
     }
   if (!si->f)
     {
        si->data = NULL; // If that was not NULL, the address was invalid.
        si->f = eina_file_open(si->path, EINA_TRUE);
        if (!si->f)
          {
             ERR("Could not open index '%s'", si->path);
             return EINA_FALSE;
          }
     }
   if (!si->data)
     {
        filesize = eina_file_size_get(si->f);
        if (filesize < sizeof(Shared_Array_Header))
          {
             ERR("Index is invalid. Got file size %d", (int) filesize);
             eina_file_close(si->f);
             si->f = NULL;
             return EINA_FALSE;
          }
        si->data = eina_file_map_all(si->f, EINA_FILE_RANDOM);
        if (!si->data)
          {
             ERR("Could not mmap index '%s'", si->path);
             eina_file_close(si->f);
             si->f = NULL;
             return EINA_FALSE;
          }
        refresh = EINA_TRUE;
     }

   if (elemsize != si->header->elemsize)
     {
        ERR("Index is invalid. Expected element size %d, got %d.",
            elemsize, si->header->elemsize);
        return EINA_FALSE;
     }

   if (si->count != si->header->count)
     {
        // generation_id should have been incremented. Maybe we are hitting
        // a race condition here, when cserve2 grows an index.
        WRN("Reported index count differs from known count: %d vs %d",
            si->header->count, si->count);
        filesize = eina_file_size_get(si->f);
        si->count = (filesize - sizeof(Shared_Array_Header)) / elemsize;
        if (si->count > si->header->count)
          {
             WRN("Index reports %d elements, but file can contain only %d",
                 si->header->count, si->count);
             si->count = si->header->count;
          }
        refresh = EINA_TRUE;
     }

   if (!si->entries_by_hkey)
     refresh = EINA_TRUE;

   if (refresh)
     {
        if (si->entries_by_hkey) eina_hash_free_buckets(si->entries_by_hkey);
        else si->entries_by_hkey = eina_hash_string_small_new(NULL);
        si->last_entry_in_hash = 0;
     }

   return EINA_TRUE;
}

static const Image_Data *
_shared_image_entry_image_data_find(Image_Entry *ie)
{
   const Image_Data *idata = NULL;
   File_Entry *fe;
   unsigned int file_id = 0;
   int k;

   DBG("Trying to find if image '%s:%s' is already loaded by cserve2",
       ie->file, ie->key);

   if (!_index.images.entries.idata || !_index.images.count)
     return NULL;

   fe = ie->data1;
   if (fe && fe->server_file_id)
     file_id = fe->server_file_id;
   else
     {
        const File_Data *fdata = _shared_image_entry_file_data_find(ie);
        if (!fdata)
          {
             ERR("File is not opened by cserve2");
             return NULL;
          }
        file_id = fdata->id;
     }

   SHARED_INDEX_CHECK(_index.images, Image_Data);

   DBG("Looking for loaded image with file id %d", file_id);
   for (k = 0; k < _index.images.count; k++)
     {
        const Image_Data *id = &(_index.images.entries.idata[k]);
        if (!id->id) return NULL;
        if (!id->refcount) continue;
        if (id->file_id != file_id) continue;

        if (_shared_image_entry_image_data_match(ie, id))
          {
             idata = id;
             break;
          }
     }

   if (!idata)
     return NULL;

   if (!_shared_string_get(idata->shm_id))
     {
        ERR("Found image but it is not loaded yet: %d (doload %d shm %s)",
            idata->id, idata->doload, _shared_string_get(idata->shm_id));
        return NULL;
     }

   DBG("Found image, loaded, in shm %s", _shared_string_get(idata->shm_id));
   return idata;
}


#endif
