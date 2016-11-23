#include <Ecore.h>
#include <Ecore_Ipc.h>
#include <Ecore_Getopt.h>

static int retval = EXIT_SUCCESS;
static int max_size = -1;
static Eina_Bool echo = EINA_FALSE;
static Eina_Bool do_flush = EINA_FALSE;
static Eina_Bool single_message = EINA_FALSE;

static Eina_Bool
_client_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Add *ev = event;

   ecore_ipc_client_data_size_max_set(ev->client, max_size);

   printf("INFO: client added %p: %s\n", ev->client, ecore_ipc_client_ip_get(ev->client));

   if (!echo)
     {
        ecore_ipc_client_send(ev->client, 1, 2, 3, 0, EINA_TRUE,
                              "Hello World!", strlen("Hello World!"));
        if (do_flush) ecore_ipc_client_flush(ev->client);
        if (single_message) ecore_ipc_client_del(ev->client);
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_client_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Del *ev = event;

   printf("INFO: client deleted %p: %s\n", ev->client, ecore_ipc_client_ip_get(ev->client));
   ecore_ipc_client_del(ev->client);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_client_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Data *ev = event;

   printf("INFO: client data %p: %s\n"
          "INFO:  - major: %d, minor: %d, ref: %d (to: %d)\n"
          "INFO:  - response?: %d\n"
          "INFO:  - size: %d\n"
          "-- BEGIN DATA --\n",
          ev->client, ecore_ipc_client_ip_get(ev->client),
          ev->major, ev->minor, ev->ref, ev->ref_to,
          ev->response,
          ev->size);

   fwrite(ev->data, ev->size, 1, stdout);
   puts("-- END DATA --");

   if (echo)
     {
        ecore_ipc_client_send(ev->client, ev->major, ev->minor,
                              ev->ref + 1,
                              ev->ref, 0, ev->data, ev->size);
        if (do_flush) ecore_ipc_client_flush(ev->client);
        if (single_message) ecore_ipc_client_del(ev->client);
     }

   return ECORE_CALLBACK_RENEW;
}

static const char *types_strs[] = {
  "user",
  "system",
  "remote",
  NULL
};

static const Ecore_Getopt options = {
  "ecore_ipc_server_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of ecore_ipc_server_add() usage.\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_CHOICE('t', "type", "Server type to use, defaults to 'user'", types_strs),
    ECORE_GETOPT_STORE_TRUE('S', "ssl", "Use SSL"),

    ECORE_GETOPT_STORE_INT('s', "max-size", "Maximum size (in bytes) for messages."),

    ECORE_GETOPT_STORE_INT('l', "clients-limit",
                            "If set will limit number of clients to accept"),
    ECORE_GETOPT_STORE_TRUE('r', "clients-reject-excess",
                            "Immediately reject excess clients (over limit)"),

    ECORE_GETOPT_STORE_TRUE('e', "echo",
                            "Behave as 'echo' server, send back to client all the data receive"),
    ECORE_GETOPT_STORE_TRUE('f', "flush", "Force a flush after every send call."),
    ECORE_GETOPT_STORE_TRUE('m', "single-message", "Send a single message and delete the client."),

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
   int clients_limit = -1;
   int port = -1;
   Eina_Bool use_ssl = EINA_FALSE;
   Eina_Bool clients_reject_excess = EINA_FALSE;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(type_choice),
     ECORE_GETOPT_VALUE_BOOL(use_ssl),

     ECORE_GETOPT_VALUE_INT(max_size),

     ECORE_GETOPT_VALUE_INT(clients_limit),
     ECORE_GETOPT_VALUE_BOOL(clients_reject_excess),

     ECORE_GETOPT_VALUE_BOOL(echo),
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
   Ecore_Ipc_Server *server;
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

   handlers[0] = ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_ADD, _client_add, NULL);
   handlers[1] = ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DEL, _client_del, NULL);
   handlers[2] = ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DATA, _client_data, NULL);

   if (use_ssl) type |= ECORE_IPC_USE_SSL;

   server = ecore_ipc_server_add(type, name, port, NULL);
   if (!server)
     {
        fprintf(stderr, "ERROR: Could not create server type=%#x, name=%s, port=%d\n", type, name, port);
        goto end;
     }

   ecore_ipc_server_client_limit_set(server, clients_limit, clients_reject_excess);

   ecore_main_loop_begin();

   ecore_ipc_server_del(server);
   server = NULL;

   ecore_event_handler_del(handlers[0]);
   ecore_event_handler_del(handlers[1]);
   ecore_event_handler_del(handlers[2]);

 end:
   ecore_ipc_shutdown();
   ecore_shutdown();

   return retval;
}
