#include <Ecore.h>
#include <unistd.h>

struct context { // helper struct to give some context to the callbacks
     const char *str1, *str2;
     Ecore_Event_Handler *handler1;
     Ecore_Event_Handler *handler2;
};

static _event_type = 0; // a new type of event will be defined and stored here

static Eina_Bool
_event_handler1_cb(void *data, int type, void *event)
{
   int *number = event;
   const char *str = data;

   printf("event_handler1: number=%d, data=\"%s\".\n", *number, str);

   if ((*number % 2) == 0)
     return ECORE_CALLBACK_DONE;

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_event_handler2_cb(void *data, int type, void *event) // event callback
{
   struct context *ctxt = data;
   int *number = event;

   printf("event_handler2: number=%d.\n", *number);

   if (*number == 5)
     {
	const char *old = NULL;
	old = ecore_event_handler_data_set(ctxt->handler1, (void *)ctxt->str2);
	printf("changed handler1 data from \"%s\" to \"%s\".\n",
	       old, ctxt->str2);
     }
   else if (*number >= 10)
     {
	printf("finish main loop.\n");
	ecore_main_loop_quit();
     }

   return ECORE_CALLBACK_DONE; // same as EINA_FALSE
}

int main(int argc, char **argv)
{
   struct context ctxt = {0};
   int i;
   ctxt.str1 = "dataone";
   ctxt.str2 = "datatwo";

   if (!ecore_init())
     {
	printf("ERROR: Cannot init Ecore!\n");
	return -1;
     }

   _event_type = ecore_event_type_new();

   ctxt.handler1 = ecore_event_handler_add(_event_type,
					   _event_handler1_cb,
					   ctxt.str1);
   ctxt.handler2 = ecore_event_handler_add(_event_type,
					   _event_handler2_cb,
					   &ctxt);

   for (i = 0; i <= 15; i++)
     {
	int *event_data = malloc(sizeof(*event_data));
	*event_data = i;
	ecore_event_add(_event_type, event_data, NULL, NULL);
     }

   printf("start the main loop.\n");

   ecore_main_loop_begin();

   ecore_shutdown();

   return 0;
}
