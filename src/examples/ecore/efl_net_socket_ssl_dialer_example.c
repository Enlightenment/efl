#include <Efl_Net.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>
#include <ctype.h>

static Eina_List *pending_send = NULL;
static size_t pending_send_offset = 0;
static Eo *ssl_ctx = NULL;

static void
_ssl_ready(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   fprintf(stderr, "INFO: SSL ready!\n");
}

static void
_ssl_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Eina_Error *perr = event->info;
   fprintf(stderr, "INFO: SSL error: %d '%s'\n", *perr, eina_error_msg_get(*perr));
   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
}

static void
_ssl_can_read(void *data EINA_UNUSED, const Efl_Event *event)
{
   char buf[63]; /* INFO: SSL read '...' will fit in 80 columns */
   Eina_Error err;
   Eina_Bool can_read = efl_io_reader_can_read_get(event->object);

   /* NOTE: this message may appear with can read=0 BEFORE
    * "read '...'" because efl_io_readr_read() will change the status
    * of can_read to FALSE prior to return so we can print it!
    */
   fprintf(stderr, "INFO: SSL can read=%d\n", can_read);
   if (!can_read) return;

   do
     {
        Eina_Rw_Slice rw_slice;

        /* reset on every read, as rw_slice will be modified! */
        rw_slice.len = sizeof(buf);
        rw_slice.mem = buf;
        err = efl_io_reader_read(event->object, &rw_slice);
        if (err)
          {
             if (err == EAGAIN) return;
             fprintf(stderr, "ERROR: could not read: %s\n", eina_error_msg_get(err));
             efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
             return;
          }

        fprintf(stderr, "INFO: SSL read '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(rw_slice));
     }
   while (efl_io_reader_can_read_get(event->object));
}

static void
_ssl_eos(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   fprintf(stderr, "INFO: SSL eos\n");
   efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
}

static void
_ssl_can_write(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Bool can_write = efl_io_writer_can_write_get(event->object);
   size_t len;

   /* NOTE: this message may appear with can write=0 BEFORE
    * "wrote '...'" because efl_io_writer_write() will change the status
    * of can_write to FALSE prior to return so we can print it!
    */
   fprintf(stderr, "INFO: SSL can write=%d (needed writes=%u offset=%zd)\n", can_write, eina_list_count(pending_send), pending_send_offset);
   if (!can_write) return;

   if (!pending_send) return;

   do
     {
        Eina_Slice slice;
        Eina_Error err;

        slice.bytes = pending_send->data;
        slice.bytes += pending_send_offset;
        slice.len = len = strlen(slice.mem);

        err = efl_io_writer_write(event->object, &slice, NULL);
        if (err)
          {
             if (err == EAGAIN) return;
             fprintf(stderr, "ERROR: could not write: %s\n", eina_error_msg_get(err));
             efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
             return;
          }

        fprintf(stderr, "INFO: SSL wrote '" EINA_SLICE_STR_FMT "'\n", EINA_SLICE_STR_PRINT(slice));

        pending_send_offset += slice.len;
        if (pending_send_offset == strlen(pending_send->data))
          {
             free(pending_send->data);
             pending_send = eina_list_remove_list(pending_send, pending_send);
             pending_send_offset = 0;
          }
     }
   while ((pending_send) && (efl_io_writer_can_write_get(event->object)));
}

static void
_ssl_closed(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: SSL closed\n");
   efl_del(event->object);
}

EFL_CALLBACKS_ARRAY_DEFINE(ssl_cbs,
                           { EFL_NET_SOCKET_SSL_EVENT_SSL_READY, _ssl_ready },
                           { EFL_NET_SOCKET_SSL_EVENT_SSL_ERROR, _ssl_error },
                           { EFL_IO_READER_EVENT_CAN_READ_CHANGED, _ssl_can_read },
                           { EFL_IO_READER_EVENT_EOS, _ssl_eos },
                           { EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, _ssl_can_write },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _ssl_closed });

