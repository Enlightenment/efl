#include <Ecore.h>
#include <unistd.h>

static double _initial_time = 0;

static Eina_Bool
_poller_print_cb(void *data)
{
   char *str = data;
   printf("Ecore Poller '%s' callback called after %0.3f seconds.\n",
	  str, ecore_time_get() - _initial_time);

   return ECORE_CALLBACK_RENEW;
}

int main(int argc, char **argv)
{
   double interval = 0.3; // tick each 0.3 seconds
   Ecore_Poller *poller1, *poller2;
   char *str1 = "poller1";
   char *str2 = "poller2";

   if (!ecore_init())
     {
	printf("ERROR: Cannot init Ecore!\n");
	return -1;
     }

   _initial_time = ecore_time_get();

   ecore_poller_poll_interval_set(ECORE_POLLER_CORE, interval);

   poller1 = ecore_poller_add(ECORE_POLLER_CORE, 4, _poller_print_cb, str1);
   poller2 = ecore_poller_add(ECORE_POLLER_CORE, 8, _poller_print_cb, str2);

   ecore_main_loop_begin();

   printf("changing poller2 interval to 16\n");

   ecore_poller_poller_interval_set(poller2, 16);
   ecore_main_loop_begin();

   ecore_poller_del(poller1);
   ecore_poller_del(poller2);

   ecore_shutdown();
}
