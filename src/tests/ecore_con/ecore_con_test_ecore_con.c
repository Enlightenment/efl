#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <Ecore.h>
#include <Ecore_Con.h>

#include "ecore_con_suite.h"

char sdata[] = "Server_info";
char cdata[] = "Client_info";

Eina_Bool
_add(void *data, int type EINA_UNUSED, void *ev)
{
   fail_if (type != ECORE_CON_EVENT_CLIENT_ADD &&
            type != ECORE_CON_EVENT_SERVER_ADD);

   /* Server */
   if (type == ECORE_CON_EVENT_CLIENT_ADD)
     {
        Ecore_Con_Event_Client_Add *event = ev;
        double timeout_val = 10, ret;
        void *del_data;

        fail_if (data != (void *) 1);
        fail_if (!event->client);

        fprintf(stderr, "client fd: %d\n",
                ecore_con_client_fd_get(event->client));
        fail_unless(ecore_con_client_fd_get(event->client));

        printf("Client with ip %s, port %d, connected = %d!\n",
               ecore_con_client_ip_get(event->client),
               ecore_con_client_port_get(event->client),
               ecore_con_client_connected_get(event->client));

        ecore_con_client_timeout_set(event->client, timeout_val);
        ret = ecore_con_client_timeout_get(event->client);
        fail_if (ret != timeout_val);

        ecore_con_client_data_set(event->client, cdata);
        del_data = ecore_con_client_data_get(event->client);
        fail_if (strcmp((char *)del_data, cdata));
     }
   else if (type == ECORE_CON_EVENT_SERVER_ADD)
     {
        Ecore_Con_Event_Server_Add *event = ev;
        const char ping[] = "PING";
        int ret;

        fail_if (data != (void *) 2);
        fail_if (!event->server);
        printf("Server with ip %s, name %s, port %d, connected = %d!\n",
               ecore_con_server_ip_get(event->server),
               ecore_con_server_name_get(event->server),
               ecore_con_server_port_get(event->server),
               ecore_con_server_connected_get(event->server));
        ret = ecore_con_server_send(event->server, ping, sizeof(ping));
        fail_if (ret != sizeof(ping));
        ecore_con_server_flush(event->server);
     }

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_del(void *data , int type EINA_UNUSED, void *ev)
{
   void *del_data;

   fail_if (type != ECORE_CON_EVENT_CLIENT_DEL &&
            type != ECORE_CON_EVENT_SERVER_DEL);

   /* Server */
   if (type == ECORE_CON_EVENT_CLIENT_DEL)
     {
        Ecore_Con_Event_Client_Del *event = ev;

        fail_if (data != (void *) 1);
        fail_if (!event->client);

        printf("Lost client with ip %s!\n",
               ecore_con_client_ip_get(event->client));
        fail_if(ecore_con_client_ip_get(event->client));

        printf("Client was connected for %0.3f seconds.\n",
               ecore_con_client_uptime_get(event->client));
        fail_unless(ecore_con_client_uptime_get(event->client));

        del_data = ecore_con_client_del(event->client);
        fail_if (strcmp((char *)del_data, cdata));
     }
   else if (type == ECORE_CON_EVENT_SERVER_DEL)
     {
        Ecore_Con_Event_Server_Del *event = ev;

        fail_if (!event->server);

        fail_if (data != (void *) 2);

        printf("Lost server with ip %s!\n",
                ecore_con_server_ip_get(event->server));
        fail_if(!ecore_con_server_ip_get(event->server));

        ecore_con_server_del(event->server);
     }
   fail ();

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_data(void *data, int type EINA_UNUSED, void *ev)
{

   fail_if (type != ECORE_CON_EVENT_CLIENT_DATA &&
            type != ECORE_CON_EVENT_SERVER_DATA);

   /* Server */
   if (type == ECORE_CON_EVENT_CLIENT_DATA)
     {
        Ecore_Con_Event_Client_Data *event = ev;
        const char pong[] = "PONG";
        int ret;

        char fmt[128];
        fail_if (data != (void *) 1);

        snprintf(fmt, sizeof(fmt),
                 "Received %i bytes from client %s port %d:\n"
                 ">>>>>\n"
                 "%%.%is\n"
                 ">>>>>\n",
                 event->size, ecore_con_client_ip_get(event->client),
                 ecore_con_client_port_get(event->client), event->size);

        printf(fmt, event->data);
        fail_if (event->size != sizeof("PING"));
        fail_if (memcmp (event->data, "PING", sizeof("PING")) != 0);

        ret = ecore_con_client_send(event->client, pong, sizeof(pong));
        fail_if (ret != sizeof(pong));
        ecore_con_client_flush(event->client);
     }
   else if (type == ECORE_CON_EVENT_SERVER_DATA)
     {
        Ecore_Con_Event_Server_Data *event = ev;
        char fmt[128];

        fail_if (data != (void *) 2);

        snprintf(fmt, sizeof(fmt),
                 "Received %i bytes from server:\n"
                 ">>>>>\n"
                 "%%.%is\n"
                 ">>>>>\n",
                 event->size, event->size);

        printf(fmt, event->data);
        fail_if (event->size != sizeof("PONG"));
        fail_if (memcmp (event->data, "PONG", sizeof("PONG")) != 0);
        ecore_main_loop_quit();
     }

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_dns_add_del(void *data, int type EINA_UNUSED, void *ev EINA_UNUSED)
{
   Eina_Bool *err_check = data;
   *err_check = EINA_FALSE;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_dns_err(void *data, int type EINA_UNUSED, void *ev EINA_UNUSED)
{
   Eina_Bool *err_check = data;
   *err_check = EINA_TRUE;
   return ECORE_CALLBACK_RENEW;
}

void _ecore_con_server_client_tests(Ecore_Con_Type compl_type, const char *name, Eina_Bool is_ssl, int server_port)
{
   Ecore_Con_Server *server = NULL;
   Ecore_Con_Server *client, *client2;
   Ecore_Con_Client *cl;
   const Eina_List *clients, *l;
   Ecore_Event_Handler *handlers[6];
   void *server_data = malloc (1);
   void *client_data = malloc (1);
   double timeout_val = 10, timeout_ret;
   int ret;
   void *del_ret;
   const char *server_name;

   handlers[0] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
                                         _add, (void *) 1);
   fail_if(handlers[0] == NULL);
   handlers[1] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
                                         _del, (void *) 1);
   fail_if(handlers[1] == NULL);
   handlers[2] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
                                         _data, (void *) 1);
   fail_if(handlers[2] == NULL);

   handlers[3] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
                                         _add, (void *) 2);
   fail_if(handlers[3] == NULL);
   handlers[4] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
                                         _del, (void *) 2);
   fail_if(handlers[4] == NULL);
   handlers[5] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
                                         _data, (void *) 2);
   fail_if(handlers[5] == NULL);

   server = ecore_con_server_add(compl_type, name, server_port,
                                 server_data);
   fail_if (server == NULL);

   if (is_ssl)
     {
        fail_unless(ecore_con_ssl_server_cert_add(server, TESTS_SRC_DIR"/server.pem"));
        fail_unless(ecore_con_ssl_server_privkey_add(server, TESTS_SRC_DIR"/server.key"));
     }

   del_ret = ecore_con_server_data_get(server);
   fail_if (del_ret != server_data);

   del_ret = ecore_con_server_data_set(server, sdata);
   fail_if (del_ret != server_data);

   fail_if (!ecore_con_server_connected_get(server));

   ecore_con_server_timeout_set(server, timeout_val);
   timeout_ret = ecore_con_server_timeout_get(server);
   fail_if (timeout_ret != timeout_val);

   ret = ecore_con_server_port_get(server);
   fail_if (ret != server_port);

   ecore_con_server_client_limit_set(server, 1, 0);

   client = ecore_con_server_connect(compl_type, name, server_port,
                                     client_data);
   fail_if (client == NULL);

   client2 = ecore_con_server_connect(compl_type, name, server_port,
                                     client_data);
   fail_if (client2 == NULL);

   if (is_ssl)
     {
        fail_unless(ecore_con_ssl_server_cafile_add(server, TESTS_SRC_DIR"/server.pem"));
        server_name = ecore_con_ssl_server_verify_name_get(server);
        ecore_con_ssl_server_verify_name_set(server,server_name);
        ecore_con_ssl_server_verify_basic(server);
        ecore_con_ssl_server_verify(server);
     }

   ecore_main_loop_begin();

   fprintf(stderr, "server fd for client 1: %d\n",
           ecore_con_server_fd_get(client));
   fail_unless(ecore_con_server_fd_get(client));

   fprintf(stderr, "server fd for client 2: %d\n",
           ecore_con_server_fd_get(client2));
   fail_unless(ecore_con_server_fd_get(client2));

   clients = ecore_con_server_clients_get(server);
   printf("Clients connected to this server when exiting: %d\n",
          eina_list_count(clients));
   EINA_LIST_FOREACH(clients, l, cl)
     {
        printf("%s\n", ecore_con_client_ip_get(cl));
     }

   printf("Server was up for %0.3f seconds\n",
          ecore_con_server_uptime_get(server));

   del_ret = ecore_con_server_del(client2);
   fail_if (del_ret != client_data);

   del_ret = ecore_con_server_del(server);
   fail_if (strcmp((char *)del_ret, sdata));
   free (server_data);
   server_data = NULL;

   del_ret = ecore_con_server_del(client);
   fail_if (del_ret != client_data);
   free (client_data);
   client_data = NULL;

   del_ret = ecore_event_handler_del (handlers[0]);
   fail_if (del_ret != (void *) 1);
   del_ret = ecore_event_handler_del (handlers[1]);
   fail_if (del_ret != (void *) 1);
   del_ret = ecore_event_handler_del (handlers[2]);
   fail_if (del_ret != (void *) 1);

   del_ret = ecore_event_handler_del (handlers[3]);
   fail_if (del_ret != (void *) 2);
   del_ret = ecore_event_handler_del (handlers[4]);
   fail_if (del_ret != (void *) 2);
   del_ret = ecore_event_handler_del (handlers[5]);
   fail_if (del_ret != (void *) 2);
}

