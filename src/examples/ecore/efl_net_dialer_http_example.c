#include <Efl_Net.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>
#include <ctype.h>

static int waiting;

static void
_closed(void *data EINA_UNUSED, const Efl_Event *event)
{
   fprintf(stderr, "INFO: closed %s\n",
           efl_name_get(event->object));
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
   fprintf(stderr, "INFO: connected %s\n",
           efl_net_dialer_address_dial_get(event->object));
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

static void
_http_headers_done(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *o = event->object;
   Efl_Net_Http_Version ver = efl_net_dialer_http_version_get(o);
   Eina_Iterator *itr;
   const Efl_Net_Http_Header *h;

   fprintf(stderr, "INFO: HTTP/%d.%d status=%d url=%s\n",
           ver / 100, ver % 100,
           efl_net_dialer_http_response_status_get(o),
           efl_net_socket_address_remote_get(o));

   itr = efl_net_dialer_http_response_headers_get(o);
   EINA_ITERATOR_FOREACH(itr, h)
     fprintf(stderr, "INFO:   %s: %s\n", h->key, h->value);

   eina_iterator_free(itr);

   fprintf(stderr, "INFO: content-type: %s, content-length: %" PRId64 "\n",
           efl_net_dialer_http_response_content_type_get(o),
           efl_net_dialer_http_response_content_length_get(o));

   fprintf(stderr, "INFO: to upload %"PRIi64" bytes\n", efl_net_dialer_http_request_content_length_get(o));
}

EFL_CALLBACKS_ARRAY_DEFINE(dialer_cbs,
                           { EFL_NET_DIALER_HTTP_EVENT_HEADERS_DONE, _http_headers_done },
                           { EFL_NET_DIALER_EVENT_CONNECTED, _connected },
                           { EFL_NET_DIALER_EVENT_RESOLVED, _resolved },
                           { EFL_NET_DIALER_EVENT_ERROR, _error },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _closed },
                           { EFL_IO_READER_EVENT_EOS, _eos });

static void
_done(void *data EINA_UNUSED, const Efl_Event *event)
{
   waiting--;
   fprintf(stderr, "INFO: done %s, waiting=%d\n",
           efl_name_get(event->object), waiting);
   if (waiting == 0)
     efl_loop_quit(efl_loop_get(event->object), EINA_VALUE_EMPTY);
}

EFL_CALLBACKS_ARRAY_DEFINE(copier_cbs,
                           { EFL_IO_COPIER_EVENT_DONE, _done },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _closed });

static const char *primary_mode_choices[] = {
  "auto",
  "download",
  "upload",
  NULL
};

static Efl_Net_Dialer_Http_Primary_Mode
_parse_primary_mode(const char *str)
{
   if (strcmp(str, "upload") == 0)
     return EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD;
   if (strcmp(str, "download") == 0)
     return EFL_NET_DIALER_HTTP_PRIMARY_MODE_DOWNLOAD;

   return EFL_NET_DIALER_HTTP_PRIMARY_MODE_AUTO;
}

static const char *http_version_choices[] = {
  "1.0",
  "1.1",
  "2.0",
  NULL
};

static Efl_Net_Http_Version
_parse_http_version(const char *str)
{
   if (strcmp(str, "1.1") == 0)
     return EFL_NET_HTTP_VERSION_V1_1;
   if (strcmp(str, "2.0") == 0)
     return EFL_NET_HTTP_VERSION_V2_0;

   return EFL_NET_HTTP_VERSION_V1_0;
}

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
  "efl_net_dialer_http_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of Efl_Net_Dialer_Http usage.\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_STR('m', "method", "HTTP method such as GET, POST, PUT..."),
    ECORE_GETOPT_CHOICE('M', "primary-mode", "Define primary operation mode.", primary_mode_choices),
    ECORE_GETOPT_CHOICE('v', "http-version", "HTTP protocol version to use", http_version_choices),
    ECORE_GETOPT_STORE_STR('U', "username", "Authentication username"),
    ECORE_GETOPT_STORE_STR('P', "password", "Authentication password"),
    ECORE_GETOPT_CHOICE('A', "authentication-method", "Authentication method", authentication_method_choices),
    ECORE_GETOPT_STORE_TRUE('R', "authentication-restricted", "Authentication method must be restricted"),
    ECORE_GETOPT_STORE_FALSE('r', "disallow-redirects", "disallow redirections by following 'Location:' headers. Allowed by default."),
    ECORE_GETOPT_STORE_DOUBLE('t', "connect-timeout", "timeout in seconds for the connection phase"),
    ECORE_GETOPT_APPEND('H', "header", "Add custom headers. Format must be 'Key: Value'", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_STORE_STR('X', "proxy", "Set a specific proxy for the connection"),
    ECORE_GETOPT_STORE_STR('c', "cookie-jar", "Set the cookie-jar file to read/save cookies from. Empty means an in-memory cookie-jar"),
    ECORE_GETOPT_STORE_STR('i', "input-file", "Input file to use when uploading"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The address (URL) to dial", "address"),
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The output file name or :stdout:",
                                   "output-file"),
    ECORE_GETOPT_SENTINEL
  }
};

