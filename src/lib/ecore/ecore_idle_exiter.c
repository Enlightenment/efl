#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS ECORE_IDLE_EXITER_CLASS

#define MY_CLASS_NAME "Ecore_Idle_Exiter"

struct _Ecore_Idle_Exiter_Data
{
   EINA_INLIST;
   Ecore_Idle_Exiter  *obj;
   Ecore_Task_Cb      func;
   void               *data;
   int                references;
   Eina_Bool          delete_me : 1;
};

typedef struct _Ecore_Idle_Exiter_Data Ecore_Idle_Exiter_Data;

static Ecore_Idle_Exiter_Data *idle_exiters = NULL;
static Ecore_Idle_Exiter_Data *idle_exiter_current = NULL;
static int idle_exiters_delete_me = 0;

static void *
_ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter);

EAPI Ecore_Idle_Exiter *
ecore_idle_exiter_add(Ecore_Task_Cb func,
                      const void   *data)
{
   Ecore_Idle_Exiter *ie = NULL;
   ie = eo_add(MY_CLASS, _ecore_parent, ecore_idle_exiter_constructor(func, data));
   eo_unref(ie);
   return ie;
}

EOLIAN static void
_ecore_idle_exiter_constructor(Eo *obj, Ecore_Idle_Exiter_Data *ie, Ecore_Task_Cb func, const void *data)
{
   _ecore_lock();
    if (EINA_UNLIKELY(!eina_main_loop_is()))
      {
         eo_error_set(obj);
         EINA_MAIN_LOOP_CHECK_RETURN;
      }


   ie->obj = obj;
   eo_manual_free_set(obj, EINA_TRUE);

   if (!func)
     {
        eo_error_set(obj);
        ERR("callback function must be set up for an object of class: '%s'", MY_CLASS_NAME);
        return;
     }

   ie->func = func;
   ie->data = (void *)data;

   idle_exiters = (Ecore_Idle_Exiter_Data *)eina_inlist_append(EINA_INLIST_GET(idle_exiters), EINA_INLIST_GET(ie));
   _ecore_unlock();
}

EAPI void *
ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter)
{
   void *data;

   if (!idle_exiter) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   _ecore_lock();
   data = _ecore_idle_exiter_del(idle_exiter);
   _ecore_unlock();
   return data;
}

static void *
_ecore_idle_exiter_del(Ecore_Idle_Exiter *obj)
{
   Ecore_Idle_Exiter_Data *idle_exiter = eo_data_scope_get(obj, MY_CLASS);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(idle_exiter->delete_me, NULL);
   idle_exiter->delete_me = 1;
   idle_exiters_delete_me = 1;
   return idle_exiter->data;
}


EOLIAN static void
_ecore_idle_exiter_eo_base_destructor(Eo *obj, Ecore_Idle_Exiter_Data *idle_exiter)
{

   idle_exiter->delete_me = 1;
   idle_exiters_delete_me = 1;

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

void
_ecore_idle_exiter_shutdown(void)
{
   Ecore_Idle_Exiter_Data *ie;
   while ((ie = idle_exiters))
     {
        idle_exiters = (Ecore_Idle_Exiter_Data *)eina_inlist_remove(EINA_INLIST_GET(idle_exiters), EINA_INLIST_GET(idle_exiters));

        eo_do(ie->obj, eo_parent_set(NULL));
        if (eo_destructed_is(ie->obj))
          eo_manual_free(ie->obj);
        else
          eo_manual_free_set(ie->obj, EINA_FALSE);
     }
   idle_exiters_delete_me = 0;
   idle_exiter_current = NULL;
}

void
_ecore_idle_exiter_call(void)
{
   if (!idle_exiter_current)
     {
        /* regular main loop, start from head */
         idle_exiter_current = idle_exiters;
     }
   else
     {
        /* recursive main loop, continue from where we were */
         idle_exiter_current =
           (Ecore_Idle_Exiter_Data *)EINA_INLIST_GET(idle_exiter_current)->next;
     }

   while (idle_exiter_current)
     {
        Ecore_Idle_Exiter_Data *ie = (Ecore_Idle_Exiter_Data *)idle_exiter_current;
        if (!ie->delete_me)
          {
             ie->references++;
             if (!_ecore_call_task_cb(ie->func, ie->data))
               {
                  if (!ie->delete_me) _ecore_idle_exiter_del(ie->obj);
               }
             ie->references--;
          }
        if (idle_exiter_current) /* may have changed in recursive main loops */
          idle_exiter_current =
            (Ecore_Idle_Exiter_Data *)EINA_INLIST_GET(idle_exiter_current)->next;
     }
   if (idle_exiters_delete_me)
     {
        Ecore_Idle_Exiter_Data *l;
        int deleted_idler_exiters_in_use = 0;

        for (l = idle_exiters; l; )
          {
             Ecore_Idle_Exiter_Data *ie = l;

             l = (Ecore_Idle_Exiter_Data *)EINA_INLIST_GET(l)->next;
             if (ie->delete_me)
               {
                  if (ie->references)
                    {
                       deleted_idler_exiters_in_use++;
                       continue;
                    }

                  idle_exiters = (Ecore_Idle_Exiter_Data *)eina_inlist_remove(EINA_INLIST_GET(idle_exiters), EINA_INLIST_GET(ie));

                  eo_do(ie->obj, eo_parent_set(NULL));
                  if (eo_destructed_is(ie->obj))
                    eo_manual_free(ie->obj);
                  else
                    eo_manual_free_set(ie->obj, EINA_FALSE);
               }
          }
        if (!deleted_idler_exiters_in_use)
          idle_exiters_delete_me = 0;
     }
}

int
_ecore_idle_exiter_exist(void)
{
   if (idle_exiters) return 1;
   return 0;
}

#include "ecore_idle_exiter.eo.c"
