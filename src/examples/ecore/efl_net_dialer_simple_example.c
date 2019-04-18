#define EFL_BETA_API_SUPPORT
#include <Efl_Net.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>
#include <ctype.h>

static Eina_Bool do_read = EINA_FALSE;
static Eina_Bool do_discard = EINA_FALSE;
static Eina_Slice line_delm_slice;

static void
_connected(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr,
           "INFO: connected to '%s' (%s)\n"
           "INFO:  - local address=%s\n"
           "INFO:  - do read=%u\n"
           "INFO:  - do discard=%u\n",
           efl_net_dialer_address_dial_get(event->object),
           efl_net_socket_address_remote_get(event->object),
           efl_net_socket_address_local_get(event->object),
           do_read, do_discard);
}

static void
_eos(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: end of stream.\n");

   /* on _error() we close it, then do not read as it has nothing */
   if (efl_io_closer_closed_get(event->object))
     return;

   fprintf(stderr,
           "-- BEGIN RECEIVED DATA --\n"
           EINA_SLICE_STR_FMT
           "-- END RECEIVED DATA--\n",
           EINA_SLICE_STR_PRINT(efl_io_buffered_stream_slice_get(event->object)));
}

static void
_can_read(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Bool can_read = efl_io_reader_can_read_get(event->object);

   /*
    * Since we have more high level events, such as "slice,changed"
    * and "line", it's not that interesting to monitor
    * "can_read,changed" anymore. We do and print out, but no actual
    * reads as we print from _line() or _eos().
    *
    * But reads can be done as usual, see the '#if' block below.
    */

   fprintf(stderr, "INFO: can read=%d\n", can_read);

#if 0
   if ((can_read) && (do_read))
     {
        char buf[4];
        Eina_Rw_Slice rw_slice = EINA_SLICE_ARRAY(buf);
        Eina_Error err;

        do
          {
             err = efl_io_reader_read(event->object, &rw_slice);
             if (err)
               {
                  if (err == EAGAIN)
                    {
                       fprintf(stderr, "ERROR: read all available data\n");
                       break;
                    }
                  fprintf(stderr, "ERROR: could not read: %s\n", eina_error_msg_get(err));
                  efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
                  return;
               }

             fprintf(stderr, "INFO: read '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(rw_slice));
          }
        while (err == 0);
     }
#endif
}


static void
_line(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Eina_Slice slice = *(const Eina_Slice *)event->info;

   if (!eina_slice_endswith(slice,  line_delm_slice))
     {
        fprintf(stderr, "WARNING: received without line-delimiter '"
                EINA_SLICE_STR_FMT "'\n",
                EINA_SLICE_STR_PRINT(slice));
     }
   else
     {
        Eina_Slice s = slice;
        s.len -= line_delm_slice.len;
        fprintf(stderr, "INFO: received '" EINA_SLICE_STR_FMT "'\n",
                EINA_SLICE_STR_PRINT(s));
     }

   /*
    * If you used the line and it's not interesting anymore, then you
    * can discard it.
    *
    * It has the same effect as calling efl_io_reader_read() as per
    * #if block below
    */
   if (do_discard)
     {
#if 1
        efl_io_buffered_stream_discard(event->object, slice.len);
#else
        {
           /* efl_io_buffered_stream_discard() paired with
            * efl_io_buffered_stream_slice_get() + 'slice,changed' or
            * 'line' events is a faster alternative than reading,
            * since it doesn't copy the data.
            */
           char *buf = malloc(slice.len);
           Eina_Rw_Slice rw_slice = {
             .mem = buf,
             .len = slice.len,
           };
           Eina_Error err = efl_io_reader_read(event->object, &rw_slice);
           fprintf(stderr, "INFO: read error=%s '" EINA_SLICE_FMT "'\n", eina_error_msg_get(err) ? eina_error_msg_get(err) : "success", EINA_SLICE_PRINT(rw_slice));
           free(buf);
        }
#endif
     }
}

static void
_resolved(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: resolved %s => %s\n",
           efl_net_dialer_address_dial_get(event->object),
           efl_net_socket_address_remote_get(event->object));
}

static void
_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Eina_Error *perr = event->info;
   fprintf(stderr, "INFO: error: %d '%s'\n", *perr, eina_error_msg_get(*perr));
   if (!efl_io_closer_closed_get(event->object))
     efl_io_closer_close(event->object);

   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
}

static void
_done_sending(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: done sending\n");
   if (!do_read)
     efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
}

static void
_done_receiving(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   fprintf(stderr, "INFO: done receiving\n");
}

static void
_done(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: done sending and receiving\n");

   efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
}

