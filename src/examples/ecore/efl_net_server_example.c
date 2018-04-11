#include <Efl_Net.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>

static Eina_Bool echo = EINA_FALSE;
static double timeout = 10.0;

/* NOTE: client i/o events are only used as debug, you can omit these */

static void
_client_can_read_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: client %s can_read=%d\n",
           efl_net_socket_address_remote_get(event->object),
           efl_io_reader_can_read_get(event->object));
}

static void
_client_can_write_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: client %s can_write=%d\n",
           efl_net_socket_address_remote_get(event->object),
           efl_io_writer_can_write_get(event->object));
}

static void
_client_eos(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: client %s eos.\n",
           efl_net_socket_address_remote_get(event->object));
}

static void
_client_closed(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: client %s closed.\n",
           efl_net_socket_address_remote_get(event->object));
}

EFL_CALLBACKS_ARRAY_DEFINE(client_cbs,
                           { EFL_IO_READER_EVENT_CAN_READ_CHANGED, _client_can_read_changed },
                           { EFL_IO_READER_EVENT_EOS, _client_eos },
                           { EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, _client_can_write_changed },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _client_closed });


/* copier events are of interest, you should hook to at least "done"
 * and "error"
 */

/* echo copier is about the same socket, you can close it right away */

static void
_echo_copier_done(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *copier = event->object;
   fprintf(stderr, "INFO: echo copier done, close and del %p\n", copier);
   efl_del(copier); /* set to close_on_destructor, will auto close copier and client */
}

static void
_echo_copier_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *copier = event->object;
   const Eina_Error *perr = event->info;

   if (*perr == ETIMEDOUT)
     {
        Eo *client = efl_io_copier_source_get(copier);
        fprintf(stderr, "INFO: client '%s' timed out, delete it.\n",
                efl_net_socket_address_remote_get(client));
        efl_del(copier);
        return;
     }

   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));

   fprintf(stderr, "ERROR: echo copier %p failed %d '%s', close and del.\n",
           copier, *perr, eina_error_msg_get(*perr));

   efl_del(copier);
}

EFL_CALLBACKS_ARRAY_DEFINE(echo_copier_cbs,
                           { EFL_IO_COPIER_EVENT_DONE, _echo_copier_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _echo_copier_error});

/* When sender and receiver peers are about different entities, you
 * can only close when both are done, otherwise the socket will be
 * prematurely closed.
 *
 * Here we use a struct with both copiers and NULL them once they are
 * done, when both are done we close the socket and free the struct.
 */
typedef struct {
   Eo *send_copier;
   Eo *recv_copier;
   Eo *client;
} Send_Recv_Data;

static Send_Recv_Data *
_send_recv_new(Eo *client)
{
   Send_Recv_Data *d = calloc(1, sizeof(Send_Recv_Data));
   if (!d) return NULL;

   /* take a reference since copiers will only hold their reference
    * while they are alive. As we're deleting them before calling
    * efl_io_closer_close(), then we need it for bit longer.
    */
   d->client = efl_ref(client);
   return d;
}

static void
_send_recv_free(Send_Recv_Data *d)
{
   efl_unref(d->client);
   free(d);
}

static void
_send_recv_done(Send_Recv_Data *d, Eo *copier)
{
   if (d->send_copier == copier) d->send_copier = NULL;
   else d->recv_copier = NULL;

   efl_del(copier);
   if (d->send_copier || d->recv_copier) return;
   efl_io_closer_close(d->client); /* manually close once both copiers are done */
   _send_recv_free(d);
}

