/*
 * This example shows how to register a eet descriptor to a ecore con client
 * and send message to ecore con server. The eet descriptor is defined in
 * ecore_con_eet_descriptor_example.c.
 */

//Compile with:
// gcc -o ecore_con_eet_client_example ecore_con_eet_client_example.c ecore_con_eet_descriptor_example.c `pkg-config --cflags --libs eet ecore ecore-con eina`

#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Eina.h>
#include "ecore_con_eet_descriptor_example.h"

Eina_Bool
_eet_svr_discnct_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply EINA_UNUSED, Ecore_Con_Server *conn EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

static Eina_Bool
_eet_svr_cnct_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply, Ecore_Con_Server *conn EINA_UNUSED)
{
   /* Only id and message parameter are sent to server. not_sending, parameter
    * is not sent, as it is not added to the eet descriptor. */
   ECE_Example toSend = {1, "Message from Client.",
                         "This is not sent to server"};

   ecore_con_eet_send(reply, EXAMPLE_STREAM, &toSend);
   return EINA_TRUE;
}

static void
_eet_raw_data_cb(void *data EINA_UNUSED, Ecore_Con_Reply *reply EINA_UNUSED, const char *protocol_name EINA_UNUSED, const char *section, void *value, size_t length EINA_UNUSED)
{
   /* Only a perticular parameter's value is sent by Server. */
   printf("Section: %s\n", section);
   printf("Value: %s\n", (char *) value);
}

int main(int argc, const char *argv[])
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
   ece_example_descriptor_init(); // Initialize a eet descriptor.

   svr = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, argv[1],
                                  atoi(argv[2]), NULL);
   if (!svr)
     {
        printf("could not connect to the server.\n");
        exit(1);
     }

   ec_eet = ecore_con_eet_client_new(svr); // Associate it with the connection.

   // Register the descriptor to send messages.
   ece_example_descriptor_register_descs(ec_eet);

   // Register call backs.
   ecore_con_eet_raw_data_callback_add(ec_eet, EXAMPLE_STREAM,
                                       _eet_raw_data_cb, NULL);
   ecore_con_eet_server_connect_callback_add(ec_eet, _eet_svr_cnct_cb, NULL);
   ecore_con_eet_server_disconnect_callback_add(ec_eet, _eet_svr_discnct_cb,
                                                NULL);

   ecore_main_loop_begin();

   // Delete all callbacks before closing connection.
   ecore_con_eet_server_disconnect_callback_del(ec_eet, _eet_svr_discnct_cb,
                                                NULL);
   ecore_con_eet_server_connect_callback_del(ec_eet, _eet_svr_discnct_cb,
                                             NULL);
   ecore_con_eet_raw_data_callback_del(ec_eet, EXAMPLE_STREAM);

   // Free the descriptor.
   ece_example_descriptor_shutdown();

   ecore_con_server_del(svr);
   ecore_con_shutdown();
   eet_shutdown();
   eina_shutdown();

   return 0;
}
