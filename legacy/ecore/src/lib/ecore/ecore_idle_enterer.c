#include "ecore_private.h"
#include "Ecore.h"

static Ecore_Idle_Enterer *idle_enterers = NULL;
static int                 idle_enterers_delete_me = 0;

/**
 * Add an idle enterer handler.
 * @param func The function to call when entering an idle state.
 * @param data The data to be passed to the @p func call
 * @return A handle to the idle enterer callback
 * 
 * This function adds an idle enterer handler and returns its handle on success
 * or NULL on failure. Idle enterers are called just before he program goes
 * int an idle state where it is waiting on timers to time out, data to come
 * in on file descriptors, etc. The program will be in a "sleeping" state
 * after all the idle enterer callbacks are called. This is a good callback to
 * use to update your program's state if it has a state engine. Do all your
 * heavy processing here to update state (like drawing etc.). When @p func
 * is called, it will be passed the pointer @p data.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_Idle_Enterer *
ecore_idle_enterer_add(int (*func) (void *data), const void *data)
{
   Ecore_Idle_Enterer *ie;

   if (!func) return NULL;
   ie = calloc(1, sizeof(Ecore_Idle_Enterer));
   if (!ie) return NULL;
   ECORE_MAGIC_SET(ie, ECORE_MAGIC_IDLE_ENTERER);
   ie->func = func;
   ie->data = (void *)data;
   idle_enterers = _ecore_list_append(idle_enterers, ie);
   return ie;
}

/**
 * Delete an idle enter handler.
 * @param idle_enterer The idle enterer to delete
 * 
 * Delete the specified @p idle_enterer from the set of idle_enterers that are
 * executed during main loop execution. On success the data pointer that was
 * being passed to the idle handler function, set by ecore_idle_enterer_add()
 * will be returned.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void *
ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer)
{
   if (!ECORE_MAGIC_CHECK(idle_enterer, ECORE_MAGIC_IDLE_ENTERER))
     {
	ECORE_MAGIC_FAIL(idle_enterer, ECORE_MAGIC_IDLE_ENTERER,
			 "ecore_idle_enterer_del");
	return NULL;
     }
   idle_enterer->delete_me = 1;
   idle_enterers_delete_me = 1;
   return idle_enterer->data;
}

void
_ecore_idle_enterer_shutdown(void)
{
   while (idle_enterers)
     {
	Ecore_Idle_Enterer *ie;
	
	ie = idle_enterers;
	idle_enterers = _ecore_list_remove(idle_enterers, ie);
	ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
	free(ie);
     }
   idle_enterers_delete_me = 0;
}

void
_ecore_idle_enterer_call(void)
{
   Ecore_List *l;
   
   for (l = (Ecore_List *)idle_enterers; l; l = l->next)
     {
	Ecore_Idle_Enterer *ie;
	
	ie = (Ecore_Idle_Enterer *)l;
	if (!ie->delete_me)
	  {
	     if (!ie->func(ie->data)) ecore_idle_enterer_del(ie);
	  }
     }
   if (idle_enterers_delete_me)
     {
	for (l = (Ecore_List *)idle_enterers; l;)
	  {
	     Ecore_Idle_Enterer *ie;
	     
	     ie = (Ecore_Idle_Enterer *)l;
	     l = l->next;
	     if (ie->delete_me)
	       {
		  idle_enterers = _ecore_list_remove(idle_enterers, ie);
		  ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
		  free(ie);
	       }
	  }
	idle_enterers_delete_me = 0;
     }
}

int
_ecore_idle_enterer_exist(void)
{
   if (idle_enterers) return 1;
   return 0;
}
