#include "ecore_private.h"
#include "Ecore.h"

static Ecore_Idler *idlers = NULL;
static int          idlers_delete_me = 0;

/**
 * Add an idler handler.
 * @param func The function to call when idling.
 * @param data The data to be passed to this @p func call
 * @return A idler handle
 * 
 * This function adds an idler handler and returns its handle on success, or
 * NULL on failure. Idlers will get called after idle enterer calls have all
 * been called and the program is in an idle state waiting on data or timers.
 * If no idler functions are set the process literally goes to sleep and uses
 * no CPU. If You have idler functions set they will be called continuously
 * during this "idle" time with your process consuming as much CPU as it can
 * get to do so. This is useful for when there are interfaces that require
 * polling and timers will mean too slow a response from the process if polling
 * is done by timers.
 */
Ecore_Idler *
ecore_idler_add(int (*func) (void *data), const void *data)
{
   Ecore_Idler *ie;

   if (!func) return NULL;
   ie = calloc(1, sizeof(Ecore_Idler));
   if (!ie) return NULL;
   ECORE_MAGIC_SET(ie, ECORE_MAGIC_IDLER);
   ie->func = func;
   ie->data = (void *)data;
   idlers = _ecore_list_append(idlers, ie);
   return ie;
}

/**
 * Delete an idler handler.
 * @param idler The idler to delete
 * 
 * Delete the specified @p idler from the set of idlers that are executed
 * during main loop execution. On success the data pointer set by
 * ecore_idler_add() and passed to the idler function is returned, or NULL
 * on failure.
 */
void *
ecore_idler_del(Ecore_Idler *idler)
{
   if (!ECORE_MAGIC_CHECK(idler, ECORE_MAGIC_IDLER))
     {
	ECORE_MAGIC_FAIL(idler, ECORE_MAGIC_IDLER,
			 "ecore_idler_del");
	return NULL;
     }
   idler->delete_me = 1;
   idlers_delete_me = 1;
   return idler->data;
}

void
_ecore_idler_shutdown(void)
{
   while (idlers)
     {
	Ecore_Idler *ie;
	
	ie = idlers;
	idlers = _ecore_list_remove(idlers, ie);
	ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
	free(ie);
     }
   idlers_delete_me = 0;
}

int
_ecore_idler_call(void)
{
   Ecore_Oldlist *l;

   for (l = (Ecore_Oldlist *)idlers; l; l = l->next)
     {
	Ecore_Idler *ie;
	
	ie = (Ecore_Idler *)l;
	if (!ie->delete_me)
	  {
	     if (!ie->func(ie->data)) ecore_idler_del(ie);
	  }
     }
   if (idlers_delete_me)
     {
	for (l = (Ecore_Oldlist *)idlers; l;)
	  {
	     Ecore_Idler *ie;
	     
	     ie = (Ecore_Idler *)l;
	     l = l->next;
	     if (ie->delete_me)
	       {
		  idlers = _ecore_list_remove(idlers, ie);
		  ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
		  free(ie);
	       }
	  }
	idlers_delete_me = 0;
     }
   if (idlers) return 1;
   return 0;
}

int
_ecore_idler_exist(void)
{
   if (idlers) return 1;
   return 0;
}
