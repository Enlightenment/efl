#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_cserve2.h"

#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

typedef enum
{
   SLAVE_PROCESS,
   SLAVE_THREAD
} Slave_Type;

struct _Slave
{
   Slave_Type type;
   int write_fd;
   int read_fd;
   Slave_Read_Cb read_cb;
   Slave_Dead_Cb dead_cb;
   const void *data;
   Eina_Binbuf *pending;

   struct {
      int size;
      int read_size;
      Slave_Command cmd;
      char *buf;
   } read;
};

struct _Slave_Proc
{
   Slave base;
   pid_t pid;
   const char *name;
   Eina_Bool killed : 1;
};

typedef struct _Slave_Thread Slave_Thread;
typedef struct _Slave_Proc Slave_Proc;

struct _Slave_Thread
{
   Slave base;
   pthread_t tid;
   Slave_Thread_Data *tdata;
};

struct _Slave_Thread_Data {
   int write_fd;
   int read_fd;
   Slave_Thread_Cb cb;
   void *cb_data;
   const void *cmddata;
   void *cmdanswer;
};

static Eina_List *slave_procs;
static Eina_List *slave_threads;
static pthread_attr_t slave_thread_attr;

static Slave_Proc *
_slave_proc_find(pid_t pid)
{
   Eina_List *l;
   Slave_Proc *s;

   EINA_LIST_FOREACH(slave_procs, l, s)
      if (s->pid == pid)
        return s;
   return NULL;
}

static void
_slave_free(Slave *s)
{
   if (s->write_fd)
     close(s->write_fd);
   if (s->read_fd)
     {
        cserve2_fd_watch_del(s->read_fd);
        close(s->read_fd);
     }

   free(s->read.buf);

   if (s->pending)
     eina_binbuf_free(s->pending);

   if (s->dead_cb)
     s->dead_cb(s, (void *)s->data);

}

static void
_slave_proc_free(Slave_Proc *s)
{
   _slave_free((Slave *)s);

   eina_stringshare_del(s->name);

   free(s);
}

static void
_slave_thread_free(Slave_Thread *s)
{
   Slave_Thread_Data *sd = s->tdata;

   close(sd->write_fd);
   close(sd->read_fd);

   free(sd);

   _slave_free((Slave *)s);

   free(s);
}

static void
_slave_proc_dead_cb(int pid, int status EINA_UNUSED)
{
   Slave_Proc *s;

   INF("Child dead with pid '%d': signal %d",
       pid, WIFSIGNALED(status) ? WTERMSIG(status) : 0);
   s = _slave_proc_find(pid);
   if (!s)
     {
        ERR("Unknown child dead '%d'.", pid);
        return;
     }

   slave_procs = eina_list_remove(slave_procs, s);
   _slave_proc_free(s);
}

static size_t
_slave_write(Slave *s, const char *data, size_t size)
{
   size_t sent = 0;

   do
     {
        ssize_t ret;
        ret = write(s->write_fd, data + sent, size - sent);
        if (ret == -1)
          {
             if (errno == EAGAIN)
               break;
             else if (errno == EPIPE)
               {
                  WRN("Slave unexpectedly gone.");
                  /* FIXME: handle dead? */
                  break;
               }
             else
               {
                  ERR("Unhandled error occurred when writing to slave: %d %m", errno);
                  break;
               }
          }
        sent += ret;
     } while(sent < size);

   return sent;
}

static void
_slave_write_cb(int fd EINA_UNUSED, Fd_Flags flags EINA_UNUSED, void *data)
{
   Slave *s = data;
   size_t sent;
   size_t size;
   const char *str;

   size = eina_binbuf_length_get(s->pending);
   str = (const char *)eina_binbuf_string_get(s->pending);
   sent = _slave_write(s, str, size);
   if (sent == size)
     {
        eina_binbuf_free(s->pending);
        s->pending = NULL;
        cserve2_fd_watch_del(s->write_fd);
        return;
     }

   eina_binbuf_remove(s->pending, 0, sent);
}

static void
_slave_read_clear(Slave *s)
{
   s->read.buf = NULL;
   s->read.cmd = 0;
   s->read.read_size = s->read.size = 0;
}

