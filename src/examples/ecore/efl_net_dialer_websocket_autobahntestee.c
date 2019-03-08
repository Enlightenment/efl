#define EFL_BETA_API_SUPPORT
#include <Efl_Net.h>
#include <Ecore_Getopt.h>
#include <fcntl.h>
#include <ctype.h>

static char *address = NULL;
static char *agent = "efl_net_dialer_websocket";
static unsigned int start_index = 0;
static unsigned int end_index = UINT32_MAX;
static unsigned int current_index = 0;
static Eina_Bool no_report_update = EINA_FALSE;
static Eina_List *case_tuples = NULL;
static Eina_Bool verbose = 0;

static Eo *pending = NULL;

/* https://www.w3.org/International/questions/qa-forms-utf-8 */
static Eina_Bool
_utf8_check(const char *text)
{
   const unsigned char * bytes = (const unsigned char *)text;
   while (*bytes)
     {
        const unsigned char c = bytes[0];

        /* ascii: [\x09\x0A\x0D\x20-\x7E] */
        if (((c >= 0x20) && (c <= 0x7e)) ||
            (c == 0x09) || (c == 0x0a) || (c == 0x0d))
          {
             bytes += 1;
             continue;
          }

        /* autobahnsuite says 0x7f is valid */
        if (c == 0x7f)
          {
             bytes += 1;
             continue;
          }

#define VALUE_BYTE_CHECK(x) ((x >= 0x80) && (x <= 0xbf))

        /* non-overlong 2-byte: [\xC2-\xDF][\x80-\xBF] */
        if ((c >= 0xc2) && (c <= 0xdf))
          {
             if (VALUE_BYTE_CHECK(bytes[1]))
               {
                  bytes += 2;
                  continue;
               }
          }

        /* excluding overlongs: \xE0[\xA0-\xBF][\x80-\xBF] */
        if (c == 0xe0)
          {
             const unsigned char d = bytes[1];
             if ((d >= 0xa0) && (d <= 0xbf))
               {
                  if (VALUE_BYTE_CHECK(bytes[2]))
                    {
                       bytes += 3;
                       continue;
                    }
               }
          }

        /* straight 3-byte: [\xE1-\xEC\xEE\xEF][\x80-\xBF]{2} */
        if (((c >= 0xe1) && (c <= 0xec)) ||
            (c == 0xee) || (c == 0xef))
          {
             if (VALUE_BYTE_CHECK(bytes[1]) && VALUE_BYTE_CHECK(bytes[2]))
               {
                  bytes += 3;
                  continue;
               }
          }

        /* excluding surrogates: \xED[\x80-\x9F][\x80-\xBF] */
        if (c == 0xed)
          {
             const unsigned char d = bytes[1];
             if ((d >= 0x80) && (d <= 0x9f))
               {
                  if (VALUE_BYTE_CHECK(bytes[2]))
                    {
                       bytes += 3;
                       continue;
                    }
               }
          }

        /* planes 1-3: \xF0[\x90-\xBF][\x80-\xBF]{2} */
        if (c == 0xf0)
          {
             const unsigned char d = bytes[1];
             if ((d >= 0x90) && (d <= 0xbf))
               {
                  if (VALUE_BYTE_CHECK(bytes[2]) && VALUE_BYTE_CHECK(bytes[3]))
                    {
                       bytes += 4;
                       continue;
                    }
               }
          }
        /* planes 4-15: [\xF1-\xF3][\x80-\xBF]{3} */
        if ((c >= 0xf1) && (c <= 0xf3))
          {
             if (VALUE_BYTE_CHECK(bytes[1]) && VALUE_BYTE_CHECK(bytes[2]) && VALUE_BYTE_CHECK(bytes[3]))
               {
                  bytes += 4;
                  continue;
               }
          }

        /* plane 16: \xF4[\x80-\x8F][\x80-\xBF]{2} */
        if (c == 0xf4)
          {
             const unsigned char d = bytes[1];
             if ((d >= 0x80) && (d <= 0x8f))
               {
                  if (VALUE_BYTE_CHECK(bytes[2]) && VALUE_BYTE_CHECK(bytes[3]))
                    {
                       bytes += 4;
                       continue;
                    }
               }
          }

        if (verbose) fprintf(stderr, "INFO: failed unicode byte #%zd '%s'\n", (const char*)bytes - text, text);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_ws_pong(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   const char *text = event->info;
   if (!verbose) return;
   fprintf(stderr, "INFO: %s got PONG: %s\n",
           efl_name_get(dialer), text);
}

static void
_ws_closed_reason(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   Efl_Net_Dialer_Websocket_Closed_Reason *reason = event->info;

   if (!_utf8_check(reason->message))
     {
        efl_net_dialer_websocket_close_request(dialer, EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_PROTOCOL_ERROR, "invalid UTF-8");
        if (verbose) fprintf(stderr, "INFO: %s got CLOSE with invalid UTF-8\n", efl_name_get(dialer));
     }

   if (!verbose) return;
   fprintf(stderr, "INFO: %s got CLOSE: %4d '%s'\n",
           efl_name_get(dialer), reason->reason, reason->message);
}

static void
_ws_message_text(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   const char *text = event->info;

   if (!verbose) return;
   fprintf(stderr, "INFO: %s got TEXT %zd bytes:\n%s\n",
           efl_name_get(dialer), strlen(text), text);
}

static void
_ws_message_binary(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   const Eina_Slice *slice = event->info;
   size_t i;

   if (!verbose) return;
   fprintf(stderr, "INFO: %s got BINARY %zd bytes\n",
           efl_name_get(dialer), slice->len);

   for (i = 0; i < slice->len; i++)
     {
        const int c = slice->bytes[i];
        if (isprint(c))
          fprintf(stderr, " %#4x(%c)", c, c);
        else
          fprintf(stderr, " %#4x", c);
     }

   fprintf(stderr, "\n");
}

static void
_closed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;

   if (!verbose) return;
   fprintf(stderr, "INFO: %s closed\n", efl_name_get(dialer));
}

static void
_eos(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;

   if (!verbose) return;
   fprintf(stderr, "INFO: %s eos\n", efl_name_get(dialer));
}

static void
_connected(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;

   if (!verbose) return;
   fprintf(stderr, "INFO: %s connected %s\n",
           efl_name_get(dialer),
           efl_net_dialer_address_dial_get(dialer));
}

static void
_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   const Eina_Error *perr = event->info;
   fprintf(stderr, "ERROR: %s error: %d '%s'\n",
           efl_name_get(dialer), *perr, eina_error_msg_get(*perr));
   efl_loop_quit(efl_loop_get(event->object), eina_value_int_init(EXIT_FAILURE));
}

