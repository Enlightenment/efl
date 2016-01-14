#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS ECORE_IDLE_ENTERER_CLASS

#define MY_CLASS_NAME "Ecore_Idle_Enterer"

struct _Ecore_Idle_Enterer_Data
{
   EINA_INLIST;
   Ecore_Idle_Enterer  *obj;
   Ecore_Task_Cb        func;
   void                 *data;
   int                  references;
   Eina_Bool            delete_me : 1;
};
typedef struct _Ecore_Idle_Enterer_Data Ecore_Idle_Enterer_Data;

static Ecore_Idle_Enterer_Data *idle_enterers = NULL;
static Ecore_Idle_Enterer_Data *idle_enterer_current = NULL;
static int idle_enterers_delete_me = 0;

static void *
_ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer);

static Eina_Bool
_ecore_idle_enterer_add(Ecore_Idle_Enterer *obj,
                    Ecore_Idle_Enterer_Data *ie,
                    Ecore_Task_Cb func,
                    const void   *data)
{
    if (EINA_UNLIKELY(!eina_main_loop_is()))
      {
         EINA_MAIN_LOOP_CHECK_RETURN_VAL(EINA_FALSE);
      }

   ie->obj = obj;
   eo_manual_free_set(obj, EINA_TRUE);

   if (!func)
     {
        ERR("callback function must be set up for an object of class: '%s'", MY_CLASS_NAME);
        return EINA_FALSE;
     }

   ie->func = func;
   ie->data = (void *)data;
   return EINA_TRUE;
}

EAPI Ecore_Idle_Enterer *
ecore_idle_enterer_add(Ecore_Task_Cb func,
                       const void   *data)
{
   Ecore_Idle_Enterer *ie = NULL;
   ie = eo_add(MY_CLASS, _ecore_parent, ecore_idle_enterer_after_constructor(func, data));
   return ie;
}

EOLIAN static void
_ecore_idle_enterer_after_constructor(Eo *obj, Ecore_Idle_Enterer_Data *ie, Ecore_Task_Cb func, const void *data)
{
   if (!_ecore_idle_enterer_add(obj, ie, func, data)) return;

   idle_enterers = (Ecore_Idle_Enterer_Data *)eina_inlist_append(EINA_INLIST_GET(idle_enterers), EINA_INLIST_GET(ie));
}

EAPI Ecore_Idle_Enterer *
ecore_idle_enterer_before_add(Ecore_Task_Cb func,
                              const void   *data)
{
   Ecore_Idle_Enterer *ie = NULL;
   ie = eo_add(MY_CLASS, _ecore_parent, ecore_idle_enterer_before_constructor(func, data));
   return ie;
}

EOLIAN static void
_ecore_idle_enterer_before_constructor(Eo *obj, Ecore_Idle_Enterer_Data *ie, Ecore_Task_Cb func, const void *data)
{
   if (!_ecore_idle_enterer_add(obj, ie, func, data)) return;

   idle_enterers = (Ecore_Idle_Enterer_Data *)eina_inlist_prepend(EINA_INLIST_GET(idle_enterers), EINA_INLIST_GET(ie));
}

EAPI void *
ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer)
{
   if (!idle_enterer) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   return _ecore_idle_enterer_del(idle_enterer);
}

static void *
_ecore_idle_enterer_del(Ecore_Idle_Enterer *obj)
{
   Ecore_Idle_Enterer_Data *idle_enterer = eo_data_scope_get(obj, MY_CLASS);

   if (!idle_enterer) return NULL;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(idle_enterer->delete_me, NULL);
   idle_enterer->delete_me = 1;
   idle_enterers_delete_me = 1;
   return idle_enterer->data;
}

EOLIAN static void
_ecore_idle_enterer_eo_base_destructor(Eo *obj, Ecore_Idle_Enterer_Data *idle_enterer)
{
   idle_enterer->delete_me = 1;
   idle_enterers_delete_me = 1;

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

EOLIAN static Eo *
_ecore_idle_enterer_eo_base_finalize(Eo *obj, Ecore_Idle_Enterer_Data *idle_enterer)
{
   if (!idle_enterer->func)
     {
        return NULL;
     }

     return eo_do_super_ret(obj, MY_CLASS, obj, eo_finalize());
}

void
_ecore_idle_enterer_shutdown(void)
{
   Ecore_Idle_Enterer_Data *ie;
   while ((ie = idle_enterers))
     {
        idle_enterers = (Ecore_Idle_Enterer_Data *)eina_inlist_remove(EINA_INLIST_GET(idle_enterers), EINA_INLIST_GET(idle_enterers));

        eo_do(ie->obj, eo_parent_set(NULL));
        if (eo_destructed_is(ie->obj))
          eo_manual_free(ie->obj);
        else
          eo_manual_free_set(ie->obj, EINA_FALSE);
     }
   idle_enterers_delete_me = 0;
   idle_enterer_current = NULL;
}

void
_ecore_idle_enterer_call(void)
{
   if (!idle_enterer_current)
     {
        /* regular main loop, start from head */
         idle_enterer_current = idle_enterers;
     }
   else
     {
        /* recursive main loop, continue from where we were */
         idle_enterer_current =
           (Ecore_Idle_Enterer_Data *)EINA_INLIST_GET(idle_enterer_current)->next;
     }

   while (idle_enterer_current)
     {
        Ecore_Idle_Enterer_Data *ie = (Ecore_Idle_Enterer_Data *)idle_enterer_current;
        if (!ie->delete_me)
          {
             ie->references++;
             eina_evlog("+idle_enterer", ie, 0.0, NULL);
             if (!_ecore_call_task_cb(ie->func, ie->data))
               {
                  if (!ie->delete_me) _ecore_idle_enterer_del(ie->obj);
               }
             eina_evlog("-idle_enterer", ie, 0.0, NULL);
             ie->references--;
          }
        if (idle_enterer_current) /* may have changed in recursive main loops */
          idle_enterer_current =
            (Ecore_Idle_Enterer_Data *)EINA_INLIST_GET(idle_enterer_current)->next;
     }
   if (idle_enterers_delete_me)
     {
        Ecore_Idle_Enterer_Data *l;
        int deleted_idler_enterers_in_use = 0;

        for (l = idle_enterers; l; )
          {
             Ecore_Idle_Enterer_Data *ie = l;
             l = (Ecore_Idle_Enterer_Data *)EINA_INLIST_GET(l)->next;
             if (ie->delete_me)
               {
                  if (ie->references)
                    {
                       deleted_idler_enterers_in_use++;
                       continue;
                    }

                  idle_enterers = (Ecore_Idle_Enterer_Data *)eina_inlist_remove(EINA_INLIST_GET(idle_enterers), EINA_INLIST_GET(ie));

                  eo_do(ie->obj, eo_parent_set(NULL));
                  if (eo_destructed_is(ie->obj))
                    eo_manual_free(ie->obj);
                  else
                    eo_manual_free_set(ie->obj, EINA_FALSE);
               }
          }
        if (!deleted_idler_enterers_in_use)
          idle_enterers_delete_me = 0;
     }
}

int
_ecore_idle_enterer_exist(void)
{
   if (idle_enterers) return 1;
   return 0;
}

#include "ecore_idle_enterer.eo.c"
