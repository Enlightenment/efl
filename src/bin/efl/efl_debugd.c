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

#define DECLARE_OPS
#include "efl_debug_common.h"

typedef struct _Client Client;

struct _Client
{
   Eo *client;

   unsigned char    *buf;
   unsigned int      buf_size;

   Ecore_Timer      *evlog_fetch_timer;
   int               evlog_on;
   FILE             *evlog_file;

   int               version;
   pid_t             pid;
};

static Eo *server;

static Eina_List *clients = NULL;

static int retval;

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


#define send_cli(cl, op, data, len)                             \
  do                                                            \
    {                                                           \
       if (!send_data(cl->client, OP_ ## op, data, len))        \
         {                                                      \
            if (!efl_io_closer_closed_get(cl->client))          \
              efl_io_closer_close(cl->client);                  \
         }                                                      \
    }                                                           \
  while (0)

static Client *
_client_pid_find(int pid)
{
   Client *c;
   Eina_List *l;

   if (pid <= 0) return NULL;
   EINA_LIST_FOREACH(clients, l, c)
     {
        if (c->pid == pid) return c;
     }

   WRN("no client pid=%d", pid);
   return NULL;
}

static Eina_Bool
_cb_evlog(void *data)
{
   Client *c = data;
   send_cli(c, EVLG, NULL, 0);
   return EINA_TRUE;
}

static void
_process_command(void *data, const char op[static 4], const Eina_Slice payload)
{
   Client *c = data;
   Client *c2;
   Eina_List *l;

   DBG("client %p (%p) [pid:%d] op=%.4s payload=%zd", c, c->client, c->pid, op, payload.len);

   if (IS_OP(HELO))
     {
        if (payload.len < sizeof(int) * 2)
          {
             fprintf(stderr, "INFO: client %p [pid: %d] sent invalid HELO\n", c, (int)c->pid);
             if (!efl_io_closer_closed_get(c->client))
               efl_io_closer_close(c->client);
          }
        else
          {
             memcpy(&c->version, payload.bytes, sizeof(int));
             memcpy(&c->pid, payload.bytes + sizeof(int), sizeof(int));
             INF("client %p (%p) HELO version=%d, pid=%d", c, c->client, c->version, c->pid);
          }
     }
   else if (IS_OP(LIST))
     {
        int n = eina_list_count(clients);
        unsigned int *pids = malloc(n * sizeof(int));
        if (pids)
          {
             int i = 0;

             EINA_LIST_FOREACH(clients, l, c2)
               {
                  if (c2->pid == 0) continue; /* no HELO yet */
                  pids[i] = c2->pid;
                  i++;
               }
             send_cli(c, CLST, pids, i * sizeof(int));
             free(pids);
          }
     }
   else if (IS_OP(PLON))
     {
        if (payload.len < sizeof(int) * 2)
          fprintf(stderr, "INFO: client %p [pid: %d] sent invalid PLON\n", c, (int)c->pid);
        else
          {
             int pid;
             unsigned int freq;
             memcpy(&pid, payload.bytes, sizeof(int));
             memcpy(&freq, payload.bytes + sizeof(int), sizeof(int));
             c2 = _client_pid_find(pid);
             if (!c2)
               {
                  fprintf(stderr, "INFO: client %p [pid: %d] sent PLON %d: no such client\n", c, (int)c->pid, pid);
               }
             else
               {
                  DBG("client %p (%p) [pid:%d] requested PLON on %p (%p) [pid:%d]",
                      c, c->client, c->pid,
                      c2, c2->client, c2->pid);
                  send_cli(c2, PLON, &freq, sizeof(freq));
               }
          }
     }
   else if (IS_OP(PLOF))
     {
        if (payload.len < sizeof(int))
          fprintf(stderr, "INFO: client %p [pid: %d] sent invalid PLOF\n", c, (int)c->pid);
        else
          {
             int pid;
             memcpy(&pid, payload.bytes, sizeof(int));
             c2 = _client_pid_find(pid);
             if (!c2)
               {
                  fprintf(stderr, "INFO: client %p [pid: %d] sent PLOF %d: no such client\n", c, (int)c->pid, pid);
               }
             else
               {
                  DBG("client %p (%p) [pid:%d] requested PLOF on %p (%p) [pid:%d]",
                      c, c->client, c->pid,
                      c2, c2->client, c2->pid);
                  send_cli(c2, PLOF, NULL, 0);
               }
          }
     }
   else if (IS_OP(EVON))
     {
        if (payload.len < sizeof(int))
          fprintf(stderr, "INFO: client %p [pid: %d] sent invalid EVON\n", c, (int)c->pid);
        else
          {
             int pid;
             memcpy(&pid, payload.bytes, sizeof(int));
             c2 = _client_pid_find(pid);
             if (!c2)
               {
                  fprintf(stderr, "INFO: client %p [pid: %d] sent EVON %d: no such client\n", c, (int)c->pid, pid);
               }
             else
               {
                  c2->evlog_on++;
                  DBG("client %p (%p) [pid:%d] requested EVON (%d) on %p (%p) [pid:%d]",
                      c, c->client, c->pid,
                      c2->evlog_on,
                      c2, c2->client, c2->pid);
                  if (c2->evlog_on == 1)
                    {
                       char buf[4096];

                       send_cli(c2, EVON, NULL, 0);
                       c2->evlog_fetch_timer = ecore_timer_add(0.2, _cb_evlog, c2);
                       snprintf(buf, sizeof(buf), "%s/efl_debug_evlog-%d.log",
                                getenv("HOME"), c2->pid);
                       c2->evlog_file = fopen(buf, "wb");
                       DBG("client %p (%p) [pid:%d] logging to %s [%p]",
                           c2, c2->client, c2->pid, buf, c2->evlog_file);
                    }
               }
          }
     }
   else if (IS_OP(EVOF))
     {
        if (payload.len < sizeof(int))
          fprintf(stderr, "INFO: client %p [pid: %d] sent invalid EVOF\n", c, (int)c->pid);
        else
          {
             int pid;
             memcpy(&pid, payload.bytes, sizeof(int));
             c2 = _client_pid_find(pid);
             if (!c2)
               {
                  fprintf(stderr, "INFO: client %p [pid: %d] sent EVOF %d: no such client\n", c, (int)c->pid, pid);
               }
             else
               {
                  c2->evlog_on--;
                  DBG("client %p (%p) [pid:%d] requested EVOF (%d) on %p (%p) [pid:%d]",
                      c, c->client, c->pid,
                      c2->evlog_on,
                      c2, c2->client, c2->pid);
                  if (c2->evlog_on == 0)
                    {
                       send_cli(c2, EVOF, NULL, 0);
                       if (c2->evlog_fetch_timer)
                         {
                            ecore_timer_del(c2->evlog_fetch_timer);
                            c2->evlog_fetch_timer = NULL;
                         }
                       if (c2->evlog_file)
                         {
                            DBG("client %p (%p) [pid:%d] finished logged to %p",
                                c2, c2->client, c2->pid, c2->evlog_file);
                            fclose(c2->evlog_file);
                            c2->evlog_file = NULL;
                         }
                    }
                  else if (c2->evlog_on < 0)
                    c2->evlog_on = 0;
               }
          }
     }
   else if (IS_OP(EVLG))
     {
        if (payload.len < sizeof(int))
          fprintf(stderr, "INFO: client %p [pid: %d] sent invalid EVLG\n", c, (int)c->pid);
        else if (!c->evlog_file)
          fprintf(stderr, "INFO: client %p [pid: %d] no matching EVON\n", c, (int)c->pid);
        else
          {
             unsigned int blocksize = payload.len - sizeof(int);
             if (blocksize > 0)
               {
                  unsigned int header[3];

                  header[0] = 0xffee211;
                  header[1] = blocksize;
                  memcpy(header + 2, payload.mem, sizeof(int));

                  if ((fwrite(header, 12, 1, c->evlog_file) != 1) ||
                      (fwrite(payload.bytes + sizeof(int), blocksize, 1, c->evlog_file) != 1))
                    {
                       fprintf(stderr, "INFO: failed to write log file for client %p [pid: %d]\n", c, (int)c->pid);
                       fclose(c->evlog_file);
                       c->evlog_file = NULL;
                       c->evlog_on = 0;

                       send_cli(c, EVOF, NULL, 0);
                       if (c->evlog_fetch_timer)
                         {
                            ecore_timer_del(c->evlog_fetch_timer);
                            c->evlog_fetch_timer = NULL;
                         }
                    }
               }
          }
     }
}

static void
_client_data(void *data, const Efl_Event *event)
{
   Client *c = data;
   if (!received_data(event->object, _process_command, c))
     {
        fprintf(stderr, "INFO: client %p [pid: %d] sent invalid data\n", c, (int)c->pid);
        if (!efl_io_closer_closed_get(event->object))
          efl_io_closer_close(event->object);
        return;
     }
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
   Client *c = data;

   clients = eina_list_remove(clients, c);
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
   efl_event_callback_array_del(c->client, _client_cbs(), c);
   INF("finished client %p (%p) [pid:%d]", c, c->client, c->pid);
   efl_unref(c->client);
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
   clients = eina_list_append(clients, c);
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
   retval = EXIT_FAILURE;
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eo *loop;
   char *path;
   Eina_Error err;

   ecore_app_no_system_modules();

   eina_init();
   ecore_init();
   ecore_con_init();

   _log_dom = eina_log_domain_register("efl_debugd", EINA_COLOR_CYAN);

   path = ecore_con_local_path_new(EINA_FALSE, "efl_debug", 0);
   if (!path)
     {
        fprintf(stderr, "ERROR: could not get local communication path\n");
        retval = EXIT_FAILURE;
        goto end;
     }

   loop = ecore_main_loop_get();

#ifdef EFL_NET_SERVER_UNIX_CLASS
   server = efl_add(EFL_NET_SERVER_SIMPLE_CLASS, loop,
                    efl_net_server_simple_inner_class_set(efl_added, EFL_NET_SERVER_UNIX_CLASS));
#else
   /* TODO: maybe start a TCP using locahost:12345?
    * Right now eina_debug_monitor is only for AF_UNIX, so not an issue.
    */
   fprintf(stderr, "ERROR: your platform doesn't support Efl.Net.Server.Unix\n");
#endif
   if (!server)
     {
        fprintf(stderr, "ERROR: could not create communication server\n");
        retval = EXIT_FAILURE;
        goto end;
     }

   efl_event_callback_add(server, EFL_NET_SERVER_EVENT_CLIENT_ADD, _client_add, NULL);
   efl_event_callback_add(server, EFL_NET_SERVER_EVENT_ERROR, _error, NULL);

#ifdef EFL_NET_SERVER_UNIX_CLASS
   {
      Eo *inner_server = efl_net_server_simple_inner_server_get(server);
      efl_net_server_unix_unlink_before_bind_set(inner_server, EINA_TRUE);
      efl_net_server_unix_leading_directories_create_set(inner_server, EINA_TRUE, 0700);
   }
#endif

   err = efl_net_server_serve(server, path);
   if (err)
     {
        fprintf(stderr, "ERROR: could not serve '%s': %s\n", path, eina_error_msg_get(err));
        retval = EXIT_FAILURE;
        goto end;
     }

   ecore_main_loop_begin();

 end:
   efl_del(server);
   free(path);

   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return retval;
}
