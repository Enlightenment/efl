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

#include "efl_debug_common.h"

static Eo *dialer;

static Eina_List *waiting;

static int retval = EXIT_SUCCESS;

static void
_process_reply(void *data EINA_UNUSED, const char op[static 4], const Eina_Slice payload)
{
   if (IS_OP(CLST))
     {
        int mypid = getpid();
        size_t offset;

        waiting = eina_list_remove(waiting, OP_CLST);

        for (offset = 0; offset + sizeof(int) <= payload.len; offset += sizeof(int))
          {
             int p;

             memcpy(&p, payload.bytes + offset, sizeof(int));

             if (p == mypid) continue;
             if (p > 0) printf("%i\n", p);
          }
     }
   else
     {
        fprintf(stderr, "ERROR: unexpected server reply: %.4s\n", op);
        retval = EXIT_FAILURE;
     }

   if (!waiting) ecore_main_loop_quit();
}

static void
_on_data(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   if (!received_data(dialer, _process_reply, NULL))
     {
        retval = EXIT_FAILURE;
        ecore_main_loop_quit();
     }
}

static Eina_Bool
_command_send(const char op[static 4], const void *data, unsigned int len)
{
   if (!send_data(dialer, op, data, len))
     {
        retval = EXIT_FAILURE;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

#define command_send(op, data, len) _command_send(OP_ ## op, data, len)

static void
_write_finished(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   if (!waiting) ecore_main_loop_quit();
}

static void
_finished(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Error *perr = event->info;

   fprintf(stderr, "ERROR: error communicating to %s: %s\n",
           efl_net_dialer_address_dial_get(dialer),
           eina_error_msg_get(*perr));
   retval = EXIT_FAILURE;
   ecore_main_loop_quit();
}

int
main(int argc, char **argv)
{
   Eo *loop;
   char *path;
   Eina_Error err;
   int i;

   if (argc < 2)
     {
        fprintf(stderr, "ERROR: missing argument.\n");
        return EXIT_FAILURE;
     }
   for (i = 1; i < argc; i++)
     {
        if ((strcmp(argv[i], "-h") != 0) &&
            (strcmp(argv[i], "--help") != 0))
          continue;

        printf("Usage:\n"
               "\n"
               "\t%s <command> [arguments]\n"
               "\n"
               "where <command> is one of:\n"
               "\tlist               list connected process (pid)\n"
               "\tpon <pid> <freq>   enable profiling for <pid> at frequency <freq> in microseconds.\n"
               "\tpoff <pid>         disable profiling for <pid>\n"
               "\tevlogon <pid>      start logging events to ~/efl_debug_evlog-<pid>.log\n"
               "\tevlogoff <pid>     stop logging events from <pid>\n",
               argv[0]);

        return EXIT_SUCCESS;
     }

   ecore_app_no_system_modules();

   eina_init();
   ecore_init();
   ecore_con_init();

   path = ecore_con_local_path_new(EINA_FALSE, "efl_debug", 0);
   if (!path)
     {
        fprintf(stderr, "ERROR: could not get local communication path\n");
        retval = EXIT_FAILURE;
        goto end;
     }

   loop = ecore_main_loop_get();

#ifdef EFL_NET_DIALER_UNIX_CLASS
   dialer = efl_add(EFL_NET_DIALER_SIMPLE_CLASS, loop,
                    efl_net_dialer_simple_inner_class_set(efl_added, EFL_NET_DIALER_UNIX_CLASS));
#else
   /* TODO: maybe start a TCP using locahost:12345?
    * Right now eina_debug_monitor is only for AF_UNIX, so not an issue.
    */
   fprintf(stderr, "ERROR: your platform doesn't support Efl.Net.Dialer.Unix\n");
#endif
   if (!dialer)
     {
        fprintf(stderr, "ERROR: could not create communication dialer\n");
        retval = EXIT_FAILURE;
        goto end;
     }
   efl_event_callback_add(dialer, EFL_IO_BUFFERED_STREAM_EVENT_ERROR, _error, NULL);
   efl_event_callback_add(dialer, EFL_IO_BUFFERED_STREAM_EVENT_SLICE_CHANGED, _on_data, NULL);
   efl_event_callback_add(dialer, EFL_IO_BUFFERED_STREAM_EVENT_WRITE_FINISHED, _write_finished, NULL);
   efl_event_callback_add(dialer, EFL_IO_BUFFERED_STREAM_EVENT_FINISHED, _finished, NULL);

   for (i = 1; i < argc; i++)
     {
        const char *cmd = argv[i];

        if (strcmp(cmd, "list") == 0)
          {
             if (!command_send(LIST, NULL, 0))
               goto end;
             waiting = eina_list_append(waiting, OP_CLST);
          }
        else if (strcmp(cmd, "pon") == 0)
          {
             if (i + 2 >= argc)
               {
                  fprintf(stderr, "ERROR: missing argument: pon <pid> <freq>\n");
                  retval = EXIT_FAILURE;
                  goto end;
               }
             else
               {
                  int data[2] = {atoi(argv[i + 1]), atoi(argv[1 + 2])};
                  if (!command_send(PLON, data, sizeof(data)))
                    goto end;
                  i += 2;
               }
          }
        else if (strcmp(cmd, "poff") == 0)
          {
             if (i + 1 >= argc)
               {
                  fprintf(stderr, "ERROR: missing argument: poff <pid>\n");
                  retval = EXIT_FAILURE;
                  goto end;
               }
             else
               {
                  int data[1] = {atoi(argv[i + 1])};
                  if (!command_send(PLOF, data, sizeof(data)))
                    goto end;
                  i++;
               }
          }
        else if (strcmp(cmd, "evlogon") == 0)
          {
             if (i + 1 >= argc)
               {
                  fprintf(stderr, "ERROR: missing argument: evlogon <pid>\n");
                  retval = EXIT_FAILURE;
                  goto end;
               }
             else
               {
                  int data[1] = {atoi(argv[i + 1])};
                  if (!command_send(EVON, data, sizeof(data)))
                    goto end;
                  i++;
               }
          }
        else if (strcmp(cmd, "evlogoff") == 0)
          {
             if (i + 1 >= argc)
               {
                  fprintf(stderr, "ERROR: missing argument: evlogoff <pid>\n");
                  retval = EXIT_FAILURE;
                  goto end;
               }
             else
               {
                  int data[1] = {atoi(argv[i + 1])};
                  if (!command_send(EVOF, data, sizeof(data)))
                    goto end;
                  i++;
               }
          }
        else
          {
             fprintf(stderr, "ERROR: unknown command: %s\n", argv[i]);
             retval = EXIT_FAILURE;
             goto end;
          }
     }
   efl_io_buffered_stream_eos_mark(dialer);

   err = efl_net_dialer_dial(dialer, path);
   if (err)
     {
        fprintf(stderr, "ERROR: could not connect '%s': %s\n", path, eina_error_msg_get(err));
        retval = EXIT_FAILURE;
        goto end;
     }

   ecore_main_loop_begin();

 end:
   eina_list_free(waiting);
   efl_del(dialer);
   free(path);

   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();
   return retval;
}
