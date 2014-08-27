#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_cserve2.h"

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_EPOLL_EVENTS 10
#define MAX_INCOMING_CONN 10

struct _Watch_Data
{
   int fd;
   Fd_Flags flags;
   Fd_Watch_Cb callback;
   const void *user_data;
   Eina_Bool deleted : 1;
};

typedef struct _Watch_Data Watch_Data;

struct _Inotify_Data
{
   EINA_INLIST;
   const char *path;
   int watchid;
   File_Change_Cb cb;
   const void *data;
};

typedef struct _Inotify_Data Inotify_Data;

static int epoll_fd = -1;
static int signal_fd = -1;
static int socket_fd = -1;
static int inotify_fd = -1;
static struct sockaddr_un socket_local;
static Eina_Hash *watch_list;
static Eina_List *deleted_watch_list;
static Eina_Hash *inotify_path_hash;
static Eina_Hash *inotify_id_hash;
static Eina_Bool running;
static Eina_Bool terminate;
static int timeout = -1; // in miliseconds
static long timeout_time = 0; // in miliseconds

static Timeout_Cb timeout_func = NULL;
static Main_Loop_Child_Dead_Cb reap_children_func = NULL;

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX sizeof(((struct sockaddr_un *)NULL)->sun_path)
#endif

static void
_signal_handle_child(struct signalfd_siginfo *sinfo EINA_UNUSED)
{
   int status;
   pid_t pid;

   while ((pid = waitpid(0, &status, WNOHANG)) > 0)
     {
        if (reap_children_func)
          {
             reap_children_func(pid, status);
             continue;
          }

        DBG("Received SIGCHLD and no handler is set.");

        if (WIFEXITED(status))
          DBG("Child '%d' exited with status '%d'.", pid,
              WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
          DBG("Child '%d' exited with signal '%d'.", pid,
              WTERMSIG(status));
     }
}

static void
_signal_handle_exit(struct signalfd_siginfo *sinfo EINA_UNUSED)
{
#if CSERVE2_LOG_LEVEL >= 4
   const char *name;

   switch (sinfo->ssi_signo)
     {
      case SIGINT:  name = "SIGINT"; break;
      case SIGTERM: name = "SIGTERM"; break;
      case SIGQUIT: name = "SIGQUIT"; break;
      default: return;
     }

   DBG("Received %s. Honoring request.", name);
#endif
   terminate = EINA_TRUE;
}

static void
_signalfd_handler(int fd, Fd_Flags flags EINA_UNUSED, void *data EINA_UNUSED)
{
   struct signalfd_siginfo sinfo;
   ssize_t ret;

   for (;;)
     {
        ret = read(fd, &sinfo, sizeof(struct signalfd_siginfo));
        if (ret == -1)
          {
             if (errno == EAGAIN)
               break;
             ERR("Error reading from signal fd: %m.");
             return;
          }

        switch(sinfo.ssi_signo)
          {
           case SIGCHLD:
             _signal_handle_child(&sinfo);
             break;
           case SIGINT:
             _signal_handle_exit(&sinfo);
             break;
           case SIGUSR1:
           case SIGUSR2:
             DBG("Ignored signal %d", sinfo.ssi_signo);
             break;
           default:
             ERR("Caught unexpected signal '%d'.", sinfo.ssi_signo);
          }
     }
}

static void
_sigint_handler(int id, siginfo_t *info EINA_UNUSED, void *data EINA_UNUSED)
{
   if (id != SIGINT) return;
   DBG("Received SIGINT. Honoring request.");
   terminate = EINA_TRUE;
}

static int
_signalfd_setup(void)
{
   sigset_t mask;
   struct sigaction action;

   sigemptyset(&mask);
   sigaddset(&mask, SIGCHLD);
   sigaddset(&mask, SIGUSR1); // ignored
   sigaddset(&mask, SIGUSR2); // ignored

   if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
     {
        ERR("Could not set mask of handled signals.");
        return -1;
     }

   signal_fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
   if (signal_fd == -1)
     ERR("Could not create file descriptor from signalfd.");

   /* ignore SIGPIPE so it's handled by write() and send() as needed */
   signal(SIGPIPE, SIG_IGN);

   action.sa_sigaction = _sigint_handler;
   action.sa_flags = SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGINT, &action, NULL);

   return signal_fd;
}

