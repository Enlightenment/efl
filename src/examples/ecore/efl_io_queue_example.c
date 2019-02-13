#define EFL_BETA_API_SUPPORT
#include <Efl_Net.h>
#include <Ecore_Getopt.h>

static Eina_List *waiting = NULL;
static Eina_List *commands = NULL;
static Eina_Slice line_delimiter;
static Eo *send_queue, *receive_queue;

static void
_command_next(void)
{
   Eina_Slice slice;
   char *cmd;

   if (!commands)
     {
        efl_io_queue_eos_mark(send_queue);
        return;
     }

   cmd = commands->data;
   commands = eina_list_remove_list(commands, commands);

   slice = (Eina_Slice)EINA_SLICE_STR(cmd);
   efl_io_writer_write(send_queue, &slice, NULL);
   fprintf(stderr, "INFO: sent '" EINA_SLICE_STR_FMT "'\n",
           EINA_SLICE_STR_PRINT(slice));

   /* don't use line_delimiter directly, 'len' may be changed! */
   slice = line_delimiter;
   efl_io_writer_write(send_queue, &slice, NULL);
   free(cmd);
}

static void
_receiver_data(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Slice slice = efl_io_queue_slice_get(event->object);

   /* this will happen when we're called when we issue our own
    * efl_io_queue_clear() below.
    */
   if (slice.len == 0) return;

   /*
    * If the server didn't send us the line terminator and closed the
    * connection (ie: efl_io_reader_eos_get() == true) or if the
    * efl_io_copier_buffer_limit_set() was reached (note we do not set
    * it in this example), then we may have a line without a trailing
    * delimiter. Check for that.
    */
   if (!eina_slice_endswith(slice,  line_delimiter))
     {
        fprintf(stderr, "WARNING: received without line-delimiter '"
                EINA_SLICE_STR_FMT "'\n",
                EINA_SLICE_STR_PRINT(slice));
     }
   else
     {
        slice.len -= line_delimiter.len;
        fprintf(stderr, "INFO: received '" EINA_SLICE_STR_FMT "'\n",
                EINA_SLICE_STR_PRINT(slice));
     }

   efl_io_queue_clear(event->object);
   _command_next();
}

static void
_dialer_connected(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: connected to %s (%s)\n",
           efl_net_dialer_address_dial_get(event->object),
           efl_net_socket_address_remote_get(event->object));

   _command_next();
}

static void
_copier_done(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: %s done\n", efl_name_get(event->object));

   waiting = eina_list_remove(waiting, event->object);
   if (!waiting)
     efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
}

static void
_copier_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Eina_Error *perr = event->info;
   fprintf(stderr, "INFO: %s error: #%d '%s'\n",
           efl_name_get(event->object), *perr, eina_error_msg_get(*perr));

   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_CALLBACKS_ARRAY_DEFINE(copier_cbs,
                           { EFL_IO_COPIER_EVENT_DONE, _copier_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _copier_error });

static char *
_unescape(const char *str)
{
   char *ret = strdup(str);
   char *c, *w;
   Eina_Bool escaped = EINA_FALSE;

   for (c = ret, w = ret; *c != '\0'; c++)
     {
        if (escaped)
          {
             escaped = EINA_FALSE;
             switch (*c)
               {
                case 'n': *w = '\n'; break;
                case 'r': *w = '\r'; break;
                case 't': *w = '\t'; break;
                default: w++; /* no change */
               }
             w++;
          }
        else
          {
             if (*c == '\\')
               escaped = EINA_TRUE;
             else
               w++;
          }
     }
   *w = '\0';
   return ret;
}

static const Ecore_Getopt options = {
  "efl_io_queue_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Efl_Io_Queue usage.\n"
  "\n"
  "This uses Efl_Io_Queue to easily interface with Efl_Io_Copier in order to "
  "send commands to a TCP server.",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_STR('d', "line-delimiter",
                           "Changes the line delimiter to be used in both send and receive. Defaults to \\r\\n"),
    ECORE_GETOPT_STORE_ULONG('l', "buffer-limit",
                             "If set will limit buffer size to this limit of bytes. If used alongside with --line-delimiter and that delimiter was not found but bffer limit was reached, the line event will be triggered without the delimiter at the end."),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),

    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The server address as\n"
                                   "IP:PORT to connect using TCP and an IPv4 (A.B.C.D:PORT) or IPv6 ([A:B:C:D::E]:PORT).\n",
                                   "server_address"),
    ECORE_GETOPT_APPEND_METAVAR(0, NULL,
                                "Commands to send",
                                "commands",
                                ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_SENTINEL
  }
};

EAPI_MAIN void
efl_pause(void *data EINA_UNUSED,
          const Efl_Event *ev EINA_UNUSED)
{
}

EAPI_MAIN void
efl_resume(void *data EINA_UNUSED,
           const Efl_Event *ev EINA_UNUSED)
{
}

