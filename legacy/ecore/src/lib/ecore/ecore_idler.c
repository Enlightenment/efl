#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"

#include "Eo.h"

#define MY_CLASS ECORE_IDLER_CLASS

#define MY_CLASS_NAME "ecore_idler"

EAPI Eo_Op ECORE_IDLER_BASE_ID = EO_NOOP;

struct _Ecore_Idler_Private_Data
{
   EINA_INLIST;
   Ecore_Idler  *obj;
   Ecore_Task_Cb func;
   void         *data;
   int           references;
   Eina_Bool     delete_me : 1;
};

typedef struct _Ecore_Idler_Private_Data Ecore_Idler_Private_Data;
static Ecore_Idler_Private_Data *idlers = NULL;
static Ecore_Idler_Private_Data *idler_current = NULL;
static int idlers_delete_me = 0;

static void *
_ecore_idler_del(Ecore_Idler *idler);

EAPI Ecore_Idler *
ecore_idler_add(Ecore_Task_Cb func,
                const void   *data)
{
   Ecore_Idler *ie = NULL;

   _ecore_lock();

   ie = eo_add_custom(MY_CLASS, _ecore_parent, ecore_idler_constructor(func, data));
   eo_unref(ie);

   _ecore_unlock();
   return ie;
}

static void
_idler_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Ecore_Task_Cb func = va_arg(*list, Ecore_Task_Cb);
   const void *data = va_arg(*list, const void *);

    if (EINA_UNLIKELY(!eina_main_loop_is()))
      {
         eo_error_set(obj);
         EINA_MAIN_LOOP_CHECK_RETURN;
      }

   Ecore_Idler_Private_Data *ie = _pd;

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
   idlers = (Ecore_Idler_Private_Data *)eina_inlist_append(EINA_INLIST_GET(idlers), EINA_INLIST_GET(ie));
}


static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

EAPI void *
ecore_idler_del(Ecore_Idler *idler)
{
   void *data = NULL;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

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
_ecore_idler_del(Ecore_Idler *obj)
{
   Ecore_Idler_Private_Data *idler = eo_data_get(obj, MY_CLASS);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(idler->delete_me, NULL);
   idler->delete_me = 1;
   idlers_delete_me = 1;
   return idler->data;
}

static void
_destructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Ecore_Idler_Private_Data *idler = _pd;

   idler->delete_me = 1;
   idlers_delete_me = 1;

   eo_do_super(obj, eo_destructor());
}

void
_ecore_idler_shutdown(void)
{
   Ecore_Idler_Private_Data *ie;
   while ((ie = idlers))
     {
        idlers = (Ecore_Idler_Private_Data *)eina_inlist_remove(EINA_INLIST_GET(idlers), EINA_INLIST_GET(idlers));

        eo_parent_set(ie->obj, NULL);
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
         idler_current = (Ecore_Idler_Private_Data *)EINA_INLIST_GET(idler_current)->next;
     }

   while (idler_current)
     {
        Ecore_Idler_Private_Data *ie = (Ecore_Idler_Private_Data *)idler_current;
        if (!ie->delete_me)
          {
             ie->references++;
             if (!_ecore_call_task_cb(ie->func, ie->data))
               {
                  if (!ie->delete_me) _ecore_idler_del(ie->obj);
               }
             ie->references--;
          }
        if (idler_current) /* may have changed in recursive main loops */
          idler_current = (Ecore_Idler_Private_Data *)EINA_INLIST_GET(idler_current)->next;
     }
   if (idlers_delete_me)
     {
        Ecore_Idler_Private_Data *l;
        int deleted_idlers_in_use = 0;
        for (l = idlers; l; )
          {
             Ecore_Idler_Private_Data *ie = l;
             l = (Ecore_Idler_Private_Data *)EINA_INLIST_GET(l)->next;
             if (ie->delete_me)
               {
                  if (ie->references)
                    {
                       deleted_idlers_in_use++;
                       continue;
                    }

                  idlers = (Ecore_Idler_Private_Data *)eina_inlist_remove(EINA_INLIST_GET(idlers), EINA_INLIST_GET(ie));

                  eo_parent_set(ie->obj, NULL);
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

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),

        EO_OP_FUNC(ECORE_IDLER_ID(ECORE_IDLER_SUB_ID_CONSTRUCTOR), _idler_constructor),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ECORE_IDLER_SUB_ID_CONSTRUCTOR, "Add an idler handler."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ECORE_IDLER_BASE_ID, op_desc, ECORE_IDLER_SUB_ID_LAST),
     NULL,
     sizeof(Ecore_Idler_Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(ecore_idler_class_get, &class_desc, EO_BASE_CLASS, NULL)
