/*
 * An example Eet descriptor is defined in ecore_con_eet_descriptor.c.
 * This example creates an Ecore Con Eet descriptor and associates it
 * with the Ecore Con Server. The required callbacks are registered.
 * This example demonstrates how to send Eet descriptor messages between
 * client and server.
 * The server runs until all clients associated with it disconnects.
 * You can run a single server and then connect multiple clients to this
 * to check the message passing.
 */

//Compile with:
// gcc -o ecore_con_eet_server_example ecore_con_eet_server_example.c ecore_con_eet_descriptor_example.c `pkg-config --cflags --libs eet ecore ecore-con eina`

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Eina.h>
#include "ecore_con_eet_descriptor_example.h"

static int _client_count = 0;

Eina_Bool
_eet_client_discnct_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply EINA_UNUSED, Ecore_Con_Client *conn EINA_UNUSED)
{
   _client_count--;
   if (!_client_count)
     ecore_main_loop_quit();

   return EINA_FALSE;
}

Eina_Bool
_eet_client_cnct_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply EINA_UNUSED, Ecore_Con_Client *conn EINA_UNUSED)
{
   _client_count++;
   return EINA_TRUE;
}

static void
_eet_data_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply, const char *protocol_name, void *value)
{
   char toSend[] = "Received your message.";// The message to send to client.
   ECE_Example *received = value;// The message received from client.

   printf("id: %d\n message: %s\n not_sending: %s\n",
           received->id, received->message, received->not_sending);
   /* The not_sending would be empty as not_sending is not added in the
    * descriptor. So that value is not sent.
    */

   /* Using eet raw send, you can send value of a particular field and
    * not the whole structure.
    */
   ecore_con_eet_raw_send(reply, protocol_name, "message", toSend,
                          strlen(toSend));
}

int main(int argc, char *argv[])
{
   Ecore_Con_Eet *ec_eet;
   Ecore_Con_Server *svr;

   if (argc < 3)
     {
        printf("Syntax: \n./ecore_con_eet_server_example <IP> <PORT>\n");
        exit(0);
     }

   eina_init();
   eet_init();
   ecore_con_init();
   ece_example_descriptor_init();// Initializing the example eet descriptor.

   svr = ecore_con_server_add(ECORE_CON_REMOTE_TCP, argv[1],
                              atoi(argv[2]), NULL);
   if (!svr)
     {
        printf("Failed to bind\n");
        exit(1);
     }

   ec_eet = ecore_con_eet_server_new(svr);

   // Register the initialized descriptor to the server.
   ece_example_descriptor_register_descs(ec_eet);

   // Registering call backs.
   ecore_con_eet_data_callback_add(ec_eet, EXAMPLE_STREAM, _eet_data_cb, NULL);
   ecore_con_eet_client_connect_callback_add(ec_eet, _eet_client_cnct_cb,
                                             NULL);
   ecore_con_eet_client_disconnect_callback_add(ec_eet, _eet_client_discnct_cb,
                                                NULL);

   ecore_main_loop_begin();

   printf("Server was up for %0.3f seconds\n",
          ecore_con_server_uptime_get(svr));

   // Delete the callbacks before closing connection.
   ecore_con_eet_data_callback_del(ec_eet, EXAMPLE_STREAM);
   ecore_con_eet_client_connect_callback_del(ec_eet, _eet_client_cnct_cb,
                                             NULL);
   ecore_con_eet_client_disconnect_callback_del(ec_eet, _eet_client_discnct_cb,
                                                NULL);
   // Delete the eet descriptor.
   ece_example_descriptor_shutdown();

   ecore_con_server_del(svr);
   ecore_con_shutdown();
   eet_shutdown();
   eina_shutdown();

   return 0;
}