EFL_START_TEST(ecore_test_ecore_con_local_user)
{
   _ecore_con_server_client_tests(ECORE_CON_LOCAL_USER, "test_sock", EINA_FALSE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_local_user_home)
{
   const char *xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");

   unsetenv("XDG_RUNTIME_DIR");
   _ecore_con_server_client_tests(ECORE_CON_LOCAL_USER, "test_soc", EINA_FALSE, 12345);
   if (xdg_runtime_dir) setenv("XDG_RUNTIME_DIR", xdg_runtime_dir, 1);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_local_user_tmp)
{
   const char *xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
   const char *homedir = getenv("HOME");

   unsetenv("XDG_RUNTIME_DIR");
   unsetenv("HOME");
   _ecore_con_server_client_tests(ECORE_CON_LOCAL_USER, "test_sock", EINA_FALSE, 12345);
   if (homedir) setenv("HOME", homedir, 1);
   if (xdg_runtime_dir) setenv("XDG_RUNTIME_DIR", xdg_runtime_dir, 1);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_local_user_none)
{
   const char *xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
   const char *homedir = getenv("HOME");
   const char *tmp = getenv("TMPDIR");

   unsetenv("XDG_RUNTIME_DIR");
   unsetenv("HOME");
   unsetenv("TMPDIR");
   _ecore_con_server_client_tests(ECORE_CON_LOCAL_USER, "test_sock", EINA_FALSE, 12345);
   if (tmp) setenv("TMPDIR", tmp, 1);
   if (homedir) setenv("HOME", homedir, 1);
   if (xdg_runtime_dir) setenv("XDG_RUNTIME_DIR", xdg_runtime_dir, 1);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_local_system)
{
   _ecore_con_server_client_tests(ECORE_CON_LOCAL_SYSTEM, "test_sock", EINA_FALSE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_local_system_fullpath)
{
   _ecore_con_server_client_tests(ECORE_CON_LOCAL_SYSTEM, "/tmp/test_sock", EINA_FALSE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_local_system_negport)
{
   _ecore_con_server_client_tests(ECORE_CON_LOCAL_SYSTEM, "test_sock", EINA_FALSE, -6);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_local_system_negport_fullpath)
{
   _ecore_con_server_client_tests(ECORE_CON_LOCAL_SYSTEM, "/tmp/test_sock", EINA_FALSE, -6);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_local_abstract)
{
   _ecore_con_server_client_tests(ECORE_CON_LOCAL_ABSTRACT, "test_sock", EINA_FALSE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_tcp)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_TCP, "127.0.0.1", EINA_FALSE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_nodelay)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_NODELAY, "127.0.0.1", EINA_FALSE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_tcp_tls)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_TCP | ECORE_CON_USE_TLS, "127.0.0.1", EINA_TRUE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_tcp_tls_load_cert)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_TCP | ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT, "127.0.0.1", EINA_TRUE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_tcp_mixed)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_TCP | ECORE_CON_USE_MIXED, "127.0.0.1", EINA_TRUE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_tcp_mixed_load_cert)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_TCP | ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT, "127.0.0.1", EINA_TRUE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_nodelay_tls)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_NODELAY | ECORE_CON_USE_TLS, "127.0.0.1", EINA_TRUE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_nodelay_tls_load_cert)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_NODELAY | ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT, "127.0.0.1", EINA_TRUE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_nodelay_mixed)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_NODELAY | ECORE_CON_USE_MIXED, "127.0.0.1", EINA_TRUE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_remote_nodelay_mixed_load_cert)
{
   _ecore_con_server_client_tests(ECORE_CON_REMOTE_NODELAY | ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT, "127.0.0.1", EINA_TRUE, 12345);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_ssl_available)
{
   int ret = ecore_con_ssl_available_get();
#ifdef HAVE_GNUTLS
   fail_if(ret != 1);
#elif HAVE_OPENSSL
   fail_if(ret != 2);
#else
   fail_if(!ret);
#endif
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_dns)
{
   Ecore_Con_Server *client;
   Ecore_Event_Handler *e_err;
   Ecore_Event_Handler *e_add;
   Eina_Bool err_check = EINA_FALSE;

   e_add = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, _dns_add_del, (void *) &err_check);
   e_err = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ERROR, _dns_err, (void *) &err_check);
   /* For timeout */
   e_err = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, _dns_add_del, (void *) &err_check);

   client = ecore_con_server_connect(ECORE_CON_REMOTE_TCP,
                                     "wongsub.wrongdns.lan", 1234, NULL);
   fail_if (client == NULL);
   ecore_con_server_timeout_set(client, 5.0);

   ecore_main_loop_begin();
   fail_if (err_check != EINA_FALSE);
   fail_if (ecore_event_handler_del(e_err) != (void *) &err_check);
   fail_if (ecore_event_handler_del(e_add) != (void *) &err_check);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_con_shutdown_bef_init)
{
   int ret = ecore_con_shutdown();
   fail_if(ret != 0);

   ret = ecore_con_shutdown();
   fail_if(ret != 0);

   ecore_con_init();
}
EFL_END_TEST