static void
_send_copier_done(void *data, const Efl_Event *event)
{
   Eo *copier = event->object;
   Eo *buffer = efl_io_copier_source_get(copier);
   Eo *client = efl_io_copier_destination_get(copier);
   Send_Recv_Data *d = data;
   Eina_Slice slice = efl_io_buffer_slice_get(buffer);

   /* show what we sent, just for debug */
   fprintf(stderr,
           "INFO: sent to %s %zd bytes:"
           "\n--BEGIN SENT DATA--\n"
           EINA_SLICE_STR_FMT
           "\n--END SENT DATA--\n",
           efl_net_socket_address_remote_get(client),
           slice.len, EINA_SLICE_STR_PRINT(slice));

   if (d->recv_copier)
     {
        /* only start the reader inactivity timeout once the sender is done */
        efl_io_copier_timeout_inactivity_set(d->recv_copier, efl_io_copier_timeout_inactivity_get(copier));
     }

   fprintf(stderr, "INFO: send copier done, check if should close %p\n", copier);
   _send_recv_done(d, copier);
}

static void
_send_copier_error(void *data, const Efl_Event *event)
{
   Eo *copier = event->object;
   Eo *buffer = efl_io_copier_source_get(copier);
   Eo *client = efl_io_copier_destination_get(copier);
   const Eina_Error *perr = event->info;
   Send_Recv_Data *d = data;
   uint64_t offset;
   Eina_Slice slice, remaining;

   if (*perr == ETIMEDOUT)
     {
        fprintf(stderr, "INFO: client '%s' timed out send, delete it.\n",
                efl_net_socket_address_remote_get(client));
        efl_io_closer_close(copier); /* forces client to be closed, thus closes the recv copier as an effect */
        return;
     }

   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));

   offset = efl_io_buffer_position_read_get(buffer);
   slice = efl_io_buffer_slice_get(buffer);

   remaining = slice;
   remaining.bytes += offset;
   remaining.len -= offset;

   slice.len = offset;

   fprintf(stderr,
           "ERROR: sent to %s only %zd bytes:"
           "\n--BEGIN SENT DATA--\n"
           EINA_SLICE_STR_FMT
           "\n--END SENT DATA--\n"
           "Remaining %zd bytes:"
           "\n--BEGIN REMAINING DATA--\n"
           EINA_SLICE_STR_FMT
           "\n--END REMAINING DATA--\n",
           efl_net_socket_address_remote_get(client),
           slice.len, EINA_SLICE_STR_PRINT(slice),
           remaining.len, EINA_SLICE_STR_PRINT(remaining));

   fprintf(stderr, "ERROR: send copier %p failed %d '%s', check if should close..\n",
           copier, *perr, eina_error_msg_get(*perr));
   _send_recv_done(d, copier);
}

EFL_CALLBACKS_ARRAY_DEFINE(send_copier_cbs,
                           { EFL_IO_COPIER_EVENT_DONE, _send_copier_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _send_copier_error});

static void
_recv_copier_done(void *data, const Efl_Event *event)
{
   Eo *copier = event->object;
   Eo *client = efl_io_copier_source_get(copier);
   Eo *buffer = efl_io_copier_destination_get(copier);
   Send_Recv_Data *d = data;
   Eina_Slice slice = efl_io_buffer_slice_get(buffer);

   /* show case, you could use a copier to Efl_Io_Stdout, a
    * file... and get progressive processing.
    *
    * Here we're using a memory buffer and printing everything at
    * once.
    *
    * You could also steal the binbuf with
    * efl_io_buffer_binbuf_steal()
    */
   fprintf(stderr,
           "INFO: recv from %s %zd bytes:"
           "\n--BEGIN RECV DATA--\n"
           EINA_SLICE_STR_FMT "\n"
           "\n--END RECV DATA--\n",
           efl_net_socket_address_remote_get(client),
           slice.len, EINA_SLICE_STR_PRINT(slice));

   fprintf(stderr, "INFO: receive copier done, check if should close %p\n", copier);
   _send_recv_done(d, copier);
}

