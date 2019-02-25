#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

// This class rely on the parent class to do all the individual holding of value,
// it only compute the average size of an item and answer for that property alone.

// FIXME: handle child being removed
typedef struct _Efl_Ui_Average_Model_Data Efl_Ui_Average_Model_Data;
struct _Efl_Ui_Average_Model_Data
{
   Efl_Ui_Average_Model_Data *parent;

   struct {
      unsigned long long width;
      unsigned long long height;
      unsigned long long wseen;
      unsigned long long hseen;
   } total;

   Eina_Bool wseen : 1;
   Eina_Bool hseen : 1;
};

typedef struct _Efl_Ui_Average_Model_Update Efl_Ui_Average_Model_Update;
struct _Efl_Ui_Average_Model_Update
{
   unsigned long long *total;
   unsigned long long *seen;
   unsigned int previous;
};

static Eina_Value
_efl_ui_average_model_update(Eo *obj EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Ui_Average_Model_Update *request = data;
   unsigned int now;

   if (!eina_value_uint_convert(&v, &now))
     goto on_error;

   *(request->total) += now - request->previous;
   if (request->seen) *(request->seen) += 1;

 on_error:
   return v;
}

static void
_efl_ui_average_model_clean(Eo *obj, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   free(data);

   efl_unref(obj);
}

static Eina_Future *
_efl_ui_average_model_prepare(Eo *obj,
                              unsigned long long *total, unsigned long long *seen,
                              const char *property, Eina_Value *value)
{
   Efl_Ui_Average_Model_Update *update;
   Eina_Value *previous;
   Eina_Future *f;

   update = calloc(1, sizeof (Efl_Ui_Average_Model_Update));
   if (!update) return efl_loop_future_rejected(obj, ENOMEM);

   previous = efl_model_property_get(obj, property);
   if (eina_value_type_get(previous) == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err;

        // Check the case when that property hasn't been set before
        if (!eina_value_error_convert(previous, &err))
          goto on_error;
        if (err != EAGAIN) goto on_error;
     }
   else if (!eina_value_uint_convert(previous, &update->previous))
     goto on_error;
   eina_value_free(previous);

   update->total = total;
   update->seen = seen;

   // As we are operating asynchronously and we want to make sure that the object
   // survive until the transaction is commited, we will ref the object here
   // and unref on clean. This is necessary so that a nested call of property_set
   // on a model returned by children_slice_get, the user doesn't have to keep a
   // reference around to do the same. It shouldn't create any problem as this
   // future would be cancelled automatically when the parent object get destroyed.
   efl_ref(obj);

   // We have to make the change after we fetch the old value, otherwise, well, no old value left
   f = efl_model_property_set(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS), property, value);

   return efl_future_then(obj, f,
                          .success = _efl_ui_average_model_update,
                          .free = _efl_ui_average_model_clean,
                          .data = update);
 on_error:
   eina_value_free(previous);
   free(update);
   return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
}

static Eina_Future *
_efl_ui_average_model_efl_model_property_set(Eo *obj, Efl_Ui_Average_Model_Data *pd, const char *property, Eina_Value *value)
{
   Eina_Future *f = NULL;

   if (!pd->parent) goto end;

   // In vertical list mode we do not need to compute the average width size
   /* if (!strcmp(property, _efl_model_property_selfw)) */
   /*   { */
   /*      f = _efl_ui_average_model_prepare(obj, &pd->parent->total.width, */
   /*                                        pd->wseen ? NULL : &pd->parent->total.wseen, */
   /*                                        property, value, EINA_TRUE); */
   /*      pd->wseen = EINA_TRUE; */
   /*   } */
   if (!strcmp(property, _efl_model_property_selfh))
     {
        f = _efl_ui_average_model_prepare(obj, &pd->parent->total.height,
                                          pd->hseen ? NULL : &pd->parent->total.hseen,
                                          property, value);
        pd->hseen = EINA_TRUE;
     }

 end:
   if (!f)
     f = efl_model_property_set(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS), property, value);

   return f;
}

static inline Eina_Value *
_efl_ui_average_model_compute(const Eo *obj, Eina_Value *r, unsigned long long total, unsigned long long seen)
{
   unsigned int count;

   eina_value_free(r);

   // Protection against divide by zero
   if (!seen) return eina_value_uint_new(0);

   count = efl_model_children_count_get(obj);
   // We are doing the multiply first in an attempt to not reduce the precision to early on.
   return eina_value_uint_new((total * count) / seen);
}

static Eina_Value *
_efl_ui_average_model_efl_model_property_get(const Eo *obj, Efl_Ui_Average_Model_Data *pd, const char *property)
{
   const Eina_Value_Type *t;
   Eina_Value *r;

   r = efl_model_property_get(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS), property);
   if (!r) return r;

   // We are checking that the parent class was able to provide an answer to the request for property "Total.Width"
   // or "Total.Height" which means that we are an object that should compute its size. This avoid computing the
   // pointer to the parent object.
   t = eina_value_type_get(r);
   if (t == EINA_VALUE_TYPE_UINT)
     {
        if (!strcmp(property, _efl_model_property_totalh))
          r = _efl_ui_average_model_compute(obj, r, pd->total.height, pd->total.hseen);
        // We do not need to average the width in vertical list mode as this is done by the parent class
        /* if (!strcmp(property, _efl_model_property_totalw)) */
        /*   r = _efl_ui_average_model_compute(obj, r, pd->total.width, pd->total.wseen); */
     }

   return r;
}

static Efl_Object *
_efl_ui_average_model_efl_object_constructor(Eo *obj, Efl_Ui_Average_Model_Data *pd)
{
   Eo *parent = efl_parent_get(obj);

   if (parent && efl_isa(parent, EFL_UI_AVERAGE_MODEL_CLASS))
     pd->parent = efl_data_scope_get(efl_parent_get(obj), EFL_UI_AVERAGE_MODEL_CLASS);

   return efl_constructor(efl_super(obj, EFL_UI_AVERAGE_MODEL_CLASS));
}

#include "efl_ui_average_model.eo.c"
