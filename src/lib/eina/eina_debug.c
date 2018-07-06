/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

# ifndef _GNU_SOURCE
#  define _GNU_SOURCE 1
# endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#ifdef HAVE_SYS_EPOLL_H
# include <sys/epoll.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#include <fcntl.h>

#ifdef _WIN32
# include <ws2tcpip.h>
# include <Evil.h>
#endif

#include "eina_alloca.h"
#include "eina_debug.h"
#include "eina_cpu.h"
#include "eina_types.h"
#include "eina_list.h"
#include "eina_mempool.h"
#include "eina_util.h"
#include "eina_evlog.h"
#include "eina_hash.h"
#include "eina_stringshare.h"
#include "eina_debug_private.h"

#ifdef EINA_HAVE_PTHREAD_SETNAME
# ifndef __linux__
# include <pthread_np.h>
# endif
#endif

#if defined(__CYGWIN__) || defined (_WIN32)
# define LIBEXT ".dll"
#else
# define LIBEXT ".so"
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SWAP_64(x) x
#define SWAP_32(x) x
#define SWAP_16(x) x
#else
#define SWAP_64(x) eina_swap64(x)
#define SWAP_32(x) eina_swap32(x)
#define SWAP_16(x) eina_swap16(x)
#endif

// yes - a global debug spinlock. i expect contention to be low for now, and
// when needed we can split this up into mroe locks to reduce contention when
// and if that day comes
Eina_Spinlock _eina_debug_lock;

#ifdef __linux__
extern char *__progname;
#endif

extern Eina_Bool eina_module_init(void);
extern Eina_Bool eina_mempool_init(void);
extern Eina_Bool eina_list_init(void);

extern Eina_Spinlock _eina_debug_thread_lock;
static Eina_List *sessions;

static Eina_Bool _debug_disabled = EINA_FALSE;

/* Local session */
/* __thread here to allow debuggers to be master and slave by using two different threads */
static Eina_Debug_Session *_last_local_session = NULL;

typedef struct
{
   const Eina_Debug_Opcode *ops;
   Eina_Debug_Opcode_Status_Cb status_cb;
   void *status_data;
   Eina_Bool sent : 1;
} _opcode_reply_info;

struct _Eina_Debug_Session
{
   Eina_List **cbs; /* Table of callbacks lists indexed by opcode id */
   Eina_List *opcode_reply_infos;
   Eina_Debug_Dispatch_Cb dispatch_cb; /* Session dispatcher */
   void *data; /* User data */
   int cbs_length; /* cbs table size */
   int fd; /* File descriptor */
   Eina_Lock lock; /* deletion lock */
   Eina_Bool deleted : 1; /* set if session is dead */
};

#ifndef _WIN32
static void _opcodes_register_all(Eina_Debug_Session *session);
#endif
static void _thread_start(Eina_Debug_Session *session);

EAPI int
eina_debug_session_send(Eina_Debug_Session *session, int dest, int op, void *data, int size)
{
   Eina_Debug_Packet_Header hdr;

   if (!session) return -1;
   if (op == EINA_DEBUG_OPCODE_INVALID) return -1;
   /* Preparation of the packet header */
   hdr.size = SWAP_32(size + sizeof(Eina_Debug_Packet_Header));
   hdr.opcode = SWAP_32(op);
   hdr.cid = SWAP_32(dest);
   e_debug("socket: %d / opcode %X / bytes to send: %d",
         session->fd, op, size + sizeof(*hdr));
#ifndef _WIN32
   eina_spinlock_take(&_eina_debug_lock);
   /* Sending header */
   if (write(session->fd, &hdr, sizeof(hdr)) != sizeof(hdr)) goto err;
   /* Sending payload */
   if (size)
     {
        if (write(session->fd, data, size) != size) goto err;
     }
   eina_spinlock_release(&_eina_debug_lock);
#else
   (void)data;
#endif
   return size;
#ifndef _WIN32
err:
   eina_spinlock_release(&_eina_debug_lock);
   e_debug("Cannot write to eina debug session");
   return 0;
#endif
}