static void
_del(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   if (pending == dialer)
     pending = NULL;
}

EFL_CALLBACKS_ARRAY_DEFINE(dialer_cbs,
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_PONG, _ws_pong },
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_CLOSED_REASON, _ws_closed_reason },
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_TEXT, _ws_message_text },
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_BINARY, _ws_message_binary },
                           { EFL_NET_DIALER_EVENT_DIALER_CONNECTED, _connected },
                           { EFL_NET_DIALER_EVENT_DIALER_ERROR, _error },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _closed },
                           { EFL_IO_READER_EVENT_EOS, _eos },
                           { EFL_EVENT_DEL, _del });

static Eo *
_websocket_new(const char *name, Eo *loop)
{
   Eo *dialer;

   dialer = efl_add(EFL_NET_DIALER_WEBSOCKET_CLASS, loop,
                    efl_name_set(efl_added, name),
                    efl_event_callback_array_add(efl_added, dialer_cbs(), NULL));
   if (!dialer)
     {
        efl_loop_quit(loop, eina_value_int_init(EXIT_FAILURE));
        fprintf(stderr, "ERROR: could not create WebSockets dialer '%s'\n", name);
        return NULL;
     }

   pending = dialer;

   return dialer;
}

static void
_closed_quit(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   efl_del(dialer);
   ecore_main_loop_quit();
}

static void
_tests_finished(Eo *loop)
{
   Eo *dialer;
   char url[4096];
   int len;
   Eina_Error err;

   if (no_report_update)
     {
        if (verbose)
          fprintf(stderr, "INFO: tests finished, user required to not update the reports\n");
        ecore_main_loop_quit();
        return;
     }

   case_tuples = eina_list_remove(case_tuples, case_tuples);
   len = snprintf(url, sizeof(url), "%s/updateReports?agent=%s",
                  address, agent);
   if (len < 0)
     {
        fprintf(stderr, "ERROR: could not create URL "
                "'%s/updateReports?agent=%s': %s",
                address, agent, strerror(errno));
        ecore_main_loop_quit();
        return;
     }
   else if ((size_t)len > sizeof(url))
     {
        fprintf(stderr, "ERROR: could not create URL "
                "'%s/updateReports?agent=%s': no space.",
                address, agent);
        ecore_main_loop_quit();
        return;
     }

   dialer = _websocket_new("update-reports", loop);
   if (!dialer)
     {
        ecore_main_loop_quit();
        return;
     }

   efl_event_callback_add(dialer, EFL_IO_CLOSER_EVENT_CLOSED, _closed_quit, NULL);

   err = efl_net_dialer_dial(dialer, url);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                url, eina_error_msg_get(err));
        efl_del(dialer);
        efl_loop_quit(loop, eina_value_int_init(EXIT_FAILURE));
        return;
     }

   if (!verbose) return;
   fprintf(stderr, "INFO: %s '%s'\n",
           efl_name_get(dialer), efl_net_dialer_address_dial_get(dialer));
}