static void
_slave_proc_read_cb(int fd, Fd_Flags flags, void *data)
{
   Slave *s = data;
   Eina_Bool done = EINA_FALSE;

   /* handle error */
   if (!(flags & FD_READ))
     return;

   if (!s->read.size)
     {
        int ints[2];
        ssize_t ret;

        ret = read(fd, ints, sizeof(int) * 2);
        if (ret < (int)sizeof(int) * 2)
          {
             return;
          }
        s->read.size = ints[0];
        s->read.cmd = ints[1];
        if (s->read.size)
          s->read.buf = malloc(s->read.size);
        else
          done = EINA_TRUE;
     }

   if (s->read.buf)
     {
        ssize_t ret;
        do {
             char *p = s->read.buf + s->read.read_size;
             int sz = s->read.size - s->read.read_size;
             ret = read(fd, p, sz);
             if (ret < 0)
               {
                  if (errno == EAGAIN)
                    break;
               }
             s->read.read_size += ret;
        } while(s->read.read_size < s->read.size);

        if (s->read.read_size == s->read.size)
          done = EINA_TRUE;
     }

   if (done)
     {
        s->read_cb(s, s->read.cmd, s->read.buf, (void *)s->data);
        _slave_read_clear(s);
     }
}

static void
_slave_thread_read_cb(int fd, Fd_Flags flags, void *data)
{
   Slave_Thread *s = data;
   Slave_Thread_Data *sd = s->tdata;

   if (!(flags & FD_READ))
     return;

   Slave_Command cmd;
   ssize_t ret;

   ret = read(fd, (char *)&cmd, sizeof(cmd));
   if (ret < (int)sizeof(int))
     {
        return;
     }

   s->base.read_cb((Slave *)s, cmd, sd->cmdanswer, (void *)s->base.data);
}