#ifndef _WIN32
static void
_daemon_greet(Eina_Debug_Session *session)
{
   /* say hello to our debug daemon - tell them our PID and protocol
      version we speak */
   /* Version + Pid + App name */
#ifdef __linux__
   char *app_name = __progname;
#else
   char *app_name = NULL;
#endif
   int size = 8 + (app_name ? strlen(app_name) : 0) + 1;
   unsigned char *buf = alloca(size);
   int version = SWAP_32(1); // version of protocol we speak
   int pid = getpid();
   pid = SWAP_32(pid);
   memcpy(buf + 0, &version, 4);
   memcpy(buf + 4, &pid, 4);
   if (app_name)
      memcpy(buf + 8, app_name, strlen(app_name) + 1);
   else
      buf[8] = '\0';
   eina_debug_session_send(session, 0, EINA_DEBUG_OPCODE_HELLO, buf, size);
}

static int
_packet_receive(Eina_Debug_Session *session, unsigned char **buffer)
{
   unsigned char *packet_buf = NULL;
   int rret = -1;
   unsigned int size = 0;

   rret = read(session->fd, &size, 4);
   if (rret == 4)
     {
        size = SWAP_32(size);
        if (size > EINA_DEBUG_MAX_PACKET_SIZE)
          {
             e_debug("Packet too big: %d. The maximum allowed is %d", size, EINA_DEBUG_MAX_PACKET_SIZE);
             rret = -1;
             goto end;
          }
        e_debug("Begin to receive a packet of %d bytes", size);
        // allocate a buffer for the next bytes to receive
        packet_buf = malloc(size);
        if (packet_buf)
          {
             unsigned int cur_packet_size = 4;
             memcpy(packet_buf, &size, 4);
             /* Receive all the remaining packet bytes */
             while (cur_packet_size < size)
               {
                  rret = read(session->fd, packet_buf + cur_packet_size, size - cur_packet_size);
                  if (rret <= 0)
                    {
                       e_debug("Error on read: %d", rret);
                       perror("Read");
                       goto end;
                    }
                  cur_packet_size += rret;
               }
             *buffer = packet_buf;
             rret = cur_packet_size;
             e_debug("Received a packet of %d bytes", cur_packet_size);
          }
        else
          {
             // we couldn't allocate memory for payload buffer
             // internal memory limit error
             e_debug("Cannot allocate %u bytes for op", size);
             goto end;
          }
     }
   else
     {
        e_debug("Invalid size read %i != %i", rret, size_sz);
        goto end;
     }
end:
   if (rret <= 0 && packet_buf) free(packet_buf);
   return rret;
}
#endif

EAPI void
eina_debug_disable()
{
   _debug_disabled = EINA_TRUE;
}

EAPI void
eina_debug_session_terminate(Eina_Debug_Session *session)
{
   /* Close fd here so the thread terminates its own session by itself */
   if (!session) return;
   close(session->fd);
   eina_lock_take(&session->lock);
   if (session->deleted)
     {
        eina_lock_release(&session->lock);
        free(session);
     }
   else
     {
        session->deleted = 1;
        eina_lock_release(&session->lock);
     }
}

EAPI void
eina_debug_session_dispatch_override(Eina_Debug_Session *session, Eina_Debug_Dispatch_Cb disp_cb)
{
   if (!session) return;
   if (!disp_cb) disp_cb = eina_debug_dispatch;
   session->dispatch_cb = disp_cb;
}

EAPI Eina_Debug_Dispatch_Cb
eina_debug_session_dispatch_get(Eina_Debug_Session *session)
{
   if (session) return session->dispatch_cb;
   else return NULL;
}

#ifndef _WIN32
static void
_static_opcode_register(Eina_Debug_Session *session,
      int op_id, Eina_Debug_Cb cb)
{
   if(session->cbs_length < op_id + 1)
     {
        int i = session->cbs_length;
        session->cbs_length = op_id + 16;
        session->cbs = realloc(session->cbs, session->cbs_length * sizeof(Eina_List *));
        for(; i < session->cbs_length; i++) session->cbs[i] = NULL;
     }
   if (cb)
     {
        session->cbs[op_id] = eina_list_append(session->cbs[op_id], cb);
     }
}

/*
 * Response of the daemon containing the ids of the requested opcodes.
 * PTR64 + (opcode id)*
 */