static void
_echo_text(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   const char *text = event->info;

   if (!_utf8_check(text))
     {
        if (verbose) fprintf(stderr, "INFO: invalid UTF-8 sequence '%s'. Close the connection.\n", text);
        efl_net_dialer_websocket_close_request(dialer, EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_INCONSISTENT_DATA, "invalid UTF-8");
        return;
     }

   efl_net_dialer_websocket_text_send(dialer, text);
}

static void
_echo_binary(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   const Eina_Slice *slice = event->info;
   efl_net_dialer_websocket_binary_send(dialer, *slice);
}

static Eina_Bool _websocket_test_next_case_tuple(Eo *loop);

static void
_test_next_case_closed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   efl_del(dialer);

   if (!_websocket_test_next_case_tuple(efl_loop_get(event->object)))
     _tests_finished(efl_loop_get(event->object));
}

EFL_CALLBACKS_ARRAY_DEFINE(_test_next_case_tuple_cbs,
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_TEXT, _echo_text },
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_BINARY, _echo_binary },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _test_next_case_closed });

static Eina_Bool
_websocket_test_next_case_tuple(Eo *loop)
{
   Eo *dialer;
   char url[4096];
   char name[256];
   char *str;
   int len;
   Eina_Error err;

   if (!case_tuples)
     return EINA_FALSE;

   str = case_tuples->data;
   case_tuples = eina_list_remove_list(case_tuples, case_tuples);
   len = snprintf(url, sizeof(url), "%s/runCase?casetuple=%s&agent=%s",
                  address, str, agent);
   if (len < 0)
     {
        fprintf(stderr, "ERROR: could not create URL "
                "'%s/runCase?casetuple=%s&agent=%s': %s",
                address, str, agent, strerror(errno));
        free(str);
        return EINA_FALSE;
     }
   else if ((size_t)len > sizeof(url))
     {
        fprintf(stderr, "ERROR: could not create URL "
                "'%s/runCase?casetuple=%s&agent=%s': no space.",
                address, str, agent);
        free(str);
        return EINA_FALSE;
     }

   snprintf(name, sizeof(name), "test_case=%s", str);
   free(str);

   dialer = _websocket_new(name, loop);
   if (!dialer) return EINA_FALSE;

   efl_event_callback_array_add(dialer, _test_next_case_tuple_cbs(), NULL);

   err = efl_net_dialer_dial(dialer, url);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                url, eina_error_msg_get(err));
        efl_del(dialer);
        efl_loop_quit(loop, eina_value_int_init(EXIT_FAILURE));
        return EINA_FALSE;
     }

   fprintf(stderr, "TEST: %s '%s'\n", efl_name_get(dialer), efl_net_dialer_address_dial_get(dialer));

   return EINA_TRUE;
}

static Eina_Bool _websocket_test_index(unsigned int idx, Eo *loop);

static void
_test_index_closed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   efl_del(dialer);

   if (!_websocket_test_index(current_index + 1, efl_loop_get(event->object)))
     _tests_finished(efl_loop_get(event->object));
}

EFL_CALLBACKS_ARRAY_DEFINE(_test_index_cbs,
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_TEXT, _echo_text },
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_BINARY, _echo_binary },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _test_index_closed });

static Eina_Bool
_websocket_test_index(unsigned int idx, Eo *loop)
{
   Eo *dialer;
   char url[4096];
   char name[64];
   int len;
   Eina_Error err;

   if (idx > end_index)
     return EINA_FALSE;

   len = snprintf(url, sizeof(url), "%s/runCase?case=%u&agent=%s",
                  address, idx, agent);
   if (len < 0)
     {
        fprintf(stderr, "ERROR: could not create URL "
                "'%s/runCase?case=%u&agent=%s': %s",
                address, idx, agent, strerror(errno));
        return EINA_FALSE;
     }
   else if ((size_t)len > sizeof(url))
     {
        fprintf(stderr, "ERROR: could not create URL "
                "'%s/runCase?case=%u&agent=%s': no space.",
                address, idx, agent);
        return EINA_FALSE;
     }

   snprintf(name, sizeof(name), "test_case=%u", idx);

   dialer = _websocket_new(name, loop);
   if (!dialer) return EINA_FALSE;

   efl_event_callback_array_add(dialer, _test_index_cbs(), NULL);

   err = efl_net_dialer_dial(dialer, url);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                url, eina_error_msg_get(err));
        efl_del(dialer);
        efl_loop_quit(loop, eina_value_int_init(EXIT_FAILURE));
        return EINA_FALSE;
     }

   current_index = idx;

   fprintf(stderr, "TEST: %s '%s'\n", efl_name_get(dialer), efl_net_dialer_address_dial_get(dialer));

   return EINA_TRUE;
}

