#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <Eina.h>

#include "evas_cs2.h"
#include "evas_cs2_private.h"
#include "evas_cs2_utils.h"

#ifdef EVAS_CSERVE2

#define TIMEOUT 1000
#define USE_SHARED_INDEX 1
#define SHARED_INDEX_ADD_TO_HASH 1
#define HKEY_LOAD_OPTS_STR_LEN 215
#define SPECIAL_RID_INDEX_LIST ((unsigned int) 0xFFFFFF42)

typedef Eina_Bool (*Op_Callback)(void *data, const void *msg, int size);

static const Evas_Image_Load_Opts empty_lo = {
  { 0, 0, 0, 0 },
  {
    0, 0, 0, 0,
    0, 0,
    0,
    0
  },
  0.0,
  0, 0,
  0,
  0,

  EINA_FALSE
};

struct _File_Entry {
   unsigned int file_id;
   unsigned int server_file_id;
   EINA_REFCOUNT;
   Eina_Stringshare *hkey;
};

struct _Client_Request {
   Msg_Base *msg;
   int msg_size;
   Op_Callback cb;
   void *data;
};

typedef struct _File_Entry File_Entry;
typedef struct _Client_Request Client_Request;

static int cserve2_init = 0;
static int socketfd = -1;
static int sr_size = 0;
static int sr_got = 0;
static char *sr_buf = NULL;

static unsigned int _rid_count = 0;
static unsigned int _file_id = 0;
static unsigned int _data_id = 0;

static Eina_List *_requests = NULL;
static Eina_Hash *_file_entries = NULL;

// Shared index table
static Index_Table _index;
static const char *_shared_string_get(int id);
static const char *_shared_string_safe_get(int id); // Do not allow remap during search
static Eina_Bool _string_index_refresh(void);
static int _server_index_list_set(Msg_Base *data, int size);
static const File_Data *_shared_file_data_get_by_id(unsigned int id);
static const Shm_Object *_shared_index_item_get_by_id(Shared_Index *si, int elemsize, unsigned int id, Eina_Bool safe);
static const File_Data *_shared_image_entry_file_data_find(Image_Entry *ie);
static const Image_Data *_shared_image_entry_image_data_find(Image_Entry *ie);
static const Font_Data *_shared_font_entry_data_find(Font_Entry *fe);
static Eina_Bool _shared_index_remap_check(Shared_Index *si, int elemsize);

static Eina_Bool _server_dispatch_until(unsigned int rid);
unsigned int _image_load_server_send(Image_Entry *ie);
static unsigned int _image_open_server_send(Image_Entry *ie);
static unsigned int _glyph_request_server_send(Font_Entry *fe, Font_Hint_Flags hints, Eina_Bool used);

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
_file_entry_free(void *data)
{
   File_Entry *fentry = data;
   if (!fentry) return;
   eina_stringshare_del(fentry->hkey);
   free(fentry);
}

static void
_socket_path_set(char *path)
{
   char *env;
   char buf[UNIX_PATH_MAX];

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        env = getenv("EVAS_CSERVE2_SOCKET");
        if (env && env[0])
          {
             eina_strlcpy(path, env, UNIX_PATH_MAX);
             return;
          }
     }

   snprintf(buf, sizeof(buf), "/tmp/.evas-cserve2-%x.socket", (int)getuid());
   /* FIXME: check we can actually create this socket */
   strcpy(path, buf);
#if 0   
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
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
             close(s);
             return EINA_FALSE;
          }
        ERR("cserve2 connect failed: [%d] %s. Retrying...", errno, strerror(errno));

        errno = 0;
        usleep(10000);
        if (errno == EINTR)
          {
             WRN("received interruption while trying to connect to cserve2!");
             close(s);
             return EINA_FALSE;
          }

        /* FIXME: Here we should identify the error, maybe signal the daemon manager
         * that we need cserve2 to [re]start or just quit and return false.
         * There probably should be a timeout of some sort also...
         * -- jpeg
         */
     }

#ifdef HAVE_FCNTL
   if (fcntl(s, F_SETFL, O_NONBLOCK) < 0) ERR("can't set non-blocking fd");
#endif

   socketfd = s;
   sr_size = 0;

   DBG("connected to cserve2 server.");
   return EINA_TRUE;
}

static void
_server_disconnect(void)
{
   if (socketfd != -1)
     close(socketfd);
   socketfd = -1;
   sr_size = 0;
}

static void
_request_answer_add(Msg_Base *msg, int size, Op_Callback cb, void *data)
{
   Client_Request *cr = calloc(1, sizeof(*cr));

   cr->msg = msg;
   cr->msg_size = size;
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
             ERR("send() failed with error %d %m", errno);
             return EINA_FALSE;
          }
        sent += ret;
     }

   return EINA_TRUE;
}

static Eina_Bool
_request_resend(unsigned int rid)
{
   Eina_List *l;
   Client_Request *cr;
   Eina_Bool found = EINA_FALSE;

   DBG("Re-sending %d requests...", eina_list_count(_requests));
   EINA_LIST_FOREACH(_requests, l, cr)
     {
        if (rid)
          {
             if (cr->msg->rid != rid)
               continue;
             found = EINA_TRUE;
          }

        DBG("Sending request %d again: type %d", cr->msg->rid, cr->msg->type);

        if (!_server_safe_send(socketfd, &cr->msg_size, sizeof(cr->msg_size)))
          return EINA_FALSE;
        if (!_server_safe_send(socketfd, cr->msg, cr->msg_size))
          return EINA_FALSE;

        if (found) break;
     }

   if (rid)
     return found;

   return EINA_TRUE;
}

static void
_shared_index_close(Shared_Index *si)
{
   if (!si) return;

   if (si->f)
     {
        if (si->data)
          eina_file_map_free(si->f, si->data);
        eina_file_close(si->f);
     }
   if (si->entries_by_hkey)
     eina_hash_free(si->entries_by_hkey);
   memset(si, 0, sizeof(Shared_Index));
}

static void
_shared_index_close_all()
{
   DBG("Closing all index files");
   if (_index.strings_entries.f)
     {
        if (_index.strings_entries.data)
          eina_file_map_free(_index.strings_entries.f, _index.strings_entries.data);
        eina_file_close(_index.strings_entries.f);
        _index.strings_entries.data = NULL;
        _index.strings_entries.f = NULL;
     }
   _shared_index_close(&_index.strings_index);
   _shared_index_close(&_index.files);
   _shared_index_close(&_index.images);
   _shared_index_close(&_index.fonts);
   _index.generation_id = 0;
}

static Eina_Bool
_server_reconnect()
{
   _shared_index_close_all();

   errno = 0;
   _server_disconnect();
   if (!_server_connect())
     goto on_error;

   if (!_server_dispatch_until(SPECIAL_RID_INDEX_LIST))
     goto on_error;

   /* NOTE: (TODO?)
    * Either we reopen all images & fonts now
    * Or we wait until new data is required again to request cserve2 to load
    * it for us. Not sure which approch is the best now.
    * So, for the moment, we'll just wait until the client needs new data.
    */

   if (!_request_resend(0))
     goto on_error;

   INF("Successfully reconnected to cserve2");
   return EINA_TRUE;

on_error:
   ERR("Unable to reconnect to server: %d %m", errno);
   return EINA_FALSE;
}

static Eina_Bool
_request_answer_required(int type, Eina_Bool *valid)
{
   switch (type)
     {
      case CSERVE2_OPEN:
      case CSERVE2_LOAD:
      case CSERVE2_PRELOAD:
      case CSERVE2_FONT_LOAD:
      case CSERVE2_FONT_GLYPHS_LOAD:
        if (valid) *valid = EINA_TRUE;
        return EINA_TRUE;
      case CSERVE2_CLOSE:
      case CSERVE2_UNLOAD:
      case CSERVE2_FONT_UNLOAD:
      case CSERVE2_FONT_GLYPHS_USED:
        if (valid) *valid = EINA_TRUE;
        return EINA_FALSE;
      default:
        ERR("Invalid message type %d", type);
        if (valid) *valid = EINA_FALSE;
        return EINA_FALSE;
     }
}

static Eina_Bool
_server_send(void *buf, int size, Op_Callback cb, void *data)
{
   Msg_Base *msg = buf;
   int type = msg->type;
   Eina_Bool valid = EINA_TRUE;

   if (!_server_safe_send(socketfd, &size, sizeof(size)))
     {
        ERR("Couldn't send message size to server.");
        goto on_error;
     }
   if (!_server_safe_send(socketfd, buf, size))
     {
        ERR("Couldn't send message body to server.");
        goto on_error;
     }

   if (_request_answer_required(type, &valid))
     _request_answer_add(msg, size, cb, data);
   else
     free(msg);

   return valid;

on_error:
   if (!_request_answer_required(type, NULL))
     {
        free(buf);
        return EINA_FALSE;
     }

   ERR("Socket error: %d %m", errno);
   switch (errno)
     {
      case EPIPE:
      case EBADF:
        WRN("Trying to reconnect to server...");
        if (!_server_reconnect())
          {
             free(buf);
             return EINA_FALSE;
          }
        return _server_send(buf, size, cb, data);
      default:
        ERR("Can not recover from this error!");
        free(buf);
        return EINA_FALSE;
     }
}

