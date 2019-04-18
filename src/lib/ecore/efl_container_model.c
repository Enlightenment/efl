#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>
#include <Eina.h>
#include <Eo.h>
#include <Ecore.h>

#include "ecore_internal.h"
#include "efl_composite_model_private.h"

typedef struct _Efl_Container_Property_Data Efl_Container_Property_Data;
typedef struct _Efl_Container_Model_Data Efl_Container_Model_Data;

struct _Efl_Container_Property_Data
{
   Eina_Stringshare *name;

   const Eina_Value_Type *type;
   Eina_Array *values;
};

struct _Efl_Container_Model_Data
{
   Efl_Container_Model_Data *parent;

   Eina_Hash *properties; // All properties value for children
   unsigned int children_count;
};

static void
_property_values_free(Eina_Array *values)
{
   Eina_Value *v;

   while ((v = eina_array_pop(values)))
     eina_value_free(v);
   eina_array_free(values);
}

static void
_property_data_free_cb(void *data)
{
   Efl_Container_Property_Data *cpd = data;

   eina_stringshare_del(cpd->name);
   _property_values_free(cpd->values);
   free(cpd);
}

static Efl_Object *
_efl_container_model_efl_object_constructor(Eo *obj,
                                            Efl_Container_Model_Data *sd)
{
   Eo *parent;

   obj = efl_constructor(efl_super(obj, EFL_CONTAINER_MODEL_CLASS));
   if (!obj) return NULL;

   parent = efl_parent_get(obj);
   if (efl_isa(parent, EFL_CONTAINER_MODEL_CLASS))
     sd->parent = efl_data_scope_get(parent, EFL_CONTAINER_MODEL_CLASS);

   sd->properties = eina_hash_stringshared_new(_property_data_free_cb);

   return obj;
}

static Efl_Object *
_efl_container_model_efl_object_finalize(Eo *obj, Efl_Container_Model_Data *sd EINA_UNUSED)
{
   if (!efl_ui_view_model_get(obj))
     {
        // Add a dummy object as source if there isn't any to allow using this class without source.
        efl_ui_view_model_set(obj, efl_add(EFL_GENERIC_MODEL_CLASS, obj));
     }
   return efl_finalize(efl_super(obj, EFL_CONTAINER_MODEL_CLASS));
}

static void
_efl_container_model_efl_object_destructor(Eo *obj,
                                           Efl_Container_Model_Data *sd)
{
   eina_hash_free(sd->properties);

   efl_destructor(efl_super(obj, EFL_CONTAINER_MODEL_CLASS));
}

static const Eina_Value_Type *
_efl_container_model_child_property_value_type_get(Eo *obj EINA_UNUSED,
                                                   Efl_Container_Model_Data *sd,
                                                   const char *property)
{
   Efl_Container_Property_Data *cpd;
   Eina_Stringshare *key;

   key = eina_stringshare_add(property);
   cpd = eina_hash_find(sd->properties, key);
   eina_stringshare_del(key);

   if (!cpd) return NULL;
   return cpd->type;
}

static Eina_Bool
_efl_container_model_child_property_add(Eo *obj,
                                        Efl_Container_Model_Data *sd,
                                        const char *name,
                                        const Eina_Value_Type *type,
                                        Eina_Iterator *values)
{
   Eina_Array *arr = NULL;
   void *original;
   Efl_Container_Property_Data *cpd = NULL;
   unsigned int i;
   Eina_Error err = EFL_MODEL_ERROR_INCORRECT_VALUE;

   name = eina_stringshare_add(name);

   if (!type || !values)
     {
        EINA_LOG_WARN("Invalid input data");
        goto on_error;
     }

   err = ENOMEM;
   arr = eina_array_new(4);
   if (!arr) goto on_error;

   EINA_ITERATOR_FOREACH(values, original)
     {
        Eina_Value *copy = eina_value_new(type);
        Eina_Bool r;

        if (type == EINA_VALUE_TYPE_STRINGSHARE ||
            type == EINA_VALUE_TYPE_STRING)
          r = eina_value_set(copy, original);
        else
          r = eina_value_pset(copy, original);

        if (!r)
          {
             eina_value_free(copy);
             copy = eina_value_error_new(EINA_ERROR_VALUE_FAILED);
          }

        eina_array_push(arr, copy);
     }
   eina_iterator_free(values);

   err = EFL_MODEL_ERROR_UNKNOWN;

   cpd = eina_hash_find(sd->properties, name);
   if (!cpd)
     {
        cpd = calloc(1, sizeof(Efl_Container_Property_Data));
        if (!cpd)
          goto on_error;

        cpd->name = eina_stringshare_ref(name);
        cpd->type = type;
        cpd->values = arr;

        if (!eina_hash_direct_add(sd->properties, name, cpd))
          goto on_error;
     }
   else
     {
        _property_values_free(cpd->values);

        cpd->type = type;
        cpd->values = arr;
     }

   for (i = sd->children_count; i < eina_array_count(arr); ++i)
     {
        Efl_Model_Children_Event cevt = { 0 };

        cevt.index = i;

        efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
     }

   if (eina_array_count(arr) > sd->children_count)
     {
        sd->children_count = eina_array_count(arr);
        efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);
     }

   eina_stringshare_del(name);
   return EINA_TRUE;

 on_error:
   eina_stringshare_del(name);

   if (cpd) free(cpd);
   if (arr) _property_values_free(arr);
   eina_error_set(err);
   return EINA_FALSE;
}

