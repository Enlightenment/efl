#include "ecore_private.h"
#include "Ecore.h"

static Ecore_Idle_Enterer *idle_enterers = NULL;
static int                 idle_enterers_delete_me = 0;

/**
 * Add an idle enterer handler.
 * @param   func The function to call when entering an idle state.
 * @param   data The data to be passed to the @p func call
 * @return  A handle to the idle enterer callback if successful.  Otherwise,
 *          NULL is returned.
 * @ingroup Idle_Group
 */
EAPI Ecore_Idle_Enterer *
ecore_idle_enterer_add(int (*func) (void *data), const void *data)
{
   Ecore_Idle_Enterer *ie;

   if (!func) return NULL;
   ie = calloc(1, sizeof(Ecore_Idle_Enterer));
   if (!ie) return NULL;
   ECORE_MAGIC_SET(ie, ECORE_MAGIC_IDLE_ENTERER);
   ie->func = func;
   ie->data = (void *)data;
   idle_enterers = _ecore_list2_append(idle_enterers, ie);
   return ie;
}

/**
 * Add an idle enterer handler at the start of the list so it gets called earlier than others.
 * @param   func The function to call when entering an idle state.
 * @param   data The data to be passed to the @p func call
 * @return  A handle to the idle enterer callback if successful.  Otherwise,
 *          NULL is returned.
 * @ingroup Idle_Group
 */
EAPI Ecore_Idle_Enterer *
ecore_idle_enterer_before_add(int (*func) (void *data), const void *data)
{
   Ecore_Idle_Enterer *ie;

   if (!func) return NULL;
   ie = calloc(1, sizeof(Ecore_Idle_Enterer));
   if (!ie) return NULL;
   ECORE_MAGIC_SET(ie, ECORE_MAGIC_IDLE_ENTERER);
   ie->func = func;
   ie->data = (void *)data;
   idle_enterers = _ecore_list2_prepend(idle_enterers, ie);
   return ie;
}

/**
 * Delete an idle enterer callback.
 * @param   idle_enterer The idle enterer to delete
 * @return  The data pointer passed to the idler enterer callback on success.
 *          NULL otherwise.
 * @ingroup Idle_Group
 */
EAPI void *
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
	idle_enterers = _ecore_list2_remove(idle_enterers, ie);
	ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
	free(ie);
     }
   idle_enterers_delete_me = 0;
}

void
_ecore_idle_enterer_call(void)
{
   Ecore_List2 *l;
   
   for (l = (Ecore_List2 *)idle_enterers; l; l = l->next)
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
	for (l = (Ecore_List2 *)idle_enterers; l;)
	  {
	     Ecore_Idle_Enterer *ie;
	     
	     ie = (Ecore_Idle_Enterer *)l;
	     l = l->next;
	     if (ie->delete_me)
	       {
		  idle_enterers = _ecore_list2_remove(idle_enterers, ie);
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
