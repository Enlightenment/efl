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

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#include <Eina.h>
#include <Ecore.h>

#define STORE(_buf, pval, sz) \
{ \
   memcpy(_buf, pval, sz); \
   _buf += sz; \
}

#define EXTRACT(_buf, pval, sz) \
{ \
   memcpy(pval, _buf, sz); \
   _buf += sz; \
}

typedef struct _Client Client;

struct _Client
{
   Eina_Stringshare *app_name;

   Ecore_Timer      *evlog_fetch_timer;
   int               evlog_on;
   FILE             *evlog_file;

   int               version;
   int               fd;
   int               cid;
   pid_t             pid;

   Eina_Bool         cl_stat_obs : 1;
   Eina_Bool         is_master : 1;
};

static Eina_List *_clients = NULL;

typedef Eina_Bool (*Opcode_Cb)(Client *client, void *buffer, int size);

static Eina_Hash *_string_to_opcode_hash = NULL;

static int _free_cid = 1;

static int _clients_stat_register_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _slave_added_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _slave_deleted_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _cid_from_pid_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _test_loop_opcode = EINA_DEBUG_OPCODE_INVALID;

typedef struct
{
   int opcode;
   Eina_Stringshare *opcode_string;
   Opcode_Cb cb;
} Opcode_Information;

#define MAX_OPCODES 1000
Opcode_Information *_opcodes[MAX_OPCODES];

/* epoll stuff */
#ifndef _WIN32
static int _epfd = -1, _listening_master_fd = -1, _listening_slave_fd = -1;
#endif

static Client *
_client_find_by_cid(int cid)
{
   Client *c;
   Eina_List *l;
   EINA_LIST_FOREACH(_clients, l, c)
      if (c->cid == cid) return c;
   return NULL;
}

static Client *
_client_find_by_pid(int pid)
{
   Client *c;
   Eina_List *l;
   EINA_LIST_FOREACH(_clients, l, c)
      if (c->pid == pid) return c;
   return NULL;
}

static Client *
_client_find_by_fd(int fd)
{
   Eina_List *itr;
   Client *c;
   EINA_LIST_FOREACH(_clients, itr, c)
      if (c->fd == fd) return c;
   return NULL;
}

static int
_send(Client *dest, int opcode, void *payload, int payload_size)
{
   int size = sizeof(Eina_Debug_Packet_Header) + payload_size;
   char *buf = alloca(size);
   Eina_Debug_Packet_Header *hdr = (Eina_Debug_Packet_Header *)buf;
   hdr->size = size;
   hdr->cid = 0;
   hdr->thread_id = 0;
   hdr->opcode = opcode;
   memcpy(buf + sizeof(Eina_Debug_Packet_Header), payload, payload_size);
   //printf("%d bytes sent (opcode %s) to %s fd %d\n", size, _opcodes[opcode]->opcode_string, dest->app_name, dest->fd);
   return send(dest->fd, buf, size, 0);
}

static void
_client_del(Client *c)
{
   Client *c2;
   if (!c) return;
   Eina_List *itr;

   _clients = eina_list_remove(_clients, c);
   if (c->evlog_fetch_timer)
     {
        ecore_timer_del(c->evlog_fetch_timer);
        c->evlog_fetch_timer = NULL;
     }
   if (c->evlog_file)
     {
        fclose(c->evlog_file);
        c->evlog_file = NULL;
     }

   /* Don't update the observers if the client is a master */
   if (c->is_master) return;

   EINA_LIST_FOREACH(_clients, itr, c2)
     {
        if (c2->cl_stat_obs) _send(c2, _slave_deleted_opcode, &c->cid, sizeof(int));
     }
   free(c);
}

static Eina_Bool
_dispatch(Client *src, void *buffer, int size)
{
   Eina_Debug_Packet_Header *hdr = (Eina_Debug_Packet_Header *)buffer;
   if (hdr->cid)
     {
        /* If the client id is given, we forward */
        Client *dest = _client_find_by_cid(hdr->cid);
        if (dest)
          {
             if (dest->is_master != src->is_master)
               {
                  hdr->cid = src->cid;
                  send(dest->fd, buffer, size, 0);
               }
             else
               {
                  /*
                   * Packets Master -> Master or Slave -> Slave are forbidden
                   * Only Master <-> Slave packets are allowed.
                   */
                  printf("Packet from %d to %d: denied (same type)\n", hdr->cid, dest->cid);
               }
          }
     }
   else
     {
        printf("Invoke %s\n", _opcodes[hdr->opcode]->opcode_string);
        if (_opcodes[hdr->opcode]->cb)
           return _opcodes[hdr->opcode]->cb(src,
                 (char *)buffer + sizeof(Eina_Debug_Packet_Header), size - sizeof(Eina_Debug_Packet_Header));
     }
   return EINA_TRUE;
}

