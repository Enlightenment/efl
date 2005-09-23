/* Ecore_Con Usage Example
 */

#include <Ecore.h>
#include <Ecore_Con.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

Ecore_Con_Server *svr;

#define SOCKET_NAME "con_example"
#define SOCKET_PORT 0

char *msg = "get http://www.enlightenment.org\n";

typedef int (*Handler_Func) (void *data, int type, void *event);

int
server_data (void *data,
	     int ev_type,
	     Ecore_Con_Event_Server_Data *ev) {
  printf("Data received from the server!  Data was:\n");
  printf("  %*s\n", ev->size, (char *)ev->data);
  ecore_main_loop_quit();
  return 0;
}

int
main (int argc,
      char *argv[]) {
  ecore_con_init();

  // Try to conect to server.
  svr = ecore_con_server_connect(ECORE_CON_REMOTE_SYSTEM, "www.enlightenment.org",
				 80, NULL);
  if (NULL == svr) {
    printf("*** ERROR: Unable to connect to server.\n");
    printf("*** Start con_server_example before running this program.\n");
    return 0;
  }
  ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
			  (Handler_Func)server_data, NULL);
  ecore_con_server_send(svr, msg, strlen(msg));

  ecore_main_loop_begin();

  ecore_con_server_del(svr);

  ecore_con_shutdown();
  return 0;
}
