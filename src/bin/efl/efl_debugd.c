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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_BETA_API_SUPPORT 1

#include <fcntl.h>
#include "eina_debug_private.h"

#ifdef _WIN32
# include <evil_private.h> /* mkdir */
#endif
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>

static int _log_dom = -1;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_log_dom, __VA_ARGS__)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SWAP_64(x) x
#define SWAP_32(x) x
#define SWAP_16(x) x
#else
#define SWAP_64(x) eina_swap64(x)
#define SWAP_32(x) eina_swap32(x)
#define SWAP_16(x) eina_swap16(x)
#endif

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
   Eo *              client;
   Eina_Stringshare *app_name;

   int               version;
   int               cid;
   pid_t             pid;

   Eina_Bool         cl_stat_obs : 1;
   Eina_Bool         is_master : 1;
};

static Eina_List *_clients = NULL;
static int _retval;

static Eo *_local_server = NULL, *_remote_server = NULL;

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

static void
_send(Client *dest, int opcode, void *payload, int payload_size)
{
   Eina_Error err;
   Eina_Slice s, r;
   Eina_Debug_Packet_Header hdr;
   int size = sizeof(Eina_Debug_Packet_Header) + payload_size;

   hdr.size = SWAP_32(size);
   hdr.cid = 0;
   hdr.opcode = SWAP_32(opcode);

   s.mem = &hdr;
   s.len = sizeof(hdr);

   err = efl_io_writer_write(dest->client, &s, &r);
   if (err || r.len) goto end;

   if (!payload_size) goto end;

   s.mem = payload;
   s.len = payload_size;
   err = efl_io_writer_write(dest->client, &s, &r);

   INF("Send packet (size = %d, opcode %s) to %s", size,
         _opcodes[opcode]->opcode_string,
         dest->app_name);

 end:
   if (err)
     {
        fprintf(stderr, "ERROR: could not queue message '%d': %s\n", opcode, eina_error_msg_get(err));
     }

   if (r.len)
     {
        fprintf(stderr, "ERROR: could not queue message '%d': out of memory\n", opcode);
     }
}

static Eina_Bool
_dispatch(Client *src, void *buffer)
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
                  Eina_Slice s;
                  s.mem = buffer;
                  s.len = hdr->size;
                  hdr->cid = SWAP_32(src->cid);
                  hdr->size = SWAP_32(hdr->size);
                  hdr->opcode = SWAP_32(hdr->opcode);
                  if (efl_io_writer_write(dest->client, &s, NULL) !=
                      EINA_ERROR_NO_ERROR)
                    ERR("Cannot write to client [%s:%i]", dest->app_name, (int)dest->pid);
                  INF("Transfer of %d bytes from %s(%d) to %s(%d): operation %s\n",
                        hdr->size,
                        src->app_name, src->pid,
                        dest->app_name, dest->pid,
                        _opcodes[hdr->opcode]->opcode_string);
               }
             else
               {
                  /*
                   * Packets Master -> Master or Slave -> Slave are forbidden
                   * Only Master <-> Slave packets are allowed.
                   */
                  ERR("Packet from %d to %d: denied (same type)\n", hdr->cid, dest->cid);
               }
          }
     }
   else
     {
        INF("Invoke %s\n", _opcodes[hdr->opcode]->opcode_string);
        if (_opcodes[hdr->opcode]->cb)
           return _opcodes[hdr->opcode]->cb(src,
                 (char *)buffer + sizeof(Eina_Debug_Packet_Header),
                 hdr->size - sizeof(Eina_Debug_Packet_Header));
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
   INF("Register %s -> opcode %d\n", op_name, op_info->opcode);
   return op_info->opcode;
}

static Eina_Bool
_hello_cb(Client *c, void *buffer, int size)
{
   Eina_List *itr;
   char *buf = (char *)buffer, *tmp;
   int version, pid, cid;

   EXTRACT(buf, &version, 4);
   EXTRACT(buf, &pid, 4);
   c->version = SWAP_32(version);
   c->pid = SWAP_32(pid);
   size -= 8;

   c->cid = _free_cid++;
   cid = SWAP_32(c->cid);
   if (size > 1)
     {
        c->app_name = eina_stringshare_add_length(buf, size);
     }
   INF("Connection of %s: pid %d - name %s -> cid %d\n",
         c->is_master ? "Master" : "Slave",
         c->pid, c->app_name, c->cid);

   if (c->is_master) return EINA_TRUE;

   /* Update the observers */
   size = 2 * sizeof(int) + (c->app_name ? strlen(c->app_name) : 0) + 1; /* cid + pid + name + \0 */
   buf = alloca(size);
   tmp = buf;
   STORE(tmp, &cid, sizeof(int));
   STORE(tmp, &pid, sizeof(int));
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
   int pid = SWAP_32(*(int *)buffer);
   Client *c = _client_find_by_pid(pid);
   int cid = c ? SWAP_32(c->cid) : 0;
   _send(src, _cid_from_pid_opcode, &cid, sizeof(int));
   return EINA_TRUE;
}