static int
_opcode_register(const char *op_name, int op_id, Opcode_Cb cb)
{
   static int free_opcode = 0;
   Opcode_Information *op_info = eina_hash_find(_string_to_opcode_hash, op_name);
   if (!op_info)
     {
        op_info = calloc(1, sizeof(*op_info));
        if (op_id == EINA_DEBUG_OPCODE_INVALID)
          {
             do
               {
                  free_opcode = (free_opcode + 1) % MAX_OPCODES;
                  op_id = free_opcode;
               }
             while(_opcodes[op_id]);
          }
        op_info->opcode = op_id;
        op_info->opcode_string = eina_stringshare_add(op_name);
        op_info->cb = cb;
        eina_hash_add(_string_to_opcode_hash, op_name, op_info);
        _opcodes[op_id] = op_info;
     }
   printf("Register %s -> opcode %d\n", op_name, op_info->opcode);
   return op_info->opcode;
}

static Eina_Bool
_hello_cb(Client *c, void *buffer, int size)
{
   Eina_List *itr;
   char *buf = (char *)buffer, *tmp;

   EXTRACT(buf, &c->version, 4);
   EXTRACT(buf, &c->pid, 4);
   size -= 8;

   c->cid = _free_cid++;
   if (size > 1)
     {
        c->app_name = eina_stringshare_add_length(buf, size);
     }
   printf("Connection from %s: pid %d - name %s\n",
         c->is_master ? "Master" : "Slave",
         c->pid, c->app_name);

   if (c->is_master) return EINA_TRUE;

   /* Update the observers */
   size = 2 * sizeof(int) + (c->app_name ? strlen(c->app_name) : 0) + 1; /* cid + pid + name + \0 */
   buf = alloca(size);
   tmp = buf;
   STORE(tmp, &c->cid, sizeof(int));
   STORE(tmp, &c->pid, sizeof(int));
   if (c->app_name)
     {
        STORE(tmp, c->app_name, strlen(c->app_name) + 1);
     }
   else
     {
        char end = '\0';
        STORE(tmp, &end, 1);
     }
   EINA_LIST_FOREACH(_clients, itr, c)
     {
        if (c->cl_stat_obs) _send(c, _slave_added_opcode, buf, size);
     }
   return EINA_TRUE;
}

static Eina_Bool
_cid_get_cb(Client *src, void *buffer, int size EINA_UNUSED)
{
   int pid = *(int *)buffer;
   Client *c = _client_find_by_pid(pid);
   int cid = c ? c->cid : 0;
   _send(src, _cid_from_pid_opcode, &cid, sizeof(int));
   return EINA_TRUE;
}

static Eina_Bool
_data_test_cb(Client *src, void *buffer, int size)
{
   printf("Data test: loop packet of %d bytes\n", size);
   _send(src, _test_loop_opcode, buffer, size);
   return EINA_TRUE;
}