EFL_CALLBACKS_ARRAY_DEFINE(dialer_cbs,
                           { EFL_NET_DIALER_EVENT_DIALER_CONNECTED, _connected }, /* optional */
                           { EFL_NET_DIALER_EVENT_DIALER_RESOLVED, _resolved }, /* optional */
                           { EFL_IO_READER_EVENT_CAN_READ_CHANGED, _can_read }, /* optional, can be used to read data, here just for monitoring */
                           { EFL_IO_READER_EVENT_EOS, _eos }, /* recommended, notifies no more incoming data */
                           { EFL_IO_BUFFERED_STREAM_EVENT_LINE, _line }, /* optional, could use 'slice,changed' or 'can_read,changed' instead */
                           { EFL_IO_BUFFERED_STREAM_EVENT_ERROR, _error }, /* recommended */
                           { EFL_IO_BUFFERED_STREAM_EVENT_WRITE_FINISHED, _done_sending }, /* optional */
                           { EFL_IO_BUFFERED_STREAM_EVENT_READ_FINISHED, _done_receiving }, /* optional, same as 'eos' */
                           { EFL_IO_BUFFERED_STREAM_EVENT_FINISHED, _done }); /* recommended, notifies both send and receive are finished */

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

static const char * protocols[] = {
  "tcp",
  "udp",
  "ssl",
#ifndef _WIN32
  "unix",
#endif
  NULL
};

static const Ecore_Getopt options = {
  "efl_net_dialer_simple_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Efl_Net_Dialer_Simple usage, sending a message and receiving a reply\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_TRUE('r', "read", "Wait for data to be read."),
    ECORE_GETOPT_STORE_TRUE('D', "discard-lines", "Lines that are read are discarded from final output."),
    ECORE_GETOPT_APPEND('s', "send", "send the given string to the server once connected.", ECORE_GETOPT_TYPE_STR),

    ECORE_GETOPT_STORE_STR('d', "line-delimiter",
                           "If set will define a line delimiter for copy operation, instead of a fixed chunk size. This will trigger line events."),
    ECORE_GETOPT_STORE_ULONG('l', "buffer-limit",
                             "If set will limit buffer size to this limit of bytes. If used alongside with --line-delimiter and that delimiter was not found but bffer limit was reached, the line event will be triggered without the delimiter at the end."),
    ECORE_GETOPT_STORE_ULONG('c', "read-chunk-size",
                             "If set will change the base chunk size used while reading."),
    ECORE_GETOPT_STORE_DOUBLE('i', "inactivity-timeout",
                              "If greater than zero, specifies the number of seconds without any reads or writes that the dialer will be timed out."),
    ECORE_GETOPT_STORE_DOUBLE('t', "connect-timeout",
                              "If greater than zero, specifies the number of seconds without any reads or writes that the dialer will be timed out."),

    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),

    ECORE_GETOPT_CHOICE_METAVAR(0, NULL, "The dialer protocol.", "protocol",
                                protocols),
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The address (URL) to dial", "address"),
    ECORE_GETOPT_SENTINEL
  }
};

static Eo *dialer = NULL;

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
   /* FIXME: For the moment the main loop doesn't get
      properly destroyed on shutdown which disallow
      relying on parent destroying their children */
   if (dialer)
     {
        efl_del(dialer);
        dialer = NULL;
     }

   fprintf(stderr, "INFO: main loop finished.\n");
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   const Efl_Class *cls;
   Eina_List *to_send = NULL;
   char *str;
   char *line_delimiter_str = NULL;
   char *address = NULL;
   char *protocol = NULL;
   unsigned long buffer_limit = 0;
   unsigned long read_chunk_size = 0;
   double timeout_inactivity = 0.0;
   double connect_timeout = 0.0;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(do_read),
     ECORE_GETOPT_VALUE_BOOL(do_discard),
     ECORE_GETOPT_VALUE_LIST(to_send),

     ECORE_GETOPT_VALUE_STR(line_delimiter_str),
     ECORE_GETOPT_VALUE_ULONG(buffer_limit),
     ECORE_GETOPT_VALUE_ULONG(read_chunk_size),
     ECORE_GETOPT_VALUE_DOUBLE(timeout_inactivity),
     ECORE_GETOPT_VALUE_DOUBLE(connect_timeout),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     /* positional argument */
     ECORE_GETOPT_VALUE_STR(protocol),
     ECORE_GETOPT_VALUE_STR(address),

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args;
   Eo *loop;
   Eina_Error err;

   args = ecore_getopt_parse(&options, values, 0, NULL);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        goto end;
     }

   if (quit_option) goto end;

   loop = ev->object;

   args = ecore_getopt_parse_positional(&options, values, 0, NULL, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        goto end;
     }

   if (!protocol)
     {
        fputs("ERROR: missing protocol.\n", stderr);
        goto end;
     }

   if (strcmp(protocol, "tcp") == 0) cls = EFL_NET_DIALER_TCP_CLASS;
   else if (strcmp(protocol, "udp") == 0) cls = EFL_NET_DIALER_UDP_CLASS;
   else if (strcmp(protocol, "ssl") == 0) cls = EFL_NET_DIALER_SSL_CLASS;