static void *
_server_read(int *size)
{
   int n;
   void *ret;

   if (socketfd < 0)
     return NULL;

   if (sr_size)
     goto get_data;

   n = recv(socketfd, &sr_size, sizeof(sr_size), 0);
   if (n < 0)
     return NULL;
   if (n == 0)
     {
        DBG("Socket connection closed by server.");
        _server_disconnect();
        return NULL;
     }

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

   _file_entries = eina_hash_string_superfast_new(EINA_FREE_CB(_file_entry_free));
   return cserve2_init;
}

int
evas_cserve2_shutdown(void)
{
   const char zeros[sizeof(Msg_Index_List)] = {0};
   Msg_Index_List *empty = (Msg_Index_List *) zeros;

   if (cserve2_init <= 0)
     {
        CRI("cserve2 is already shutdown");
        return -1;
     }

   if ((--cserve2_init) > 0)
     return cserve2_init;

   DBG("Disconnecting from cserve2.");
   empty->base.type = CSERVE2_INDEX_LIST;
   _server_index_list_set((Msg_Base *) empty, sizeof(Msg_Index_List));
   _server_disconnect();

   eina_hash_free(_file_entries);
   _file_entries = NULL;

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
   Eina_Bool found = EINA_FALSE;

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
        return SPECIAL_RID_INDEX_LIST;
      default:
        break;
     }

   // Normal client to server requests
   EINA_LIST_FOREACH_SAFE(_requests, l, l_next, cr)
     {
        Eina_Bool remove = EINA_TRUE;

        if (cr->msg->rid != msg->rid) // dispatch this answer
          continue;

        found = EINA_TRUE;
        if (cr->cb)
          remove = cr->cb(cr->data, msg, size);
        if (remove)
          {
             _requests = eina_list_remove_list(_requests, l);
             free(cr->msg);
             free(cr);
          }
     }

   rid = msg->rid;
   if (!found)
     {
        if (msg->type == CSERVE2_ERROR)
          {
             Msg_Error *error = (Msg_Error *) msg;
             ERR("Cserve2 sent error %d for rid %d", error->error, rid);
          }
        else WRN("Got unexpected response %d for request %d", msg->type, rid);
     }

   free(msg);
   return rid;
}

