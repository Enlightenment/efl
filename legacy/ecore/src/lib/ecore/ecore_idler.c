/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "ecore_private.h"
#include "Ecore.h"

static Ecore_Idler *idlers = NULL;
static int          idlers_delete_me = 0;

/**
 * Add an idler handler.
 * @param  func The function to call when idling.
 * @param  data The data to be passed to this @p func call.
 * @return A idler handle if successfully added.  NULL otherwise.
 * @ingroup Idle_Group
 *
 * Add an idler handle to the event loop, returning a handle on success and
 * NULL otherwise.  The function @p func will be called repeatedly while
 * no other events are ready to be processed, as long as it returns 1
 * (or ECORE_CALLBACK_RENEW). A return of 0 (or ECORE_CALLBACK_CANCEL) deletes
 * the idler.
 *
 * Idlers are useful for progressively prossessing data without blocking.
 */
EAPI Ecore_Idler *
ecore_idler_add(int (*func) (void *data), const void *data)
{
   Ecore_Idler *ie;

   if (!func) return NULL;
   ie = calloc(1, sizeof(Ecore_Idler));
   if (!ie) return NULL;
   ECORE_MAGIC_SET(ie, ECORE_MAGIC_IDLER);
   ie->func = func;
   ie->data = (void *)data;
   idlers = _ecore_list2_append(idlers, ie);
   return ie;
}

/**
 * Delete an idler callback from the list to be executed.
 * @param  idler The handle of the idler callback to delete
 * @return The data pointer passed to the idler callback on success.  NULL
 *         otherwise.
 * @ingroup Idle_Group
 */
EAPI void *
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
	idlers = _ecore_list2_remove(idlers, ie);
	ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
	free(ie);
     }
   idlers_delete_me = 0;
}

int
_ecore_idler_call(void)
{
   Ecore_List2 *l;

   for (l = (Ecore_List2 *)idlers; l; l = l->next)
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
	for (l = (Ecore_List2 *)idlers; l;)
	  {
	     Ecore_Idler *ie;

	     ie = (Ecore_Idler *)l;
	     l = l->next;
	     if (ie->delete_me)
	       {
		  idlers = _ecore_list2_remove(idlers, ie);
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
