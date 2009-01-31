/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "ecore_private.h"
#include "Ecore.h"

static Ecore_Idle_Exiter *idle_exiters = NULL;
static int                idle_exiters_delete_me = 0;

/**
 * Add an idle exiter handler.
 * @param func The function to call when exiting an idle state.
 * @param data The data to be passed to the @p func call
 * @return A handle to the idle exiter callback on success.  NULL otherwise.
 * @ingroup Idle_Group
 */
EAPI Ecore_Idle_Exiter *
ecore_idle_exiter_add(int (*func) (void *data), const void *data)
{
   Ecore_Idle_Exiter *ie;

   if (!func) return NULL;
   ie = calloc(1, sizeof(Ecore_Idle_Exiter));
   if (!ie) return NULL;
   ECORE_MAGIC_SET(ie, ECORE_MAGIC_IDLE_EXITER);
   ie->func = func;
   ie->data = (void *)data;
   idle_exiters = _ecore_list2_append(idle_exiters, ie);
   return ie;
}

/**
 * Delete an idle exiter handler from the list to be run on exiting idle state.
 * @param idle_exiter The idle exiter to delete
 * @return The data pointer that was being being passed to the handler if
 *         successful.  NULL otherwise.
 * @ingroup Idle_Group
 */
EAPI void *
ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter)
{
   if (!ECORE_MAGIC_CHECK(idle_exiter, ECORE_MAGIC_IDLE_EXITER))
     {
	ECORE_MAGIC_FAIL(idle_exiter, ECORE_MAGIC_IDLE_EXITER,
			 "ecore_idle_exiter_del");
	return NULL;
     }
   idle_exiter->delete_me = 1;
   idle_exiters_delete_me = 1;
   return idle_exiter->data;
}

void
_ecore_idle_exiter_shutdown(void)
{
   while (idle_exiters)
     {
	Ecore_Idle_Exiter *ie;

	ie = idle_exiters;
	idle_exiters = _ecore_list2_remove(idle_exiters, ie);
	ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
	free(ie);
     }
   idle_exiters_delete_me = 0;
}

void
_ecore_idle_exiter_call(void)
{
   Ecore_List2 *l;

   for (l = (Ecore_List2 *)idle_exiters; l; l = l->next)
     {
	Ecore_Idle_Exiter *ie;

	ie = (Ecore_Idle_Exiter *)l;
	if (!ie->delete_me)
	  {
	     if (!ie->func(ie->data)) ecore_idle_exiter_del(ie);
	  }
     }
   if (idle_exiters_delete_me)
     {
	for (l = (Ecore_List2 *)idle_exiters; l;)
	  {
	     Ecore_Idle_Exiter *ie;

	     ie = (Ecore_Idle_Exiter *)l;
	     l = l->next;
	     if (ie->delete_me)
	       {
		  idle_exiters = _ecore_list2_remove(idle_exiters, ie);
		  ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
		  free(ie);
	       }
	  }
	idle_exiters_delete_me = 0;
     }
}

int
_ecore_idle_exiter_exist(void)
{
   if (idle_exiters) return 1;
   return 0;
}
