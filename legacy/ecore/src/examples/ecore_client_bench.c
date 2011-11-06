#include <stdio.h>
#include <Ecore.h>
#include <Ecore_Con.h>

/* Ecore_Con client example
 * 2010 Mike Blumenkrantz
 */

#define NUM_CLIENTS 30000

static Eina_Counter *counter;
static int add = 0;
static int del = 0;

Eina_Bool
_add(void *data, int type, Ecore_Con_Event_Server_Add *ev)
{
   ++add;
   printf("Connection #%i!\n", add);
   if (add == NUM_CLIENTS)
     ecore_main_loop_quit();

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_del(void *data, int type, Ecore_Con_Event_Server_Add *ev)
{
   ++del;
   printf("Connection lost! #%i!\n", del);

   return ECORE_CALLBACK_RENEW;
}

static void
_spawn(void *data)
{
   int x;
   
   for (x = 0; x < NUM_CLIENTS; x++)
     {
//        printf("Creating connection %i\n", x);
        if (!ecore_con_server_connect(ECORE_CON_REMOTE_NODELAY, "127.0.0.1", 8080, NULL))
          {
             printf("CRITICAL ERROR!\n"
                    "Could not create connection #%i!\n", x);
             exit(1);
          }
     }
     printf("***Job done***\n");
}

int main(void)
{
   double done;
   eina_init();
   ecore_init();
   ecore_con_init();

   eina_log_domain_level_set("ecore_con", EINA_LOG_LEVEL_ERR);
   eina_log_domain_level_set("eina", EINA_LOG_LEVEL_ERR);
   counter = eina_counter_new("client");
   eina_counter_start(counter);
   done = ecore_time_get();

   ecore_job_add(_spawn, NULL);

/* set event handler for server connect */
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, (Ecore_Event_Handler_Cb)_add, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, (Ecore_Event_Handler_Cb)_del, NULL);

/* start client */
   ecore_main_loop_begin();
   eina_counter_stop(counter, 1);
   printf("\nTime elapsed for %i connections: %f seconds\n%s", NUM_CLIENTS, ecore_time_get() - done, eina_counter_dump(counter));
   return 0;
}