static void
_lookup_done_cb(const char *canonname, const char *ip, struct sockaddr *addr, int addrlen, void *data)
{
   fail_if(strncmp(canonname,(char *)data, strlen((char *)data)));

   fprintf(stderr, "canonname = %s\n", canonname);
   fprintf(stderr, "ip = %s\n", ip);
   fprintf(stderr, "addr = %p\n", addr);
   fprintf(stderr, "addrlen = %d\n", addrlen);

   ecore_main_loop_quit();
}

EFL_START_TEST(ecore_test_ecore_con_dns_lookup)
{
   const char link[] = "www.google.com";

   fail_unless(ecore_con_lookup(link, _lookup_done_cb, link));

   ecore_main_loop_begin();
}
EFL_END_TEST

void ecore_con_test_ecore_con(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_con_local_user);
   tcase_add_test(tc, ecore_test_ecore_con_local_user_home);
   tcase_add_test(tc, ecore_test_ecore_con_local_user_tmp);
   tcase_add_test(tc, ecore_test_ecore_con_local_user_none);
   tcase_add_test(tc, ecore_test_ecore_con_local_system);
   tcase_add_test(tc, ecore_test_ecore_con_local_system_fullpath);
   tcase_add_test(tc, ecore_test_ecore_con_local_system_negport);
   tcase_add_test(tc, ecore_test_ecore_con_local_system_negport_fullpath);
   tcase_add_test(tc, ecore_test_ecore_con_local_abstract);
   tcase_add_test(tc, ecore_test_ecore_con_remote_tcp);
   tcase_add_test(tc, ecore_test_ecore_con_remote_tcp_tls);
   tcase_add_test(tc, ecore_test_ecore_con_remote_tcp_tls_load_cert);
   tcase_add_test(tc, ecore_test_ecore_con_remote_tcp_mixed);
   tcase_add_test(tc, ecore_test_ecore_con_remote_tcp_mixed_load_cert);
   tcase_add_test(tc, ecore_test_ecore_con_remote_nodelay);
   tcase_add_test(tc, ecore_test_ecore_con_remote_nodelay_tls);
   tcase_add_test(tc, ecore_test_ecore_con_remote_nodelay_tls_load_cert);
   tcase_add_test(tc, ecore_test_ecore_con_remote_nodelay_mixed);
   tcase_add_test(tc, ecore_test_ecore_con_remote_nodelay_mixed_load_cert);
   tcase_add_test(tc, ecore_test_ecore_con_ssl_available);
   tcase_add_test(tc, ecore_test_ecore_con_dns);
   tcase_add_test(tc, ecore_test_ecore_con_dns_lookup);
   tcase_add_test(tc, ecore_test_ecore_con_shutdown_bef_init);
}
