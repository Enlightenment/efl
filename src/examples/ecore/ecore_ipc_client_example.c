#include <Ecore.h>
#include <Ecore_Ipc.h>
#include <Ecore_Getopt.h>

static int retval = EXIT_SUCCESS;
static Ecore_Ipc_Server *server = NULL;
static Eina_Bool do_flush = EINA_FALSE;
static Eina_Bool single_message = EINA_FALSE;

static Eina_Bool
_server_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Server_Add *ev = event;

   printf("INFO: server added %p: %s\n", ev->server, ecore_ipc_server_ip_get(ev->server));

   puts("INFO: start typing some lines of text to send to server...");

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_server_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Server_Del *ev = event;

   printf("INFO: server deleted %p: %s\n", ev->server, ecore_ipc_server_ip_get(ev->server));

   ecore_ipc_server_del(server);
   server = NULL;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_server_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Server_Data *ev = event;

   printf("INFO: server data %p: %s\n"
          "INFO:  - major: %d, minor: %d, ref: %d (to: %d)\n"
          "INFO:  - response?: %d\n"
          "INFO:  - size: %d\n"
          "-- BEGIN DATA --\n",
          ev->server, ecore_ipc_server_ip_get(ev->server),
          ev->major, ev->minor, ev->ref, ev->ref_to,
          ev->response,
          ev->size);

   fwrite(ev->data, ev->size, 1, stdout);
   puts("-- END DATA --");

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_on_stdin(void *data EINA_UNUSED, Ecore_Fd_Handler *fdh EINA_UNUSED)
{
   char *line = NULL;
#ifdef _WIN32
   char lbuf[4096] = "";
   ssize_t r;
   if (fgets(lbuf, sizeof(lbuf), stdin) == NULL)
     r = -1;
   else
     {
        line = strdup(lbuf);
        r = strlen(line);
     }
#else
   size_t len = 0;
   ssize_t r = getline(&line, &len, stdin);
#endif

   if (r < 0)
     {
        fprintf(stderr, "ERROR: could not read from stdin: %s\n", strerror(errno));
        return ECORE_CALLBACK_CANCEL;
     }

   if (!server)
     fputs("WARNING: not connected to server, ignored input.\n", stderr);
   else
     {
        ecore_ipc_server_send(server, 1, 2, 0, 0, 0, line, r);
        printf("INFO: sent %zd bytes to server.\n", r);
        if (do_flush) ecore_ipc_server_flush(server);
        if (single_message)
          {
             ecore_ipc_server_del(server);
             server = NULL;
             ecore_main_loop_quit();
          }
     }

   free(line);

   return ECORE_CALLBACK_RENEW;
}

static const char *types_strs[] = {
  "user",
  "system",
  "remote",
  NULL
};

static const Ecore_Getopt options = {
  "ecore_ipc_client_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of ecore_ipc_server_connect() usage.\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_CHOICE('t', "type", "Server type to use, defaults to 'user'", types_strs),
    ECORE_GETOPT_STORE_TRUE('S', "ssl", "Use SSL"),
    ECORE_GETOPT_STORE_TRUE('P', "no-proxy", "Do not use SOCKS proxy for remote connections."),

    ECORE_GETOPT_STORE_INT('s', "max-size", "Maximum size (in bytes) for messages."),

    ECORE_GETOPT_STORE_TRUE('f', "flush", "Force a flush after every send call."),
    ECORE_GETOPT_STORE_TRUE('m', "single-message", "Send a single message and delete the server."),

    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),

    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL, "The server name.", "name"),
    ECORE_GETOPT_STORE_METAVAR_INT(0, NULL, "The server port.", "port"),

    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char **argv)
{
   Ecore_Ipc_Type type;
   char *name = NULL;
   char *type_choice = NULL;
   int port = -1;
   int max_size = -1;
   Eina_Bool use_ssl = EINA_FALSE;
   Eina_Bool no_proxy = EINA_FALSE;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(type_choice),
     ECORE_GETOPT_VALUE_BOOL(use_ssl),
     ECORE_GETOPT_VALUE_BOOL(no_proxy),

     ECORE_GETOPT_VALUE_INT(max_size),

     ECORE_GETOPT_VALUE_BOOL(do_flush),
     ECORE_GETOPT_VALUE_BOOL(single_message),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     /* positional argument */
     ECORE_GETOPT_VALUE_STR(name),
     ECORE_GETOPT_VALUE_INT(port),

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args;
   Ecore_Event_Handler *handlers[3];

   ecore_init();
   ecore_ipc_init();

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   if (quit_option) goto end;

   args = ecore_getopt_parse_positional(&options, values, argc, argv, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   if (!type_choice) type_choice = "user";

   if (strcmp(type_choice, "user") == 0)
     type = ECORE_IPC_LOCAL_USER;
   else if (strcmp(type_choice, "system") == 0)
     type = ECORE_IPC_LOCAL_SYSTEM;
   else if (strcmp(type_choice, "remote") == 0)
     type = ECORE_IPC_REMOTE_SYSTEM;
   else
     {
        fprintf(stderr, "ERROR: unsupported --type/-t '%s'\n", type_choice);
        retval = EXIT_FAILURE;
        goto end;
     }

   handlers[0] = ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD, _server_add, NULL);
   handlers[1] = ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL, _server_del, NULL);
   handlers[2] = ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA, _server_data, NULL);

   ecore_main_fd_handler_add(STDIN_FILENO, ECORE_FD_READ, _on_stdin, NULL, NULL, NULL);

   if (use_ssl) type |= ECORE_IPC_USE_SSL;
   if (no_proxy) type |= ECORE_IPC_NO_PROXY;

   server = ecore_ipc_server_connect(type, name, port, NULL);
   if (!server)
     {
        fprintf(stderr, "ERROR: Could not connect to server type=%#x, name=%s, port=%d\n", type, name, port);
        goto end;
     }

   ecore_ipc_server_data_size_max_set(server, max_size);

   ecore_main_loop_begin();

   if (server)
     {
        ecore_ipc_server_del(server);
        server = NULL;
     }

   ecore_event_handler_del(handlers[0]);
   ecore_event_handler_del(handlers[1]);
   ecore_event_handler_del(handlers[2]);

 end:
   ecore_ipc_shutdown();
   ecore_shutdown();

   return retval;
}