static void
_load_tests_text(void *data EINA_UNUSED, const Efl_Event *event)
{
   const char *text = event->info;
   unsigned int n = strtoul(text, NULL, 10);

   if (start_index == 0)
     start_index = 1;
   else if (start_index > n)
     start_index = n;

   if (end_index == 0 || end_index > n)
     end_index = n;

   if (!verbose) return;
   fprintf(stderr, "INFO: test count: %u, start_index=%u, end_index=%u\n",
           n, start_index, end_index);
}

static void
_load_tests_closed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = event->object;
   efl_del(dialer);

   if (!_websocket_test_index(start_index, efl_loop_get(event->object)))
     _tests_finished(efl_loop_get(event->object));
}

EFL_CALLBACKS_ARRAY_DEFINE(_load_tests_cbs,
                           { EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_TEXT, _load_tests_text },
                           { EFL_IO_CLOSER_EVENT_CLOSED, _load_tests_closed });

static Eina_Bool
_websocket_load_tests(Eo *loop)
{
   Eo *dialer;
   char url[4096];
   int len;
   Eina_Error err;

   len = snprintf(url, sizeof(url), "%s/getCaseCount", address);
   if (len < 0)
     {
        fprintf(stderr, "ERROR: could not create URL '%s/getCaseCount': %s",
                address, strerror(errno));
        return EINA_FALSE;
     }
   else if ((size_t)len > sizeof(url))
     {
        fprintf(stderr, "ERROR: could not create URL '%s/getCaseCount': no space.",
                address);
        return EINA_FALSE;
     }

   dialer = _websocket_new("get-case-count", loop);
   if (!dialer) return EINA_FALSE;

   efl_event_callback_array_add(dialer, _load_tests_cbs(), NULL);

   err = efl_net_dialer_dial(dialer, url);
   if (err != 0)
     {
        fprintf(stderr, "ERROR: could not dial '%s': %s",
                url, eina_error_msg_get(err));
        efl_del(dialer);
        efl_loop_quit(loop, eina_value_int_init(EXIT_FAILURE));
        return EINA_FALSE;
     }

   if (verbose) fprintf(stderr, "INFO: %s '%s'\n", efl_name_get(dialer), efl_net_dialer_address_dial_get(dialer));

   return EINA_TRUE;
}

static const Ecore_Getopt options = {
  "efl_net_dialer_websocket_autobahntestee", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Use Efl_Net_Dialer_Websocket to implement a testee client for the Autobahn Test Suite."
  "\n"
  "Autobahn Test Suite http://autobahn.ws/testsuite provides a fully automated test suite to verify client and server implementations of the WebSocket Protocol for specification conformance and implementation robustness."
  "\n"
  "This is a client to talk to their test server, that should be executed as:\n"
  "   wstest -m fuzzingserver\n"
  "\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_STORE_UINT('s', "start-index", "when running batch, specifies the start (first) index"),
    ECORE_GETOPT_STORE_UINT('e', "end-index", "when running batch, specifies the end (last) index"),
    ECORE_GETOPT_STORE_TRUE('n', "no-report-update", "do not trigger autobahn to update report"),
    ECORE_GETOPT_STORE_TRUE('v', "verbose", "print messages"),
    ECORE_GETOPT_APPEND('t', "test-case", "the test-case tuple such as '1.2.8'", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_STORE_STR('a', "agent", "the agent identifier"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL,
                                   "The address (URL) to dial, such as ws://127.0.0.1:9001", "address"),
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
   /* FIXME: For the moment the main loop doesn't get
      properly destroyed on shutdown which disallow
      relying on parent destroying their children */
   if (pending)
     {
        efl_del(pending);
        pending = NULL;
     }
   if (verbose) fprintf(stderr, "INFO: main loop finished.\n");
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_UINT(start_index),
     ECORE_GETOPT_VALUE_UINT(end_index),
     ECORE_GETOPT_VALUE_BOOL(no_report_update),
     ECORE_GETOPT_VALUE_BOOL(verbose),
     ECORE_GETOPT_VALUE_LIST(case_tuples),
     ECORE_GETOPT_VALUE_STR(agent),

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
   Eina_Bool r;

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

   if (case_tuples)
     r = _websocket_test_next_case_tuple(ev->object);
   else if (start_index == end_index)
     r = _websocket_test_index(start_index, ev->object);
   else
     r = _websocket_load_tests(ev->object);

   if (r)
     {
        ecore_main_loop_begin();
     }

   if (pending)
     efl_del(pending);

   return ;

 end:
   efl_loop_quit(efl_loop_get(ev->object), eina_value_int_init(EXIT_FAILURE));
}

EFL_MAIN_EX();