Eina_Bool
cserve2_slaves_init(void)
{
   cserve2_on_child_dead_set(_slave_proc_dead_cb);

   if (pthread_attr_init(&slave_thread_attr))
     {
        ERR("Could not initialize attributes for thread.");
        cserve2_on_child_dead_set(NULL);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

void
cserve2_slaves_shutdown(void)
{
   Slave_Proc *sp;
   Slave_Thread *st;
   Eina_List *l;

   cserve2_on_child_dead_set(NULL);

   if (!slave_procs && !slave_threads)
     return;

   DBG("Shutting down slaves subsystem with %d slaves alive!",
       eina_list_count(slave_procs));

   EINA_LIST_FREE(slave_procs, sp)
     {
        kill(sp->pid, SIGKILL);
        _slave_proc_free(sp);
     }

   EINA_LIST_FOREACH(slave_threads, l, st)
      pthread_cancel(st->tid);

   EINA_LIST_FREE(slave_threads, st)
     {
        pthread_join(st->tid, NULL);
        _slave_thread_free(st);
     }
}

static const char *
_slave_proc_path_get(const char *name)
{
   char buf[PATH_MAX], cwd[PATH_MAX];

   if (name[0] == '/')
     {
        if (access(name, X_OK))
          return NULL;
        return eina_stringshare_add(name);
     }

   if (!getcwd(cwd, sizeof(cwd)))
     {
        WRN("getcwd() write returned < 0.");
        return NULL;
     }
   snprintf(buf, sizeof(buf), "%s/%s", cwd, name);
   if (!access(buf, X_OK))
     return eina_stringshare_add(buf);

   snprintf(buf, sizeof(buf), "%s/evas/cserve2/bin/" MODULE_ARCH "/%s",
            eina_prefix_lib_get(_evas_cserve2_pfx), name);
   if (!access(buf, X_OK))
     return eina_stringshare_add(buf);

   return NULL;
}

static Slave *
_cserve2_slave_proc_run(const char *exe, Slave_Read_Cb read_cb, Slave_Dead_Cb dead_cb, const void *data)
{
   Slave_Proc *s;
   Slave *sb;
   pid_t pid;
   int child[2], parent[2];
   int flags;
   const char *name;

   name = _slave_proc_path_get(exe);
   if (!name)
     {
        ERR("Cannot execute slave '%s'. Not found or not executable.", exe);
        return NULL;
     }
   DBG("Running slave '%s', resolved to path: %s", exe, name);

   s = calloc(1, sizeof(Slave_Proc));
   if (!s)
     {
        ERR("Could not create Slave_Proc handler.");
        eina_stringshare_del(name);
        return NULL;
     }

   sb = (Slave *)s;

   if (pipe(child))
     {
        ERR("Could not create pipes for child.");
        eina_stringshare_del(name);
        free(s);
        return NULL;
     }

   if (pipe(parent))
     {
        ERR("Could not create pipes for parent.");
        eina_stringshare_del(name);
        free(s);
        close(child[0]);
        close(child[1]);
        return NULL;
     }

   pid = fork();
   if (pid < 0)
     {
        ERR("Could not create sub process.");
        eina_stringshare_del(name);
        close(child[0]);
        close(child[1]);
        close(parent[0]);
        close(parent[1]);
        free(s);
        return NULL;
     }

   if (!pid)
     {
        char *args[4], readfd[12], writefd[12];

        close(child[1]);
        close(parent[0]);

        sprintf(readfd, "%d", child[0]);
        sprintf(writefd, "%d", parent[1]);
        args[0] = (char *)name;
        args[1] = writefd;
        args[2] = readfd;
        args[3] = NULL;
        execvp(name, args);
        /* we only get here if execvp fails, which should not
         * happen and if it does, it's baaaaaaaaad */
        ERR("execvp() for slave at: '%s' failed! '%m'", name);
        exit(1);
     }

   s->pid = pid;
   s->name = name;
   sb->type = SLAVE_PROCESS;
   sb->write_fd = child[1];
   flags = fcntl(sb->write_fd, F_GETFL);
   flags |= O_NONBLOCK;
   if (fcntl(sb->write_fd, F_SETFL, flags) < 0) ERR("fcntl NONBLOCK failed");
   sb->read_fd = parent[0];
   flags = fcntl(sb->read_fd, F_GETFL);
   flags |= O_NONBLOCK;
   if (fcntl(sb->read_fd, F_SETFL, flags) < 0) ERR("fcntl NONBLOCK failed");
   sb->read_cb = read_cb;
   sb->dead_cb = dead_cb;
   sb->data = data;
   cserve2_fd_watch_add(sb->read_fd, FD_READ, _slave_proc_read_cb, sb);

   close(child[0]);
   close(parent[1]);

   slave_procs = eina_list_append(slave_procs, s);

   return sb;
}

Slave *
cserve2_slave_run(const char *name, Slave_Read_Cb read_cb, Slave_Dead_Cb dead_cb, const void *data)
{
   return _cserve2_slave_proc_run(name, read_cb, dead_cb, data);
}

static void *
_slave_thread_cb(void *data)
{
   ssize_t n;
   Slave_Command cmd;

   Slave_Thread_Data *sd = data;

   n = read(sd->read_fd, &cmd, sizeof(cmd));
   while (n != 0)
     {
        /* EINTR means we were interrupted by a signal before anything
         * was sent, and if we are back here it means that signal was
         * not meant for us to die. Any other error here is fatal and
         * should result in the slave terminating.
         */
        if (errno == EINTR)
          continue;

        if (n != sizeof(cmd))
          {
             ERR("Slave thread read invalid size of command from server: %zu",
                 n);
             continue;
          }
        sd->cmdanswer = sd->cb(sd, &cmd, sd->cmddata, sd->cb_data);
        if (write(sd->write_fd, &cmd, sizeof(cmd)) < 0)
          {
             WRN("_slave_thread_cb() write returned < 0.");
          }
        n = read(sd->read_fd, &cmd, sizeof(cmd));
     }

   ERR("Pipe was closed on the side. Slave thread exiting...");

   return NULL;
}

Slave *
cserve2_slave_thread_run(Slave_Thread_Cb thread_cb, void *thread_data, Slave_Read_Cb read_cb, Slave_Dead_Cb dead_cb, const void *data)
{
   Slave_Thread_Data *sd;
   Slave_Thread *s;
   Slave *sb;
   pthread_t tid;
   int child[2], parent[2];
   int flags;

   s = calloc(1, sizeof(Slave_Thread));
   if (!s)
     {
        ERR("Could not create Slave_Thread handler.");
        return NULL;
     }

   sb = (Slave *)s;

   sd = calloc(1, sizeof(Slave_Thread_Data));
   if (!sd)
     {
        ERR("Could not create Slave_Thread_Data.");
        free(s);
        return NULL;
     }

   if (pipe(child))
     {
        ERR("Could not create pipes for child.");
        free(s);
        free(sd);
        return NULL;
     }

   if (pipe(parent))
     {
        ERR("Could not create pipes for parent.");
        free(s);
        free(sd);
        close(child[0]);
        close(child[1]);
        return NULL;
     }

   /* Setting data for slave thread */
   sd->read_fd = child[0];
   sd->write_fd = parent[1];

   sd->cb = thread_cb;
   sd->cb_data = thread_data;

   if (pthread_create(&tid, &slave_thread_attr, _slave_thread_cb, sd))
     {
        ERR("Could not start slave thread.");
        free(s);
        free(sd);
        close(child[0]);
        close(child[1]);
        close(parent[0]);
        close(parent[1]);
        return NULL;
     }

   s->tid = tid;
   s->tdata = sd;
   sb->type = SLAVE_THREAD;
   sb->write_fd = child[1];
   flags = fcntl(sb->write_fd, F_GETFL);
   flags |= O_NONBLOCK;
   fcntl(sb->write_fd, F_SETFL, flags);
   sb->read_fd = parent[0];
   flags = fcntl(sb->read_fd, F_GETFL);
   flags |= O_NONBLOCK;
   fcntl(sb->read_fd, F_SETFL, flags);
   sb->read_cb = read_cb;
   sb->dead_cb = dead_cb;
   sb->data = data;
   cserve2_fd_watch_add(sb->read_fd, FD_READ, _slave_thread_read_cb, sb);

   slave_threads = eina_list_append(slave_threads, s);

   return sb;
}

static void
_slave_send_aux(Slave *s, const char *data, size_t size)
{
   size_t sent;

   if (s->pending)
     {
        eina_binbuf_append_length(s->pending, (unsigned char *)data, size);
        return;
     }

   sent = _slave_write(s, data, size);
   if (sent < size)
     {
        s->pending = eina_binbuf_new();
        eina_binbuf_append_length(s->pending, (unsigned char *)data + sent,
                                  size - sent);
        cserve2_fd_watch_add(s->write_fd, FD_WRITE, _slave_write_cb, s);
     }
}

void
_cserve2_slave_proc_send(Slave *s, Slave_Command cmd, const char *data, size_t size)
{
   int ints[2];

   ints[0] = size;
   ints[1] = cmd;
   _slave_send_aux(s, (char *)ints, sizeof(int) * 2);
   if (size)
     _slave_send_aux(s, (char *)data, size);
}

void
_cserve2_slave_thread_send(Slave_Thread *s, Slave_Command cmd, const char *data)
{
   s->tdata->cmddata = data;

   _slave_send_aux((Slave *)s, (char *)&cmd, sizeof(cmd));
}

void
cserve2_slave_send(Slave *s, Slave_Command cmd, const char *data, size_t size)
{
   if (s->type == SLAVE_PROCESS)
     _cserve2_slave_proc_send(s, cmd, data, size);
   else
     _cserve2_slave_thread_send((Slave_Thread *)s, cmd, data);
}

static void
_cserve2_slave_proc_kill(Slave_Proc *s)
{
   if (s->killed)
     {
        if (!kill(s->pid, 0))
          DBG("Slave %p(%d) requested to kill, but it's still alive.",
              s, s->pid);
     }

   s->killed = EINA_TRUE;
   kill(s->pid, SIGTERM);
}

void
cserve2_slave_kill(Slave *s)
{
   _cserve2_slave_proc_kill((Slave_Proc *)s);
}