static Eina_Bool
_data_test_cb(Client *src, void *buffer, int size)
{
   DBG("Data test: loop packet of %d bytes\n", size);
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
             int cid, pid;
             if (c->is_master) continue;
             size = 2 * sizeof(int) + (c->app_name ? strlen(c->app_name) : 0) + 1;
             buffer = alloca(size);
             tmp = buffer;
             cid = SWAP_32(c->cid);
             pid = SWAP_32(c->pid);
             STORE(tmp, &cid, sizeof(int));
             STORE(tmp, &pid, sizeof(int));
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
        *opcodes++ = SWAP_32(_opcode_register(ops_buf, EINA_DEBUG_OPCODE_INVALID, NULL));
        ops_buf += len;
        ops_size -= len;
     }

   _send(src, EINA_DEBUG_OPCODE_REGISTER, buf, (char *)opcodes - (char *)buf);

   return EINA_TRUE;
}

static void
_client_data(void *data, const Efl_Event *event)
{
   static unsigned char *buffer = NULL;
   unsigned int size = 0;
   Eina_Debug_Packet_Header *hdr;
   Client *c = data;
   Eina_Slice slice;

   if (!c) return;

   if (!buffer) buffer = malloc(EINA_DEBUG_MAX_PACKET_SIZE);

   slice = efl_io_buffered_stream_slice_get(c->client);

   if (slice.len < sizeof(*hdr)) return;

   hdr = (Eina_Debug_Packet_Header *)slice.mem;
   size = SWAP_32(hdr->size);
   if (size < sizeof(*hdr)) /* must contain at least the header */
     {
        fprintf(stderr, "ERROR: invalid message header, size=%u\n", hdr->size);
        goto err;
     }

   if (size > EINA_DEBUG_MAX_PACKET_SIZE)
     {
        fprintf(stderr, "ERROR: packet too big (max: %d), size=%u\n",
              EINA_DEBUG_MAX_PACKET_SIZE, hdr->size);
        goto err;
     }

   /* Incomplete packet: need to wait */
   if (size > slice.len) return;

   memcpy(buffer, slice.mem, size);
   hdr = (Eina_Debug_Packet_Header *)buffer;
   hdr->size = SWAP_32(hdr->size);
   hdr->opcode = SWAP_32(hdr->opcode);
   hdr->cid = SWAP_32(hdr->cid);

   if(!_dispatch(c, buffer))
     {
        // something we don't understand
        fprintf(stderr, "Dispatch: unknown command: %d\n", hdr->opcode);
     }
   efl_io_buffered_stream_discard(c->client, size);
   return;
err:
   if (!efl_io_closer_closed_get(event->object))
      efl_io_closer_close(event->object);
   fprintf(stderr, "INFO: client %p [pid: %d] sent invalid data\n", c, (int)c->pid);
}

static void
_client_error(void *data, const Efl_Event *event)
{
   Client *c = data;
   Eina_Error *perr = event->info;
   WRN("client %p [pid: %d] error: %s",
       c, (int)c->pid, eina_error_msg_get(*perr));
   fprintf(stderr, "INFO: client %p [pid: %d] error: %s\n",
           c, (int)c->pid, eina_error_msg_get(*perr));
}

static void
_client_eos(void *data, const Efl_Event *event EINA_UNUSED)
{
   Client *c = data;
   DBG("client %p (%p) [pid: %d] closed, pending read %zu, write %zu",
       c, c->client, (int)c->pid,
       efl_io_buffered_stream_pending_read_get(c->client),
       efl_io_buffered_stream_pending_write_get(c->client));
   efl_io_closer_close(c->client);
}

