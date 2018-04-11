#include <Efl_Net.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>

static double timeout = 30.0;
static Eo *ssl_ctx = NULL;

/* NOTE: client i/o events are only used as debug, you can omit these */

static void
_ssl_can_read_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: ssl %s can_read=%d\n",
           efl_net_socket_address_remote_get(event->object),
           efl_io_reader_can_read_get(event->object));
}

static void
_ssl_can_write_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: ssl %s can_write=%d\n",
           efl_net_socket_address_remote_get(event->object),
           efl_io_writer_can_write_get(event->object));
}

static void
_ssl_eos(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: ssl %s eos.\n",
           efl_net_socket_address_remote_get(event->object));
}

static void
_ssl_closed(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: ssl %s closed.\n",
           efl_net_socket_address_remote_get(event->object));
}

EFL_CALLBACKS_ARRAY_DEFINE(ssl_cbs,
                           { EFL_IO_READER_EVENT_CAN_READ_CHANGED, _ssl_can_read_changed },
                           { EFL_IO_READER_EVENT_EOS, _ssl_eos },
                           { EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, _ssl_can_write_changed },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _ssl_closed });


/* copier events are of interest, you should hook to at least "done"
 * and "error"
 */

/* echo copier is about the same socket, you can close it right away */

static void
_echo_copier_done(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *copier = event->object;
   fprintf(stderr, "INFO: echo copier done, close and del %p\n", copier);
   efl_del(copier); /* set to close_on_destructor, will auto close copier and ssl */
}

static void
_echo_copier_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *copier = event->object;
   const Eina_Error *perr = event->info;

   if (*perr == ETIMEDOUT)
     {
        Eo *ssl = efl_io_copier_source_get(copier);
        fprintf(stderr, "INFO: ssl '%s' timed out, delete it.\n",
                efl_net_socket_address_remote_get(ssl));
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

/* server events are mandatory, afterall you need to define what's
 * going to happen after a client socket is connected. This is the
 * "client,add" event.
 */
static void
_server_client_add(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Net_Socket *client = event->info;
   Efl_Net_Socket_Ssl *ssl;
   Eo *echo_copier;

   fprintf(stderr, "INFO: accepted client %s\n",
           efl_net_socket_address_remote_get(client));

   /* to use a client, you must efl_ref() it. Here we're not doing it
    * explicitly because Efl.Net.Socket.Ssl do take a reference.
    */
   ssl = efl_add(EFL_NET_SOCKET_SSL_CLASS, efl_loop_get(client),
                 efl_net_socket_ssl_adopt(efl_added, client, ssl_ctx), /* mandatory inside efl_add() */
                 efl_event_callback_array_add(efl_added, ssl_cbs(), NULL) /* optional, for debug purposes */
                 );
   if (!ssl)
     {
        fprintf(stderr, "ERROR: failed to wrap client=%p in SSL\n", client);
        efl_loop_quit(efl_loop_get(client), eina_value_int_init(EXIT_FAILURE));
        return;
     }

   /*
    * SSL will do a handshake and keep can_read/can_write as false
    * until it's finished, thus we can create the echo copier right
    * away.
    *
    * remember to NEVER add a copier, read or write from wrapped
    * socket, doing that will bypass the SSL layer and thus result in
    * incorrect operation. You can forget about it
    */

   echo_copier = efl_add(EFL_IO_COPIER_CLASS, efl_parent_get(ssl),
                         efl_io_copier_source_set(efl_added, ssl),
                         efl_io_copier_destination_set(efl_added, ssl),
                         efl_io_copier_timeout_inactivity_set(efl_added, timeout),
                         efl_event_callback_array_add(efl_added, echo_copier_cbs(), ssl),
                         efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE) /* we want to auto-close as we have a single copier */
                         );

   fprintf(stderr, "INFO: using an echo copier=%p for ssl %s\n",
           echo_copier, efl_net_socket_address_remote_get(ssl));
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
}

EFL_CALLBACKS_ARRAY_DEFINE(server_cbs,
                           { EFL_NET_SERVER_EVENT_CLIENT_ADD, _server_client_add },
                           { EFL_NET_SERVER_EVENT_ERROR, _server_error },
                           { EFL_NET_SERVER_EVENT_SERVING, _server_serving });

static const char *ciphers_strs[] = {
  "auto",
  "sslv3",
  "tlsv1",
  "tlsv1.1",
  "tlsv1.2",
  NULL
};