static Eina_Iterator *
_efl_container_model_efl_model_properties_get(const Eo *obj EINA_UNUSED,
                                              Efl_Container_Model_Data *sd)
{
   EFL_COMPOSITE_MODEL_PROPERTIES_SUPER(props,
                                        obj, EFL_CONTAINER_MODEL_CLASS,
                                        eina_hash_iterator_key_new(sd->parent->properties));
   return props;
}

static Eina_Future *
_efl_container_model_efl_model_property_set(Eo *obj,
                                            Efl_Container_Model_Data *pd,
                                            const char *property,
                                            Eina_Value *value)
{
   if (pd->parent)
     {
        Efl_Container_Property_Data *cpd;
        Eina_Stringshare *name;
        unsigned int index;
        Eina_Value r = EINA_VALUE_EMPTY;

        name = eina_stringshare_add(property);
        cpd = eina_hash_find(pd->parent->properties, name);
        eina_stringshare_del(name);

        // Check if this is a property of this object
        if (!cpd) goto not_mine;
        // Check if we can get the expected type for this property
        if (!cpd->values)
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

        index = efl_composite_model_index_get(obj);

        // Try converting the type before touching any data inside the Model
        if (eina_value_type_get(value) != cpd->type)
          {
             eina_value_setup(&r, cpd->type);

             if (!eina_value_convert(value, &r))
               return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
          }
        else
          {
             eina_value_copy(&r, value);
          }

        // Expand the array to have enough space for this value if it wasn't already the case
        while (index >= eina_array_count(cpd->values))
          eina_array_push(cpd->values, eina_value_error_new(EFL_MODEL_ERROR_INCORRECT_VALUE));

        // Clean the previous value and introduce the new one
        eina_value_free(eina_array_data_get(cpd->values, index));
        // Same type as the expected type, we can just replace the value
        eina_array_data_set(cpd->values, index, eina_value_dup(&r));

        eina_value_free(value);
        return efl_loop_future_resolved(obj, r);
     }

 not_mine:
   return efl_model_property_set(efl_super(obj, EFL_CONTAINER_MODEL_CLASS), property, value);
}

static Eina_Value *
_efl_container_model_efl_model_property_get(const Eo *obj,
                                            Efl_Container_Model_Data *pd,
                                            const char *property)
{
   if (pd->parent)
     {
        Efl_Container_Property_Data *cpd;
        Eina_Stringshare *name;
        Eina_Value *copy;
        unsigned int index;

        name = eina_stringshare_add(property);
        cpd = eina_hash_find(pd->parent->properties, name);
        eina_stringshare_del(name);

        // Check if this is a property of this object
        if (!cpd) goto not_mine;
        // Check if we can get the expected type for this property
        if (!cpd->values) return eina_value_error_new(EFL_MODEL_ERROR_INCORRECT_VALUE);

        index = efl_composite_model_index_get(obj);

        if (index >= eina_array_count(cpd->values))
          return eina_value_error_new(EFL_MODEL_ERROR_INCORRECT_VALUE);

        copy = eina_value_dup(eina_array_data_get(cpd->values, index));

        return copy;
     }

 not_mine:
   return efl_model_property_get(efl_super(obj, EFL_CONTAINER_MODEL_CLASS), property);
}

static unsigned int
_efl_container_model_efl_model_children_count_get(const Eo *obj EINA_UNUSED, Efl_Container_Model_Data *sd)
{
   unsigned int pcount;

   pcount = efl_model_children_count_get(efl_super(obj, EFL_CONTAINER_MODEL_CLASS));

   return pcount > sd->children_count ? pcount : sd->children_count;
}

#include "efl_container_model.eo.c"
