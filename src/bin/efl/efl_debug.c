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

#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1
#include "efl_debug_common.h"

static Eo *dialer;
static Eo *input;
static Eo *output;
static Eo *send_copier;
static Eo *recv_copier;

static Eina_List *waiting;

static int retval = EXIT_SUCCESS;

static const char CLST[4] = "CLST";

static void
_process_reply(const char op[static 4], const Eina_Slice payload)
{
#define IS_OP(x) memcmp(op, x, 4) == 0

   if (IS_OP(CLST))
     {
        int mypid = getpid();
        size_t offset;

        waiting = eina_list_remove(waiting, CLST);

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

#undef IS_OP
}

static void
_on_data(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Eina_Slice slice, payload;
   Efl_Debug_Message_Header msgheader;

   if (!efl_io_queue_slice_get(output, &slice))
     return;

   if (slice.len < sizeof(msgheader))
     return;

   memcpy(&msgheader, slice.mem, sizeof(msgheader));
   if (msgheader.size < 4) /* must contain at last 4 byte opcode */
     {
        fprintf(stderr, "ERROR: invalid message header, size=%u\n", msgheader.size);
        retval = EXIT_FAILURE;
        ecore_main_loop_quit();
        return;
     }

   if (msgheader.size + 4 > slice.len)
     return;

   payload.bytes = slice.bytes + sizeof(msgheader);
   payload.len = msgheader.size - 4;

   _process_reply(msgheader.op, payload);

   efl_io_queue_discard(output, sizeof(msgheader) + payload.len);
}

static Eina_Bool
_command_send(const char op[static 4], const void *data, unsigned int len)
{
   Eina_Error err;
   Efl_Debug_Message_Header msghdr = {
     .size = 4 + len,
   };
   Eina_Slice s, r;

   memcpy(msghdr.op, op, 4);

   s.mem = &msghdr;
   s.len = sizeof(msghdr);

   err = efl_io_writer_write(input, &s, &r);
   if (err || r.len) goto end;

   if (!len) goto end;

   s.mem = data;
   s.len = len;
   err = efl_io_writer_write(input, &s, &r);

 end:
   if (err)
     {
        fprintf(stderr, "ERROR: could not queue message '%.4s': %s\n", op, eina_error_msg_get(err));
        retval = EXIT_FAILURE;
        return EINA_FALSE;
     }

   if (r.len)
     {
        fprintf(stderr, "ERROR: could not queue message '%.4s': out of memory\n", op);
        retval = EXIT_FAILURE;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_finished_sending(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   if (!waiting) ecore_main_loop_quit();
}

static void
_dialer_eos(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_dialer_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Error *perr = event->info;

   fprintf(stderr, "ERROR: error communicating to %s: %s\n",
           efl_net_dialer_address_dial_get(dialer),
           eina_error_msg_get(*perr));
   retval = EINA_TRUE;
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
   dialer = efl_add(EFL_NET_DIALER_UNIX_CLASS, loop);
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
   efl_event_callback_add(dialer, EFL_NET_DIALER_EVENT_ERROR, _dialer_error, NULL);
   efl_event_callback_add(dialer, EFL_IO_READER_EVENT_EOS, _dialer_eos, NULL);

   input = efl_add(EFL_IO_QUEUE_CLASS, loop);
   if (!input)
     {
        fprintf(stderr, "ERROR: could not create input queue\n");
        retval = EXIT_FAILURE;
        goto end;
     }

   output = efl_add(EFL_IO_QUEUE_CLASS, loop,
                    efl_event_callback_add(efl_added, EFL_IO_QUEUE_EVENT_SLICE_CHANGED, _on_data, NULL));
   if (!output)
     {
        fprintf(stderr, "ERROR: could not create output queue\n");
        retval = EXIT_FAILURE;
        goto end;
     }

   send_copier = efl_add(EFL_IO_COPIER_CLASS, loop,
                         efl_io_copier_source_set(efl_added, input),
                         efl_io_copier_destination_set(efl_added, dialer),
                         efl_io_closer_close_on_destructor_set(efl_added, EINA_FALSE),
                         efl_event_callback_add(efl_added, EFL_IO_COPIER_EVENT_DONE, _finished_sending, NULL));
   if (!send_copier)
     {
        fprintf(stderr, "ERROR: could not create send copier\n");
        retval = EXIT_FAILURE;
        goto end;
     }

   recv_copier = efl_add(EFL_IO_COPIER_CLASS, loop,
                         efl_io_copier_source_set(efl_added, dialer),
                         efl_io_copier_destination_set(efl_added, output),
                         efl_io_closer_close_on_destructor_set(efl_added, EINA_FALSE));
   if (!recv_copier)
     {
        fprintf(stderr, "ERROR: could not create receive copier\n");
        retval = EXIT_FAILURE;
        goto end;
     }

   for (i = 1; i < argc; i++)
     {
        const char *cmd = argv[i];

        if (strcmp(cmd, "list") == 0)
          {
             if (!_command_send("LIST", NULL, 0))
               goto end;
             waiting = eina_list_append(waiting, CLST);
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
                  if (!_command_send("PLON", data, sizeof(data)))
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
                  if (!_command_send("PLOFF", data, sizeof(data)))
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
                  if (!_command_send("EVON", data, sizeof(data)))
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
                  if (!_command_send("EVOF", data, sizeof(data)))
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
   efl_io_queue_eos_mark(input);

   err = efl_net_dialer_dial(dialer, path);
   if (err)
     {
        fprintf(stderr, "ERROR: could not connect '%s': %s\n", path, eina_error_msg_get(err));
        retval = EXIT_FAILURE;
        goto end;
     }

   ecore_main_loop_begin();

   while ((!efl_io_closer_closed_get(dialer)) &&
          efl_io_queue_usage_get(input))
     efl_io_copier_flush(send_copier, EINA_TRUE, EINA_TRUE);

 end:
   eina_list_free(waiting);
   efl_del(input);
   efl_del(output);
   efl_del(dialer);
   efl_del(send_copier);
   efl_del(recv_copier);
   free(path);

   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();
   return retval;
}