static void
_connected(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *ssl;
   const char *hostname;

   fprintf(stderr,
           "INFO: connected to '%s' (%s)\n"
           "INFO:  - local address=%s\n",
           efl_net_dialer_address_dial_get(event->object),
           efl_net_socket_address_remote_get(event->object),
           efl_net_socket_address_local_get(event->object));

   ssl = efl_add(EFL_NET_SOCKET_SSL_CLASS, efl_parent_get(event->object),
                 efl_net_socket_ssl_adopt(efl_added, event->object, ssl_ctx),
                 efl_event_callback_array_add(efl_added, ssl_cbs(), NULL));
   if (!ssl)
     {
        fprintf(stderr, "ERROR: failed to wrap dialer=%p in SSL\n", event->object);
        efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
        return;
     }

   hostname = efl_net_socket_ssl_hostname_override_get(ssl);
   if (!hostname) hostname = "<none>";

   fprintf(stderr,
           "INFO:  - verify-mode=%d\n"
           "INFO:  - hostname-verify=%d\n"
           "INFO:  - hostname-override='%s'\n",
           efl_net_socket_ssl_verify_mode_get(ssl),
           efl_net_socket_ssl_hostname_verify_get(ssl),
           hostname);
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
                           { EFL_NET_DIALER_EVENT_CONNECTED, _connected },
                           { EFL_NET_DIALER_EVENT_RESOLVED, _resolved },
                           { EFL_NET_DIALER_EVENT_ERROR, _error });


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

/*
 * Define USE_DEFAULT_CONTEXT will remove all context-setup functions
 * and use a default context for dialers, what most applications
 * should use.
 */
//#define USE_DEFAULT_CONTEXT 1

