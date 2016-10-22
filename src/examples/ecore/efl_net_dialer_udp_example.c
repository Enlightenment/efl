#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>
#include <ctype.h>

static int retval = EXIT_SUCCESS;
static int needed_reads = 0;

static void
_connected(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: connected %s\n",
           efl_net_dialer_address_dial_get(event->object));
}

static void
_can_read(void *data EINA_UNUSED, const Efl_Event *event)
{
   char buf[1024];
   Eina_Rw_Slice rw_slice = {.mem = buf, .len = sizeof(buf)};
   Eina_Error err;
   Eina_Bool can_read = efl_io_reader_can_read_get(event->object);

   /* NOTE: this message may appear with can read=0 BEFORE
    * "read '...'" because efl_io_readr_read() will change the status
    * of can_read to FALSE prior to return so we can print it!
    */
   fprintf(stderr, "INFO: can read=%d (needed reads=%d)\n", can_read, needed_reads);
   if (!can_read) return;

   if (!needed_reads) return;

   err = efl_io_reader_read(event->object, &rw_slice);
   if (err)
     {
        fprintf(stderr, "ERROR: could not read: %s\n", eina_error_msg_get(err));
        retval = EXIT_FAILURE;
        ecore_main_loop_quit();
        return;
     }

   fprintf(stderr, "INFO: read '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(rw_slice));

   needed_reads--;
   if (!needed_reads)
     {
        retval = EXIT_SUCCESS;
        ecore_main_loop_quit();
     }
}

static void
_can_write(void *data EINA_UNUSED, const Efl_Event *event)
{
   static int needed_writes = 2;
   Eina_Slice slice;
   Eina_Error err;
   Eina_Bool can_write = efl_io_writer_can_write_get(event->object);

   /* NOTE: this message may appear with can write=0 BEFORE
    * "wrote '...'" because efl_io_writer_write() will change the status
    * of can_write to FALSE prior to return so we can print it!
    */
   fprintf(stderr, "INFO: can write=%d (needed writes=%d)\n", can_write, needed_writes);
   if (!can_write) return;

   if (needed_writes == 2)
     {
        slice = (Eina_Slice)EINA_SLICE_STR_LITERAL("Hello World");

        err = efl_io_writer_write(event->object, &slice, NULL);
        if (err)
          {
             fprintf(stderr, "ERROR: could not write: %s\n", eina_error_msg_get(err));
             retval = EXIT_FAILURE;
             ecore_main_loop_quit();
             return;
          }

        fprintf(stderr, "INFO: wrote '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(slice));
     }
   else if (needed_writes == 1)
     {
        slice = (Eina_Slice)EINA_SLICE_STR_LITERAL("Second Write");
        err = efl_io_writer_write(event->object, &slice, NULL);
        if (err)
          {
             fprintf(stderr, "ERROR: could not write: %s\n", eina_error_msg_get(err));
             retval = EXIT_FAILURE;
             ecore_main_loop_quit();
             return;
          }

        fprintf(stderr, "INFO: wrote '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(slice));

        /* if CORK was used, then say we're done to generate the single final datagram */
        efl_net_socket_udp_cork_set(event->object, EINA_FALSE);
     }
   else if (needed_writes == 0)
     {
        if (needed_reads)
          fprintf(stderr, "INFO: done writing, now wait for %d reads\n", needed_reads);
        else
          {
             fprintf(stderr, "INFO: we're done\n");
             retval = EXIT_SUCCESS;
             ecore_main_loop_quit();
          }
        return;
     }

   needed_writes--;
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
   retval = EXIT_FAILURE;
}

EFL_CALLBACKS_ARRAY_DEFINE(dialer_cbs,
                           { EFL_NET_DIALER_EVENT_CONNECTED, _connected },
                           { EFL_NET_DIALER_EVENT_RESOLVED, _resolved },
                           { EFL_NET_DIALER_EVENT_ERROR, _error },
                           { EFL_IO_READER_EVENT_CAN_READ_CHANGED, _can_read },
                           { EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, _can_write }
                           );

static const Ecore_Getopt options = {
  "efl_net_dialer_udp_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Efl_Net_Dialer_Udp usage, sending a message and receiving a reply\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_BOOL('r', "read-after-write", "Do a read after writes are done."),
    ECORE_GETOPT_STORE_BOOL('c', "cork", "use UDP_CORK around messages to generate a single datagram."),
    ECORE_GETOPT_STORE_BOOL('R', "dont-route", "Do not route packets via a gateway."),
    ECORE_GETOPT_STORE_DOUBLE('t', "connect-timeout", "timeout in seconds for the connection phase"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The address (URL) to dial", "address"),
    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char **argv)
{
   char *address = NULL;
   Eina_Bool cork = EINA_FALSE;
   Eina_Bool do_read = EINA_FALSE;
   Eina_Bool dont_route = EINA_FALSE;
   Eina_Bool quit_option = EINA_FALSE;
   double timeout_dial = 30.0;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(do_read),
     ECORE_GETOPT_VALUE_BOOL(cork),
     ECORE_GETOPT_VALUE_BOOL(dont_route),
     ECORE_GETOPT_VALUE_DOUBLE(timeout_dial),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     /* positional argument */
     ECORE_GETOPT_VALUE_STR(address),

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args;
   Eo *dialer, *loop;
   Eina_Error err;

   ecore_init();
   ecore_con_init();

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   if (quit_option) goto end;

   loop = ecore_main_loop_get();

   args = ecore_getopt_parse_positional(&options, values, argc, argv, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   dialer = efl_add(EFL_NET_DIALER_UDP_CLASS, loop,
                    efl_name_set(efl_added, "dialer"),
                    efl_net_socket_udp_cork_set(efl_added, cork),
                    efl_net_socket_udp_dont_route_set(efl_added, dont_route),
                    efl_net_dialer_timeout_dial_set(efl_added, timeout_dial),
                    efl_event_callback_array_add(efl_added, dialer_cbs(), NULL));

   err = efl_net_dialer_dial(dialer, address);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                address, eina_error_msg_get(err));
        goto no_mainloop;
     }

   fprintf(stderr,
           "INFO: dialed %s\n"
           "INFO:  - read-after-write=%hhu\n"
           "INFO:  - cork=%hhu\n"
           "INFO:  - timeout_dial=%fs\n",
           efl_net_dialer_address_dial_get(dialer),
           do_read,
           efl_net_socket_udp_cork_get(dialer),
           efl_net_dialer_timeout_dial_get(dialer));

   if (do_read) needed_reads = cork ? 1 : 2;

   ecore_main_loop_begin();

   fprintf(stderr, "INFO: main loop finished.\n");

 no_mainloop:
   efl_del(dialer);

 end:
   ecore_con_shutdown();
   ecore_shutdown();

   return retval;
}
