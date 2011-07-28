#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"


struct _Ecore_Idler
{
   EINA_INLIST;
   ECORE_MAGIC;
   Ecore_Task_Cb func;
   void        *data;
   int          references;
   Eina_Bool    delete_me : 1;
};


static Ecore_Idler *idlers = NULL;
static Ecore_Idler *idler_current = NULL;
static int          idlers_delete_me = 0;

static void *
_ecore_idler_del(Ecore_Idler *idler);

/**
 * @addtogroup Ecore_Group Ecore - Main Loop and Job Functions.
 *
 * @{
 */

/**
 * @addtogroup Ecore_Idle_Group Ecore Idle functions
 *
Callbacks that are called when the program enters or exits an idle state.

The ecore main loop enters an idle state when it is waiting for timers
to time out, data to come in on a file descriptor or any other event
to occur.  You can set callbacks to be called when the main loop
enters an idle state, during an idle state or just after the program
wakes up.

Enterer callbacks are good for updating your program's state, if it
has a state engine.  Once all of the enterer handlers are called, the
program will enter a "sleeping" state.

Idler callbacks are called when the main loop has called all enterer
handlers.  They are useful for interfaces that require polling and
timers would be too slow to use.

If no idler callbacks are specified, then the process literally goes
to sleep.  Otherwise, the idler callbacks are called continuously
while the loop is "idle", using as much CPU as is available to the
process.

Exiter callbacks are called when the main loop wakes up from an idle
state.

@note Idle state doesn't mean that the @b program is idle, but that the <b>main
loop</b> is idle. It doesn't have any timers, events, fd handlers or anything
else to process (which in most <em>event driven</em> programs also means that the @b
program is idle too, but it's not a rule). The program itself may be doing a lot of
processing in the idler, or in another thread, for example.

Example with functions that deal with idle state:

@li @ref ecore_idler_example_c

 * @{
 */

/**
 * Add an idler handler.
 * @param  func The function to call when idling.
 * @param  data The data to be passed to this @p func call.
 * @return A idler handle if successfully added.  NULL otherwise.
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
ecore_idler_add(Ecore_Task_Cb func, const void *data)
{
   Ecore_Idler *ie = NULL;

   _ecore_lock();
   if (!func) goto unlock;
   ie = calloc(1, sizeof(Ecore_Idler));
   if (!ie) goto unlock;
   ECORE_MAGIC_SET(ie, ECORE_MAGIC_IDLER);
   ie->func = func;
   ie->data = (void *)data;
   idlers = (Ecore_Idler *) eina_inlist_append(EINA_INLIST_GET(idlers), EINA_INLIST_GET(ie));
unlock:
   _ecore_unlock();
   return ie;
}

/**
 * Delete an idler callback from the list to be executed.
 * @param  idler The handle of the idler callback to delete
 * @return The data pointer passed to the idler callback on success.  NULL
 *         otherwise.
 */
EAPI void *
ecore_idler_del(Ecore_Idler *idler)
{
   void *data = NULL;

   if (!ECORE_MAGIC_CHECK(idler, ECORE_MAGIC_IDLER))
     {
        ECORE_MAGIC_FAIL(idler, ECORE_MAGIC_IDLER,
                         "ecore_idler_del");
        return NULL;
     }

   _ecore_lock();
   data = _ecore_idler_del(idler);
   _ecore_unlock();
   return data;
}

/**
 * @}
 */

/**
 * @}
 */

static void *
_ecore_idler_del(Ecore_Idler *idler)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(idler->delete_me, NULL);
   idler->delete_me = 1;
   idlers_delete_me = 1;
   return idler->data;
}


void
_ecore_idler_shutdown(void)
{
   Ecore_Idler *ie;
   while ((ie = idlers))
     {
        idlers = (Ecore_Idler *) eina_inlist_remove(EINA_INLIST_GET(idlers), EINA_INLIST_GET(idlers));
        ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
        free(ie);
     }
   idlers_delete_me = 0;
   idler_current = NULL;
}

int
_ecore_idler_call(void)
{
   if (!idler_current)
     {
        /* regular main loop, start from head */
        idler_current = idlers;
     }
   else
     {
        /* recursive main loop, continue from where we were */
        idler_current = (Ecore_Idler *)EINA_INLIST_GET(idler_current)->next;
     }

   while (idler_current)
     {
        Ecore_Idler *ie = (Ecore_Idler *)idler_current;
        if (!ie->delete_me)
          {
             Eina_Bool ret;
             Ecore_Task_Cb func;
             void *data;

             func = ie->func;
             data = ie->data;
             ie->references++;
             _ecore_unlock();
             ret = func(data);
             _ecore_lock();
             if (!ret)
               {
                  if (!ie->delete_me) _ecore_idler_del(ie);
               }
             ie->references--;
          }
        if (idler_current) /* may have changed in recursive main loops */
          idler_current = (Ecore_Idler *)EINA_INLIST_GET(idler_current)->next;
     }
   if (idlers_delete_me)
     {
        Ecore_Idler *l;
        int deleted_idlers_in_use = 0;
        for (l = idlers; l;)
          {
             Ecore_Idler *ie = l;
             l = (Ecore_Idler *) EINA_INLIST_GET(l)->next;
             if (ie->delete_me)
               {
                  if (ie->references)
                    {
                       deleted_idlers_in_use++;
                       continue;
                    }

                  idlers = (Ecore_Idler *) eina_inlist_remove(EINA_INLIST_GET(idlers), EINA_INLIST_GET(ie));
                  ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
                  free(ie);
               }
          }
        if (!deleted_idlers_in_use)
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