#ifndef USE_DEFAULT_CONTEXT
static const char *verify_mode_strs[] = {
  "none",
  "optional",
  "required",
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
#endif

static const Ecore_Getopt options = {
  "efl_net_socket_ssl_dialer_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of 'upgrading' a regular Efl.Net.Dialer.Tcp to a SSL socket.",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_STR('d', "line-delimiter",
                           "Changes the line delimiter to be used in both send and receive. Defaults to \\r\\n"),
    ECORE_GETOPT_APPEND('s', "send", "send the given string to the server once connected.", ECORE_GETOPT_TYPE_STR),

#ifndef USE_DEFAULT_CONTEXT
    ECORE_GETOPT_CHOICE('c', "cipher", "Cipher to use, defaults to 'auto'", ciphers_strs),
    ECORE_GETOPT_APPEND(0, "certificate", "certificate path to use.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "private-key", "private key path to use.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "crl", "certificate revocation list to use.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "ca", "certificate authorities path to use.", ECORE_GETOPT_TYPE_STR),

    ECORE_GETOPT_STORE_FALSE(0, "no-default-paths", "Do not use default certificate paths from your system."),
    ECORE_GETOPT_CHOICE(0, "verify-mode", "One of: none (do not verify), optional (verify if provided), required (require and verify). Defaults to required", verify_mode_strs),
    ECORE_GETOPT_STORE_FALSE(0, "no-hostname-verify", "Do not Verify hostname"),
    ECORE_GETOPT_STORE_STR(0, "hostname-override", "Use this hostname instead of server provided one"),
#endif

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

#ifndef USE_DEFAULT_CONTEXT
static Eina_List *certificates = NULL;
static Eina_List *private_keys = NULL;
static Eina_List *crls = NULL;
static Eina_List *cas = NULL;
#endif

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
   if (dialer || ssl_ctx)
     {
#ifndef USE_DEFAULT_CONTEXT
        char *str;
#endif

        efl_io_closer_close(dialer); /* just del won't do as ssl has an extra ref */
        efl_del(dialer);
        dialer = NULL;

        efl_unref(ssl_ctx);
        ssl_ctx = NULL;

        EINA_LIST_FREE(pending_send, str) free(str);

#ifndef USE_DEFAULT_CONTEXT
        EINA_LIST_FREE(certificates, str) free(str);
        EINA_LIST_FREE(private_keys, str) free(str);
        EINA_LIST_FREE(crls, str) free(str);
        EINA_LIST_FREE(cas, str) free(str);
#endif
     }

   fprintf(stderr, "INFO: main loop finished.\n");
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   char *address = NULL;
   char *line_delimiter_str = NULL;
   char *str;
   Eina_List *to_send = NULL;
#ifndef USE_DEFAULT_CONTEXT
   Eina_Iterator *it;
   char *verify_mode_choice = "required";
   char *cipher_choice = "auto";
   Eina_Bool default_paths_load = EINA_TRUE;
   Efl_Net_Ssl_Verify_Mode verify_mode = EFL_NET_SSL_VERIFY_MODE_OPTIONAL;
   Efl_Net_Ssl_Cipher cipher = EFL_NET_SSL_CIPHER_AUTO;
   Eina_Bool hostname_verify = EINA_TRUE;
   char *hostname_override = NULL;
#endif
   Eina_Bool quit_option = EINA_FALSE;
   double timeout_dial = 30.0;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(line_delimiter_str),
     ECORE_GETOPT_VALUE_LIST(to_send),

#ifndef USE_DEFAULT_CONTEXT
     ECORE_GETOPT_VALUE_STR(cipher_choice),

     ECORE_GETOPT_VALUE_LIST(certificates),
     ECORE_GETOPT_VALUE_LIST(private_keys),
     ECORE_GETOPT_VALUE_LIST(crls),
     ECORE_GETOPT_VALUE_LIST(cas),

     ECORE_GETOPT_VALUE_BOOL(default_paths_load),
     ECORE_GETOPT_VALUE_STR(verify_mode_choice),
     ECORE_GETOPT_VALUE_BOOL(hostname_verify),
     ECORE_GETOPT_VALUE_STR(hostname_override),
#endif

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

   ecore_init();
   ecore_con_init();

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

#ifndef USE_DEFAULT_CONTEXT
   if (verify_mode_choice)
     {
        if (strcmp(verify_mode_choice, "none") == 0)
          verify_mode = EFL_NET_SSL_VERIFY_MODE_NONE;
        else if (strcmp(verify_mode_choice, "optional") == 0)
          verify_mode = EFL_NET_SSL_VERIFY_MODE_OPTIONAL;
        else if (strcmp(verify_mode_choice, "required") == 0)
          verify_mode = EFL_NET_SSL_VERIFY_MODE_REQUIRED;
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
#endif

   if (to_send)
     {
        line_delimiter_str = _unescape(line_delimiter_str ? line_delimiter_str : "\\r\\n");
        if (line_delimiter_str[0] == '\0')
          {
             pending_send = to_send;
             to_send = NULL;
          }
        else
          {
             EINA_LIST_FREE(to_send, str)
               {
                  /* ignore empty sends, but add line delimiter, so we can do HTTP's last line :-) */
                  if (str[0] == '\0')
                    free(str);
                  else
                    pending_send = eina_list_append(pending_send, str);

                  if (line_delimiter_str[0])
                    pending_send = eina_list_append(pending_send, strdup(line_delimiter_str));
               }
          }
        free(line_delimiter_str);
        line_delimiter_str = NULL;
     }

   /* create a new SSL context with command line configurations.
    * another option would be to use the default dialer context */
#ifndef USE_DEFAULT_CONTEXT
   ssl_ctx = efl_add_ref(EFL_NET_SSL_CONTEXT_CLASS, NULL,
                     efl_net_ssl_context_certificates_set(efl_added, eina_list_iterator_new(certificates)),
                     efl_net_ssl_context_private_keys_set(efl_added, eina_list_iterator_new(private_keys)),
                     efl_net_ssl_context_certificate_revocation_lists_set(efl_added, eina_list_iterator_new(crls)),
                     efl_net_ssl_context_certificate_authorities_set(efl_added, eina_list_iterator_new(cas)),
                     efl_net_ssl_context_default_paths_load_set(efl_added, default_paths_load),
                     efl_net_ssl_context_verify_mode_set(efl_added, verify_mode),
                     efl_net_ssl_context_hostname_verify_set(efl_added, hostname_verify),
                     efl_net_ssl_context_hostname_set(efl_added, hostname_override),
                     efl_net_ssl_context_setup(efl_added, cipher, EINA_TRUE));
#else
   ssl_ctx = efl_net_ssl_context_default_dialer_get(EFL_NET_SSL_CONTEXT_CLASS);
   fprintf(stderr, "INFO: using default context for dialers.\n");
#endif

   if (!ssl_ctx)
     {
        fprintf(stderr, "ERROR: could not create the SSL context!\n");
        goto no_ssl_ctx;
     }

   /* no point in printing default context, it's all empty */
#ifndef USE_DEFAULT_CONTEXT
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

#endif

   fprintf(stderr,
           "INFO:  - verify-mode=%d\n",
           efl_net_ssl_context_verify_mode_get(ssl_ctx));

   dialer = efl_add(EFL_NET_DIALER_TCP_CLASS, loop,
                    efl_name_set(efl_added, "dialer"),
                    efl_net_dialer_timeout_dial_set(efl_added, timeout_dial),
                    efl_event_callback_array_add(efl_added, dialer_cbs(), NULL));

   err = efl_net_dialer_dial(dialer, address);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                address, eina_error_msg_get(err));
        goto no_mainloop;
     }

   return ;

 no_mainloop:
   efl_io_closer_close(dialer); /* just del won't do as ssl has an extra ref */
   efl_del(dialer);
 no_ssl_ctx:
   efl_unref(ssl_ctx);

 end:
   EINA_LIST_FREE(pending_send, str) free(str);

#ifndef USE_DEFAULT_CONTEXT
   EINA_LIST_FREE(certificates, str) free(str);
   EINA_LIST_FREE(private_keys, str) free(str);
   EINA_LIST_FREE(crls, str) free(str);
   EINA_LIST_FREE(cas, str) free(str);
#endif

   efl_loop_quit(ev->object, eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
