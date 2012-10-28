#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"

#include "Eo.h"

#define MY_CLASS ECORE_IDLE_EXITER_CLASS

#define MY_CLASS_NAME "ecore_idle_exiter"

EAPI Eo_Op ECORE_IDLE_EXITER_BASE_ID = EO_NOOP;

struct _Ecore_Idle_Exiter_Private_Data
{
   EINA_INLIST;
   Ecore_Idle_Exiter  *obj;
   Ecore_Task_Cb      func;
   void               *data;
   int                references;
   Eina_Bool          delete_me : 1;
};

typedef struct _Ecore_Idle_Exiter_Private_Data Ecore_Idle_Exiter_Private_Data;

static Ecore_Idle_Exiter_Private_Data *idle_exiters = NULL;
static Ecore_Idle_Exiter_Private_Data *idle_exiter_current = NULL;
static int idle_exiters_delete_me = 0;

static void *
_ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter);

/**
 * @addtogroup Ecore_Idle_Group
 *
 * @{
 */

/**
 * Add an idle exiter handler.
 * @param func The function to call when exiting an idle state.
 * @param data The data to be passed to the @p func call
 * @return A handle to the idle exiter callback on success.  NULL otherwise.
 * @note The function func will be called every time the main loop is exiting
 * idle state, as long as it returns 1 (or ECORE_CALLBACK_RENEW). A return of 0
 * (or ECORE_CALLBACK_CANCEL) deletes the idle exiter.
 */
EAPI Ecore_Idle_Exiter *
ecore_idle_exiter_add(Ecore_Task_Cb func,
                      const void   *data)
{
   Ecore_Idle_Exiter *ie = NULL;
   ie = eo_add_custom(MY_CLASS, _ecore_parent, ecore_idle_exiter_constructor(func, data));
   eo_unref(ie);
   return ie;
}

static void
_idle_exiter_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Ecore_Task_Cb func = va_arg(*list, Ecore_Task_Cb);
   const void *data = va_arg(*list, const void *);

   _ecore_lock();
    if (EINA_UNLIKELY(!eina_main_loop_is()))
      {
         eo_error_set(obj);
         EINA_MAIN_LOOP_CHECK_RETURN;
      }

   Ecore_Idle_Exiter_Private_Data *ie = _pd;

   ie->obj = obj;
   eo_do_super(obj, eo_constructor());
   eo_manual_free_set(obj, EINA_TRUE);

   if (!func)
     {
        eo_error_set(obj);
        ERR("callback function must be set up for an object of class: '%s'", MY_CLASS_NAME);
        return;
     }

   ie->func = func;
   ie->data = (void *)data;

   idle_exiters = (Ecore_Idle_Exiter_Private_Data *)eina_inlist_append(EINA_INLIST_GET(idle_exiters), EINA_INLIST_GET(ie));
   _ecore_unlock();
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

/**
 * Delete an idle exiter handler from the list to be run on exiting idle state.
 * @param idle_exiter The idle exiter to delete
 * @return The data pointer that was being being passed to the handler if
 *         successful.  NULL otherwise.
 */
EAPI void *
ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter)
{
   void *data;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   _ecore_lock();
   data = _ecore_idle_exiter_del(idle_exiter);
   _ecore_unlock();
   return data;
}

/**
 * @}
 */
static void *
_ecore_idle_exiter_del(Ecore_Idle_Exiter *obj)
{
   Ecore_Idle_Exiter_Private_Data *idle_exiter = eo_data_get(obj, MY_CLASS);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(idle_exiter->delete_me, NULL);
   idle_exiter->delete_me = 1;
   idle_exiters_delete_me = 1;
   return idle_exiter->data;
}


static void
_destructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Ecore_Idle_Exiter_Private_Data *idle_exiter = _pd;

   idle_exiter->delete_me = 1;
   idle_exiters_delete_me = 1;

   eo_do_super(obj, eo_destructor());
}

void
_ecore_idle_exiter_shutdown(void)
{
   Ecore_Idle_Exiter_Private_Data *ie;
   while ((ie = idle_exiters))
     {
        idle_exiters = (Ecore_Idle_Exiter_Private_Data *)eina_inlist_remove(EINA_INLIST_GET(idle_exiters), EINA_INLIST_GET(idle_exiters));

        eo_parent_set(ie->obj, NULL);
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
           (Ecore_Idle_Exiter_Private_Data *)EINA_INLIST_GET(idle_exiter_current)->next;
     }

   while (idle_exiter_current)
     {
        Ecore_Idle_Exiter_Private_Data *ie = (Ecore_Idle_Exiter_Private_Data *)idle_exiter_current;
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
            (Ecore_Idle_Exiter_Private_Data *)EINA_INLIST_GET(idle_exiter_current)->next;
     }
   if (idle_exiters_delete_me)
     {
        Ecore_Idle_Exiter_Private_Data *l;
        int deleted_idler_exiters_in_use = 0;

        for (l = idle_exiters; l; )
          {
             Ecore_Idle_Exiter_Private_Data *ie = l;

             l = (Ecore_Idle_Exiter_Private_Data *)EINA_INLIST_GET(l)->next;
             if (ie->delete_me)
               {
                  if (ie->references)
                    {
                       deleted_idler_exiters_in_use++;
                       continue;
                    }

                  idle_exiters = (Ecore_Idle_Exiter_Private_Data *)eina_inlist_remove(EINA_INLIST_GET(idle_exiters), EINA_INLIST_GET(ie));

                  eo_parent_set(ie->obj, NULL);
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

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),

        EO_OP_FUNC(ECORE_IDLE_EXITER_ID(ECORE_IDLE_EXITER_SUB_ID_CONSTRUCTOR), _idle_exiter_constructor),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ECORE_IDLE_EXITER_SUB_ID_CONSTRUCTOR, "Add an idle exiter handler."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ECORE_IDLE_EXITER_BASE_ID, op_desc, ECORE_IDLE_EXITER_SUB_ID_LAST),
     NULL,
     sizeof(Ecore_Idle_Exiter_Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(ecore_idle_exiter_class_get, &class_desc, EO_BASE_CLASS, NULL)