static void
_recv_copier_error(void *data, const Efl_Event *event)
{
   Eo *copier = event->object;
   Eo *buffer = efl_io_copier_destination_get(copier);
   Eo *client = efl_io_copier_source_get(copier);
   const Eina_Error *perr = event->info;
   Send_Recv_Data *d = data;
   Eina_Slice slice;

   if (*perr == ETIMEDOUT)
     {
        fprintf(stderr, "INFO: client '%s' timed out recv, delete it.\n",
                efl_net_socket_address_remote_get(client));
        efl_io_closer_close(copier); /* forces client to be closed, thus closes the send copier as an effect */
        return;
     }

   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));

   slice = efl_io_buffer_slice_get(buffer);
   fprintf(stderr,
           "ERROR: recv to %s only %zd bytes:"
           "\n--BEGIN RECV DATA--\n"
           EINA_SLICE_STR_FMT "\n"
           "\n--END RECV DATA--\n",
           efl_net_socket_address_remote_get(client),
           slice.len, EINA_SLICE_STR_PRINT(slice));

   fprintf(stderr, "ERROR: receive copier %p failed %d '%s', check if should close..\n",
           copier, *perr, eina_error_msg_get(*perr));
   _send_recv_done(d, copier);
}

EFL_CALLBACKS_ARRAY_DEFINE(recv_copier_cbs,
                           { EFL_IO_COPIER_EVENT_DONE, _recv_copier_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _recv_copier_error});


/* server events are mandatory, afterall you need to define what's
 * going to happen after a client socket is connected. This is the
 * "client,add" event.
 *
 * if clients_limit and clients_reject_excess are set, then
 * "client,rejected" is dispatched for rejected sockets, they contain
 * the string with socket identification.
 */
static void
_server_client_add(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Net_Socket *client = event->info;

   fprintf(stderr, "INFO: accepted client %s\n",
           efl_net_socket_address_remote_get(client));

   /* to use a client, you must efl_ref() it. Here we're not doing it
    * explicitly because copiers do take a reference.
    */

   /*
    * monitor the client socket for debug purposes (optional)
    */
   efl_event_callback_array_add(client, client_cbs(), NULL);

   /*
    * Since sockets are reader/writer/closer objects, we can use the
    * Efl_Io_Copier utility.
    */

   if (echo)
     {
        /*
         * An echo copier is pretty simple, use the socket as both
         * source and destination.
         */
        Eo *echo_copier = efl_add(EFL_IO_COPIER_CLASS, efl_parent_get(client),
                                  efl_io_copier_source_set(efl_added, client),
                                  efl_io_copier_destination_set(efl_added, client),
                                  efl_io_copier_timeout_inactivity_set(efl_added, timeout),
                                  efl_event_callback_array_add(efl_added, echo_copier_cbs(), client),
                                  efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE) /* we want to auto-close as we have a single copier */
                                  );

        fprintf(stderr, "INFO: using an echo copier=%p for client %s\n",
                echo_copier, efl_net_socket_address_remote_get(client));
        return;
     }
   else
     {
        /*
         * Here we create a fixed buffer with a string to send:
         *   - "Hello World!"
         * and another one to store the received buffer so we can print as
         * a single blob at the end.
         *
         * One can change these to Efl_Io_File or event pipe to something
         * else like Efl_Io_Stdin, Efl_Io_Stdout and it would just work.
         */
        static const Eina_Slice hello_world_slice = EINA_SLICE_STR_LITERAL("Hello World!");
        Send_Recv_Data *d;
        Eo *send_buffer, *recv_buffer;

        d = _send_recv_new(client);
        if (!d)
          {
             fprintf(stderr, "ERROR: could not allocate memory\n");
             return;
          }

        send_buffer = efl_add_ref(EFL_IO_BUFFER_CLASS, NULL,
                              efl_io_buffer_adopt_readonly(efl_added, hello_world_slice));

        /* Unlimited buffer to store the received data. */
        recv_buffer = efl_add_ref(EFL_IO_BUFFER_CLASS, NULL);

        /* an input copier that takes data from send_buffer and pushes to client */
        d->send_copier = efl_add(EFL_IO_COPIER_CLASS, efl_parent_get(client),
                                 efl_io_copier_source_set(efl_added, send_buffer),
                                 efl_io_copier_destination_set(efl_added, client),
                                 efl_io_copier_timeout_inactivity_set(efl_added, timeout),
                                 efl_event_callback_array_add(efl_added, send_copier_cbs(), d),
                                 efl_io_closer_close_on_invalidate_set(efl_added, EINA_FALSE) /* we must wait both copiers to finish before we close! */
                                 );

        fprintf(stderr, "INFO: using sender buffer %p with copier %p for client %s\n",
                send_buffer, d->send_copier, efl_net_socket_address_remote_get(client));

        efl_unref(send_buffer); /* d->send_copier adds a reference */
        if (!d->send_copier)
          fprintf(stderr, "ERROR: failed to create sender copier\n");


        /* an output copier that takes data from socket and pushes to recv_buffer. */
        d->recv_copier = efl_add(EFL_IO_COPIER_CLASS, efl_parent_get(client),
                                 efl_io_copier_source_set(efl_added, client),
                                 efl_io_copier_destination_set(efl_added, recv_buffer),
                                 efl_io_copier_timeout_inactivity_set(efl_added, 0.0), /* we'll only set an inactivity timeout once the sender is done */
                                 efl_event_callback_array_add(efl_added, recv_copier_cbs(), d),
                                 efl_io_closer_close_on_invalidate_set(efl_added, EINA_FALSE) /* we must wait both copiers to finish before we close! */
                                 );

        fprintf(stderr, "INFO: using receiver buffer %p with copier %p for client %s\n",
                recv_buffer, d->recv_copier, efl_net_socket_address_remote_get(client));

        efl_unref(recv_buffer); /* d->recv_copier adds a reference */
        if (!d->recv_copier)
          fprintf(stderr, "ERROR: failed to create receiver copier\n");

        if (!d->recv_copier && !d->send_copier)
          _send_recv_free(d);
     }
}

