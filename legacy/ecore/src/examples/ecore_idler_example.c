#include <Ecore.h>
#include <unistd.h>

struct context { // helper struct to give some context to the callbacks
     int count;
     Ecore_Idle_Enterer *enterer;
     Ecore_Idler *idler;
     Ecore_Idle_Exiter *exiter;
     Ecore_Event_Handler *handler;
     Ecore_Timer *timer;
};

static _event_type = 0; // a new type of event will be defined and stored here

static Eina_Bool
_enterer_cb(void *data) // the idle enterer callback
{
   printf("IDLE ENTERER: Ecore entering in idle state.\n");

   return ECORE_CALLBACK_RENEW; // same as EINA_TRUE
}

static Eina_Bool
_exiter_cb(void *data) // the idle exiter callback
{
   printf("IDLE EXITER: Ecore exiting idle state.\n");

   return ECORE_CALLBACK_RENEW; // same as EINA_TRUE
}

static Eina_Bool
_idler_cb(void *data) // the idler callback - ran while the mainloop is idle
{
   struct context *ctxt = data;
   printf("IDLER: executing idler callback while in idle state.\n");

   ctxt->count++;

   /* each 10 times that the callback gets called, generate an event that
    * will wake up the main loop, triggering idle enterers, exiters, etc. */
   if ((ctxt->count % 10) == 0)
     ecore_event_add(_event_type, NULL, NULL, NULL);

   return ECORE_CALLBACK_RENEW; // same as EINA_TRUE
}

static Eina_Bool
_event_handler_cb(void *data, int type, void *event) // event callback
{
   struct context *ctxt = data;

   printf("EVENT: processing callback for the event received.\n");

   if (ctxt->count > 100)
     {
	ecore_idle_enterer_del(ctxt->enterer);
	ecore_idle_exiter_del(ctxt->exiter);
	ecore_idler_del(ctxt->idler);

	ctxt->enterer = NULL;
	ctxt->exiter = NULL;
	ctxt->idler = NULL;

	if (ctxt->timer)
	  {
	     ecore_timer_del(ctxt->timer);
	     ctxt->timer = NULL;
	  }

	ecore_main_loop_quit();
	return ECORE_CALLBACK_CANCEL; // same as EINA_FALSE
     }

   return ECORE_CALLBACK_RENEW; // same as EINA_TRUE
}

static Eina_Bool
_timer_cb(void *data)
{
   struct context *ctxt = data;
   printf("TIMER: timer callback called.\n");

   if (ctxt->timer)
     ctxt->timer = NULL;

   return ECORE_CALLBACK_CANCEL; // same as EINA_TRUE
}

int main(int argc, char **argv)
{
   struct context ctxt = {0};

   if (!ecore_init())
     {
	printf("ERROR: Cannot init Ecore!\n");
	return -1;
     }

   _event_type = ecore_event_type_new();

   ctxt.enterer = ecore_idle_enterer_add(_enterer_cb, &ctxt);
   ctxt.exiter = ecore_idle_exiter_add(_exiter_cb, &ctxt);
   ctxt.idler = ecore_idler_add(_idler_cb, &ctxt);
   ctxt.handler = ecore_event_handler_add(_event_type,
					  _event_handler_cb,
					  &ctxt);
   ctxt.timer = ecore_timer_add(0.0005, _timer_cb, &ctxt);

   ecore_main_loop_begin();
   ecore_shutdown();

   return 0;
}