static const Ecore_Getopt options = {
  "efl_net_socket_ssl_server_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of 'upgrading' a regular Efl.Net.Socket received from an Efl.Net.Server.Tcp to a SSL socket, then serving as 'echo' server.",
  EINA_FALSE,
  {
    ECORE_GETOPT_CHOICE('c', "cipher", "Cipher to use, defaults to 'auto'", ciphers_strs),

    ECORE_GETOPT_APPEND(0, "certificate", "certificate path to use.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "private-key", "private key path to use.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "crl", "certificate revocation list to use.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "ca", "certificate authorities path to use.", ECORE_GETOPT_TYPE_STR),

    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),

    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The server address to listen, such as "
                                   "IPv4:PORT, [IPv6]:PORT, Unix socket path...",
                                   "address"),

    ECORE_GETOPT_SENTINEL
  }
};

static Eo *server = NULL;
static Eina_List *certificates = NULL;
static Eina_List *private_keys = NULL;
static Eina_List *crls = NULL;
static Eina_List *cas = NULL;

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
   if (server || ssl_ctx)
     {
        char *str;

        efl_del(server);
        server = NULL;

        efl_unref(ssl_ctx);
        ssl_ctx = NULL;

        EINA_LIST_FREE(certificates, str) free(str);
        EINA_LIST_FREE(private_keys, str) free(str);
        EINA_LIST_FREE(crls, str) free(str);
        EINA_LIST_FREE(cas, str) free(str);
     }

   fprintf(stderr, "INFO: main loop finished.\n");
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   char *address = NULL;
   char *cipher_choice = "auto";
   char *str;
   Efl_Net_Ssl_Cipher cipher = EFL_NET_SSL_CIPHER_AUTO;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(cipher_choice),

     ECORE_GETOPT_VALUE_LIST(certificates),
     ECORE_GETOPT_VALUE_LIST(private_keys),
     ECORE_GETOPT_VALUE_LIST(crls),
     ECORE_GETOPT_VALUE_LIST(cas),

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
   Eina_Iterator *it;
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

   if (!ssl_ctx)
     {
        fprintf(stderr, "ERROR: could not create the SSL context!\n");
        goto end;
     }

   fprintf(stderr, "INFO:  - certificates in use:\n");
   it = efl_net_ssl_context_certificates_get(ssl_ctx);
   EINA_ITERATOR_FOREACH(it, str)
     fprintf(stderr, "INFO:     * %s\n", str);
   eina_iterator_free(it);

   fprintf(stderr, "INFO:  - private keys in use:\n");
   it = efl_net_ssl_context_private_keys_get(ssl_ctx);
   EINA_ITERATOR_FOREACH(it, str)
     fprintf(stderr, "INFO:     * %s\n", str);
   eina_iterator_free(it);

   fprintf(stderr, "INFO:  - certificate revocation lists in use:\n");
   it = efl_net_ssl_context_certificate_revocation_lists_get(ssl_ctx);
   EINA_ITERATOR_FOREACH(it, str)
     fprintf(stderr, "INFO:     * %s\n", str);
   eina_iterator_free(it);

   fprintf(stderr, "INFO:  - certificate authorities in use:\n");
   it = efl_net_ssl_context_certificate_authorities_get(ssl_ctx);
   EINA_ITERATOR_FOREACH(it, str)
     fprintf(stderr, "INFO:     * %s\n", str);
   eina_iterator_free(it);

   server = efl_add(EFL_NET_SERVER_TCP_CLASS, ev->object, /* it's mandatory to use a main loop provider as the server parent */
                    efl_net_server_ip_ipv6_only_set(efl_added, EINA_FALSE), /* optional, but helps testing IPv4 on IPv6 servers */
                    efl_net_server_fd_reuse_address_set(efl_added, EINA_TRUE), /* optional, but nice for testing */
                    efl_net_server_fd_reuse_port_set(efl_added, EINA_TRUE), /* optional, but nice for testing... not secure unless you know what you're doing */
                    efl_event_callback_array_add(efl_added, server_cbs(), NULL)); /* mandatory to have "client,add" in order to be useful */
   if (!server)
     {
        fprintf(stderr, "ERROR: could not create class Efl.Net.Server.Tcp\n");
        goto end_ctx;
     }

   err = efl_net_server_serve(server, address);
   if (err)
     {
        fprintf(stderr, "ERROR: could not serve(%s): %s\n",
                address, eina_error_msg_get(err));
        goto end_server;
     }

   return ;

 end_server:
   efl_del(server);
   server = NULL;
 end_ctx:
   efl_unref(ssl_ctx);

 end:
   EINA_LIST_FREE(certificates, str) free(str);
   EINA_LIST_FREE(private_keys, str) free(str);
   EINA_LIST_FREE(crls, str) free(str);
   EINA_LIST_FREE(cas, str) free(str);

   efl_loop_quit(ev->object, eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
