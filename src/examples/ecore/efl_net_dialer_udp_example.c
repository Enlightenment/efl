#define EFL_BETA_API_SUPPORT
#include <Efl_Net.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>
#include <ctype.h>

static int needed_reads = 0;

static void
_connected(void *data EINA_UNUSED, const Efl_Event *event)
{
   const char *str;
   Eina_Iterator *it;

   fprintf(stderr,
           "INFO: connected to '%s' (%s)\n"
           "INFO:  - local address=%s\n"
           "INFO:  - read-after-write=%d reads required\n"
           "INFO:  - cork=%hhu\n"
           "INFO:  - timeout_dial=%fs\n"
           "INFO:  - reuse address=%hhu\n"
           "INFO:  - reuse port=%hhu\n"
           "INFO:  - multicast TTL: %u\n"
           "INFO:  - multicast loopback: %u\n"
           "INFO:  - multicast groups:\n",
           efl_net_dialer_address_dial_get(event->object),
           efl_net_socket_address_remote_get(event->object),
           efl_net_socket_address_local_get(event->object),
           needed_reads,
           efl_net_socket_udp_cork_get(event->object),
           efl_net_dialer_timeout_dial_get(event->object),
           efl_net_socket_udp_reuse_address_get(event->object),
           efl_net_socket_udp_reuse_port_get(event->object),
           efl_net_socket_udp_multicast_time_to_live_get(event->object),
           efl_net_socket_udp_multicast_loopback_get(event->object));

   it = efl_net_socket_udp_multicast_groups_get(event->object);
   EINA_ITERATOR_FOREACH(it, str)
     fprintf(stderr, "   * %s\n", str);
   eina_iterator_free(it);
}

static void
_can_read(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Rw_Slice rw_slice;
   Eina_Error err;
   Eina_Bool can_read = efl_io_reader_can_read_get(event->object);

   /* NOTE: this message may appear with can read=0 BEFORE
    * "read '...'" because efl_io_readr_read() will change the status
    * of can_read to FALSE prior to return so we can print it!
    */
   fprintf(stderr, "INFO: can read=%d (needed reads=%d)\n", can_read, needed_reads);
   if (!can_read) return;

   if (!needed_reads) return;

   rw_slice.len = efl_net_socket_udp_next_datagram_size_query(event->object);
   rw_slice.mem = malloc(rw_slice.len);
   EINA_SAFETY_ON_NULL_RETURN(rw_slice.mem);
   err = efl_io_reader_read(event->object, &rw_slice);
   if (err)
     {
        free(rw_slice.mem);
        if (err == EAGAIN) /* EAGAIN for spurious packets */
          return;
        fprintf(stderr, "ERROR: could not read: %s\n", eina_error_msg_get(err));
        efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
        return;
     }

   fprintf(stderr, "INFO: read '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(rw_slice));

   needed_reads--;
   if (!needed_reads)
     efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
   free(rw_slice.mem);
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
             efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
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
             efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
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
             efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
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

   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_CALLBACKS_ARRAY_DEFINE(dialer_cbs,
                           { EFL_NET_DIALER_EVENT_DIALER_CONNECTED, _connected },
                           { EFL_NET_DIALER_EVENT_DIALER_RESOLVED, _resolved },
                           { EFL_NET_DIALER_EVENT_DIALER_ERROR, _error },
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
    ECORE_GETOPT_STORE_STR('b', "bind", "Bind to a particular address in the format IP:PORT."),
    ECORE_GETOPT_STORE_TRUE('r', "read-after-write", "Do a read after writes are done."),
    ECORE_GETOPT_STORE_TRUE('c', "cork", "use UDP_CORK around messages to generate a single datagram."),
    ECORE_GETOPT_STORE_TRUE('R', "dont-route", "Do not route packets via a gateway."),
    ECORE_GETOPT_STORE_DOUBLE('t', "connect-timeout", "timeout in seconds for the connection phase"),
    ECORE_GETOPT_STORE_UINT(0, "multicast-ttl",
                            "Multicast time to live in number of hops from 0-255. Defaults to 1 (only local network)."),
    ECORE_GETOPT_STORE_FALSE(0, "multicast-noloopback",
                            "Disable multicast loopback."),
    ECORE_GETOPT_APPEND('M', "multicast-group", "Join a multicast group in the form 'IP@INTERFACE', with optional '@INTERFACE', where INTERFACE is the IP address of the interface to join the multicast.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),
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
   char *address = NULL;
   char *bind_address = NULL;
   Eina_List *mcast_groups = NULL, *lst;
   char *str;
   Eina_Bool cork = EINA_FALSE;
   Eina_Bool do_read = EINA_FALSE;
   Eina_Bool dont_route = EINA_FALSE;
   unsigned mcast_ttl = 1;
   Eina_Bool mcast_loopback = EINA_TRUE;
   Eina_Bool quit_option = EINA_FALSE;
   double timeout_dial = 30.0;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(bind_address),
     ECORE_GETOPT_VALUE_BOOL(do_read),
     ECORE_GETOPT_VALUE_BOOL(cork),
     ECORE_GETOPT_VALUE_BOOL(dont_route),
     ECORE_GETOPT_VALUE_DOUBLE(timeout_dial),
     ECORE_GETOPT_VALUE_UINT(mcast_ttl),
     ECORE_GETOPT_VALUE_BOOL(mcast_loopback),
     ECORE_GETOPT_VALUE_LIST(mcast_groups),

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

   dialer = efl_add(EFL_NET_DIALER_UDP_CLASS, loop,
                    efl_name_set(efl_added, "dialer"),
                    efl_net_socket_udp_bind_set(efl_added, bind_address),
                    efl_net_socket_udp_cork_set(efl_added, cork),
                    efl_net_socket_udp_dont_route_set(efl_added, dont_route),
                    efl_net_socket_udp_reuse_address_set(efl_added, EINA_TRUE), /* optional, but nice for testing */
                    efl_net_socket_udp_reuse_port_set(efl_added, EINA_TRUE), /* optional, but nice for testing... not secure unless you know what you're doing */
                    efl_net_socket_udp_multicast_time_to_live_set(efl_added, mcast_ttl),
                    efl_net_socket_udp_multicast_loopback_set(efl_added, mcast_loopback),
                    efl_net_dialer_timeout_dial_set(efl_added, timeout_dial),
                    efl_event_callback_array_add(efl_added, dialer_cbs(), NULL));

   EINA_LIST_FOREACH(mcast_groups, lst, str)
     efl_net_socket_udp_multicast_join(dialer, str);

   err = efl_net_dialer_dial(dialer, address);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                address, eina_error_msg_get(err));
        goto no_mainloop;
     }

   if (do_read) needed_reads = cork ? 1 : 2;

   return ;

 no_mainloop:
   efl_del(dialer);

 end:
   EINA_LIST_FREE(mcast_groups, str)
     free(str);

   efl_loop_quit(efl_loop_get(ev->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