static void
_client_write_finished(void *data, const Efl_Event *event EINA_UNUSED)
{
   Client *c = data;
   DBG("client %p (%p) [pid: %d] finished writing, pending read %zu",
       c, c->client, (int)c->pid, efl_io_buffered_stream_pending_read_get(c->client));
}

static void
_client_read_finished(void *data, const Efl_Event *event EINA_UNUSED)
{
   Client *c = data;
   DBG("client %p (%p) [pid: %d] finished reading, pending write %zu",
       c, c->client, (int)c->pid, efl_io_buffered_stream_pending_write_get(c->client));
}

static Efl_Callback_Array_Item *_client_cbs(void);

static void
_client_finished(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eina_List *itr;
   Client *c = data, *c2;
   int cid = SWAP_32(c->cid);
   efl_event_callback_array_del(c->client, _client_cbs(), c);
   INF("finished client %p (%p) [pid:%d]", c, c->client, c->pid);
   _clients = eina_list_remove(_clients, c);
   efl_unref(c->client);

   /* Don't update the observers if the client is a master */
   if (c->is_master) return;

   EINA_LIST_FOREACH(_clients, itr, c2)
     {
        if (c2->cl_stat_obs) _send(c2, _slave_deleted_opcode, &cid, sizeof(int));
     }
   free(c);
}

EFL_CALLBACKS_ARRAY_DEFINE(_client_cbs,
                           { EFL_IO_READER_EVENT_EOS, _client_eos },
                           { EFL_IO_BUFFERED_STREAM_EVENT_ERROR, _client_error },
                           { EFL_IO_BUFFERED_STREAM_EVENT_READ_FINISHED, _client_read_finished },
                           { EFL_IO_BUFFERED_STREAM_EVENT_WRITE_FINISHED, _client_write_finished },
                           { EFL_IO_BUFFERED_STREAM_EVENT_FINISHED, _client_finished },
                           { EFL_IO_BUFFERED_STREAM_EVENT_SLICE_CHANGED, _client_data });

static void
_client_add(void *data EINA_UNUSED, const Efl_Event *event)
{
   Client *c = calloc(1, sizeof(Client));

   EINA_SAFETY_ON_NULL_RETURN(c);
   c->client = efl_ref(event->info);
   c->is_master = (event->object == _remote_server);
   _clients = eina_list_append(_clients, c);
   efl_event_callback_array_add(c->client, _client_cbs(), c);
   INF("server %p new client %p (%p)", event->object, c, c->client);
}

static void
_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Error *perr = event->info;
   ERR("server %p error: %s", event->object, eina_error_msg_get(*perr));
   fprintf(stderr, "ERROR: %s\n", eina_error_msg_get(*perr));
   ecore_main_loop_quit();
   _retval = EXIT_FAILURE;
}

static Eina_Bool
_local_server_create(void)
{
   Eo *loop;
   Eina_Error err;
   mode_t mask = 0;
   char path[512];
   Eina_Bool ret = EINA_FALSE;

   eina_vpath_resolve_snprintf(path, sizeof(path), "(:usr.run:)/%s", LOCAL_SERVER_PATH);
   if (mkdir(path, S_IRWXU) < 0 && errno != EEXIST)
     {
        perror("mkdir SERVER_PATH");
        goto end;
     }
   eina_vpath_resolve_snprintf(path, sizeof(path), "(:usr.run:)/%s/%s", LOCAL_SERVER_PATH, LOCAL_SERVER_NAME);
   if (mkdir(path, S_IRWXU) < 0 && errno != EEXIST)
     {
        perror("mkdir SERVER_NAME");
        goto end;
     }
   mask = umask(S_IRWXG | S_IRWXO);
   eina_vpath_resolve_snprintf(path, sizeof(path) - 1, "(:usr.run:)/%s/%s/%i",
         LOCAL_SERVER_PATH, LOCAL_SERVER_NAME, LOCAL_SERVER_PORT);

   loop = efl_main_loop_get();

#ifdef EFL_NET_SERVER_UNIX_CLASS
   _local_server = efl_add(EFL_NET_SERVER_SIMPLE_CLASS, loop,
                    efl_net_server_simple_inner_class_set(efl_added, EFL_NET_SERVER_UNIX_CLASS));
#else
   /* TODO: maybe start a TCP using locahost:12345?
    * Right now eina_debug_monitor is only for AF_UNIX, so not an issue.
    */
   fprintf(stderr, "ERROR: your platform doesn't support Efl.Net.Server.Unix\n");
#endif
   if (!_local_server)
     {
        fprintf(stderr, "ERROR: could not create communication server\n");
        goto end;
     }

   efl_event_callback_add(_local_server, EFL_NET_SERVER_EVENT_CLIENT_ADD, _client_add, NULL);
   efl_event_callback_add(_local_server, EFL_NET_SERVER_EVENT_SERVER_ERROR, _error, NULL);

#ifdef EFL_NET_SERVER_UNIX_CLASS
   {
      Eo *inner_server = efl_net_server_simple_inner_server_get(_local_server);
      efl_net_server_unix_leading_directories_create_set(inner_server, EINA_TRUE, 0700);
   }
#endif

   err = efl_net_server_serve(_local_server, path);
   if (err)
     {
        fprintf(stderr, "ERROR: could not serve '%s': %s\n", path, eina_error_msg_get(err));
        goto end;
     }
   ret = EINA_TRUE;
end:
   umask(mask);
   if (!ret)
     {
        efl_del(_local_server);
        _local_server = NULL;
     }
   return ret;
}