static void
_server_client_rejected(void *data EINA_UNUSED, const Efl_Event *event)
{
   const char *client_address = event->info;
   fprintf(stderr, "INFO: rejected client %s\n", client_address);
}

static void
_server_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Eina_Error *perr = event->info;
   fprintf(stderr, "ERROR: %d '%s'\n", *perr, eina_error_msg_get(*perr));
   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
}

static void
_server_serving(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: serving at %s\n",
           efl_net_server_address_get(event->object));

   if (efl_class_get(event->object) == EFL_NET_SERVER_TCP_CLASS)
     {
        fprintf(stderr,
                "TCP options:\n"
                " - IPv6 only: %u\n",
                efl_net_server_ip_ipv6_only_get(event->object));
     }
   else if (efl_class_get(event->object) == EFL_NET_SERVER_UDP_CLASS)
     {
        Eina_Iterator *it;
        const char *str;

        fprintf(stderr,
                "UDP options:\n"
                " - IPv6 only: %u\n"
                " - don't route: %u\n"
                " - multicast TTL: %u\n"
                " - multicast loopback: %u\n"
                " - multicast groups:\n",
                efl_net_server_ip_ipv6_only_get(event->object),
                efl_net_server_udp_dont_route_get(event->object),
                efl_net_server_udp_multicast_time_to_live_get(event->object),
                efl_net_server_udp_multicast_loopback_get(event->object));

        it = efl_net_server_udp_multicast_groups_get(event->object);
        EINA_ITERATOR_FOREACH(it, str)
          fprintf(stderr, "   * %s\n", str);
        eina_iterator_free(it);
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(server_cbs,
                           { EFL_NET_SERVER_EVENT_CLIENT_ADD, _server_client_add },
                           { EFL_NET_SERVER_EVENT_CLIENT_REJECTED, _server_client_rejected },
                           { EFL_NET_SERVER_EVENT_ERROR, _server_error },
                           { EFL_NET_SERVER_EVENT_SERVING, _server_serving });

static const char * protocols[] = {
  "tcp",
  "udp",
  "ssl",
#ifdef EFL_NET_SERVER_WINDOWS_CLASS
  "windows",
#endif
#ifdef EFL_NET_SERVER_UNIX_CLASS
  "unix",
#endif
  NULL
};

static const char *ciphers_strs[] = {
  "auto",
  "sslv3",
  "tlsv1",
  "tlsv1.1",
  "tlsv1.2",
  NULL
};

static const Ecore_Getopt options = {
  "efl_net_server_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Efl_Net_Server objects usage.\n"
  "\n"
  "This example spawns a server of the given protocol at the given address.",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_TRUE('e', "echo",
                            "Behave as 'echo' server, send back to client all the data receive"),
    ECORE_GETOPT_STORE_TRUE(0, "socket-activated",
                            "Try to use $LISTEN_FDS from systemd, if not do a regular serve()"),
    ECORE_GETOPT_STORE_UINT('l', "clients-limit",
                            "If set will limit number of clients to accept"),
    ECORE_GETOPT_STORE_TRUE('r', "clients-reject-excess",
                            "Immediately reject excess clients (over limit)"),
    ECORE_GETOPT_STORE_FALSE(0, "ipv4-on-ipv6",
                            "IPv4 clients will be automatically converted into IPv6 and handled transparently."),
    ECORE_GETOPT_STORE_DOUBLE('t', "inactivity-timeout",
                              "The timeout in seconds to disconnect a client. The timeout is restarted for each client when there is some activity. It's particularly useful for UDP where there is no disconnection event."),

    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),

    ECORE_GETOPT_CATEGORY("udp", "UDP options"),
    ECORE_GETOPT_STORE_TRUE(0, "udp-dont-route",
                            "If true, datagrams won't be routed using a gateway, being restricted to the local network."),
    ECORE_GETOPT_STORE_UINT(0, "udp-multicast-ttl",
                            "Multicast time to live in number of hops from 0-255. Defaults to 1 (only local network)."),
    ECORE_GETOPT_STORE_FALSE(0, "udp-multicast-noloopback",
                            "Disable multicast loopback."),
    ECORE_GETOPT_APPEND('M', "udp-multicast-group", "Join a multicast group in the form 'IP@INTERFACE', with optional '@INTERFACE', where INTERFACE is the IP address of the interface to join the multicast.", ECORE_GETOPT_TYPE_STR),

    ECORE_GETOPT_CATEGORY("ssl", "SSL options"),
    ECORE_GETOPT_CHOICE('c', "ssl-cipher", "Cipher to use, defaults to 'auto'", ciphers_strs),
    ECORE_GETOPT_APPEND(0, "ssl-certificate", "certificate path to use.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "ssl-private-key", "private key path to use.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "ssl-crl", "certificate revocation list to use.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "ssl-ca", "certificate authorities path to use.", ECORE_GETOPT_TYPE_STR),

    ECORE_GETOPT_CHOICE_METAVAR(0, NULL, "The server protocol.", "protocol",
                                protocols),
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The server address to listen, such as "
                                   "IPv4:PORT, [IPv6]:PORT, Unix socket path...",
                                   "address"),

    ECORE_GETOPT_SENTINEL
  }
};