EAPI_MAIN void
efl_terminate(void *data EINA_UNUSED,
              const Efl_Event *ev EINA_UNUSED)
{
  if (waiting)
    {
       fprintf(stderr, "ERROR: %d operations were waiting!\n",
               eina_list_count(waiting));
       eina_list_free(waiting);
       waiting = NULL;
    }
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   char *address = NULL;
   char *line_delimiter_str = NULL;
   char *cmd;
   unsigned long buffer_limit = 0;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(line_delimiter_str),
     ECORE_GETOPT_VALUE_ULONG(buffer_limit),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     /* positional argument */
     ECORE_GETOPT_VALUE_STR(address),
     ECORE_GETOPT_VALUE_LIST(commands),

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   Eina_Error err;
   int args;
   Eo *dialer, *sender, *receiver, *loop;

   args = ecore_getopt_parse(&options, values, 0, NULL);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        goto end;
     }

   if (quit_option) goto end;

   args = ecore_getopt_parse_positional(&options, values, 0, NULL, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        goto end;
     }

   line_delimiter_str = _unescape(line_delimiter_str ? line_delimiter_str : "\\r\\n");

   if (!commands)
     {
        fputs("ERROR: missing commands to send.\n", stderr);
        goto end;
     }

   line_delimiter = (Eina_Slice)EINA_SLICE_STR(line_delimiter_str);

   /*
    * Without a send_queue we'd have to manually implement an
    * Efl_Io_Reader object that would provide partial data when
    * Efl_Io_Reader.read() is called by Efl_Io_Copier. This is
    * cumbersome... we just want to write a full command and have the
    * queue to handle that for us.
    *
    * Our example's usage is to write each command at once followed by
    * the line_delimiter, then wait for a reply from the server, then
    * write another.
    */
   send_queue = efl_add_ref(EFL_IO_QUEUE_CLASS, NULL,
                        efl_name_set(efl_added, "send_queue"),
                        efl_io_queue_limit_set(efl_added, buffer_limit));
   if (!send_queue)
     {
        fprintf(stderr, "ERROR: could not create Efl_Io_Queue (send)\n");
        goto end;
     }

   /*
    * Without a receive_queue we'd have to manually implement an
    * Efl_Io_Writer object that would handle write of partial data
    * with Efl_Io_Writer.write() is called by Efl_Io_Copier.
    *
    * For output we could have another solution as well: use NULL
    * destination and handle "line" or "data" events manually,
    * stealing the buffer so it doesn't grow.
    *
    * Our example's usage is to peek its data with slice_get() then
    * clear().
    */
   receive_queue = efl_add_ref(EFL_IO_QUEUE_CLASS, NULL,
                           efl_name_set(efl_added, "receive_queue"),
                           efl_io_queue_limit_set(efl_added, buffer_limit),
                           efl_event_callback_add(efl_added, EFL_IO_QUEUE_EVENT_SLICE_CHANGED, _receiver_data, NULL));
   if (!receive_queue)
     {
        fprintf(stderr, "ERROR: could not create Efl_Io_Queue (receive)\n");
        goto error_receive_queue;
     }

   /*
    * From here on it's mostly the same all Efl_Io_Copier would do,
    * check efl_io_copier_simple_example.c and efl_io_copier_example.c
    */

   /*
    * some objects such as the Efl.Io.Copier and Efl.Net.Dialer.Tcp
    * depend on main loop, thus their parent must be a loop
    * provider. We use the loop itself.
    */
   loop = ev->object;

   /* The TCP client to use to send/receive network data */
   dialer = efl_add(EFL_NET_DIALER_TCP_CLASS, loop,
                    efl_name_set(efl_added, "dialer"),
                    efl_event_callback_add(efl_added, EFL_NET_DIALER_EVENT_CONNECTED, _dialer_connected, NULL));
   if (!dialer)
     {
        fprintf(stderr, "ERROR: could not create Efl_Net_Dialer_Tcp\n");
        goto error_dialer;
     }

   /* sender: send_queue->network */
   sender = efl_add(EFL_IO_COPIER_CLASS, loop,
                    efl_name_set(efl_added, "sender"),
                    efl_io_copier_line_delimiter_set(efl_added, line_delimiter),
                    efl_io_copier_source_set(efl_added, send_queue),
                    efl_io_copier_destination_set(efl_added, dialer),
                    efl_event_callback_array_add(efl_added, copier_cbs(), NULL));
   if (!sender)
     {
        fprintf(stderr, "ERROR: could not create Efl_Io_Copier (sender)\n");
        goto error_sender;
     }

   /* receiver: network->receive_queue */
   receiver = efl_add(EFL_IO_COPIER_CLASS, loop,
                      efl_name_set(efl_added, "receiver"),
                      efl_io_copier_line_delimiter_set(efl_added, line_delimiter),
                      efl_io_copier_source_set(efl_added, dialer),
                      efl_io_copier_destination_set(efl_added, receive_queue),
                      efl_event_callback_array_add(efl_added, copier_cbs(), NULL));
   if (!receiver)
     {
        fprintf(stderr, "ERROR: could not create Efl_Io_Copier (receiver)\n");
        goto error_receiver;
     }

   err = efl_net_dialer_dial(dialer, address);
   if (err)
     {
        fprintf(stderr, "ERROR: could not dial %s: %s\n",
                address, eina_error_msg_get(err));
        goto error_dialing;
     }

   waiting = eina_list_append(waiting, sender);
   waiting = eina_list_append(waiting, receiver);

   return ;

 error_dialing:
   efl_io_closer_close(receiver);
   efl_del(receiver);
 error_receiver:
   efl_io_closer_close(sender);
   efl_del(sender);
 error_sender:
   efl_del(dialer);
 error_dialer:
   efl_unref(receive_queue);
 error_receive_queue:
   efl_unref(send_queue);
 end:
   EINA_LIST_FREE(commands, cmd)
     {
        fprintf(stderr, "ERROR: unsent command: %s\n", cmd);
        free(cmd);
     }

   efl_loop_quit(efl_loop_get(ev->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