#ifdef EFL_NET_DIALER_UNIX_CLASS
   else if (strcmp(protocol, "unix") == 0) cls = EFL_NET_DIALER_UNIX_CLASS;
#endif
#ifdef EFL_NET_DIALER_WINDOWS_CLASS
   else if (strcmp(protocol, "windows") == 0) cls = EFL_NET_DIALER_WINDOWS_CLASS;
#endif
   else
     {
        fprintf(stderr, "ERROR: unsupported protocol: %s\n", protocol);
        goto end;
     }

   /* A delimiter is optional, if empty or unset, buffered stream uses
    * a copier that will execute writes based on read_chunk_size and
    * only event "data" is emitted.
    *
    * If a line delimiter is set, it will hold writes until the
    * delimiter is found, source reached End-of-Stream (eos) or the
    * copier buffer limit is reached. The "line" event is emitted.
    */
   line_delimiter_str = _unescape(line_delimiter_str ? line_delimiter_str : "\\r\\n");
   if (line_delimiter_str)
     line_delm_slice = (Eina_Slice)EINA_SLICE_STR(line_delimiter_str);

   dialer = efl_add(EFL_NET_DIALER_SIMPLE_CLASS, loop,
                    efl_name_set(efl_added, "dialer"),
                    efl_net_dialer_simple_inner_class_set(efl_added, cls), /* alternatively you could create the inner dialer and set with efl_io_buffered_stream_inner_io_set() */
                    efl_io_buffered_stream_line_delimiter_set(efl_added, line_delm_slice), /* optional */
                    efl_io_buffered_stream_max_queue_size_input_set(efl_added, buffer_limit), /* optional, defaults to unlimited */
                    efl_io_buffered_stream_max_queue_size_output_set(efl_added, buffer_limit), /* optional, defaults to unlimited */
                    efl_io_buffered_stream_read_chunk_size_set(efl_added, read_chunk_size), /* optional, defaults to 4096 */
                    efl_io_buffered_stream_timeout_inactivity_set(efl_added, timeout_inactivity), /* optional, defaults to 0.0 (disabled) */
                    efl_net_dialer_timeout_dial_set(efl_added, connect_timeout), /* optional, defaults to 0.0 (disabled) */
                    efl_event_callback_array_add(efl_added, dialer_cbs(), NULL));

   err = efl_net_dialer_dial(dialer, address);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial %s '%s': %s",
                protocol, address, eina_error_msg_get(err));
        goto no_mainloop;
     }

   /* unlike low-level I/O that wouldn't write data until it's
    * connected, the simple dialer will queue it in memory, sending when
    * it's ready. Thus just write & forget.
    */
   if (!to_send)
     {
        if (!do_read)
          {
             Eina_Slice s = EINA_SLICE_STR_LITERAL("Hello World!");

             fprintf(stderr, "INFO: sending '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(s));
             efl_io_writer_write(dialer, &s, NULL);

             if (line_delm_slice.len)
               {
                  Eina_Slice sw = line_delm_slice;
                  efl_io_writer_write(dialer, &sw, NULL);
               }
          }
        else
          fprintf(stderr, "INFO: nothing to send, just read...\n");
     }
   else
     {
        EINA_LIST_FREE(to_send, str)
          {
             /* ignore empty sends, but add line delimiter, so we can do HTTP's last line :-) */
             if (str[0] != '\0')
               {
                  Eina_Slice s = EINA_SLICE_STR(str);
                  fprintf(stderr, "INFO: sending '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(s));
                  efl_io_writer_write(dialer, &s, NULL);
               }
             free(str);

             if (line_delm_slice.len)
               {
                  Eina_Slice sw = line_delm_slice;
                  efl_io_writer_write(dialer, &sw, NULL);
               }
          }
     }
   efl_io_buffered_stream_eos_mark(dialer); /* we're done sending */

   return ;

 no_mainloop:
   efl_del(dialer);

 end:
   EINA_LIST_FREE(to_send, str) free(str);

   efl_loop_quit(efl_loop_get(ev->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
