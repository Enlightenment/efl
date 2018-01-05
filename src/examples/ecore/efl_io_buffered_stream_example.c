#include <Efl_Net.h>
#include <Ecore_Getopt.h>

static int retval = EXIT_SUCCESS;
static Eina_List *commands = NULL;
static Eina_Slice line_delimiter;
static Eo *stream = NULL;

static void
_command_next(void)
{
   Eina_Slice slice;
   char *cmd;

   if (!commands)
     {
        efl_io_buffered_stream_eos_mark(stream);
        return;
     }

   cmd = commands->data;
   commands = eina_list_remove_list(commands, commands);

   slice = (Eina_Slice)EINA_SLICE_STR(cmd);
   efl_io_writer_write(stream, &slice, NULL);
   fprintf(stderr, "INFO: sent '" EINA_SLICE_STR_FMT "'\n",
           EINA_SLICE_STR_PRINT(slice));

   /* don't use line_delimiter directly, 'len' may be changed! */
   slice = line_delimiter;
   efl_io_writer_write(stream, &slice, NULL);
   free(cmd);
}

static void
_receiver_data(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Slice slice = efl_io_buffered_stream_slice_get(event->object);

   /* this will happen when we're called when we issue our own
    * efl_io_buffered_stream_clear() below.
    */
   if (slice.len == 0) return;

   /*
    * If the server didn't send us the line terminator and closed the
    * connection (ie: efl_io_reader_eos_get() == true) or if the
    * efl_io_buffered_stream_max_queue_size_input_set() was reached,
    * then we may have a line without a trailing delimiter. Check for
    * that.
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

   efl_io_buffered_stream_clear(event->object);
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
_stream_write_finished(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: %s done sending\n", efl_name_get(event->object));
}

static void
_stream_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Eina_Error *perr = event->info;
   fprintf(stderr, "INFO: %s error: #%d '%s'\n",
           efl_name_get(event->object), *perr, eina_error_msg_get(*perr));
   retval = EXIT_FAILURE;
   efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
}

static void
_stream_eos(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: %s eos, quit\n", efl_name_get(event->object));
   efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
}

EFL_CALLBACKS_ARRAY_DEFINE(stream_cbs,
                           { EFL_IO_BUFFERED_STREAM_EVENT_LINE, _receiver_data },
                           { EFL_IO_READER_EVENT_EOS, _stream_eos },
                           { EFL_IO_BUFFERED_STREAM_EVENT_WRITE_FINISHED, _stream_write_finished },
                           { EFL_IO_BUFFERED_STREAM_EVENT_ERROR, _stream_error });

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
  "efl_io_buffered_stream_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Efl_Io_Buffered_Stream usage.\n"
  "\n"
  "This uses Efl_Io_Buffered_Stream to easily interface with Efl_Net_Dialer_Tcp.",
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
   Eo *dialer;

   args = ecore_getopt_parse(&options, values, 0, NULL);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   if (quit_option) goto end;

   args = ecore_getopt_parse_positional(&options, values, 0, NULL, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   line_delimiter_str = _unescape(line_delimiter_str ? line_delimiter_str : "\\r\\n");

   if (!commands)
     {
        fputs("ERROR: missing commands to send.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   /*
    * some objects such as the Efl.Io.Buffered_Stream and
    * Efl.Net.Dialer.Tcp depend on main loop, thus their parent must
    * be a loop provider. We use the loop itself that come from event.
    */
   /* The TCP client to use to send/receive network data */
   dialer = efl_add(EFL_NET_DIALER_TCP_CLASS, ev->object,
                    efl_name_set(efl_added, "dialer"),
                    efl_event_callback_add(efl_added, EFL_NET_DIALER_EVENT_CONNECTED, _dialer_connected, NULL));
   if (!dialer)
     {
        fprintf(stderr, "ERROR: could not create Efl_Net_Dialer_Tcp\n");
        retval = EXIT_FAILURE;
        goto end;
     }

   line_delimiter = (Eina_Slice)EINA_SLICE_STR(line_delimiter_str);

   /*
    * Without the buffered stream we'd have to create two Efl.Io.Queue
    * ourselves, as well as two Efl.Io.Copier to link them with the
    * dialer.
    *
    * Our example's usage is to write each command at once followed by
    * the line_delimiter, then wait for a reply from the server, then
    * write another.
    *
    * On incoming data we peek at it with slice_get() and then clear().
    */
   stream = efl_add(EFL_IO_BUFFERED_STREAM_CLASS, ev->object,
                    efl_name_set(efl_added, "stream"),
                    efl_io_buffered_stream_inner_io_set(efl_added, dialer), /* mandatory! */
                    efl_io_buffered_stream_line_delimiter_set(efl_added, line_delimiter),
                    efl_io_buffered_stream_max_queue_size_input_set(efl_added, buffer_limit),
                    efl_io_buffered_stream_max_queue_size_output_set(efl_added, buffer_limit),
                    efl_event_callback_array_add(efl_added, stream_cbs(), NULL));
   if (!stream)
     {
        fprintf(stderr, "ERROR: could not create Efl_Io_Buffered_Stream\n");
        retval = EXIT_FAILURE;
        goto error_stream;
     }

   /*
    * From here on it's mostly the same all Efl_Io_Copier would do,
    * check efl_io_copier_simple_example.c and efl_io_copier_example.c
    */
   err = efl_net_dialer_dial(dialer, address);
   if (err)
     {
        fprintf(stderr, "ERROR: could not dial %s: %s\n",
                address, eina_error_msg_get(err));
        goto error_dialing;
     }

   return ;

 error_dialing:
   efl_io_closer_close(stream);
   efl_del(stream);
 error_stream:
   efl_del(dialer);
 end:
   EINA_LIST_FREE(commands, cmd)
     {
        fprintf(stderr, "ERROR: unsent command: %s\n", cmd);
        free(cmd);
     }

   efl_loop_quit(ev->object, eina_value_int_init(retval));
}

EFL_MAIN();
