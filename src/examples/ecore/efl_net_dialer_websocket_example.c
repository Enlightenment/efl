#define EFL_BETA_API_SUPPORT
#include <Efl_Net.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>
#include <ctype.h>

static int lines_text = 0;
static int lines_binary = 0;

static void
_dummy_send(Eo *dialer, Eina_Bool text, size_t lines)
{
    size_t len = lines * 80;
    char *buf = malloc(len + 1);
    const size_t az_range = 'Z' - 'A';
    size_t i;

    for (i = 0; i < lines; i++) {
        char *ln = buf + i * 80;
        uint8_t chr;

        snprintf(ln, 11, "%9zd ", i + 1);
        if (text)
            chr = (i % az_range) + 'A';
        else
            chr = i & 0xff;
        memset(ln + 10, chr, 69);
        ln[79] = '\n';
    }
    buf[len] = '\0';

    if (text)
      efl_net_dialer_websocket_text_send(dialer, buf);
    else
      {
         Eina_Slice slice = {.mem = buf, .len = len};
         efl_net_dialer_websocket_binary_send(dialer, slice);
      }
    free(buf);
}

static void
_ws_pong(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: got PONG: %s\n", (const char *)event->info);

   efl_net_dialer_websocket_close_request(event->object,
                                          EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_NORMAL,
                                          "close it!");
}

static void
_ws_closed_reason(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Net_Dialer_Websocket_Closed_Reason *reason = event->info;
   fprintf(stderr, "INFO: got CLOSE: %4d '%s'\n",
           reason->reason, reason->message);
}

static void
_ws_message_text(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: got TEXT:\n%s\n", (const char *)event->info);

   if (lines_text < 5)
     _dummy_send(event->object, EINA_TRUE, ++lines_text);
   else
     _dummy_send(event->object, EINA_FALSE, ++lines_binary);
}

static void
_ws_message_binary(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Eina_Slice *slice = event->info;
   size_t i;

   fprintf(stderr, "INFO: got BINARY %zd bytes\n", slice->len);

   for (i = 0; i < slice->len; i++)
     {
        const int c = slice->bytes[i];
        if (isprint(c))
          fprintf(stderr, " %#4x(%c)", c, c);
        else
          fprintf(stderr, " %#4x", c);
     }

   fprintf(stderr, "\n");

   if (lines_binary < 5)
     _dummy_send(event->object, EINA_FALSE, ++lines_binary);
   else
     efl_net_dialer_websocket_ping(event->object, "will close on pong");
}

static void
_closed(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: closed %s\n",
           efl_name_get(event->object));
   efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
}

static void
_eos(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: eos %s\n",
           efl_name_get(event->object));
}

static void
_connected(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Stringshare *protocol;
   Eina_Iterator *itr;
   fprintf(stderr, "INFO: connected %s\n",
           efl_net_dialer_address_dial_get(event->object));

   itr = efl_net_dialer_websocket_response_protocols_get(event->object);
   EINA_ITERATOR_FOREACH(itr, protocol)
     fprintf(stderr, "INFO: server protocol: %s\n", protocol);
   eina_iterator_free(itr);

   _dummy_send(event->object, EINA_TRUE, ++lines_text);
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
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_PONG, _ws_pong },
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_CLOSED_REASON, _ws_closed_reason },
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_TEXT, _ws_message_text },
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_BINARY, _ws_message_binary },
                           { EFL_NET_DIALER_EVENT_CONNECTED, _connected },
                           { EFL_NET_DIALER_EVENT_RESOLVED, _resolved },
                           { EFL_NET_DIALER_EVENT_ERROR, _error },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _closed },
                           { EFL_IO_READER_EVENT_EOS, _eos });


static const char *authentication_method_choices[] = {
  "none",
  "basic",
  "digest",
  "negotiate",
  "ntlm",
  "ntlm_winbind",
  "any_safe",
  "any",
  NULL,
};

static Efl_Net_Http_Authentication_Method
_parse_authentication_method(const char *str)
{
   if (strcmp(str, "basic") == 0)
     return EFL_NET_HTTP_AUTHENTICATION_METHOD_BASIC;
   if (strcmp(str, "digest") == 0)
     return EFL_NET_HTTP_AUTHENTICATION_METHOD_DIGEST;
   if (strcmp(str, "negotiate") == 0)
     return EFL_NET_HTTP_AUTHENTICATION_METHOD_NEGOTIATE;
   if (strcmp(str, "ntlm") == 0)
     return EFL_NET_HTTP_AUTHENTICATION_METHOD_NTLM;
   if (strcmp(str, "ntlm_winbind") == 0)
     return EFL_NET_HTTP_AUTHENTICATION_METHOD_NTLM_WINBIND;
   if (strcmp(str, "any_safe") == 0)
     return EFL_NET_HTTP_AUTHENTICATION_METHOD_ANY_SAFE;
   if (strcmp(str, "any") == 0)
     return EFL_NET_HTTP_AUTHENTICATION_METHOD_ANY;

   return EFL_NET_HTTP_AUTHENTICATION_METHOD_NONE;
}