static Eo *dialer = NULL;
static Eo *sender = NULL;
static Eo *receiver = NULL;

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
   if (sender ||
       receiver ||
       dialer)
     {
        efl_io_closer_close(sender);
        efl_del(sender);
        sender = NULL;

        efl_io_closer_close(receiver);
        efl_del(receiver);
        receiver = NULL;

        efl_del(dialer);
        dialer = NULL;
     }
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   char *method = "GET";
   char *primary_mode_str = "auto";
   char *http_version_str = "1.1";
   char *username = NULL;
   char *password = NULL;
   char *authentication_method_str = "basic";
   char *address = NULL;
   char *output_fname = NULL;
   char *input_fname = NULL;
   char *proxy = NULL;
   char *cookie_jar = NULL;
   Eina_Bool quit_option = EINA_FALSE;
   Eina_Bool authentication_restricted = EINA_FALSE;
   Eina_Bool allow_redirects = EINA_TRUE;
   double timeout_dial = 30.0;
   Eina_List *headers = NULL;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(method),
     ECORE_GETOPT_VALUE_STR(primary_mode_str),
     ECORE_GETOPT_VALUE_STR(http_version_str),
     ECORE_GETOPT_VALUE_STR(username),
     ECORE_GETOPT_VALUE_STR(password),
     ECORE_GETOPT_VALUE_STR(authentication_method_str),
     ECORE_GETOPT_VALUE_BOOL(authentication_restricted),
     ECORE_GETOPT_VALUE_BOOL(allow_redirects),
     ECORE_GETOPT_VALUE_DOUBLE(timeout_dial),
     ECORE_GETOPT_VALUE_LIST(headers),
     ECORE_GETOPT_VALUE_STR(proxy),
     ECORE_GETOPT_VALUE_STR(cookie_jar),
     ECORE_GETOPT_VALUE_STR(input_fname),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     /* positional argument */
     ECORE_GETOPT_VALUE_STR(address),
     ECORE_GETOPT_VALUE_STR(output_fname),

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args;
   Eo *input, *output, *loop;
   Efl_Net_Dialer_Http_Primary_Mode primary_mode;
   Efl_Net_Http_Version http_version;
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

   if ((!input_fname) || (strcmp(input_fname, ":stdin:") == 0))
     {
        input = efl_add(EFL_IO_STDIN_CLASS, loop,
                        efl_name_set(efl_added, "stdin"),
                        efl_event_callback_add(efl_added, EFL_IO_READER_EVENT_EOS, _eos, NULL));
     }
   else
     {
        input = efl_add(EFL_IO_FILE_CLASS, loop,
                        efl_name_set(efl_added, "input"),
                        efl_file_set(efl_added, input_fname, NULL),
                        efl_io_file_flags_set(efl_added, O_RDONLY),
                        efl_io_closer_close_on_exec_set(efl_added, EINA_TRUE), /* recommended, set *after* flags, or include O_CLOEXEC in flags -- be careful with _WIN32 that doesn't support it. */
                        efl_event_callback_add(efl_added, EFL_IO_READER_EVENT_EOS, _eos, NULL));
     }

   if ((!output_fname) || (strcmp(output_fname, ":stdout:") == 0))
     {
        output = efl_add(EFL_IO_STDOUT_CLASS, loop,
                         efl_name_set(efl_added, "stdout"));
     }
   else
     {
        output = efl_add(EFL_IO_FILE_CLASS, loop,
                         efl_name_set(efl_added, "output"),
                         efl_file_set(efl_added, output_fname, NULL),
                         efl_io_file_mode_set(efl_added, 0644),
                         efl_io_file_flags_set(efl_added, O_WRONLY | O_TRUNC | O_CREAT),
                         efl_io_closer_close_on_exec_set(efl_added, EINA_TRUE)); /* recommended, set *after* flags, or include O_CLOEXEC in flags -- be careful with _WIN32 that doesn't support it. */
     }

   primary_mode = _parse_primary_mode(primary_mode_str);
   http_version = _parse_http_version(http_version_str);
   authentication_method = _parse_authentication_method(authentication_method_str);

   if (cookie_jar && cookie_jar[0] == ' ')
     cookie_jar = "";

   dialer = efl_add(EFL_NET_DIALER_HTTP_CLASS, loop,
                    efl_name_set(efl_added, "dialer"),
                    efl_net_dialer_http_method_set(efl_added, method),
                    efl_net_dialer_http_primary_mode_set(efl_added, primary_mode),
                    efl_net_dialer_http_version_set(efl_added, http_version),
                    efl_net_dialer_http_authentication_set(efl_added, username, password, authentication_method, authentication_restricted),
                    efl_net_dialer_http_allow_redirects_set(efl_added, allow_redirects),
                    efl_net_dialer_http_cookie_jar_set(efl_added, cookie_jar),
                    efl_net_dialer_proxy_set(efl_added, proxy),
                    efl_net_dialer_timeout_dial_set(efl_added, timeout_dial),
                    efl_event_callback_array_add(efl_added, dialer_cbs(), NULL));

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
        efl_net_dialer_http_request_header_add(dialer, str, p);
        free(str);
     }

   waiting = 1;
   if (efl_net_dialer_http_primary_mode_get(dialer) == EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD)
     waiting++;

   sender = efl_add(EFL_IO_COPIER_CLASS, loop,
                    efl_name_set(efl_added, "sender"),
                    efl_io_copier_source_set(efl_added, input),
                    efl_io_copier_destination_set(efl_added, dialer),
                    efl_event_callback_array_add(efl_added, copier_cbs(), NULL));

   fprintf(stderr, "INFO: sending %s->%s\n",
           efl_name_get(input),
           efl_name_get(dialer));

   receiver = efl_add(EFL_IO_COPIER_CLASS, loop,
                      efl_name_set(efl_added, "receiver"),
                      efl_io_copier_source_set(efl_added, dialer),
                      efl_io_copier_destination_set(efl_added, output),
                      efl_event_callback_array_add(efl_added, copier_cbs(), NULL));
   fprintf(stderr, "INFO: receiving %s->%s\n",
           efl_name_get(dialer),
           efl_name_get(output));

   err = efl_net_dialer_dial(dialer, address);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                address, eina_error_msg_get(err));
        goto end;
     }

   fprintf(stderr,
           "INFO: dialed %s\n"
           "INFO:  - method=%s\n"
           "INFO:  - primary_mode=%d\n"
           "INFO:  - version=%d\n"
           "INFO:  - allow_redirects=%d\n"
           "INFO:  - cookie_jar=%s\n"
           "INFO:  - timeout_dial=%fs\n"
           "INFO:  - proxy=%s\n"
           "INFO:  - request headers:\n",
           efl_net_dialer_address_dial_get(dialer),
           efl_net_dialer_http_method_get(dialer),
           efl_net_dialer_http_primary_mode_get(dialer),
           efl_net_dialer_http_version_get(dialer),
           efl_net_dialer_http_allow_redirects_get(dialer),
           efl_net_dialer_http_cookie_jar_get(dialer),
           efl_net_dialer_timeout_dial_get(dialer),
           efl_net_dialer_proxy_get(dialer));

   itr = efl_net_dialer_http_request_headers_get(dialer);
   EINA_ITERATOR_FOREACH(itr, header)
     fprintf(stderr, "INFO:       %s: %s\n", header->key, header->value);
   eina_iterator_free(itr);

   return ;

 end:
   efl_loop_quit(ev->object, eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