static Eina_Bool
_callbacks_register_cb(Eina_Debug_Session *session, int src_id EINA_UNUSED, void *buffer, int size)
{
   _opcode_reply_info *info = NULL, *info2;
   Eina_List *itr;
   int *os;
   unsigned int count, i;

   uint64_t info_64;
   memcpy(&info_64, buffer, sizeof(uint64_t));
   // cast to a ptr, so on 32bit we just take the lower 32bits and on 64
   // we take all of it so we're fine
   info = (_opcode_reply_info *)info_64;

   if (!info) return EINA_FALSE;
   EINA_LIST_FOREACH(session->opcode_reply_infos, itr, info2)
     {
        if (info2 == info)
          {
             os = (int *)((unsigned char *)buffer + sizeof(uint64_t));
             count = (size - sizeof(uint64_t)) / sizeof(int);

             for (i = 0; i < count; i++)
               {
                  int op = SWAP_32(os[i]);
                  if (info->ops[i].opcode_id) *(info->ops[i].opcode_id) = op;
                  _static_opcode_register(session, op, info->ops[i].cb);
                  e_debug("Opcode %s -> %d", info->ops[i].opcode_name, op);
               }
             if (info->status_cb) info->status_cb(info->status_data, EINA_TRUE);
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}
#endif

static void
_opcodes_registration_send(Eina_Debug_Session *session,
      _opcode_reply_info *info)
{
   unsigned char *buf;

   int count = 0;
   int size = sizeof(uint64_t);
   Eina_Bool already_sent;

   eina_spinlock_take(&_eina_debug_lock);
   already_sent = info->sent;
   info->sent = EINA_TRUE;
   eina_spinlock_release(&_eina_debug_lock);
   if (already_sent) return;

   while (info->ops[count].opcode_name)
     {
        size += strlen(info->ops[count].opcode_name) + 1;
        count++;
     }

   buf = malloc(size);

   // cast to a ptr, so on 32bit we just pad out the upper 32bits with 0
   // and on 64bit we are fine as we use all of it
   uint64_t info_64 = (uint64_t)(uintptr_t)info;
   memcpy(buf, &info_64, sizeof(uint64_t));
   int size_curr = sizeof(uint64_t);

   count = 0;
   while (info->ops[count].opcode_name)
     {
        int len = strlen(info->ops[count].opcode_name) + 1;
        memcpy(buf + size_curr, info->ops[count].opcode_name, len);
        size_curr += len;
        count++;
     }

   eina_debug_session_send(session, 0, EINA_DEBUG_OPCODE_REGISTER, buf, size);
   free(buf);
}

#ifndef _WIN32
static void
_opcodes_register_all(Eina_Debug_Session *session)
{
   Eina_List *l;
   _opcode_reply_info *info = NULL;

   _static_opcode_register(session,
         EINA_DEBUG_OPCODE_REGISTER, _callbacks_register_cb);
   EINA_LIST_FOREACH(session->opcode_reply_infos, l, info)
        _opcodes_registration_send(session, info);;
}

static void
_opcodes_unregister_all(Eina_Debug_Session *session)
{
   Eina_List *l;
   int i;
   _opcode_reply_info *info = NULL;

   if (!session) return;
   for (i = 0; i < session->cbs_length; i++)
      eina_list_free(session->cbs[i]);
   free(session->cbs);
   session->cbs_length = 0;
   session->cbs = NULL;

   EINA_LIST_FOREACH(session->opcode_reply_infos, l, info)
     {
        const Eina_Debug_Opcode *op = info->ops;
        while(!op->opcode_name)
          {
             if (op->opcode_id) *(op->opcode_id) = EINA_DEBUG_OPCODE_INVALID;
             op++;
          }
        if (info->status_cb) info->status_cb(info->status_data, EINA_FALSE);
     }
}

static const char *
_socket_home_get()
{
   // get possible debug daemon socket directory base
   const char *dir = getenv("XDG_RUNTIME_DIR");
   if (!dir) dir = eina_environment_home_get();
   if (!dir) dir = eina_environment_tmp_get();
   return dir;
}

#define LENGTH_OF_SOCKADDR_UN(s) \
   (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))
#endif

static Eina_Debug_Session *
_session_create(int fd)
{
   Eina_Debug_Session *session = calloc(1, sizeof(*session));
   if (!session) return NULL;
   session->dispatch_cb = eina_debug_dispatch;
   session->fd = fd;
   eina_lock_new(&session->lock);
   sessions = eina_list_append(sessions, session);
   // start the monitor thread
   _thread_start(session);
   return session;
}

EAPI Eina_Debug_Session *
eina_debug_remote_connect(int port)
{
   int fd;
   struct sockaddr_in server;

   //Create socket
   fd = socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0) goto err;
   // set the socket to close when we exec things so they don't inherit it
   if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) goto err;

   //Prepare the sockaddr_in structure
   server.sin_family = AF_INET;
   if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr.s_addr) != 1) goto err;
   server.sin_port = htons(port);

   if (connect(fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        goto err;
    }
   return _session_create(fd);
err:
   // some kind of connection failure here, so close a valid socket and
   // get out of here
   if (fd >= 0) close(fd);
   return NULL;
}

