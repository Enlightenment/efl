/* Ecore_Con Usage Example
 */

#include <Ecore.h>
#include <Ecore_Con.h>

//#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

Ecore_Con_Server *svr;
char *msg = "Hello client! Received your message";

typedef int (*Handler_Func) (void *data, int type, void *event);

int
client_connect (Ecore_Con_Client *client,
		int ev_type,
		Ecore_Con_Event_Client_Add *ev) {
  printf("Client connected.\n");
  return 0;
}

int 
client_disconnect (Ecore_Con_Client *client,
		   int ev_type,
		   Ecore_Con_Event_Client_Del *ev){
  printf("Client disconnected.\n");
  return 0;
}

int
client_data (void *data,
	     int ev_type,
	     Ecore_Con_Event_Client_Data *ev) {
  printf("Client sent data!  Replying...");
  ecore_con_client_send(ev->client, msg, strlen(msg));
  printf("done!  Data was:\n");
  printf("  %*s\n", ev->size, (char *)ev->data);
  return 0;
}

int main (int argc, char *argv[]) {
  ecore_con_init();

  printf("Server is running: waiting for connections\n");
  svr = ecore_con_server_add(ECORE_CON_LOCAL_USER, "con_example", 0, NULL);
  ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
			  (Handler_Func)client_connect, NULL);
  ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
			  (Handler_Func)client_disconnect, NULL);
  ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
			  (Handler_Func)client_data, NULL);
  ecore_main_loop_begin();

  ecore_con_shutdown();
  return 0;
}
