#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS ECORE_IDLER_CLASS

#define MY_CLASS_NAME "Ecore_Idler"

struct _Ecore_Idler_Data
{
   EINA_INLIST;
   Ecore_Idler  *obj;
   Ecore_Task_Cb func;
   void         *data;
   int           references;
   Eina_Bool     delete_me : 1;
};

typedef struct _Ecore_Idler_Data Ecore_Idler_Data;
static Ecore_Idler_Data *idlers = NULL;
static Ecore_Idler_Data *idler_current = NULL;
static int idlers_delete_me = 0;

static void *
_ecore_idler_del(Ecore_Idler *idler);

EAPI Ecore_Idler *
ecore_idler_add(Ecore_Task_Cb func,
                const void   *data)
{
   return eo_add(MY_CLASS, _ecore_parent, ecore_idler_constructor(func, data));
}

EOLIAN static void
_ecore_idler_constructor(Eo *obj, Ecore_Idler_Data *ie, Ecore_Task_Cb func, const void *data)
{
    if (EINA_UNLIKELY(!eina_main_loop_is()))
      {
         EINA_MAIN_LOOP_CHECK_RETURN;
      }

   ie->obj = obj;
   eo_manual_free_set(obj, EINA_TRUE);

   if (!func)
     {
        ERR("callback function must be set up for an object of class: '%s'", MY_CLASS_NAME);
        return;
     }

   ie->func = func;
   ie->data = (void *)data;
   idlers = (Ecore_Idler_Data *)eina_inlist_append(EINA_INLIST_GET(idlers), EINA_INLIST_GET(ie));
}

EAPI void *
ecore_idler_del(Ecore_Idler *idler)
{
   if (!idler) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   return _ecore_idler_del(idler);
}

static void *
_ecore_idler_del(Ecore_Idler *obj)
{
   Ecore_Idler_Data *idler = eo_data_scope_get(obj, MY_CLASS);
   if (!idler) return NULL;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(idler->delete_me, NULL);
   idler->delete_me = 1;
   idlers_delete_me = 1;
   return idler->data;
}

EOLIAN static void
_ecore_idler_eo_base_destructor(Eo *obj, Ecore_Idler_Data *idler)
{
   idler->delete_me = 1;
   idlers_delete_me = 1;

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

EOLIAN static Eo *
_ecore_idler_eo_base_finalize(Eo *obj, Ecore_Idler_Data *idler)
{
   if (!idler->func)
     {
        return NULL;
     }

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_finalize());
}

void
_ecore_idler_shutdown(void)
{
   Ecore_Idler_Data *ie;
   while ((ie = idlers))
     {
        idlers = (Ecore_Idler_Data *)eina_inlist_remove(EINA_INLIST_GET(idlers), EINA_INLIST_GET(idlers));

        eo_do(ie->obj, eo_parent_set(NULL));
        if (eo_destructed_is(ie->obj))
          eo_manual_free(ie->obj);
        else
          eo_manual_free_set(ie->obj, EINA_FALSE);
     }
   idlers_delete_me = 0;
   idler_current = NULL;
}

int
_ecore_idler_all_call(void)
{
   if (!idler_current)
     {
        /* regular main loop, start from head */
         idler_current = idlers;
     }
   else
     {
        /* recursive main loop, continue from where we were */
         idler_current = (Ecore_Idler_Data *)EINA_INLIST_GET(idler_current)->next;
     }

   while (idler_current)
     {
        Ecore_Idler_Data *ie = (Ecore_Idler_Data *)idler_current;
        if (!ie->delete_me)
          {
             ie->references++;
             eina_evlog("+idler", ie, 0.0, NULL);
             if (!_ecore_call_task_cb(ie->func, ie->data))
               {
                  if (!ie->delete_me) _ecore_idler_del(ie->obj);
               }
             eina_evlog("-idler", ie, 0.0, NULL);
             ie->references--;
          }
        if (idler_current) /* may have changed in recursive main loops */
          idler_current = (Ecore_Idler_Data *)EINA_INLIST_GET(idler_current)->next;
     }
   if (idlers_delete_me)
     {
        Ecore_Idler_Data *l;
        int deleted_idlers_in_use = 0;
        for (l = idlers; l; )
          {
             Ecore_Idler_Data *ie = l;
             l = (Ecore_Idler_Data *)EINA_INLIST_GET(l)->next;
             if (ie->delete_me)
               {
                  if (ie->references)
                    {
                       deleted_idlers_in_use++;
                       continue;
                    }

                  idlers = (Ecore_Idler_Data *)eina_inlist_remove(EINA_INLIST_GET(idlers), EINA_INLIST_GET(ie));

                  eo_do(ie->obj, eo_parent_set(NULL));
                  if (eo_destructed_is(ie->obj))
                     eo_manual_free(ie->obj);
                  else
                     eo_manual_free_set(ie->obj, EINA_FALSE);
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

#include "ecore_idler.eo.c"