static Eina_Bool
_cl_stat_obs_register_cb(Client *src, void *buffer, int size)
{
   Client *c;
   if (!src) return EINA_FALSE;
   if (!src->is_master) return EINA_FALSE;
   if (!src->cl_stat_obs)
     {
        Eina_List *itr;
        src->cl_stat_obs = EINA_TRUE;
        size = 0;
        EINA_LIST_FOREACH(_clients, itr, c)
          {
             char *tmp;
             if (c->is_master) continue;
             size = 2 * sizeof(int) + (c->app_name ? strlen(c->app_name) : 0) + 1;
             buffer = alloca(size);
             tmp = buffer;
             STORE(tmp, &c->cid, sizeof(int));
             STORE(tmp, &c->pid, sizeof(int));
             if (c->app_name)
               {
                  STORE(tmp, c->app_name, strlen(c->app_name) + 1);
               }
             else
               {
                  char end = '\0';
                  STORE(tmp, &end, 1);
               }
             _send(src, _slave_added_opcode, buffer, size);
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_opcode_register_cb(Client *src, void *buffer, int size)
{
   char *buf = (char *)buffer;
   char *ops_buf = buf;
   int ops_size = size;

   ops_buf += sizeof(uint64_t);
   ops_size -= sizeof(uint64_t);
   int *opcodes = (int *)ops_buf;

   while (ops_size > 0)
     {
        int len = strlen(ops_buf) + 1;
        *opcodes++ = _opcode_register(ops_buf, EINA_DEBUG_OPCODE_INVALID, NULL);
        ops_buf += len;
        ops_size -= len;
     }

   _send(src, EINA_DEBUG_OPCODE_REGISTER, buf, (char *)opcodes - (char *)buf);

   return EINA_TRUE;
}

static int
_data_receive(Client *c, unsigned char **buffer)
{
   unsigned char *recv_buf = NULL;
   int rret;
   int size = 0;

   if (!c) return -1;

   rret = recv(c->fd, &size, sizeof(int), MSG_PEEK);

   if (rret == sizeof(int))
     {
        int cur_packet_size = 0;
        // allocate a buffer for the next bytes to receive
        recv_buf = malloc(size);
        if (!recv_buf) goto error;
        while (cur_packet_size < size)
          {
             rret = recv(c->fd, recv_buf + cur_packet_size, size - cur_packet_size, 0);
             if (rret <= 0) goto error;
             cur_packet_size += rret;
          }
     }
   if (buffer) *buffer = recv_buf;
   //printf("%d bytes received from client %s fd %d\n", size, c->app_name, c->fd);
   return size;
error:
   if (rret == -1) perror("Read from socket");
   if (recv_buf) free(recv_buf);
   return -1;
}

static void
_monitor()
{
#ifndef _WIN32
#define MAX_EVENTS 1000
   int ret = 0;
   struct epoll_event events[MAX_EVENTS];
   Client *c;

   // sit forever processing commands or timeouts
   for (; ret != -1;)
     {
        ret = epoll_wait (_epfd, events, MAX_EVENTS, -1);

        // if the fd for debug daemon says it's alive, process it
        if (ret > 0)
          {
             int i;
             //check which fd are set/ready for read
             for (i = 0; i < ret; i++)
               {
                  if (events[i].events & EPOLLHUP)
                    {
                       c = _client_find_by_fd(events[i].data.fd);
                       close(events[i].data.fd);
                       if (c) _client_del(c);
                    }
                  if (events[i].events & EPOLLIN)
                    {
                       // Someone wants to connect
                       if(events[i].data.fd == _listening_master_fd || events[i].data.fd == _listening_slave_fd)
                         {
                            int new_fd = accept(events[i].data.fd, NULL, NULL);
                            if (new_fd < 0) perror("Accept");
                            else
                              {
                                 struct epoll_event event;
                                 c = calloc(1, sizeof(*c));
                                 c->fd = new_fd;
                                 c->is_master = (events[i].data.fd == _listening_master_fd);
                                 _clients = eina_list_append(_clients, c);
                                 event.data.fd = new_fd;
                                 event.events = EPOLLIN;
                                 epoll_ctl (_epfd, EPOLL_CTL_ADD, new_fd, &event);
                              }
                            continue;
                         }

                       c = _client_find_by_fd(events[i].data.fd);
                       if (c)
                          {
                             int size;
                             unsigned char *buffer;
                             size = _data_receive(c, &buffer);
                             // if not negative - we have a real message
                             if (size > 0)
                               {
                                  if(!_dispatch(c, buffer, size))
                                    {
                                       // something we don't understand
                                       fprintf(stderr, "Dispatch: unknown command");
                                    }
                                  free(buffer);
                               }
                             else
                               {
                                  // major failure on debug daemon control fd - get out of here.
                                  //   else goto fail;
                                  close(events[i].data.fd);
                                  //TODO if its not main session we will tell the main_loop
                                  //that it disconneted
                               }
                          }
                    }
               }
          }
#if 0
        else
          {
             if (poll_time && poll_timer_cb)
               {
                  if (!poll_timer_cb()) poll_time = 0;
               }
          }
#endif
     }
#endif
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

#ifndef _WIN32
#define LENGTH_OF_SOCKADDR_UN(s) \
   (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))
static int
_local_listening_socket_create(const char *path)
{
   struct sockaddr_un socket_unix;
   int socket_unix_len, curstate = 0;
   // create the socket
   int fd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (fd < 0) goto err;
   // set the socket to close when we exec things so they don't inherit it
   if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) goto err;
   // set up some socket options on addr re-use
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&curstate,
                  sizeof(curstate)) < 0)
     goto err;
   // sa that it's a unix socket and where the path is
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, path, sizeof(socket_unix.sun_path) - 1);
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   unlink(socket_unix.sun_path);
   if (bind(fd, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
     {
        perror("ERROR on binding");
        goto err;
     }
   listen(fd, 5);
   return fd;
err:
   if (fd >= 0) close(fd);
   return -1;
}
#endif

static Eina_Bool
_server_launch()
{
#ifndef _WIN32
   char buf[4096];
   struct epoll_event event = {0};
   mode_t mask = 0;
   const char *socket_home_path = _socket_home_get();
   char *socket_path = NULL;
   if (!socket_home_path) return EINA_FALSE;
   _epfd = epoll_create (MAX_EVENTS);
   socket_path = strdup(socket_home_path);

   snprintf(buf, sizeof(buf), "%s/%s", socket_path, SERVER_PATH);
   if (mkdir(buf, S_IRWXU) < 0 && errno != EEXIST)
     {
        perror("mkdir SERVER_PATH");
        goto err;
     }
   snprintf(buf, sizeof(buf), "%s/%s/%s", socket_path, SERVER_PATH, SERVER_NAME);
   if (mkdir(buf, S_IRWXU) < 0 && errno != EEXIST)
     {
        perror("mkdir SERVER_NAME");
        goto err;
     }
   mask = umask(S_IRWXG | S_IRWXO);
   snprintf(buf, sizeof(buf), "%s/%s/%s/%i", socket_path, SERVER_PATH, SERVER_NAME, SERVER_MASTER_PORT);
   _listening_master_fd = _local_listening_socket_create(buf);
   if (_listening_master_fd <= 0) goto err;
   event.data.fd = _listening_master_fd;
   event.events = EPOLLIN;
   epoll_ctl (_epfd, EPOLL_CTL_ADD, _listening_master_fd, &event);
   snprintf(buf, sizeof(buf), "%s/%s/%s/%i", socket_path, SERVER_PATH, SERVER_NAME, SERVER_SLAVE_PORT);
   _listening_slave_fd = _local_listening_socket_create(buf);
   if (_listening_slave_fd <= 0) goto err;
   event.data.fd = _listening_slave_fd;
   event.events = EPOLLIN;
   epoll_ctl (_epfd, EPOLL_CTL_ADD, _listening_slave_fd, &event);
   umask(mask);
   return EINA_TRUE;
err:
   if (mask) umask(mask);
   if (_listening_master_fd >= 0) close(_listening_master_fd);
   _listening_master_fd = -1;
   if (_listening_slave_fd >= 0) close(_listening_slave_fd);
   _listening_slave_fd = -1;
   free(socket_path);
#endif
   return EINA_FALSE;
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   eina_debug_disable();
   eina_init();
   ecore_init();

   _string_to_opcode_hash = eina_hash_string_superfast_new(NULL);
   _opcode_register("daemon/opcode/register", EINA_DEBUG_OPCODE_REGISTER, _opcode_register_cb);
   _opcode_register("daemon/greet", EINA_DEBUG_OPCODE_HELLO, _hello_cb);
   _clients_stat_register_opcode = _opcode_register("daemon/observer/client/register", EINA_DEBUG_OPCODE_INVALID, _cl_stat_obs_register_cb);
   _slave_added_opcode = _opcode_register("daemon/observer/slave_added", EINA_DEBUG_OPCODE_INVALID, NULL);
   _slave_deleted_opcode = _opcode_register("daemon/observer/slave_deleted", EINA_DEBUG_OPCODE_INVALID, NULL);
   _cid_from_pid_opcode = _opcode_register("daemon/info/cid_from_pid", EINA_DEBUG_OPCODE_INVALID, _cid_get_cb);
   _test_loop_opcode = _opcode_register("daemon/test/loop", EINA_DEBUG_OPCODE_INVALID, _data_test_cb);

   _server_launch();
   _monitor();

   ecore_shutdown();
   eina_shutdown();

   return 0;
}