static Eina_Bool
_remote_server_create(void)
{
   Eo *loop;
   Eina_Error err;
   mode_t mask = 0;
   Eina_Bool ret = EINA_FALSE;
   char address[256];

   loop = efl_main_loop_get();

   _remote_server = efl_add(EFL_NET_SERVER_SIMPLE_CLASS, loop,
                    efl_net_server_simple_inner_class_set(efl_added, EFL_NET_SERVER_TCP_CLASS));
   if (!_remote_server)
     {
        fprintf(stderr, "ERROR: could not create communication server\n");
        goto end;
     }

   {
      Eo *inner_server = efl_net_server_simple_inner_server_get(_remote_server);
      efl_net_server_fd_reuse_address_set(inner_server, EINA_TRUE);
   }
   efl_event_callback_add(_remote_server, EFL_NET_SERVER_EVENT_CLIENT_ADD, _client_add, NULL);
   efl_event_callback_add(_remote_server, EFL_NET_SERVER_EVENT_SERVER_ERROR, _error, NULL);

   sprintf(address, "127.0.0.1:%d", REMOTE_SERVER_PORT);
   err = efl_net_server_serve(_remote_server, address);
   if (err)
     {
        fprintf(stderr, "ERROR: could not serve port '%d': %s\n",
              REMOTE_SERVER_PORT, eina_error_msg_get(err));
        goto end;
     }
   ret = EINA_TRUE;
end:
   umask(mask);
   if (!ret)
     {
        efl_del(_remote_server);
        _remote_server = NULL;
     }
   return ret;
}

static Eina_Bool
_server_launch(void)
{
   if (_local_server_create() <= 0) goto err;
   if (_remote_server_create() <= 0) goto err;

   return EINA_TRUE;
err:
   efl_del(_local_server);
   efl_del(_remote_server);
   return EINA_FALSE;
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   eina_debug_disable();
   ecore_app_no_system_modules();

   eina_init();
   ecore_init();
   ecore_con_init();

   _retval = EXIT_SUCCESS;
   _log_dom = eina_log_domain_register("efl_debugd", EINA_COLOR_CYAN);

   _string_to_opcode_hash = eina_hash_string_superfast_new(NULL);
   _opcode_register("Daemon/opcode_register", EINA_DEBUG_OPCODE_REGISTER, _opcode_register_cb);
   _opcode_register("Daemon/greet", EINA_DEBUG_OPCODE_HELLO, _hello_cb);
   _clients_stat_register_opcode = _opcode_register("Daemon/Client/register_observer", EINA_DEBUG_OPCODE_INVALID, _cl_stat_obs_register_cb);
   _slave_added_opcode = _opcode_register("Daemon/Client/added", EINA_DEBUG_OPCODE_INVALID, NULL);
   _slave_deleted_opcode = _opcode_register("Daemon/Client/deleted", EINA_DEBUG_OPCODE_INVALID, NULL);
   _cid_from_pid_opcode = _opcode_register("Daemon/Client/cid_from_pid", EINA_DEBUG_OPCODE_INVALID, _cid_get_cb);
   _test_loop_opcode = _opcode_register("Test/data_loop", EINA_DEBUG_OPCODE_INVALID, _data_test_cb);

   if (_server_launch()) ecore_main_loop_begin();
   else _retval = EXIT_FAILURE;

   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return _retval;
}