static Eo *server = NULL;

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
   if (server)
     {
        efl_del(server);
        server = NULL;
     }

   fprintf(stderr, "INFO: main loop finished.\n");
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   const Efl_Class *cls;
   char *protocol = NULL;
   char *address = NULL;
   Eina_List *udp_mcast_groups = NULL;
   char *str;
   unsigned int clients_limit = 0;
   unsigned udp_mcast_ttl = 1;
   Eina_Bool clients_reject_excess = EINA_FALSE;
   Eina_Bool ipv6_only = EINA_TRUE;
   Eina_Bool udp_dont_route = EINA_FALSE;
   Eina_Bool udp_mcast_loopback = EINA_TRUE;
   Eina_List *certificates = NULL;
   Eina_List *private_keys = NULL;
   Eina_List *crls = NULL;
   Eina_List *cas = NULL;
   char *cipher_choice = NULL;
   Eina_Bool socket_activated = EINA_FALSE;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(echo),
     ECORE_GETOPT_VALUE_BOOL(socket_activated),
     ECORE_GETOPT_VALUE_UINT(clients_limit),
     ECORE_GETOPT_VALUE_BOOL(clients_reject_excess),
     ECORE_GETOPT_VALUE_BOOL(ipv6_only),
     ECORE_GETOPT_VALUE_DOUBLE(timeout),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     ECORE_GETOPT_VALUE_BOOL(quit_option), /* category: udp */
     ECORE_GETOPT_VALUE_BOOL(udp_dont_route),
     ECORE_GETOPT_VALUE_UINT(udp_mcast_ttl),
     ECORE_GETOPT_VALUE_BOOL(udp_mcast_loopback),
     ECORE_GETOPT_VALUE_LIST(udp_mcast_groups),

     ECORE_GETOPT_VALUE_BOOL(quit_option), /* category: ssl */
     ECORE_GETOPT_VALUE_STR(cipher_choice),
     ECORE_GETOPT_VALUE_LIST(certificates),
     ECORE_GETOPT_VALUE_LIST(private_keys),
     ECORE_GETOPT_VALUE_LIST(crls),
     ECORE_GETOPT_VALUE_LIST(cas),

     /* positional argument */
     ECORE_GETOPT_VALUE_STR(protocol),
     ECORE_GETOPT_VALUE_STR(address),

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args;
   Eina_Error err;

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

   if (!protocol)
     {
        fputs("ERROR: missing protocol.\n", stderr);
        goto end;
     }

   if (strcmp(protocol, "tcp") == 0) cls = EFL_NET_SERVER_TCP_CLASS;
   else if (strcmp(protocol, "udp") == 0) cls = EFL_NET_SERVER_UDP_CLASS;
   else if (strcmp(protocol, "ssl") == 0) cls = EFL_NET_SERVER_SSL_CLASS;
