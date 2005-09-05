/* Ecore Event Handler Example. */
#include <Ecore.h>

#include <stdlib.h>
#include <stdio.h>
 
Ecore_Event_Handler *handler1 = NULL, *handler2 = NULL;
 
int event_hup(void *data, int ev_type, void *ev)
{
   printf("Hup signal! Remove the exit handler.\n");
   if (handler1)
     {
	ecore_event_handler_del(handler1);
	handler1 = NULL;
     }
   return 0;
}

int event_exit(void *data, int ev_type, void *ev)
{
   Ecore_Event_Signal_Exit *e;
   
   e = (Ecore_Event_Signal_Exit *)ev;
   printf("This callback handles event type: %i\n", ECORE_EVENT_SIGNAL_EXIT);
   printf("Event type recieved: %i\n", ev_type);
   if (e->interrupt) printf("Exit: interrupt\n");
   if (e->quit)      printf("Exit: quit\n");
   if (e->terminate) printf("Exit: terminate\n");
   ecore_main_loop_quit();
   return 1;
}

int main(int argc, char **argv)
{
   ecore_init();
   ecore_app_args_set(argc, (const char **) argv);
   handler1 = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, 
				      event_exit, NULL);
   handler2 = ecore_event_handler_add(ECORE_EVENT_SIGNAL_HUP, 
				      event_hup, NULL);
   ecore_main_loop_begin();
   ecore_shutdown();
   return 0;
}
