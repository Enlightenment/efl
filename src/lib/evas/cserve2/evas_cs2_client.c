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
     WRN("Message rid (%d) differs from expected rid (open_rid: %d)", answer->rid, ie->open_rid);
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
     WRN("Message rid (%d) differs from expected rid (load_rid: %d)", answer->rid, ie->load_rid);
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
   if (!ie)
     return CSERVE2_GENERIC;

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
   if (!ie)
     return CSERVE2_GENERIC;

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

#endif