#ifdef EFL_NET_SERVER_WINDOWS_CLASS
   else if (strcmp(protocol, "windows") == 0) cls = EFL_NET_SERVER_WINDOWS_CLASS;
#endif
#ifdef EFL_NET_SERVER_UNIX_CLASS
   else if (strcmp(protocol, "unix") == 0) cls = EFL_NET_SERVER_UNIX_CLASS;
#endif
   else
     {
        fprintf(stderr, "ERROR: unsupported protocol: %s\n", protocol);
        goto end;
     }

   server = efl_add(cls, ev->object, /* it's mandatory to use a main loop provider as the server parent */
                    efl_net_server_clients_limit_set(efl_added,
                                                     clients_limit,
                                                     clients_reject_excess), /* optional */
                    efl_event_callback_array_add(efl_added, server_cbs(), NULL)); /* mandatory to have "client,add" in order to be useful */
   if (!server)
     {
        fprintf(stderr, "ERROR: could not create class %p (%s)\n",
                cls, efl_class_name_get(cls));
        goto end;
     }

   if (cls == EFL_NET_SERVER_TCP_CLASS)
     {
        efl_net_server_ip_ipv6_only_set(server, ipv6_only);
        efl_net_server_fd_reuse_address_set(server, EINA_TRUE); /* optional, but nice for testing */
        efl_net_server_fd_reuse_port_set(server, EINA_TRUE); /* optional, but nice for testing... not secure unless you know what you're doing */

        if (socket_activated) efl_net_server_fd_socket_activate(server, address);
     }
   else if (cls == EFL_NET_SERVER_UDP_CLASS)
     {
        const Eina_List *lst;

        efl_net_server_ip_ipv6_only_set(server, ipv6_only);
        efl_net_server_udp_dont_route_set(server, udp_dont_route);

        efl_net_server_udp_multicast_time_to_live_set(server, udp_mcast_ttl);
        efl_net_server_udp_multicast_loopback_set(server, udp_mcast_loopback);

        EINA_LIST_FOREACH(udp_mcast_groups, lst, str)
          efl_net_server_udp_multicast_join(server, str);


        efl_net_server_fd_reuse_address_set(server, EINA_TRUE); /* optional, but nice for testing */
        efl_net_server_fd_reuse_port_set(server, EINA_TRUE); /* optional, but nice for testing... not secure unless you know what you're doing */
        if (socket_activated) efl_net_server_fd_socket_activate(server, address);
     }
   else if (cls == EFL_NET_SERVER_SSL_CLASS)
     {
        Eo *ssl_ctx;
        Efl_Net_Ssl_Cipher cipher = EFL_NET_SSL_CIPHER_AUTO;
        if (cipher_choice)
          {
             if (strcmp(cipher_choice, "auto") == 0)
               cipher = EFL_NET_SSL_CIPHER_AUTO;
             else if (strcmp(cipher_choice, "sslv3") == 0)
               cipher = EFL_NET_SSL_CIPHER_SSLV3;
             else if (strcmp(cipher_choice, "tlsv1") == 0)
               cipher = EFL_NET_SSL_CIPHER_TLSV1;
             else if (strcmp(cipher_choice, "tlsv1.1") == 0)
               cipher = EFL_NET_SSL_CIPHER_TLSV1_1;
             else if (strcmp(cipher_choice, "tlsv1.2") == 0)
               cipher = EFL_NET_SSL_CIPHER_TLSV1_2;
          }

        ssl_ctx = efl_add_ref(EFL_NET_SSL_CONTEXT_CLASS, NULL,
                          efl_net_ssl_context_certificates_set(efl_added, eina_list_iterator_new(certificates)),
                          efl_net_ssl_context_private_keys_set(efl_added, eina_list_iterator_new(private_keys)),
                          efl_net_ssl_context_certificate_revocation_lists_set(efl_added, eina_list_iterator_new(crls)),
                          efl_net_ssl_context_certificate_authorities_set(efl_added, eina_list_iterator_new(cas)),
                          efl_net_ssl_context_setup(efl_added, cipher, EINA_FALSE /* a server! */));

        efl_net_server_ssl_context_set(server, ssl_ctx);

        efl_net_server_fd_reuse_address_set(server, EINA_TRUE); /* optional, but nice for testing */
        efl_net_server_fd_reuse_port_set(server, EINA_TRUE); /* optional, but nice for testing... not secure unless you know what you're doing */
        if (socket_activated) efl_net_server_fd_socket_activate(server, address);
     }
#ifdef EFL_NET_SERVER_UNIX_CLASS
   else if (cls == EFL_NET_SERVER_UNIX_CLASS)
     {
        efl_net_server_unix_unlink_before_bind_set(server, EINA_TRUE); /* makes testing easier */
        if (socket_activated) efl_net_server_fd_socket_activate(server, address);
     }
#endif

   /* an explicit call to efl_net_server_serve() after the object is
    * constructed allows for more complex setup, such as interacting
    * with the object to add more properties that couldn't be done
    * during efl_add().
    */
   if (!efl_net_server_serving_get(server))
     {
        if (socket_activated)
          fprintf(stderr, "WARNING: --socket-activated, but not able to use $LISTEN_FDS descriptors. Try to start the server...\n");

        err = efl_net_server_serve(server, address);
        if (err)
          {
             fprintf(stderr, "ERROR: could not serve(%s): %s\n",
                     address, eina_error_msg_get(err));
             goto end_server;
          }
     }

   return ;

 end_server:
   efl_del(server);
   server = NULL;

 end:
   EINA_LIST_FREE(udp_mcast_groups, str)
     free(str);

   efl_loop_quit(efl_loop_get(ev->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