static const Ecore_Getopt options = {
  "efl_net_dialer_websocket_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Efl_Net_Dialer_Websocket usage in message-based mode.\n"
  "In this example couple of text and binary messages are sent to the server, "
  "as well as a ping. On pong the websocket is closed."
  "\n"
  "For the EFL I/O interfaces example, see efl_io_copier_example.c"
  "\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_APPEND('p', "websocket-protocol", "WebSocket protocol to request", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_STORE_STR('U', "username", "Authentication username"),
    ECORE_GETOPT_STORE_STR('P', "password", "Authentication password"),
    ECORE_GETOPT_CHOICE('A', "authentication-method", "Authentication method", authentication_method_choices),
    ECORE_GETOPT_STORE_TRUE('R', "authentication-restricted", "Authentication method must be restricted"),
    ECORE_GETOPT_STORE_FALSE('r', "disallow-redirects", "disallow redirections by following 'Location:' headers. Allowed by default."),
    ECORE_GETOPT_STORE_DOUBLE('t', "connect-timeout", "timeout in seconds for the connection phase"),
    ECORE_GETOPT_APPEND('H', "header", "Add custom headers. Format must be 'Key: Value'", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_STORE_STR('X', "proxy", "Set a specific proxy for the connection"),
    ECORE_GETOPT_STORE_STR('c', "cookie-jar", "Set the cookie-jar file to read/save cookies from. Empty means an in-memory cookie-jar"),

    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The address (URL) to dial, such as wss://echo.websocket.org", "address"),
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
   char *username = NULL;
   char *password = NULL;
   char *authentication_method_str = "basic";
   char *proxy = NULL;
   char *cookie_jar = NULL;
   Eina_Bool authentication_restricted = EINA_FALSE;
   Eina_Bool allow_redirects = EINA_TRUE;
   double timeout_dial = 30.0;
   Eina_List *headers = NULL;
   Eina_List *protocols = NULL;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_LIST(protocols),
     ECORE_GETOPT_VALUE_STR(username),
     ECORE_GETOPT_VALUE_STR(password),
     ECORE_GETOPT_VALUE_STR(authentication_method_str),
     ECORE_GETOPT_VALUE_BOOL(authentication_restricted),
     ECORE_GETOPT_VALUE_BOOL(allow_redirects),
     ECORE_GETOPT_VALUE_DOUBLE(timeout_dial),
     ECORE_GETOPT_VALUE_LIST(headers),
     ECORE_GETOPT_VALUE_STR(proxy),
     ECORE_GETOPT_VALUE_STR(cookie_jar),

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
   Efl_Net_Http_Authentication_Method authentication_method;
   Efl_Net_Http_Header *header;
   Eina_Iterator *itr;
   Eina_Error err;
   char *str;

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

   authentication_method = _parse_authentication_method(authentication_method_str);

   if (cookie_jar && cookie_jar[0] == ' ')
     cookie_jar = "";

   dialer = efl_add(EFL_NET_DIALER_WEBSOCKET_CLASS, loop,
                    efl_name_set(efl_added, "dialer"),
                    efl_net_dialer_websocket_authentication_set(efl_added, username, password, authentication_method, authentication_restricted),
                    efl_net_dialer_websocket_allow_redirects_set(efl_added, allow_redirects),
                    efl_net_dialer_websocket_cookie_jar_set(efl_added, cookie_jar),
                    efl_net_dialer_proxy_set(efl_added, proxy),
                    efl_net_dialer_timeout_dial_set(efl_added, timeout_dial),
                    efl_event_callback_array_add(efl_added, dialer_cbs(), NULL));
   if (!dialer)
     {
        fprintf(stderr, "ERROR: could not create WebSockets dialer\n");
        goto end;
     }

   EINA_LIST_FREE(headers, str)
     {
        char *p = strchr(str, ':');
        if (p)
          {
             p[0] = '\0';
             p++;
             while ((p[0] != '\0') && isspace(p[0]))
               p++;
          }
        efl_net_dialer_websocket_request_header_add(dialer, str, p);
        free(str);
     }

   EINA_LIST_FREE(protocols, str)
     {
        efl_net_dialer_websocket_request_protocol_add(dialer, str);
        free(str);
     }

   err = efl_net_dialer_dial(dialer, address);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                address, eina_error_msg_get(err));
        goto no_mainloop;
     }

   fprintf(stderr,
           "INFO: dialed %s\n"
           "INFO:  - allow_redirects=%d\n"
           "INFO:  - cookie_jar=%s\n"
           "INFO:  - timeout_dial=%fs\n"
           "INFO:  - proxy=%s\n"
           "INFO:  - request headers:\n",
           efl_net_dialer_address_dial_get(dialer),
           efl_net_dialer_websocket_allow_redirects_get(dialer),
           efl_net_dialer_websocket_cookie_jar_get(dialer),
           efl_net_dialer_timeout_dial_get(dialer),
           efl_net_dialer_proxy_get(dialer));
   itr = efl_net_dialer_websocket_request_headers_get(dialer);
   EINA_ITERATOR_FOREACH(itr, header)
     fprintf(stderr, "INFO:       %s: %s\n", header->key, header->value);
   eina_iterator_free(itr);

   fprintf(stderr, "INFO:  - request protocols:\n");
   itr = efl_net_dialer_websocket_request_protocols_get(dialer);
   EINA_ITERATOR_FOREACH(itr, str)
     fprintf(stderr, "INFO:       %s\n", str);
   eina_iterator_free(itr);

   return ;

 no_mainloop:
   efl_del(dialer);

 end:
   efl_loop_quit(efl_loop_get(ev->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