static void
_signalfd_finish(void)
{
   sigset_t mask;

   cserve2_fd_watch_del(signal_fd);

   sigemptyset(&mask);
   sigprocmask(SIG_BLOCK, NULL, &mask);

   close(signal_fd);
   sigprocmask(SIG_UNBLOCK, &mask, NULL);

   signal(SIGPIPE, SIG_DFL);
}

static void
_socketfd_handler(int fd EINA_UNUSED, Fd_Flags flags EINA_UNUSED, void *data EINA_UNUSED)
{
   struct sockaddr_un remote;
   unsigned int len;
   int s;

   len = sizeof(struct sockaddr_un);
   s = accept(socket_fd, &remote, &len);
   if (s == -1)
     {
        ERR("Could not accept socket: \"%s\"", strerror(errno));
        return;
     }
   /* TODO: when porting to windows, do:
    * SetHandleInformation(s, HANDLE_FLAG_INHERIT, 0)
    */
   if (fcntl(s, F_SETFD, FD_CLOEXEC) < 0) ERR("Cannot set CLOEXEC on fd");

   cserve2_client_accept(s);
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
   return;
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

static int
_socketfd_setup(void)
{
   int s;
   int len;

   s = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
   if (s == -1)
     {
        ERR("Could not create socketfd: \"%s\"", strerror(errno));
        return -1;
     }

   socket_local.sun_family = AF_UNIX;
   _socket_path_set(socket_local.sun_path);
   DBG("Using '%s' as server socket.", socket_local.sun_path);
   len = strlen(socket_local.sun_path) + sizeof(socket_local.sun_family);
   if (bind(s, (struct sockaddr *)&socket_local, len) == -1)
     {
        if (connect(s, (struct sockaddr *)&socket_local, len) != -1)
          {
             if (errno == EADDRINUSE)
               ERR("cserve2 service already there: \"%s\"", strerror(errno));
             else
               ERR("cserve2 could connect the socket: %d \"%s\"", errno, strerror(errno));
             close(s);
             return -1;
          }
        unlink(socket_local.sun_path);
        if (bind(s, (struct sockaddr *)&socket_local, len) == -1)
          {
             ERR("Could not bind socketfd: \"%s\"", strerror(errno));
             close(s);
             return -1;
          }
     }

   if (listen(s, MAX_INCOMING_CONN) == -1)
     {
        ERR("Could not listen on socketfd: \"%s\"", strerror(errno));
        close(s);
        unlink(socket_local.sun_path);
        return -1;
     }

   socket_fd = s;

   return s;
}

static void
_socketfd_finish(void)
{
   close(socket_fd);
   unlink(socket_local.sun_path);
}

static void
_inotifyfd_handler(int fd, Fd_Flags flags, void *data EINA_UNUSED)
{
   char buffer[16384];
   int i = 0;
   ssize_t size;

   if (flags & FD_ERROR)
     {
        ERR("Error on inotify file handler, what to do?");
        return;
     }

   size = read(fd, buffer, sizeof(buffer));
   while (i < size)
     {
        struct inotify_event *event;
        int event_size;
        Eina_Inlist *ids, *itr;
        Inotify_Data *id;
        Eina_Bool deleted;

        event = (struct inotify_event *)&buffer[i];
        event_size = sizeof(struct inotify_event) + event->len;
        i += event_size;

        ids = eina_hash_find(inotify_id_hash, &event->wd);
        if (!ids) continue;

        deleted = !!(event->mask
                     & (IN_DELETE_SELF | IN_MOVE_SELF
                        | IN_IGNORED | IN_UNMOUNT));
        EINA_INLIST_FOREACH_SAFE(ids, itr, id)
         id->cb(id->path, deleted, (void *)id->data);
     }
}

static void
_inotify_id_hash_free_cb(void *data)
{
   Eina_Inlist *list = data;

   while (list)
     {
        Inotify_Data *id;
        id = EINA_INLIST_CONTAINER_GET(list, Inotify_Data);
        list = eina_inlist_remove(list, list);
        eina_stringshare_del(id->path);
        free(id);
     }
}

static int
_inotifyfd_setup(void)
{
   inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
   if (inotify_fd == -1)
     {
        ERR("Could not create inotifyfd: \"%s\"", strerror(errno));
        return -1;
     }
   inotify_path_hash = eina_hash_string_superfast_new(NULL);
   inotify_id_hash = eina_hash_int32_new(_inotify_id_hash_free_cb);

   return inotify_fd;
}

static void
_inotifyfd_finish(void)
{
   close(inotify_fd);

   eina_hash_free(inotify_path_hash);
   eina_hash_free(inotify_id_hash);
}

static void
_watch_data_free_cb(void *data)
{
   Watch_Data *wd = data;
   wd->deleted = EINA_TRUE;
   deleted_watch_list = eina_list_append(deleted_watch_list, wd);
}

Eina_Bool
cserve2_main_loop_setup(void)
{
   int sfd;
   int sock;
   int ifd;

   epoll_fd = epoll_create1(EPOLL_CLOEXEC);

   if (epoll_fd < 0)
     {
        ERR("Could not create epoll fd.");
        return EINA_FALSE;
     }

   watch_list = eina_hash_int32_new(_watch_data_free_cb);
   if (!watch_list)
     {
        ERR("Could not create watch list hash struct.");
        close(epoll_fd);
        return EINA_FALSE;
     }

   sfd = _signalfd_setup();
   if (sfd == -1)
     {
        ERR("Could not setup signalfd.");
        close(epoll_fd);
        eina_hash_free(watch_list);
        return EINA_FALSE;
     }

   DBG("Add watch for signal fd: %d", sfd);
   if (!cserve2_fd_watch_add(sfd, FD_READ, _signalfd_handler, NULL))
     {
        ERR("Could not add watch for signalfd.");
        close(sfd);
        close(epoll_fd);
        eina_hash_free(watch_list);
        return EINA_FALSE;
     }

   sock = _socketfd_setup();
   if (sock == -1)
     {
        ERR("Could not setup socketfd.");
        goto error_socket;
     }

   DBG("Add watch for socket fd: %d", sock);
   if (!cserve2_fd_watch_add(sock, FD_READ, _socketfd_handler, NULL))
     {
        ERR("Could not add watch for socketf.");
        close(sock);
        goto error_socket;
     }

   ifd = _inotifyfd_setup();
   if (ifd == -1)
     {
        ERR("Could not setup inotify.");
        goto error_inotify;
     }

   DBG("Add watch for inotify fd: %d", ifd);
   if (!cserve2_fd_watch_add(ifd, FD_READ, _inotifyfd_handler, NULL))
     {
        ERR("Could not add watch for inotifyfd.");
        close(ifd);
        goto error_inotify;
     }

   return EINA_TRUE;

error_inotify:
   _socketfd_finish();

error_socket:
   close(sfd);
   close(epoll_fd);
   eina_hash_free(watch_list);
   return EINA_FALSE;
}

void
cserve2_main_loop_finish(void)
{
   Watch_Data *wd;

   _socketfd_finish();

   _signalfd_finish();

   _inotifyfd_finish();

   eina_hash_free(watch_list);
   EINA_LIST_FREE(deleted_watch_list, wd)
     free(wd);

   close(epoll_fd);
}

Eina_Bool
cserve2_fd_watch_add(int fd, Fd_Flags flags, Fd_Watch_Cb cb, const void *data)
{
   Watch_Data *w_data;
   struct epoll_event ev;
   int err;

   DBG("Add watch for fd %d, flags = 0x%x", fd, flags);

   if ((fd < 0) || (!cb))
     {
        ERR("Can't add watch: fd = %d, callback = %p", fd, cb);
        return EINA_FALSE;
     }

   w_data = calloc(1, sizeof(*w_data));
   w_data->fd = fd;
   w_data->flags = flags;
   w_data->callback = cb;
   w_data->user_data = data;

   if (!eina_hash_add(watch_list, &fd, w_data))
     {
        ERR("Could not add watch for fd %d to the hash.", fd);
        free(w_data);
        return EINA_FALSE;
     }

   memset(&ev, 0, sizeof(ev));
   if (flags & FD_READ)
     ev.events |= EPOLLIN;
   if (flags & FD_WRITE)
     ev.events |= EPOLLOUT;
   ev.data.ptr = w_data;

   err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
   if (err < 0)
     {
        ERR("Could not create epoll watch for fd %d.", fd);
        eina_hash_del(watch_list, &fd, NULL);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
cserve2_fd_watch_flags_set(int fd, Fd_Flags flags)
{
   Watch_Data *w_data;
   struct epoll_event ev;
   int err;

   DBG("Set watch flags for fd %d, flags = 0x%x", fd, flags);

   if (fd < 0)
     {
        ERR("Can't modify watch: fd = %d", fd);
        return EINA_FALSE;
     }

   w_data = eina_hash_find(watch_list, &fd);
   if (!w_data)
     {
        ERR("Couldn't find data for fd %d: not being watched.", fd);
        return EINA_FALSE;
     }

   if (flags == w_data->flags)
     return EINA_TRUE;

   memset(&ev, 0, sizeof(ev));
   if (flags & FD_READ)
     ev.events |= EPOLLIN;
   if (flags & FD_WRITE)
     ev.events |= EPOLLOUT;
   ev.data.ptr = w_data;

   err = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
   if (err < 0)
     {
        ERR("Could not modify epoll watch for fd: %d", fd);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
cserve2_fd_watch_flags_get(int fd, Fd_Flags *flags)
{
   Watch_Data *w_data;
   if (fd < 0)
     {
        ERR("Can't get flags for watch: fd = %d", fd);
        return EINA_FALSE;
     }

   w_data = eina_hash_find(watch_list, &fd);
   if (!w_data)
     {
        ERR("Couldn't find data for fd: %d. Is it really being watched?", fd);
        return EINA_FALSE;
     }

   *flags = w_data->flags;

   return EINA_TRUE;
}

Eina_Bool
cserve2_fd_watch_del(int fd)
{
   int err;

   DBG("Remove watch for fd %d", fd);

   if (fd < 0)
     return EINA_FALSE;

   if (!eina_hash_del(watch_list, &fd, NULL))
     ERR("Could not remove watch for fd %d from watch list hash.", fd);

   err = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
   if (err < 0)
     {
        ERR("Could not remove epoll watch for fd %d.", fd);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
cserve2_file_change_watch_add(const char *path, File_Change_Cb cb, const void *data)
{
   Inotify_Data *id;
   Eina_Inlist *ids;
   unsigned int mask = IN_ATTRIB | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF;

   id = eina_hash_find(inotify_path_hash, path);
   if (id)
     {
        ERR("Attempt to watch changes for path '%s', which is already "
            "being watched.", path);
        return EINA_FALSE;
     }

   id = calloc(1, sizeof(Inotify_Data));
   if (!id)
     {
        ERR("Could not alloc Inotify_Data instance.");
        return EINA_FALSE;
     }

   id->watchid = inotify_add_watch(inotify_fd, path, mask);
   if (id->watchid == -1)
     {
        ERR("Could not add inotify watch for %s: %m.", path);
        free(id);
        return EINA_FALSE;
     }

   id->path = eina_stringshare_add(path);
   id->cb = cb;
   id->data = data;

   eina_hash_direct_add(inotify_path_hash, id->path, id);

   ids = eina_hash_find(inotify_id_hash, &id->watchid);
   ids = eina_inlist_append(ids, EINA_INLIST_GET(id));
   eina_hash_set(inotify_id_hash, &id->watchid, ids);

   return EINA_TRUE;
}

Eina_Bool
cserve2_file_change_watch_del(const char *path)
{
   Inotify_Data *id;
   Eina_Inlist *ids;
   int wd;

   id = eina_hash_set(inotify_path_hash, path, NULL);
   if (!id)
     {
        ERR("Requested to remove change watch for %s, but it's not being "
            "watched.", path);
        return EINA_FALSE;
     }

   ids = eina_hash_find(inotify_id_hash, &id->watchid);
   ids = eina_inlist_remove(ids, EINA_INLIST_GET(id));
   eina_hash_set(inotify_id_hash, &id->watchid, ids);

   wd = id->watchid;
   eina_stringshare_del(id->path);
   free(id);

   if (!ids)
     {
        if (inotify_rm_watch(inotify_fd, wd) == -1)
          {
             ERR("Could not remove change watch for %s: %m", path);
             return EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

static void
_update_timeout(void)
{
   struct timeval timev;
   long cur_time;

   if (timeout <= 0)
     return;

   gettimeofday(&timev, NULL);
   cur_time = timev.tv_sec * 1000 + timev.tv_usec / 1000;
   timeout -= cur_time - timeout_time;
   timeout_time = cur_time;

   if (timeout < 0)
     timeout = 0;
}

void
cserve2_timeout_cb_set(int t, Timeout_Cb cb)
{
   struct timeval timev;
   if (cb && t <= 0)
     {
        ERR("timeout must be a value greater than 0 to set a callback."
            " given timeout: %d miliseconds", t);
        return;
     }

   if (!cb)
     {
        DBG("Removing timeout callback.");
        timeout = -1;
        cb = NULL;
        return;
     }

   //DBG("Setting timeout to: %d miliseconds", t);
   gettimeofday(&timev, NULL);
   timeout_time = timev.tv_sec * 1000 + timev.tv_usec / 1000;
   timeout = t;
   timeout_func = cb;
}

void
cserve2_on_child_dead_set(Main_Loop_Child_Dead_Cb cb)
{
   reap_children_func = cb;
}

void
cserve2_main_loop_run(void)
{
   running = EINA_TRUE;
   terminate = EINA_FALSE;

   for (;;)
     {
        struct epoll_event events[MAX_EPOLL_EVENTS];
        int n, nfds;
        Watch_Data *data;

        if (terminate)
          break;

        nfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, timeout);
        if (nfds < 0)
          {
             if (errno == EINTR && !terminate)
               {
                  INF("Ignoring interruption during epoll_wait.");
                  continue;
               }
             else
               {
                  ERR("An error occurred when reading the epoll fd: %s.", strerror(errno));
                  break;
               }
          }
        if (nfds == 0) // timeout occurred
          {
             timeout = -1;
             if (!timeout_func)
               ERR("Timeout expired, but no timeout function set.");
             else
               timeout_func();
          }

        for (n = 0; n < nfds; n++)
          {
             data = events[n].data.ptr;
             Fd_Flags flags = 0;

             if (!data)
               continue;

             if (data->deleted)
               continue;

             if (!data->callback)
               continue;

             if (events[n].events & EPOLLIN)
               flags |= FD_READ;
             if (events[n].events & EPOLLOUT)
               flags |= FD_WRITE;
             if (events[n].events & EPOLLERR)
               flags |= FD_ERROR;
             data->callback(data->fd, flags, (void *)data->user_data);
          }

        EINA_LIST_FREE(deleted_watch_list, data)
          free(data);

        _update_timeout();
     }

   running = EINA_FALSE;
}

ssize_t
cserve2_client_read(Client *client, void *buf, size_t len)
{
   return recv(client->socket, buf, len, MSG_DONTWAIT);
}

ssize_t
cserve2_client_write(Client *client, const void *data, size_t size)
{
   return send(client->socket, data, size, MSG_NOSIGNAL | MSG_DONTWAIT);
}
