//Compile with:
// gcc -o ecore_con_client_example ecore_con_client_example.c `pkg-config --libs --cflags ecore ecore-con eina`

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Ecore_Getopt.h>

/* Ecore_Con client example
 * 2010 Mike Blumenkrantz
 */

static Ecore_Con_Server *svr;
static int retval = EXIT_SUCCESS;
static Eina_Bool do_flush = EINA_FALSE;
static Eina_Bool single_message = EINA_FALSE;
static Eina_Bool verify = EINA_TRUE;
static Eina_Bool hostname_verify = EINA_TRUE;
static Eina_Bool do_ssl_upgrade = EINA_FALSE;
static char *starttls_local_command = NULL;

static Eina_Bool
_setup_ssl(void)
{
   Eina_Iterator *it;
   const char *ca;

   if (!(it = eina_file_ls("/etc/ssl/certs")))
     {
        retval = EXIT_FAILURE;
        return EINA_FALSE;
     }

   /* add all the CAs */
   EINA_ITERATOR_FOREACH(it, ca)
     {
        if (!ecore_con_ssl_server_cafile_add(svr, ca))
          fprintf(stderr, "WARNING: could not load CA: %s!\n", ca);
        eina_stringshare_del(ca);
     }

   eina_iterator_free(it);

   if (verify)
     ecore_con_ssl_server_verify(svr);

   if (hostname_verify)
     ecore_con_ssl_server_verify_basic(svr);

   return EINA_TRUE;
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

   if (!svr)
     fputs("WARNING: not connected to server, ignored input.\n", stderr);
   else
     {
        size_t clen = strlen(starttls_local_command);
        if (do_ssl_upgrade && ((size_t)r > clen) &&
            (strncmp(line, starttls_local_command, clen) == 0) &&
            (line[clen] == '\n' || line[clen] == '\r'))
          {
             printf("INFO: starting SSL communication...\n");

             if (!ecore_con_ssl_server_upgrade(svr, ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT))
               {
                  printf("ERROR: failed to upgrade to SSL!\n");
                  retval = EXIT_FAILURE;
                  ecore_con_server_del(svr);
                  ecore_main_loop_quit();
               }

#if 1
             /* This just works since EFL v 1.19.  Prior to this,
              * upgrade couldn't get any extra setup, such as
              * certificate or verification mode as OpenSSL would
              * complain.
              */
             else if (!_setup_ssl())
               {
                  retval = EXIT_FAILURE;
                  ecore_con_server_del(svr);
                  ecore_main_loop_quit();
               }
#endif

             svr = NULL; /* it's considered dead until ECORE_CON_EVENT_SERVER_UPGRADE */
             goto end;
          }
        ecore_con_server_send(svr, line, r);
        printf("INFO: sent %zd bytes to server.\n", r);
        if (do_flush) ecore_con_server_flush(svr);
        if (single_message)
          {
             ecore_con_server_del(svr);
             svr = NULL;
             ecore_main_loop_quit();
          }
     }

 end:
   free(line);

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_add(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Add *ev)
{
   printf("Server with ip %s connected!\n", ecore_con_server_ip_get(ev->server));
   if (do_ssl_upgrade)
     printf("INFO: Not sending 'hello!' in tcp+ssl mode. Use: %s to upgrade the connection\n", starttls_local_command);
   else
     ecore_con_server_send(ev->server, "hello!", strlen("hello!"));
   if (do_flush) ecore_con_server_flush(ev->server);

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_del(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Del *ev)
{
   printf("Lost server with ip %s!\n", ecore_con_server_ip_get(ev->server));
   ecore_con_server_del(svr);
   svr = NULL;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_data(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Data *ev)
{
   printf("Received %i bytes from server:\n"
          ">>>>>\n"
          "%.*s\n"
          ">>>>>\n",
          ev->size,
          ev->size, (const char *)ev->data);

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_write_(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Write *ev)
{
   printf("Sent %d bytes to server\n", ev->size);
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_error(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Error *ev)
{
   printf("Server Error: %s\n", ev->error);
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_upgrade(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Server_Upgrade *ev)
{
   printf("Server upgraded to SSL %p %s\n", ev->server, ecore_con_server_ip_get(ev->server));
   svr = ev->server;
   return ECORE_CALLBACK_RENEW;
}

static const char *types_strs[] = {
  "tcp",
  "udp",
  "ssl",
  "tcp+ssl",
  "local-user",
  "local-system",
  NULL
};

static const Ecore_Getopt options = {
  "ecore_con_client_example", /* program name */
  NULL, /* usage line */
  "1", /* version */
  "(C) 2016 Enlightenment Project; 2010 Mike Blumenkrantz", /* copyright */
  "BSD 2-Clause", /* license */
  /* long description, may be multiline and contain \n */
  "Example of ecore_con_server_connect()\n",
  EINA_FALSE,
  {
    ECORE_GETOPT_CHOICE('t', "type", "Server type to use, defaults to 'tcp'", types_strs),
    ECORE_GETOPT_STORE_TRUE('P', "no-proxy", "Do not use SOCKS proxy for remote connections."),

    ECORE_GETOPT_STORE_TRUE('f', "flush", "Force a flush after every send call."),
    ECORE_GETOPT_STORE_TRUE('m', "single-message", "Send a single message and delete the server."),

    ECORE_GETOPT_STORE_STR('c', "starttls-local-command", "The string to use as a local command (it's NOT sent to remote peer) to upgrade connections when -t/--type=tcp+ssl. Defaults to STARTTLS, however if you need to send that to the server, change the string with this option."),

    ECORE_GETOPT_STORE_FALSE(0, "no-verify", "Do not verify server's certificate"),
    ECORE_GETOPT_STORE_FALSE(0, "no-hostname-verify", "Do not Verify server's hostname based on its certificate."),

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
main(int argc, char *argv[])
{
   char *name = NULL;
   char *type_choice = NULL;
   Ecore_Con_Type type;
   int port = -1;
   Eina_Bool no_proxy = EINA_FALSE;
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(type_choice),
     ECORE_GETOPT_VALUE_BOOL(no_proxy),

     ECORE_GETOPT_VALUE_BOOL(do_flush),
     ECORE_GETOPT_VALUE_BOOL(single_message),

     ECORE_GETOPT_VALUE_STR(starttls_local_command),

     ECORE_GETOPT_VALUE_BOOL(verify),
     ECORE_GETOPT_VALUE_BOOL(hostname_verify),

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

   eina_init();
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

   args = ecore_getopt_parse_positional(&options, values, argc, argv, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   if (!type_choice) type_choice = "tcp";

   if (strcmp(type_choice, "tcp") == 0)
     type = ECORE_CON_REMOTE_TCP;
   else if (strcmp(type_choice, "udp") == 0)
     type = ECORE_CON_REMOTE_UDP;
   else if (strcmp(type_choice, "ssl") == 0)
     type = ECORE_CON_REMOTE_TCP | ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT;
   else if (strcmp(type_choice, "tcp+ssl") == 0)
     {
        type = ECORE_CON_REMOTE_TCP;
        do_ssl_upgrade = EINA_TRUE;
     }
   else if (strcmp(type_choice, "local-user") == 0)
     type = ECORE_CON_LOCAL_USER;
   else if (strcmp(type_choice, "local-system") == 0)
     type = ECORE_CON_LOCAL_SYSTEM;
   else
     {
        fprintf(stderr, "ERROR: unsupported --type/-t '%s'\n", type_choice);
        retval = EXIT_FAILURE;
        goto end;
     }

   if ((!starttls_local_command) || (starttls_local_command[0] == '\0'))
     starttls_local_command = "STARTTLS";

   if (no_proxy) type |= ECORE_CON_NO_PROXY;

   svr = ecore_con_server_connect(type, name, port, NULL);
   if (!svr) goto end;

   if (strcmp(type_choice, "ssl") == 0)
     {
        if (!_setup_ssl())
          goto no_mainloop;
     }

/* set event handler for server connect */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, (Ecore_Event_Handler_Cb)_add, NULL);
/* set event handler for server disconnect */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, (Ecore_Event_Handler_Cb)_del, NULL);
/* set event handler for receiving server data */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, (Ecore_Event_Handler_Cb)_data, NULL);
/* set event handler that notifies of sent data */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_WRITE, (Ecore_Event_Handler_Cb)_write_, NULL);
/* set event handler that notifies of errors */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ERROR, (Ecore_Event_Handler_Cb)_error, NULL);
/* set event handler that notifies of upgrades */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_UPGRADE, (Ecore_Event_Handler_Cb)_upgrade, NULL);

   ecore_main_fd_handler_add(STDIN_FILENO, ECORE_FD_READ, _on_stdin, NULL, NULL, NULL);

/* start client */
   ecore_main_loop_begin();

 no_mainloop:
   if (svr)
     {
        ecore_con_server_del(svr);
        svr = NULL;
     }

end:
   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return retval;
}
