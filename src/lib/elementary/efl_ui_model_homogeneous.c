#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

// For now only vertical logic is implemented. Horizontal list and grid are not supported.

typedef struct _Efl_Ui_Model_Homogeneous_Data Efl_Ui_Model_Homogeneous_Data;
struct _Efl_Ui_Model_Homogeneous_Data
{
   Efl_Ui_Model_Homogeneous_Data *parent;

   struct {
      unsigned int width;
      unsigned int height;

      struct {
         Eina_Bool width;
         Eina_Bool height;
      } defined;
   } item;
};

static Eina_Future *
_efl_ui_model_homogeneous_property_set(Eo *obj, Eina_Value *value,
                                       Eina_Bool *defined, unsigned int *r)
{
   Eina_Future *f;

   if (*defined)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
   if (!eina_value_uint_convert(value, r))
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
   *defined = EINA_TRUE;
   f = efl_loop_future_resolved(obj, *value);
   eina_value_free(value);
   return f;
}

static Eina_Future *
_efl_ui_model_homogeneous_efl_model_property_set(Eo *obj,
                                                 Efl_Ui_Model_Homogeneous_Data *pd,
                                                 const char *property, Eina_Value *value)
{
   if (pd->parent)
     {
        if (!strcmp(property, _efl_model_property_selfw))
          return _efl_ui_model_homogeneous_property_set(obj, value,
                                                        &pd->parent->item.defined.width,
                                                        &pd->parent->item.width);
        if (!strcmp(property, _efl_model_property_selfh))
          return _efl_ui_model_homogeneous_property_set(obj, value,
                                                        &pd->parent->item.defined.height,
                                                        &pd->parent->item.height);
        if (!strcmp(property, _efl_model_property_totalw) ||
            !strcmp(property, _efl_model_property_totalh))
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
     }
   if (!strcmp(property, _efl_model_property_itemw))
     {
        return _efl_ui_model_homogeneous_property_set(obj, value,
                                                      &pd->item.defined.width,
                                                      &pd->item.width);
     }
   if (!strcmp(property, _efl_model_property_itemh))
     {
        return _efl_ui_model_homogeneous_property_set(obj, value,
                                                      &pd->item.defined.height,
                                                      &pd->item.height);
     }

   return efl_model_property_set(efl_super(obj, EFL_UI_MODEL_HOMOGENEOUS_CLASS),
                                 property, value);
}

static Eina_Value *
_efl_ui_model_homogeneous_efl_model_property_get(const Eo *obj,
                                                 Efl_Ui_Model_Homogeneous_Data *pd,
                                                 const char *property)
{
   if (pd->parent)
     {
        if (!strcmp(property, _efl_model_property_selfw))
          {
             if (pd->parent->item.defined.width)
               return eina_value_uint_new(pd->parent->item.width);
             goto not_ready;
          }
        if (!strcmp(property, _efl_model_property_selfh))
          {
             if (pd->parent->item.defined.height)
               return eina_value_uint_new(pd->parent->item.height);
             goto not_ready;
          }
     }
   if (!strcmp(property, _efl_model_property_itemw))
     {
        if (pd->item.defined.width)
          return eina_value_uint_new(pd->item.width);
        goto not_ready;
     }
   if (!strcmp(property, _efl_model_property_itemh))
     {
        if (pd->item.defined.height)
          return eina_value_uint_new(pd->item.height);
        goto not_ready;
     }
   if (!strcmp(property, _efl_model_property_totalh))
     {
        if (pd->item.defined.height)
          return eina_value_uint_new(pd->item.height *
                                     efl_model_children_count_get(obj));
        goto not_ready;
     }
   if (!strcmp(property, _efl_model_property_totalw))
     {
        if (pd->item.defined.width)
          // We only handle vertical list at this point, so total width is the width of one item.
          return eina_value_uint_new(pd->item.width);
        goto not_ready;
     }

   return efl_model_property_get(efl_super(obj, EFL_UI_MODEL_HOMOGENEOUS_CLASS), property);

 not_ready:
   return eina_value_error_new(EAGAIN);
}

static Efl_Object *
_efl_ui_model_homogeneous_efl_object_constructor(Eo *obj, Efl_Ui_Model_Homogeneous_Data *pd)
{
   Eo *parent = efl_parent_get(obj);

   if (parent && efl_isa(parent, EFL_UI_MODEL_HOMOGENEOUS_CLASS))
     pd->parent = efl_data_scope_get(efl_parent_get(obj), EFL_UI_MODEL_HOMOGENEOUS_CLASS);

   return efl_constructor(efl_super(obj, EFL_UI_MODEL_HOMOGENEOUS_CLASS));
}

#include "efl_ui_model_homogeneous.eo.c"