EAPI Eina_Debug_Session *
eina_debug_local_connect(Eina_Bool is_master)
{
#ifndef _WIN32
   char buf[4096];
   int fd, socket_unix_len, curstate = 0;
   struct sockaddr_un socket_unix;

   if (is_master) return eina_debug_remote_connect(REMOTE_SERVER_PORT);

   // try this socket file - it will likely be:
   //   ~/.ecore/efl_debug/0
   // or maybe
   //   /var/run/UID/.ecore/efl_debug/0
   snprintf(buf, sizeof(buf), "%s/%s/%s/%i", _socket_home_get(),
         LOCAL_SERVER_PATH, LOCAL_SERVER_NAME, LOCAL_SERVER_PORT);
   // create the socket
   fd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (fd < 0) goto err;
   // set the socket to close when we exec things so they don't inherit it
   if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) goto err;
   // set up some socket options on addr re-use
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&curstate,
                  sizeof(curstate)) < 0)
     goto err;
   // sa that it's a unix socket and where the path is
   memset(&socket_unix, 0, sizeof(socket_unix));
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path) - 1);
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   // actually connect to efl_debugd service
   if (connect(fd, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
      goto err;

   _last_local_session = _session_create(fd);

   return _last_local_session;
err:
   // some kind of connection failure here, so close a valid socket and
   // get out of here
   if (fd >= 0) close(fd);
#else
   (void) is_master;
#endif
   return NULL;
}

// this is a DEDICATED debug thread to monitor the application so it works
// even if the mainloop is blocked or the app otherwise deadlocked in some
// way. this is an alternative to using external debuggers so we can get
// users or developers to get useful information about an app at all times

#ifndef _WIN32
static void *
_monitor(void *_data)
{
   Eina_Debug_Session *session = _data;

   _daemon_greet(session);
   _opcodes_register_all(session);

   // set a name for this thread for system debugging
#ifdef EINA_HAVE_PTHREAD_SETNAME
# ifndef __linux__
   pthread_set_name_np
# else
   pthread_setname_np
# endif
     (pthread_self(), "Edbg-mon");
#endif

   // sit forever processing commands or timeouts in the debug monitor
   // thread - this is separate to the rest of the app so it shouldn't
   // impact the application specifically
   while (1)
     {
        unsigned char *buffer = NULL;
        int size;

        size = _packet_receive(session, &buffer);
        // if not negative - we have a real message
        if (size > 0)
          {
             Eina_Debug_Packet_Header *hdr = (Eina_Debug_Packet_Header *)buffer;
             hdr->cid = SWAP_32(hdr->cid);
             hdr->opcode = SWAP_32(hdr->opcode);
             if (EINA_TRUE != session->dispatch_cb(session, buffer))
               {
                  // something we don't understand
                  e_debug("EINA DEBUG ERROR: Unknown command");
               }
             /* Free the buffer only if the default dispatcher is used */
             if (session->dispatch_cb == eina_debug_dispatch)
                free(buffer);
          }
        else
          {
             _opcodes_unregister_all(session);
             eina_debug_session_terminate(session);
             break;
          }
     }
   return NULL;
}
#endif

// start up the debug monitor if we haven't already
static void
_thread_start(Eina_Debug_Session *session)
{
#ifndef _WIN32
   pthread_t monitor_thread;
   int err;
   sigset_t oldset, newset;

   sigemptyset(&newset);
   sigaddset(&newset, SIGPIPE);
   sigaddset(&newset, SIGALRM);
   sigaddset(&newset, SIGCHLD);
   sigaddset(&newset, SIGUSR1);
   sigaddset(&newset, SIGUSR2);
   sigaddset(&newset, SIGHUP);
   sigaddset(&newset, SIGQUIT);
   sigaddset(&newset, SIGINT);
   sigaddset(&newset, SIGTERM);
#ifdef SIGPWR
   sigaddset(&newset, SIGPWR);
#endif
   pthread_sigmask(SIG_BLOCK, &newset, &oldset);

   err = pthread_create(&monitor_thread, NULL, _monitor, session);

   pthread_sigmask(SIG_SETMASK, &oldset, NULL);
   if (err != 0)
     {
        e_debug("EINA DEBUG ERROR: Can't create monitor debug thread!");
        abort();
     }
#else
   (void)session;
#endif
}

/*
 * Sends to daemon:
 * - Pointer to ops: returned in the response to determine which opcodes have been added
 * - List of opcode names separated by \0
 */
EAPI void
eina_debug_opcodes_register(Eina_Debug_Session *session, const Eina_Debug_Opcode ops[],
      Eina_Debug_Opcode_Status_Cb status_cb, void *data)
{
   if (!session) session = _last_local_session;
   if (!session) return;

   _opcode_reply_info *info = malloc(sizeof(*info));
   info->ops = ops;
   info->status_cb = status_cb;
   info->status_data = data;
   info->sent = EINA_FALSE;

   session->opcode_reply_infos = eina_list_append(
         session->opcode_reply_infos, info);

   /* Send only if session's fd connected.
    * Otherwise, it will be sent when connected */
   if(session && session->fd!= -1)
      _opcodes_registration_send(session, info);
}

EAPI Eina_Bool
eina_debug_dispatch(Eina_Debug_Session *session, void *buffer)
{
   Eina_Debug_Packet_Header *hdr = buffer;
   Eina_List *itr;
   int opcode = hdr->opcode;
   Eina_Debug_Cb cb = NULL;

   if (opcode >= session->cbs_length)
     {
        e_debug("Invalid opcode %d", opcode);
        return EINA_FALSE;
     }

   EINA_LIST_FOREACH(session->cbs[opcode], itr, cb)
     {
        if (!cb) continue;
        Eina_Bool ret = cb(session, hdr->cid,
              (unsigned char *)buffer + sizeof(*hdr),
              hdr->size - sizeof(*hdr));
        if (ret == EINA_FALSE) return ret;
     }
   return EINA_TRUE;
}

EAPI void
eina_debug_session_data_set(Eina_Debug_Session *session, void *data)
{
   if (session) session->data = data;
}

EAPI void *
eina_debug_session_data_get(Eina_Debug_Session *session)
{
   if (session) return session->data;
   else return NULL;
}

Eina_Bool
eina_debug_init(void)
{
   pthread_t self;

   eina_threads_init();
   // For Windows support GetModuleFileName can be used
   // set up thread things
   eina_spinlock_new(&_eina_debug_lock);
   eina_spinlock_new(&_eina_debug_thread_lock);
   self = pthread_self();
   _eina_debug_thread_mainloop_set(&self);
   _eina_debug_thread_add(&self);
   _eina_debug_cpu_init();
   _eina_debug_bt_init();
   _eina_debug_timer_init();
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   // if we are setuid - don't debug!
   if (getuid() != geteuid()) return EINA_TRUE;
#endif
   // if someone uses the EFL_NODEBUG env var or disabled debug - do not do
   // debugging. handy for when this debug code is buggy itself

   if (!getenv("EFL_NODEBUG") && !_debug_disabled)
     {
        eina_debug_local_connect(EINA_FALSE);
     }
   return EINA_TRUE;
}

Eina_Bool
eina_debug_shutdown(void)
{
   Eina_Debug_Session *session;
   pthread_t self = pthread_self();

   EINA_LIST_FREE(sessions, session)
     eina_debug_session_terminate(session);

   _eina_debug_timer_shutdown();
   _eina_debug_bt_shutdown();
   _eina_debug_cpu_shutdown();
   _eina_debug_thread_del(&self);
   eina_spinlock_free(&_eina_debug_lock);
   eina_spinlock_free(&_eina_debug_thread_lock);
   eina_threads_shutdown();
   return EINA_TRUE;
}

EAPI void
eina_debug_fork_reset(void)
{
   extern Eina_Bool fork_resetting;

   fork_resetting = EINA_TRUE;
   eina_debug_shutdown();
   eina_debug_init();
   fork_resetting = EINA_FALSE;
}