static Eina_Bool
_server_dispatch_until(unsigned int rid)
{
   Eina_Bool failed;
   unsigned int rrid;
   sigset_t sigmask;

   // We want to block some signals from interrupting pselect().
   // If the kernel implements TIF_RESTORE_SIGMASK, the
   // signal handlers should be called right after pselect
   // SIGCHLD: apps can have children that just terminated
   sigprocmask(0, NULL, &sigmask);
   sigaddset(&sigmask, SIGCHLD);

   while (1)
     {
        rrid = _server_dispatch(&failed);
        if (rrid == rid) break;
#if TIMEOUT
        else if (failed)
          {
             fd_set rfds;
             struct timespec ts;
             int sel;

             if (socketfd == -1)
               {
                  DBG("Reconnecting to server...");
                  if (!_server_connect())
                    {
                       ERR("Could not reconnect to cserve2!");
                       return EINA_FALSE;
                    }
                  if (socketfd == -1) return EINA_FALSE;
               }

             //DBG("Waiting for request %d...", rid);
             FD_ZERO(&rfds);
             FD_SET(socketfd, &rfds);
             ts.tv_sec = TIMEOUT / 1000;
             ts.tv_nsec = (TIMEOUT % 1000) * 1000000;
             sel = pselect(socketfd + 1, &rfds, NULL, NULL, &ts, &sigmask);
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
                       /* FIXME: Actually we might want to cancel our request
                        * ONLY when we received a SIGINT, but at this point
                        * there is no way we can know which signal we got.
                        * So we assume SIGINT and abandon this request.
                        */
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
#endif
     }
   return EINA_TRUE;
}

static Eina_Bool
_image_opened_cb(void *data, const void *msg_received, int size)
{
   const Msg_Base *answer = msg_received;
   const Msg_Opened *msg = msg_received;
   Image_Entry *ie = data;

   /* FIXME: Maybe we could have more asynchronous loading in the server side
    * and so we would have to check that open_rid is equal to answer->rid.
    * -- jpeg
    */
   DBG("Received OPENED for RID: %d [open_rid: %d]", answer->rid, ie->open_rid);

   if (ie->server_id && !ie->open_rid)
     return EINA_TRUE;

   if (answer->rid != ie->open_rid)
     {
        WRN("Message rid (%d) differs from expected rid (open_rid: %d)", answer->rid, ie->open_rid);
        return EINA_TRUE;
     }
   ie->open_rid = 0;

   if ((answer->type != CSERVE2_OPENED) || (size < (int) sizeof(*msg)))
     {
        File_Entry *fentry = ie->data1;
        if (answer->type == CSERVE2_ERROR)
          {
             const Msg_Error *msg_error = msg_received;
             ERR("Couldn't open image: '%s':'%s'; error: %d",
                 ie->file, ie->key, msg_error->error);
          }
        else
          ERR("Invalid message type received: %d (%s)", answer->type, __FUNCTION__);
        EINA_REFCOUNT_UNREF(fentry)
          {
             eina_hash_del(_file_entries, fentry->hkey, fentry);
             ie->data1 = NULL;
          }
        return EINA_TRUE;
     }

   ie->w = msg->image.w;
   ie->h = msg->image.h;
   ie->flags.alpha = msg->image.alpha;
   ie->animated.loop_hint = msg->image.loop_hint;
   ie->animated.loop_count = msg->image.loop_count;
   ie->animated.frame_count = msg->image.frame_count;
   ie->animated.animated = msg->image.animated;

   return EINA_TRUE;
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

static Eina_Bool
_image_loaded_cb(void *data, const void *msg_received, int size)
{
   const Msg_Base *answer = msg_received;
   const Msg_Loaded *msg = msg_received;
   Image_Entry *ie = data;

   DBG("Received LOADED for RID: %d [load_rid: %d]", answer->rid, ie->load_rid);

   if (!ie->load_rid)
     return EINA_TRUE;

   if (answer->rid != ie->load_rid)
     {
        WRN("Message rid (%d) differs from expected rid (load_rid: %d)", answer->rid, ie->load_rid);
        return EINA_TRUE;
     }
   ie->load_rid = 0;

   if (!ie->data2)
     {
        ERR("No data2 for loaded file");
        return EINA_TRUE;
     }

   if (answer->type != CSERVE2_LOADED)
     {
        if (answer->type == CSERVE2_ERROR)
          {
             const Msg_Error *msg_error = msg_received;
             ERR("Couldn't load image: '%s':'%s'; error: %d",
                 ie->file, ie->key, msg_error->error);

             if (msg_error->error == CSERVE2_NOT_LOADED)
               {
                  DBG("Trying to reopen the image");
                  ie->open_rid = _image_open_server_send(ie);
                  if (_server_dispatch_until(ie->open_rid))
                    if (_request_resend(answer->rid))
                      return EINA_TRUE;
               }
          }
        else
          ERR("Invalid message type received: %d (%s)", answer->type, __FUNCTION__);
        free(ie->data2);
        ie->data2 = NULL;
        return EINA_TRUE;
     }

   _loaded_handle(ie, msg, size);
   return EINA_TRUE;
}

static Eina_Bool
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
        return EINA_TRUE;
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
        return EINA_TRUE;
     }

   _loaded_handle(ie, msg_received, size);

   dentry = ie->data2;
   if (dentry && (dentry->preloaded_cb))
     {
        dentry->preloaded_cb(data, EINA_TRUE);
        dentry->preloaded_cb = NULL;
     }

   return EINA_TRUE;
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

// NOTE: Copy & paste from evas_cserve2_cache.c (TODO: Merge into common file)
static Eina_Bool
_evas_image_load_opts_empty(Evas_Image_Load_Opts *lo)
{
   if (!lo) return EINA_TRUE;

   return ((lo->scale_down_by == 0)
           && (lo->dpi == 0.0)
           && (lo->w == 0) && (lo->h == 0)
           && (lo->region.x == 0) && (lo->region.y == 0)
           && (lo->region.w == 0) && (lo->region.h == 0)
           && (lo->orientation == 0)
           && (lo->scale_load.src_x == 0) && (lo->scale_load.src_y == 0)
           && (lo->scale_load.src_w == 0) && (lo->scale_load.src_h == 0)
           && (lo->scale_load.dst_w == 0) && (lo->scale_load.dst_h == 0)
           && (lo->scale_load.scale_hint == 0)); // Skip smooth
}

// Valgrind complains about uninitialized memory, because the load_opts
// can be allocated on the stack, and each field is set independently,
// leaving empty spaces in the struct non initialized. So, compare fields
// one by one, don't take shortcuts like memcmp.

static Eina_Bool
_evas_image_load_opts_equal(const Evas_Image_Load_Opts *lo1,
                            const Evas_Image_Load_Opts *lo2)
{
   return ((lo1->scale_down_by == lo2->scale_down_by)
       && (lo1->dpi == lo2->dpi)
       && (lo1->w == lo2->w)
       && (lo1->h == lo2->h)
       && (lo1->region.x == lo2->region.x)
       && (lo1->region.y == lo2->region.y)
       && (lo1->region.w == lo2->region.w)
       && (lo1->region.h == lo2->region.h)
       && (lo1->scale_load.src_x == lo2->scale_load.src_x)
       && (lo1->scale_load.src_y == lo2->scale_load.src_y)
       && (lo1->scale_load.src_w == lo2->scale_load.src_w)
       && (lo1->scale_load.src_h == lo2->scale_load.src_h)
       && (lo1->scale_load.dst_w == lo2->scale_load.dst_w)
       && (lo1->scale_load.dst_h == lo2->scale_load.dst_h)
       && (lo1->scale_load.smooth == lo2->scale_load.smooth)
       && (lo1->scale_load.scale_hint == lo2->scale_load.scale_hint)
       && (lo1->orientation == lo2->orientation)
       && (lo1->degree == lo2->degree));
}

static void
_evas_image_load_opts_set(Evas_Image_Load_Opts *lo1,
                          const Evas_Image_Load_Opts *lo2)
{
   memset(lo1, 0, sizeof(Evas_Image_Load_Opts));
   lo1->scale_down_by = lo2->scale_down_by;
   lo1->dpi = lo2->dpi;
   lo1->w = lo2->w;
   lo1->h = lo2->h;
   lo1->region.x = lo2->region.x;
   lo1->region.y = lo2->region.y;
   lo1->region.w = lo2->region.w;
   lo1->region.h = lo2->region.h;
   lo1->scale_load.src_x = lo2->scale_load.src_x;
   lo1->scale_load.src_y = lo2->scale_load.src_y;
   lo1->scale_load.src_w = lo2->scale_load.src_w;
   lo1->scale_load.src_h = lo2->scale_load.src_h;
   lo1->scale_load.dst_w = lo2->scale_load.dst_w;
   lo1->scale_load.dst_h = lo2->scale_load.dst_h;
   lo1->scale_load.smooth = lo2->scale_load.smooth;
   lo1->scale_load.scale_hint = lo2->scale_load.scale_hint;
   lo1->orientation = lo2->orientation;
   lo1->degree = lo2->degree;
}

static void
_file_hkey_get(char *buf, size_t sz, const char *path, const char *key,
               Evas_Image_Load_Opts *lo)
{
   // Same as _evas_cache_image_loadopts_append() but not optimized :)
   if (lo && _evas_image_load_opts_empty(lo))
     lo = NULL;

   if (!lo)
     snprintf(buf, sz, "%s:%s", path, key);
   else
     {
        if (lo->orientation)
          {
             snprintf(buf, sz, "%s:%s//@/%d/%f/%dx%d/%d+%d.%dx%d",
                      path, key, lo->scale_down_by, lo->dpi, lo->w, lo->h,
                      lo->region.x, lo->region.y, lo->region.w, lo->region.h);
          }
        else
          {
             snprintf(buf, sz, "%s:%s//@/%d/%f/%dx%d/%d+%d.%dx%d/o",
                      path, key, lo->scale_down_by, lo->dpi, lo->w, lo->h,
                      lo->region.x, lo->region.y, lo->region.w, lo->region.h);
          }
     }
}

static unsigned int
_image_open_server_send(Image_Entry *ie)
{
   int flen, klen;
   int size, hkey_len;
   char *buf;
   char filebuf[PATH_MAX];
   char *hkey;
   Msg_Open msg_open;
   File_Entry *fentry;
   Data_Entry *dentry;
   const char *file, *key;
   Evas_Image_Load_Opts opts;
   Eina_Bool has_load_opts = EINA_FALSE;

   if (cserve2_init == 0)
     {
        ERR("Server not initialized.");
        return 0;
     }

   if (!_evas_image_load_opts_empty(&ie->load_opts))
     {
        _evas_image_load_opts_set(&opts, &ie->load_opts);
        has_load_opts = EINA_TRUE;
     }

   file = ie->file;
   key = ie->key;

   flen = _build_absolute_path(file, filebuf, sizeof(filebuf));
   if (!flen)
     {
        ERR("Could not find absolute path for %s", file);
        return 0;
     }
   flen++;

   if (!key) key = "";
   klen = strlen(key) + 1;

   hkey_len = flen + klen + 1024;
   hkey = alloca(hkey_len);
   _file_hkey_get(hkey, hkey_len, filebuf, key, (has_load_opts ? &opts : NULL));
   fentry = eina_hash_find(_file_entries, hkey);
   if (!fentry)
     {
        fentry = calloc(1, sizeof(*fentry));
        if (!fentry)
          return 0;

        fentry->file_id = ++_file_id;
        fentry->hkey = eina_stringshare_add(hkey);
        EINA_REFCOUNT_INIT(fentry);
        eina_hash_direct_add(_file_entries, fentry->hkey, fentry);
     }
   else
     EINA_REFCOUNT_REF(fentry);

   if (!ie->data2)
     {
        dentry = calloc(1, sizeof(*dentry));
        if (!dentry)
          {
             EINA_REFCOUNT_UNREF(fentry)
               eina_hash_del(_file_entries, fentry->hkey, fentry);
             return 0;
          }
        dentry->image_id = ++_data_id;
     }
   else dentry = ie->data2;

   memset(&msg_open, 0, sizeof(msg_open));
   msg_open.base.rid = _next_rid();
   msg_open.base.type = CSERVE2_OPEN;
   msg_open.file_id = fentry->file_id;
   msg_open.path_offset = 0;
   msg_open.key_offset = flen;
   msg_open.has_load_opts = has_load_opts;
   msg_open.image_id = dentry->image_id;

   size = sizeof(msg_open) + flen + klen;
   if (has_load_opts)
     size += sizeof(opts);
   buf = malloc(size);
   if (!buf)
     {
        EINA_REFCOUNT_UNREF(fentry)
          eina_hash_del(_file_entries, fentry->hkey, fentry);
        free(dentry);
        ie->data2 = NULL;
        return 0;
     }
   memcpy(buf, &msg_open, sizeof(msg_open));
   memcpy(buf + sizeof(msg_open), filebuf, flen);
   memcpy(buf + sizeof(msg_open) + flen, key, klen);
   if (has_load_opts)
     memcpy(buf + sizeof(msg_open) + flen + klen, &opts, sizeof(opts));

   if (!_server_send(buf, size, _image_opened_cb, ie))
     {
        ERR("Couldn't send message to server.");
        EINA_REFCOUNT_UNREF(fentry)
          eina_hash_del(_file_entries, fentry->hkey, fentry);
        free(dentry);
        ie->data2 = NULL;
        return 0;
     }

   ie->data1 = fentry;
   dentry->image_id = msg_open.image_id;
   ie->data2 = dentry;

   return msg_open.base.rid;
}

unsigned int
_image_load_server_send(Image_Entry *ie)
{
   Data_Entry *dentry;
   Msg_Load *msg;
   unsigned int rid;

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

   msg = calloc(1, sizeof(Msg_Load));
   msg->base.rid = _next_rid();
   msg->base.type = CSERVE2_LOAD;
   msg->image_id = dentry->image_id;

   rid = msg->base.rid;
   if (!_server_send(msg, sizeof(Msg_Load), _image_loaded_cb, ie))
     return 0;

   return rid;
}

unsigned int
_image_preload_server_send(Image_Entry *ie, void (*preloaded_cb)(void *im, Eina_Bool success))
{
   Data_Entry *dentry;
   Msg_Preload *msg;
   unsigned int rid;

   if (cserve2_init == 0)
     return 0;

   dentry = ie->data2;
   if (!dentry)
     {
        ERR("No data2 for opened file.");
        return 0;
     }
   dentry->preloaded_cb = preloaded_cb;

   msg = calloc(1, sizeof(Msg_Preload));
   msg->base.rid = _next_rid();
   msg->base.type = CSERVE2_PRELOAD;
   msg->image_id = dentry->image_id;

   rid = msg->base.rid;
   if (!_server_send(msg, sizeof(Msg_Preload), _image_preloaded_cb, ie))
     return 0;

   return rid;
}

unsigned int
_image_close_server_send(Image_Entry *ie)
{
   Msg_Close *msg;
   File_Entry *fentry;
   unsigned int rid;

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

   msg = calloc(1, sizeof(Msg_Close));
   msg->base.rid = _next_rid();
   msg->base.type = CSERVE2_CLOSE;
   msg->file_id = fentry->file_id;

   EINA_REFCOUNT_UNREF(fentry)
     eina_hash_del(_file_entries, fentry->hkey, fentry);
   ie->data1 = NULL;

   rid = msg->base.rid;
   if (!_server_send(msg, sizeof(Msg_Close), NULL, NULL))
     return 0;

   return rid;
}

unsigned int
_image_unload_server_send(Image_Entry *ie)
{
   Msg_Unload *msg;
   Data_Entry *dentry;
   unsigned int rid;

   if (cserve2_init == 0)
     return 0;

   if (!ie->data2)
     return 0;

   dentry = ie->data2;

   if (dentry->shm.data)
     eina_file_map_free(dentry->shm.f, dentry->shm.data);
   if (dentry->shm.f)
     eina_file_close(dentry->shm.f);

   msg = calloc(1, sizeof(Msg_Unload));
   msg->base.rid = _next_rid();
   msg->base.type = CSERVE2_UNLOAD;
   msg->image_id = dentry->image_id;

   free(dentry);
   ie->data2 = NULL;

   rid = msg->base.rid;
   if (!_server_send(msg, sizeof(Msg_Unload), NULL, NULL))
     return 0;

   return rid;
}

Eina_Bool
evas_cserve2_image_load(Image_Entry *ie)
{
   unsigned int rid;

   if (!ie)
     return EINA_FALSE;

   rid = _image_open_server_send(ie);
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
   Eina_Bool failed;
   unsigned int rrid, rid;

   if (!ie)
     return CSERVE2_GENERIC;

   if (!ie->open_rid)
     return CSERVE2_NONE;

   rid = ie->open_rid;
   rrid = _server_dispatch(&failed);
   if (rid == rrid)
     return CSERVE2_NONE;

#if USE_SHARED_INDEX
   fd = _shared_image_entry_file_data_find(ie);
   if (fd && fd->valid)
     {
        DBG("Bypassing socket wait (open_rid %d)", ie->open_rid);
        ie->w = fd->w;
        ie->h = fd->h;
        ie->flags.alpha = fd->alpha;
        ie->animated.loop_hint = fd->loop_hint;
        ie->animated.loop_count = fd->loop_count;
        ie->animated.frame_count = fd->frame_count;
        ie->animated.animated = fd->animated;
        ie->server_id = fd->id;
        ie->open_rid = 0;
     }
#endif

   if (ie->open_rid)
     {
        if (!_server_dispatch_until(ie->open_rid))
          return CSERVE2_GENERIC;
        if (!ie->data1)
          return CSERVE2_GENERIC;
     }

   if (ie->animated.animated)
     {
        WRN("This image is animated. cserve2 does not support animations");
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

#if USE_SHARED_INDEX
   idata = _shared_image_entry_image_data_find(ie);
   if (idata && idata->valid)
     {
        // FIXME: Ugly copy & paste from _loaded_handle
        Data_Entry *dentry = ie->data2;
        RGBA_Image *im = (RGBA_Image *)ie;
        const char *shmpath;

        shmpath = _shared_string_get(idata->shm_id);
        if (!shmpath) goto load_wait;
        DBG("Bypassing image load socket wait. Image found: %d in %s",
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
#endif

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

   char *hkey;
   int font_data_id;

   unsigned int rid; // open

   Glyph_Map *map;
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
   Shared_Index index;
   Shared_Buffer mempool;
   Eina_Clist glyphs;
   Eina_List *mempool_lru;
};

struct _CS_Glyph_Out
{
   RGBA_Font_Glyph_Out base;
   Eina_Clist map_entry;
   Eina_Clist used_list;
   unsigned int idx;
   unsigned int rid;
   Glyph_Map *map;
   Shared_Buffer *sb;
   unsigned int offset;
   unsigned int size;
   unsigned int hint;
   Eina_Bool used;
   int pending_ref;
   EINA_REFCOUNT;
};

static void
_glyphs_map_free(Glyph_Map *map)
{
   Shared_Buffer *mempool;

   if (!map) return;

   EINA_LIST_FREE(map->mempool_lru, mempool)
     {
        eina_file_map_free(mempool->f, mempool->data);
        eina_file_close(mempool->f);
        free(mempool);
     }
   eina_file_map_free(map->mempool.f, map->mempool.data);
   eina_file_close(map->mempool.f);
   eina_file_map_free(map->index.f, map->index.data);
   eina_file_close(map->index.f);
   eina_hash_free(map->index.entries_by_hkey);
   map->fe->map = NULL;
   free(map);
}

static void
_glyph_out_free(void *gl)
{
   CS_Glyph_Out *glout = gl;

   if (glout->used)
     eina_clist_remove(&glout->used_list);

   if (glout->map)
     {
        eina_clist_remove(&glout->map_entry);
        if (eina_clist_empty(&glout->map->glyphs))
          _glyphs_map_free(glout->map);
     }

   free(glout);
}

static void
_font_entry_free(Font_Entry *fe)
{
   int i;

   for (i = 0; i < 3; i++)
     if (fe->fash[i])
       fash_gl_free(fe->fash[i]);

   if (_index.fonts.entries_by_hkey)
     eina_hash_del_by_key(_index.fonts.entries_by_hkey, fe->hkey);

   free(fe->hkey);
   eina_stringshare_del(fe->source);
   eina_stringshare_del(fe->name);
   _glyphs_map_free(fe->map);
   free(fe);
}

static Eina_Bool
_font_loaded_cb(void *data, const void *msg, int size)
{
   const Msg_Base *m = msg;
   Font_Entry *fe = data;

   fe->rid = 0;

   if ((size < (int) sizeof(*m))
       || (m->type == CSERVE2_ERROR))
     fe->failed = EINA_TRUE;

   return EINA_TRUE;
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

   ret = msg->base.rid;
   if (!_server_send(msg, size, cb, fe))
     return 0;

   return ret;
}

Font_Entry *
evas_cserve2_font_load(const char *source, const char *name, int size, int dpi,
                       Font_Rend_Flags wanted_rend)
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

   eina_clist_init(&fe->glyphs_queue);
   eina_clist_init(&fe->glyphs_used);

   if (asprintf(&fe->hkey, "%s:%s/%u:%u:%u", fe->name, fe->source,
                fe->size, fe->dpi, (unsigned int) fe->wanted_rend) == -1)
     fe->hkey = NULL;

   return fe;
}

int
evas_cserve2_font_load_wait(Font_Entry *fe)
{
#if USE_SHARED_INDEX
   const Font_Data *fd;
   Eina_Bool failed;
   unsigned int rid, rrid;

   rid = fe->rid;
   rrid = _server_dispatch(&failed);
   if ((rid == rrid) && !fe->failed)
     return CSERVE2_NONE;

   fd = _shared_font_entry_data_find(fe);
   if (fd)
     {
        DBG("Bypassing socket wait (rid %d)", fe->rid);
        fe->failed = EINA_FALSE;
        fe->rid = 0;
        return CSERVE2_NONE;
     }
#endif

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

static Glyph_Map *
_glyph_map_open(Font_Entry *fe, const char *indexpath, const char *datapath)
{
   Glyph_Map *map;

   if (!fe) return NULL;
   if (fe->map) return fe->map;

   map = calloc(1, sizeof(*map));
   if (!map) return NULL;

   map->fe = fe;
   eina_clist_init(&map->glyphs);
   eina_strlcpy(map->mempool.path, datapath, SHARED_BUFFER_PATH_MAX);

   if (indexpath)
     {
        eina_strlcpy(map->index.path, indexpath, SHARED_BUFFER_PATH_MAX);
        map->index.generation_id = _index.generation_id;
        _shared_index_remap_check(&map->index, sizeof(Glyph_Data));
     }

   map->mempool.f = eina_file_open(map->mempool.path, EINA_TRUE);
   map->mempool.size = eina_file_size_get(map->mempool.f);
   map->mempool.data = eina_file_map_all(map->mempool.f, EINA_FILE_RANDOM);

   fe->map = map;
   return map;
}

static Eina_Bool
_glyph_map_remap_check(Glyph_Map *map, const char *idxpath, const char *datapath)
{
   Eina_Bool changed = EINA_FALSE;
   Shared_Buffer *oldbuf = NULL;
   int oldcount;

   // Note: Since the shm name contains cserve2's PID it should most likely
   // always change in case of crash/restart

   if (datapath && idxpath &&
       ((strncmp(datapath, map->mempool.path, SHARED_BUFFER_PATH_MAX) != 0) ||
        (strncmp(idxpath, map->index.path, SHARED_BUFFER_PATH_MAX) != 0)))
     {
        CS_Glyph_Out *gl, *cursor;

        WRN("Glyph pool has changed location.");

        // Force reopening index
        _shared_index_close(&map->index);
        eina_strlcpy(map->index.path, idxpath, SHARED_BUFFER_PATH_MAX);

        // Reopen mempool
        if (EINA_REFCOUNT_GET(&map->mempool) > 0)
          {
             oldbuf = calloc(1, sizeof(*oldbuf));
             oldbuf->f = map->mempool.f;
             oldbuf->data = map->mempool.data;
             oldbuf->size = map->mempool.size;
             eina_strlcpy(oldbuf->path, map->mempool.path, SHARED_BUFFER_PATH_MAX);
             map->mempool_lru = eina_list_append(map->mempool_lru, oldbuf);
          }
        else
          {
             eina_file_map_free(map->mempool.f, map->mempool.data);
             eina_file_close(map->mempool.f);
          }

        eina_strlcpy(map->mempool.path, datapath, SHARED_BUFFER_PATH_MAX);
        map->mempool.f = eina_file_open(datapath, EINA_TRUE);
        map->mempool.data = eina_file_map_all(map->mempool.f, EINA_FILE_RANDOM);
        map->mempool.size = eina_file_size_get(map->mempool.f);
        EINA_REFCOUNT_GET(&map->mempool) = 0;

        EINA_CLIST_FOR_EACH_ENTRY_SAFE(gl, cursor, &map->glyphs,
                                       CS_Glyph_Out, map_entry)
          {
             if (!EINA_REFCOUNT_GET(gl))
               {
                  gl->sb = NULL;
                  gl->base.bitmap.buffer = NULL;
               }
             else
               {
                  gl->sb = oldbuf;
                  if (gl->sb)
                    EINA_REFCOUNT_REF(gl->sb);
                  else
                    {
                       ERR("Glyph pool can not be remapped! (invalid refs)");
                       eina_clist_remove(&gl->map_entry);
                    }
               }
          }

        if (!eina_clist_count(&map->glyphs))
          return EINA_TRUE;

        map->index.generation_id = _index.generation_id;
        _shared_index_remap_check(&map->index, sizeof(Glyph_Data));
        return EINA_TRUE;
     }
   else if (eina_file_refresh(map->mempool.f) ||
            (eina_file_size_get(map->mempool.f) != (size_t) map->mempool.size))
     {
        CS_Glyph_Out *gl;

        WRN("Glyph pool has been resized.");

        // Queue old mempool into mempool_lru unless refcount == 0
        // We want to keep the old glyph bitmap data in memory because of
        // asynchronous rendering and also because remap could happen
        // after some glyphs have been requested but not all for the current
        // draw.

        if (EINA_REFCOUNT_GET(&map->mempool) > 0)
          {
             oldbuf = calloc(1, sizeof(*oldbuf));
             oldbuf->f = eina_file_dup(map->mempool.f);
             oldbuf->data = map->mempool.data;
             oldbuf->size = map->mempool.size;
             eina_strlcpy(oldbuf->path, map->mempool.path, SHARED_BUFFER_PATH_MAX);
             map->mempool_lru = eina_list_append(map->mempool_lru, oldbuf);
          }
        else
          eina_file_map_free(map->mempool.f, map->mempool.data);
        map->mempool.data = eina_file_map_all(map->mempool.f, EINA_FILE_RANDOM);
        map->mempool.size = eina_file_size_get(map->mempool.f);
        EINA_REFCOUNT_GET(&map->mempool) = 0;

        // Remap unused but loaded glyphs
        EINA_CLIST_FOR_EACH_ENTRY(gl, &map->glyphs,
                                  CS_Glyph_Out, map_entry)
          {
             if (!EINA_REFCOUNT_GET(gl))
               {
                  gl->sb = &map->mempool;
                  gl->base.bitmap.buffer = (unsigned char *) gl->sb->data + gl->offset;
               }
             else if (oldbuf)
               {
                  gl->sb = oldbuf;
                  EINA_REFCOUNT_REF(gl->sb);
               }
             else
               ERR("Invalid refcounting here.");
          }

        changed = EINA_TRUE;
     }

   map->index.generation_id = _index.generation_id;
   oldcount = map->index.count;
   _shared_index_remap_check(&map->index, sizeof(Glyph_Data));
   changed |= (oldcount != map->index.count);

   return changed;
}

#if USE_SHARED_INDEX
static int
_font_entry_glyph_map_rebuild_check(Font_Entry *fe, Font_Hint_Flags hints)
{
   Eina_Bool changed = EINA_FALSE;
   int cnt = 0;
   const char *idxpath = NULL, *datapath = NULL;

   _string_index_refresh();
   if (!fe->map)
     {
        const Font_Data *fd;

        fd = _shared_font_entry_data_find(fe);
        if (!fd) return -1;

        idxpath = _shared_string_safe_get(fd->glyph_index_shm);
        datapath = _shared_string_safe_get(fd->mempool_shm);
        if (!idxpath || !datapath) return -1;

        fe->map =_glyph_map_open(fe, idxpath, datapath);
        if (!fe->map) return -1;

        changed = EINA_TRUE;
     }

   changed |= _glyph_map_remap_check(fe->map, idxpath, datapath);
   if (changed && fe->map->index.data && fe->map->mempool.data)
     {
        CS_Glyph_Out *gl;
        const Glyph_Data *gd;
        int k, tot = 0;

        DBG("Rebuilding font hash based on shared index...");
        for (k = 0; k < fe->map->index.count; k++)
          {
             gd = &(fe->map->index.entries.gldata[k]);
             if (!gd->id) break;
             if (!gd->refcount) continue;
             if (gd->hint != hints) continue;

             tot++;
             gl = fash_gl_find(fe->fash[hints], gd->index);
             if (gl && gl->base.bitmap.buffer) continue;

             if (!gl)
               {
                  gl = calloc(1, sizeof(*gl));
                  gl->idx = gd->index;
                  eina_clist_element_init(&gl->map_entry);
                  eina_clist_element_init(&gl->used_list);
                  fash_gl_add(fe->fash[hints], gl->idx, gl);
               }
             gl->map = fe->map;
             gl->sb = &fe->map->mempool;
             gl->offset = gd->offset;
             gl->size = gd->size;
             gl->hint = gd->hint;
             gl->base.bitmap.rows = gd->rows;
             gl->base.bitmap.width = gd->width;
             gl->base.bitmap.pitch = gd->pitch;
             gl->base.bitmap.buffer = NULL;
             gl->base.rle = (unsigned char *)
               fe->map->mempool.data + gl->offset;
             gl->base.rle_size = gl->size;
             gl->base.bitmap.rle_alloc = EINA_FALSE;
             gl->idx = gd->index;
             gl->rid = 0;

             if (!eina_clist_element_is_linked(&gl->map_entry))
               eina_clist_add_head(&fe->map->glyphs, &gl->map_entry);
             fash_gl_add(fe->fash[hints], gd->index, gl);
             cnt++;
          }
        if (cnt)
          DBG("Added %d glyphs to the font hash (out of %d scanned)", cnt, tot);
     }

   return cnt;
}
#endif

static Eina_Bool
_glyph_request_cb(void *data, const void *msg, int size)
{
   const Msg_Font_Glyphs_Loaded *resp = msg;
   Glyph_Request_Data *grd = data;
   Font_Entry *fe = grd->fe;
   const char *buf;
   int i, nglyphs;
   int shmname_len, idxname_len;
   const char *shmname, *idxname;
   int pos;

   if (!fe || !fe->fash[grd->hints])
     goto end;

   if (resp->base.type == CSERVE2_ERROR)
     {
        const Msg_Error *err = msg;
        ERR("We got an error message when waiting for glyphs: %d", err->error);

        if (err->error == CSERVE2_NOT_LOADED)
          {
             // This can happen in case cserve2 restarted.
             DBG("Reloading the font: %s from %s", fe->name, fe->source);

             if (!(fe->rid = _font_load_server_send(fe, CSERVE2_FONT_LOAD)))
               {
                  ERR("Failed to send font load message");
                  free(data);
                  return EINA_TRUE;
               }

             if (fe->glyphs_queue_count)
               _glyph_request_server_send(fe, grd->hints, EINA_FALSE);

             if (fe->glyphs_used_count)
               _glyph_request_server_send(fe, grd->hints, EINA_TRUE);

             DBG("Resending glyph load message now...");
             if (!_request_resend(err->base.rid))
               {
                  free(data);
                  return EINA_TRUE;
               }
             // Keep this request in the list for now
             return EINA_FALSE;
          }
        free(data);
        return EINA_TRUE;
     }

   if (size <= (int) sizeof(*resp)) goto end;

   buf = (const char *)resp + sizeof(*resp);
   pos = buf - (const char*) resp;

   pos += sizeof(int);
   if (pos > size) goto end;

   memcpy(&shmname_len, buf, sizeof(int));
   buf += sizeof(int);

   pos += shmname_len + sizeof(int);
   if (pos > size) goto end;

   shmname = buf;
   buf += shmname_len;

   memcpy(&idxname_len, buf, sizeof(int));
   buf += sizeof(int);

   pos += idxname_len + sizeof(int);
   if (pos > size) goto end;

   idxname = buf;
   buf += idxname_len;

   memcpy(&nglyphs, buf, sizeof(int));
   buf += sizeof(int);

   if (fe->map)
     _glyph_map_remap_check(fe->map, idxname, shmname);

   if (!fe->map)
     fe->map = _glyph_map_open(fe, idxname, shmname);

   for (i = 0; i < nglyphs; i++)
     {
        string_t shm_id;
        unsigned int idx, offset, glsize, hints;
        int rows, width, pitch;
        CS_Glyph_Out *gl;

        pos = buf - (const char*) resp;
        pos += 8 * sizeof(int);
        if (pos > size) goto end;

        memcpy(&idx, buf, sizeof(int));
        buf += sizeof(int);
        memcpy(&shm_id, buf, sizeof(string_t));
        buf += sizeof(string_t);
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
        memcpy(&hints, buf, sizeof(int));
        buf += sizeof(int);
        if (hints != grd->hints)
          {
             WRN("Invalid hints received: %d vs %d. Skip.", hints, grd->hints);
             continue;
          }

        gl = fash_gl_find(fe->fash[hints], idx);
        if (!gl)
          {
             gl = calloc(1, sizeof(*gl));
             gl->idx = idx;
             eina_clist_element_init(&gl->map_entry);
             eina_clist_element_init(&gl->used_list);
             fash_gl_add(fe->fash[hints], idx, gl);
          }
        gl->map = fe->map;
        gl->sb = &fe->map->mempool;
        gl->offset = offset;
        gl->size = glsize;
        gl->hint = hints;
        gl->base.bitmap.rows = rows;
        gl->base.bitmap.width = width;
        gl->base.bitmap.pitch = pitch;
        gl->base.bitmap.buffer = NULL;
        gl->base.bitmap.rle_alloc = 0;
        gl->base.bitmap.no_free_glout = 1;
        gl->base.rle =
              (unsigned char *) gl->map->mempool.data + gl->offset;
        gl->base.rle_size = gl->size;
        gl->rid = 0;

        if (!eina_clist_element_is_linked(&gl->map_entry))
          eina_clist_add_head(&fe->map->glyphs, &gl->map_entry);
     }

   free(grd);
   return EINA_TRUE;

end:
   ERR("An unknown error occured when waiting for glyph data!");
   free(grd);
   return EINA_TRUE;
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
             eina_clist_remove(&gl->map_entry);
             gl->rid = msg->base.rid;
          }
        else
          {
             gl = EINA_CLIST_ENTRY(itr, CS_Glyph_Out, used_list);
             eina_clist_remove(&gl->used_list);
             gl->used = EINA_FALSE;
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

   ret = msg->base.rid;
   if (!_server_send(msg, size, cb, grd))
     {
        free(grd);
        return 0;
     }

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

   // Check map is still valid, otherwise we want to request the glyph again
   if (glyph && glyph->map && (&glyph->map->mempool != glyph->sb))
     {
        if (!EINA_REFCOUNT_GET(glyph))
          {
             fash_gl_del(fash, idx);
             glyph = NULL;
          }
     }

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
        // This can happen in case of cserve2 restart. (corner case)
        eina_clist_add_head(&fe->glyphs_used, &glyph->used_list);
        fe->glyphs_used_count++;
        glyph->used = EINA_TRUE;
     }

   /* FIXME crude way to manage a queue, but it will work for now */
   if (fe->glyphs_queue_count >= 50)
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

   // Glyph was requested but the bitmap data was not loaded yet.
   if (!glyph->map)
     return EINA_TRUE;

   // Glyph was stored in a dead buffer. Need to reload it :)
   if (&glyph->map->mempool != glyph->sb)
     {
        if (!EINA_REFCOUNT_GET(glyph))
          {
             fash_gl_del(fash, idx);
             return EINA_FALSE;
          }
        else
          {
             // Keep old buffer, it is still valid.
             return EINA_TRUE;
          }
     }

   if (glyph->used)
     return EINA_TRUE;

   eina_clist_add_head(&fe->glyphs_used, &glyph->used_list);
   fe->glyphs_used_count++;
   glyph->used = EINA_TRUE;

   return EINA_TRUE;
}

RGBA_Font_Glyph_Out *
evas_cserve2_font_glyph_bitmap_get(Font_Entry *fe, unsigned int idx,
                                   Font_Hint_Flags hints)
{
   Fash_Glyph2 *fash;
   CS_Glyph_Out *out;

   if (fe->failed)
     return NULL;

   /* quick hack, flush pending queue when we are asked for a bitmap */
   if (fe->glyphs_queue_count)
     _glyph_request_server_send(fe, hints, EINA_FALSE);

   if (fe->glyphs_used_count >= 50)
     _glyph_request_server_send(fe, hints, EINA_TRUE);

   fash = fe->fash[hints];
   if (!fash)
     {
        // this should not happen really, so let the user know he fucked up
        ERR("was called with a hinting value that was not requested!");
        return NULL;
     }

try_again:
   out = fash_gl_find(fash, idx);
   if (!out)
     {
        // again, if we are asking for a bitmap we were supposed to already
        // have requested the glyph, it must be there
        ERR("was called with a glyph index that was not requested!");
        return NULL;
     }

   if (out->map && (&out->map->mempool != out->sb))
     {
        // If the map is not valid, this is a good time to reload the glyph.
        if (!EINA_REFCOUNT_GET(out))
          {
             fash_gl_del(fash, idx);
             if (!evas_cserve2_font_glyph_request(fe, idx, hints))
               return NULL;

             DBG("Requesting again this glyph: %d", idx);
             goto try_again;
          }
        else // Can't reload now since the map is used.
          return &out->base;
     }


#if USE_SHARED_INDEX
   _font_entry_glyph_map_rebuild_check(fe, hints);
#endif

   if (out->rid)
     if (!_server_dispatch_until(out->rid))
       {
          ERR("failed to load the requested glyphs, resending request");
          if (!_request_resend(out->rid))
            return NULL;
       }

   // promote shm and font entry in lru or something
   return &(out->base);
}

void
evas_cserve2_font_glyph_ref(RGBA_Font_Glyph_Out *glyph, Eina_Bool incref)
{
   CS_Glyph_Out *glout;

   EINA_SAFETY_ON_FALSE_RETURN(evas_cserve2_use_get());

   // glout = (CS_Glyph_Out *) glyph;
   glout = (CS_Glyph_Out *) (((char *) glyph) - offsetof(CS_Glyph_Out, base));

   if (incref)
     {
        if (!glout->sb)
          {
             // This can happen when cserve2 restarted.
             glout->pending_ref++;
             return;
          }
        else if (!EINA_REFCOUNT_GET(glout))
          EINA_REFCOUNT_REF(glout->sb);
        if (glout->pending_ref)
          {
             EINA_REFCOUNT_GET(glout) += glout->pending_ref;
             glout->pending_ref = 0;
          }
        EINA_REFCOUNT_REF(glout);
        return;
     }

   EINA_SAFETY_ON_FALSE_RETURN
     ((EINA_REFCOUNT_GET(glout) + glout->pending_ref) > 0);

   if (!glout->sb)
     {
        glout->pending_ref--;
        return;
     }

   if (glout->pending_ref)
     {
        if (!EINA_REFCOUNT_GET(glout) && (glout->pending_ref > 0))
          EINA_REFCOUNT_REF(glout->sb);
        EINA_REFCOUNT_GET(glout) += glout->pending_ref;
        glout->pending_ref = 0;
     }

   EINA_REFCOUNT_UNREF(glout)
     {
        Eina_Bool noref = EINA_FALSE;

        EINA_SAFETY_ON_FALSE_RETURN(EINA_REFCOUNT_GET(glout->sb) > 0);
        EINA_REFCOUNT_UNREF(glout->sb)
          noref = EINA_TRUE;

        if (glout->sb != &glout->map->mempool)
          {
             if (noref)
               {
                  DBG("Glyph shared buffer reached refcount 0. "
                      "Unmapping %p from %s",
                      glout->sb->data, glout->sb->path);
                  glout->map->mempool_lru =
                    eina_list_remove(glout->map->mempool_lru, glout->sb);
                  eina_file_map_free(glout->sb->f, glout->sb->data);
                  eina_file_close(glout->sb->f);
                  free(glout->sb);
               }
             fash_gl_del(glout->map->fe->fash[glout->hint], glout->idx);
          }
     }
}



// Fast access to shared index tables

static Eina_Bool
_string_index_refresh(void)
{
   size_t sz;
   Eina_Bool ret = EINA_FALSE;

   if (!_index.strings_entries.data
       && _index.strings_entries.path[0]
       && _index.strings_index.path[0])
     {
        _index.strings_entries.f = eina_file_open(_index.strings_entries.path, EINA_TRUE);
        if (_index.strings_entries.f)
          _index.strings_entries.size = eina_file_size_get(_index.strings_entries.f);
        else
          _index.strings_entries.size = 0;
        if (_index.strings_entries.size > 0)
          _index.strings_entries.data = eina_file_map_all(_index.strings_entries.f, EINA_FILE_RANDOM);

        if (!_index.strings_entries.data)
          {
             ERR("Could not map strings entries from: '%s'", _index.strings_entries.path);
             if (_index.strings_entries.f) eina_file_close(_index.strings_entries.f);
             _index.strings_entries.f = NULL;
             _index.strings_entries.data = NULL;
             ret = EINA_FALSE;
          }
        else
          {
             DBG("Mapped string entries from %s", _index.strings_entries.path);
             ret = EINA_TRUE;
          }
     }

   if (_index.strings_entries.data &&
       (!_index.strings_index.data && _index.strings_index.path[0]))
     {
        _index.strings_index.f = eina_file_open(_index.strings_index.path, EINA_TRUE);
        if (_index.strings_index.f)
          sz = eina_file_size_get(_index.strings_index.f);
        else
          {
             sz = 0;
             _index.strings_index.data = NULL;
          }
        if (sz >= sizeof(Shared_Array_Header))
          _index.strings_index.data = eina_file_map_all(_index.strings_index.f, EINA_FILE_RANDOM);

        if (_index.strings_index.data)
          {
             DBG("Mapped string indexes from %s", _index.strings_index.path);
             sz = eina_file_size_get(_index.strings_index.f);
             _index.strings_index.count = (sz - sizeof(Shared_Array_Header)) / sizeof(Index_Entry);
             if (_index.strings_index.count > _index.strings_index.header->count)
               {
                  WRN("Detected larger index than advertised: %d > %d",
                      _index.strings_index.count, _index.strings_index.header->count);
                  _index.strings_index.count = _index.strings_index.header->count;
               }
             ret = EINA_TRUE;
          }
        else
          {
             ERR("Could not map string indexes from %s", _index.strings_index.path);
             if (_index.strings_index.f) eina_file_close(_index.strings_index.f);
             if (_index.strings_entries.f)
               {
                  if (_index.strings_entries.data)
                    eina_file_map_free(_index.strings_entries.f, _index.strings_entries.data);
                  eina_file_close(_index.strings_entries.f);
               }
             _index.strings_index.f = NULL;
             _index.strings_entries.f = NULL;
             _index.strings_entries.data = NULL;
             ret = EINA_FALSE;
          }
     }

   _shared_index_remap_check(&_index.strings_index, sizeof(Index_Entry));
   if (_index.strings_entries.data)
     {
        if (eina_file_refresh(_index.strings_entries.f)
            || (_index.strings_entries.size != (int) eina_file_size_get(_index.strings_entries.f)))
          {
             eina_file_map_free(_index.strings_entries.f, _index.strings_entries.data);
             _index.strings_entries.data = eina_file_map_all(_index.strings_entries.f, EINA_FILE_RANDOM);
             _index.strings_entries.size = eina_file_size_get(_index.strings_entries.f);
             return EINA_TRUE;
          }
     }

   return ret;
}

// Returns the number of correctly opened index arrays
static int
_server_index_list_set(Msg_Base *data, int size)
{
   Msg_Index_List *msg = (Msg_Index_List *) data;

   // TODO #1: Check populate rule.
   // TODO #2: Protect memory for read-only access.
   // TODO #3: Optimize file reopen/remap (esp. strings table)

   if (size != sizeof(*msg) || msg->base.type != CSERVE2_INDEX_LIST)
     {
        CRI("Invalid message! type: %d, size: %d (expected %d)",
            msg->base.type, size, (int) sizeof(*msg));
        return -1;
     }

   if (_index.generation_id == msg->generation_id)
     {
        WRN("New index generation_id is the same as before: %d",
            _index.generation_id);
     }

   _index.generation_id = msg->generation_id;

   // 1. Strings (indexes and entries)

   if (_index.strings_entries.data
       && strncmp(_index.strings_entries.path, msg->strings_entries_path,
                  SHARED_BUFFER_PATH_MAX) != 0)
     {
        DBG("Updating string entries shm to: '%s'", msg->strings_entries_path);
        eina_file_map_free(_index.strings_entries.f, _index.strings_entries.data);
        eina_file_close(_index.strings_entries.f);
        _index.strings_entries.f = NULL;
        _index.strings_entries.data = NULL;
     }

   if (_index.strings_index.data
       && strncmp(_index.strings_index.path, msg->strings_index_path,
                  SHARED_BUFFER_PATH_MAX) != 0)
     {
        DBG("Updating string indexes shm to: '%s'", msg->strings_index_path);
        _shared_index_close(&_index.strings_index);
     }

   eina_strlcpy(_index.strings_entries.path, msg->strings_entries_path, SHARED_BUFFER_PATH_MAX);
   eina_strlcpy(_index.strings_index.path, msg->strings_index_path, SHARED_BUFFER_PATH_MAX);
   _string_index_refresh();


   // 2. File indexes

   eina_strlcpy(_index.files.path, msg->files_index_path, SHARED_BUFFER_PATH_MAX);
   _shared_index_remap_check(&_index.files, sizeof(File_Data));


   // 3. Image indexes

   eina_strlcpy(_index.images.path, msg->images_index_path, SHARED_BUFFER_PATH_MAX);
   _shared_index_remap_check(&_index.images, sizeof(Image_Data));


   // 4. Font indexes

   eina_strlcpy(_index.fonts.path, msg->fonts_index_path, SHARED_BUFFER_PATH_MAX);
   _shared_index_remap_check(&_index.fonts, sizeof(Font_Data));

   return 0;
}

// FIXME: (almost) copy & paste from evas_cserve2_cache.c
static const char *
_shared_string_internal_get(int id, Eina_Bool safe)
{
   Index_Entry *ie;

   if (!_index.strings_entries.data)
     {
        CRI("Strings table is not valid: no data");
        return NULL;
     }

   ie = (Index_Entry *)
     _shared_index_item_get_by_id(&_index.strings_index, sizeof(*ie), id, safe);
   if (!ie) return NULL;
   if (ie->offset < 0) return NULL;
   if (!ie->refcount) return NULL;
   if (ie->offset + ie->length > _index.strings_entries.size)
     {
        if (safe) return NULL;
        if (eina_file_refresh(_index.strings_entries.f)
            || (_index.strings_entries.size != (int) eina_file_size_get(_index.strings_entries.f)))
          {
             DBG("String entries size has changed from %d to %d",
                 _index.strings_entries.size, (int) eina_file_size_get(_index.strings_entries.f));
             if (_string_index_refresh())
               return _shared_string_internal_get(id, EINA_FALSE);
          }
        return NULL;
     }

   return _index.strings_entries.data + ie->offset;
}

static const char *
_shared_string_safe_get(int id)
{
   return _shared_string_internal_get(id, EINA_TRUE);
}

static const char *
_shared_string_get(int id)
{
   return _shared_string_internal_get(id, EINA_FALSE);
}

#define SHARED_INDEX_CHECK(si, typ) \
   do { if (!_shared_index_remap_check(&(si), sizeof(typ))) { \
   CRI("Failed to remap index"); return NULL; } } while (0)

static const File_Data *
_shared_image_entry_file_data_find(Image_Entry *ie)
{
   const File_Data *fdata = NULL;
   File_Entry *fe;
   Eina_Bool add_to_hash = SHARED_INDEX_ADD_TO_HASH;
   char hkey[PATH_MAX];
   int k;

   DBG("Trying to find if image '%s:%s' is already opened by cserve2",
       ie->file, ie->key);

   SHARED_INDEX_CHECK(_index.files, File_Data);

   if (!_index.strings_index.header || !_index.strings_entries.data)
     return NULL;

   if (!_index.files.header || !_index.files.entries.filedata)
     return NULL;

   if (!ie->data1)
     return NULL;

   // Direct access
   fe = ie->data1;
   if (fe->server_file_id)
     {
        if ((fdata = _shared_file_data_get_by_id(fe->server_file_id)) != NULL)
          if (!fdata->changed)
            return fdata;
        fe->server_file_id = 0;
     }

   // Check hash
   _file_hkey_get(hkey, sizeof(hkey), ie->file, ie->key, &ie->load_opts);
   fdata = eina_hash_find(_index.files.entries_by_hkey, hkey);
   if (fdata)
     return fdata;

   // Scan shared index
   _string_index_refresh();
   for (k = _index.files.last_entry_in_hash;
        k < _index.files.count && k < _index.files.header->emptyidx; k++)
     {
        const char *file, *key;
        const File_Data *fd;
        char fd_hkey[PATH_MAX];
        Evas_Image_Load_Opts lo = empty_lo;

        fd = &(_index.files.entries.filedata[k]);
        if (!fd->id) break;
        if (!fd->refcount) continue;
        if (fd->changed) continue;

        key = _shared_string_safe_get(fd->key);
        file = _shared_string_safe_get(fd->path);
        if (!file)
          {
             ERR("Could not find filename for file %d: path id: %d",
                 fd->id, fd->path);
             add_to_hash = EINA_FALSE;
             continue;
          }

        lo.region.x = fd->lo.region.x;
        lo.region.y = fd->lo.region.y;
        lo.region.w = fd->lo.region.w;
        lo.region.h = fd->lo.region.h;
        lo.dpi = fd->lo.dpi;
        lo.w = fd->lo.w;
        lo.h = fd->lo.h;
        lo.scale_down_by = fd->lo.scale_down_by;
        lo.orientation = fd->lo.orientation;

        _file_hkey_get(fd_hkey, sizeof(fd_hkey), file, key, &lo);

        if (add_to_hash)
          {
             eina_hash_add(_index.files.entries_by_hkey, fd_hkey, fd);
             _index.files.last_entry_in_hash = k;
          }

        if (!strcmp(hkey, fd_hkey))
          return fd;
     }

   return NULL;
}

static const Shm_Object *
_shared_index_item_get_by_id(Shared_Index *si, int elemsize, unsigned int id,
                             Eina_Bool safe)
{
   const Shm_Object *obj;
   const char *base;
   int low = 0, high, start_high;
   int cur;

   if (!si || elemsize <= 0 || !id)
     return NULL;

   // FIXME: HACK (consider all arrays always sorted by id)
   high = si->header->emptyidx; // Should be si->header->sortedidx

   if (high > si->count)
     {
        if ((!safe) && eina_file_refresh(si->f))
          {
             WRN("Refreshing indexes.");
             _string_index_refresh();
             _shared_index_remap_check(si, elemsize);
             high = MIN(si->header->emptyidx, si->count);
          }
        else
          high = si->count;
     }

   base = si->data  + sizeof(Shared_Array_Header);

   // Direct access, works for non-repacked arrays
   if ((int) id < high)
     {
        obj = (Shm_Object *) (base + (elemsize * id));
        if (obj->id == id)
          return obj;
     }

   // Binary search
   start_high = high;
   while(high != low)
     {
        cur = low + ((high - low) / 2);
        obj = (Shm_Object *) (base + (elemsize * cur));
        if (obj->id == id)
          return obj;
        if (obj->id < id)
          low = cur + 1;
        else
          high = cur;
     }

   // Linear search
   for (cur = start_high; cur < si->count; cur++)
     {
        obj = (Shm_Object *) (base + (elemsize * cur));
        if (!obj->id)
          return NULL;
        if (obj->id == id)
          return obj;
     }

   return NULL;
}

static const File_Data *
_shared_file_data_get_by_id(unsigned int id)
{
   return (const File_Data *)
     _shared_index_item_get_by_id(&_index.files, sizeof(File_Data), id, EINA_FALSE);
}

static inline Eina_Bool
_shared_image_entry_image_data_match(Image_Entry *ie, const Image_Data *id)
{
   const Evas_Image_Load_Opts *lo1, *lo2;

   lo1 = &ie->load_opts;
   lo2 = &id->opts;

   if (_evas_image_load_opts_equal(lo1, lo2))
     {
        DBG("Found loaded image entry at %d", id->id);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_shared_index_remap_check(Shared_Index *si, int elemsize)
{
   size_t filesize;
   Eina_Bool refresh = EINA_FALSE;

   // Note: all checks are unlikely to be true.

   if (!si || elemsize <= 0)
     return EINA_FALSE;

   if (!si->path[0])
     {
        _shared_index_close(si);
        return EINA_FALSE;
     }

   if (si->generation_id != _index.generation_id)
     {
        DBG("Generation ID changed from %d to %d.",
            si->generation_id, _index.generation_id);
        if (si->f)
          {
             if (strncmp(si->path, eina_file_filename_get(si->f),
                         SHARED_BUFFER_PATH_MAX) != 0)
               {
                  DBG("Index file changed. Closing and reopening.");
                  eina_file_map_free(si->f, si->data);
                  eina_file_close(si->f);
                  si->f = NULL;
                  si->data = NULL;
               }
             else
               {
                  if (si->data)
                    {
                       if (eina_file_refresh(si->f))
                         {
                            DBG("Remapping index.");
                            eina_file_map_free(si->f, si->data);
                            si->data = NULL;
                         }
                    }
                  else
                    {
                       eina_file_close(si->f);
                       si->f = NULL;
                    }
               }
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
        si->count = (filesize - sizeof(Shared_Array_Header)) / elemsize;
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
        int oldcount;
        // generation_id should have been incremented. Maybe we are hitting
        // a race condition here, when cserve2 grows an index.
        WRN("Reported index count differs from known count: %d vs %d",
            si->header->count, si->count);
        oldcount = si->count;
        eina_file_refresh(si->f);
        filesize = eina_file_size_get(si->f);
        si->count = (filesize - sizeof(Shared_Array_Header)) / elemsize;
        if (si->count != oldcount)
          {
             DBG("Remapping current index");
             eina_file_map_free(si->f, si->data);
             si->data = eina_file_map_all(si->f, EINA_FILE_RANDOM);
             if (!si->data)
               {
                  ERR("Failed to remap index: %m");
                  return EINA_FALSE;
               }
          }
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
        si->entries.p = si->data + sizeof(Shared_Array_Header);
     }

   return EINA_TRUE;
}

static const Image_Data *
_shared_image_entry_image_data_find(Image_Entry *ie)
{
   const Image_Data *idata = NULL;
   const char *shmpath;
   File_Entry *fe;
   unsigned int file_id = 0;
   Eina_Bool add_to_hash = SHARED_INDEX_ADD_TO_HASH;
   int k;

   DBG("Trying to find if image '%s:%s' is already loaded by cserve2",
       ie->file, ie->key);

   if (!_index.strings_entries.data || !_index.strings_index.data)
     return NULL;

   if (!_index.images.entries.idata || !_index.images.count)
     return NULL;

   if (!ie->cache_key)
     {
        if (ie->flags.dirty)
          {
             DBG("Image is dirty (file changed?), not in cache.");
             return NULL;
          }
        CRI("Looking for an image in remote cache without hash key?");
        return NULL;
     }

   fe = ie->data1;
   if (fe && fe->server_file_id)
     file_id = fe->server_file_id;
   else
     {
        const File_Data *fdata = _shared_image_entry_file_data_find(ie);
        if (!fdata)
          {
             DBG("File is not opened by cserve2");
             return NULL;
          }
        file_id = fdata->id;
     }

   SHARED_INDEX_CHECK(_index.images, Image_Data);

   // Find in known entries hash. O(log n)
   DBG("Looking for %s in hash", ie->cache_key);
   idata = (const Image_Data *)
         eina_hash_find(_index.images.entries_by_hkey, ie->cache_key);
   if (idata)
     {
        DBG("Image found in shared index (by cache_key).");
        goto found;
     }

   // Linear search in non-hashed entries. O(n)
   DBG("Looking for loaded image with file id %d", file_id);
   _string_index_refresh();
   for (k = _index.images.last_entry_in_hash; k < _index.images.count; k++)
     {
        const char *file, *key;
        size_t keylen, filelen;
        const File_Data *fd;
        char *hkey;
        const Image_Data *id = &(_index.images.entries.idata[k]);

        if (!id->id) return NULL;
        if (!id->refcount) continue;

        if (add_to_hash)
          {
             fd = _shared_file_data_get_by_id(id->file_id);
             if (!fd)
               {
                  ERR("Did not find file data for %d", id->file_id);
                  add_to_hash = EINA_FALSE;
                  continue;
               }

             if (fd->changed || !fd->valid) continue;
             key = _shared_string_safe_get(fd->key);
             file = _shared_string_safe_get(fd->path);
             if (!file)
               {
                  ERR("No filename for file %d", fd->id);
                  add_to_hash = EINA_FALSE;
                  continue;
               }

             keylen = key ? strlen(key) : 0;
             filelen = strlen(file);

             hkey = alloca(filelen + keylen + HKEY_LOAD_OPTS_STR_LEN);
             evas_cache2_image_cache_key_create(hkey, file, filelen,
                                                key, keylen, &id->opts);
             eina_hash_add(_index.images.entries_by_hkey, hkey, id);
             _index.images.last_entry_in_hash = k;
          }

        if (id->file_id != file_id) continue;

        if (_shared_image_entry_image_data_match(ie, id))
          {
             idata = id;
             goto found;
          }
     }

   if (!idata)
     return NULL;

found:
   if (!idata->valid)
     {
        DBG("Found image but it is not ready yet: %d", idata->id);
        return NULL;
     }

   shmpath = _shared_string_safe_get(idata->shm_id);
   if (!shmpath)
     {
        ERR("Found image but it is not loaded yet: %d (doload %d)",
            idata->id, idata->doload);
        return NULL;
     }

   DBG("Found image, loaded, in shm %s", shmpath);
   return idata;
}

static const Font_Data *
_shared_font_entry_data_get_by_id(int id)
{
   return (Font_Data *)
     _shared_index_item_get_by_id(&_index.fonts, sizeof(Font_Data), id, EINA_FALSE);
}

static const Font_Data *
_shared_font_entry_data_find(Font_Entry *fe)
{
   const Font_Data *fd = NULL;
   Eina_Bool add_to_hash = SHARED_INDEX_ADD_TO_HASH;
   char hkey[PATH_MAX];
   int k;

   if (!_index.strings_entries.data || !_index.strings_index.data)
     return NULL;

   if (!fe || !fe->hkey)
     return NULL;

   if (fe->font_data_id)
     {
        fd = _shared_font_entry_data_get_by_id(fe->font_data_id);
        if (fd) return fd;
        fe->font_data_id = 0;
     }

   // Find in hash
   fd = eina_hash_find(_index.fonts.entries_by_hkey, fe->hkey);
   if (fd) return fd;

   // Find in shared index
   for (k = _index.fonts.last_entry_in_hash;
        k < _index.fonts.count && k < _index.fonts.header->emptyidx; k++)
     {
        const Font_Data *cur;
        const char *name, *source;

        cur = &(_index.fonts.entries.fontdata[k]);
        if (!cur->id) return NULL;
        if (!cur->refcount) continue;

        name = _shared_string_safe_get(cur->name);
        source = _shared_string_safe_get(cur->file);
        snprintf(hkey, PATH_MAX, "%s:%s/%u:%u:%u", source, name,
                 cur->size, cur->dpi, cur->rend_flags);

        if (add_to_hash)
          {
             eina_hash_add(_index.fonts.entries_by_hkey, hkey, cur);
             _index.fonts.last_entry_in_hash++;
          }

        if (!strcmp(hkey, fe->hkey))
          {
             fe->font_data_id = cur->id;
             return cur;
          }
     }

   return fd;
}

#endif
