#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>
#include <Ecore.h>

#include "ecore_internal.h"

#define MY_CLASS EFL_MODEL_ITEM_CLASS

typedef struct _Efl_Model_Item_Data Efl_Model_Item_Data;
struct _Efl_Model_Item_Data
{
   Eina_Hash                        *properties;
   Eina_List                        *childrens;
};

static void
_item_value_free_cb(void *data)
{
   eina_value_free(data);
}

static Eina_Bool
_stringshared_keys_free(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data EINA_UNUSED, void *fdata EINA_UNUSED)
{
   eina_stringshare_del(key);
   return EINA_TRUE;
}

static Efl_Object *
_efl_model_item_efl_object_constructor(Eo *obj, Efl_Model_Item_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   if (!obj)
     return NULL;

   sd->properties = eina_hash_stringshared_new(_item_value_free_cb);

   return obj;
}

static void
_efl_model_item_efl_object_destructor(Eo *obj, Efl_Model_Item_Data *sd)
{
   eina_list_free(sd->childrens);
   eina_hash_foreach(sd->properties, _stringshared_keys_free, NULL);
   eina_hash_free(sd->properties);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Iterator *
_efl_model_item_efl_model_properties_get(const Eo *obj EINA_UNUSED, Efl_Model_Item_Data *pd)
{
   return eina_hash_iterator_key_new(pd->properties);
}

static Eina_Future *
_efl_model_item_efl_model_property_set(Eo *obj, Efl_Model_Item_Data *pd, const char *property, Eina_Value *value)
{
   Eina_Stringshare *prop;
   Eina_Value *exist;
   Efl_Model_Property_Event evt;

   prop = eina_stringshare_add(property);
   exist = eina_hash_find(pd->properties, prop);

   if (!exist)
     {
        exist = eina_value_new(eina_value_type_get(value));
        if (!exist)
          goto value_failed;

        if (!eina_hash_direct_add(pd->properties, eina_stringshare_ref(prop), exist))
          goto hash_failed;
     }

   if (!eina_value_copy(value, exist))
     goto value_failed;

   {
      char *v = eina_value_to_string(value);
      char *e = eina_value_to_string(exist);

      free(v);
      free(e);
   }

   eina_stringshare_del(prop);

   evt.changed_properties = eina_array_new(1);
   eina_array_push(evt.changed_properties, prop);

   efl_event_callback_call(obj, EFL_MODEL_EVENT_PROPERTIES_CHANGED, &evt);

   eina_array_free(evt.changed_properties);

   return eina_future_resolved(efl_loop_future_scheduler_get(obj),
                               eina_value_reference_copy(value));

 hash_failed:
   eina_value_free(exist);
 value_failed:
   eina_stringshare_del(prop);

   return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                               ENOMEM);
}

static Eina_Value *
_efl_model_item_efl_model_property_get(const Eo *obj EINA_UNUSED,
                                       Efl_Model_Item_Data *pd,
                                       const char *property)
{
   Eina_Stringshare *prop;
   Eina_Value *value;

   prop = eina_stringshare_add(property);
   value = eina_hash_find(pd->properties, prop);
   eina_stringshare_del(prop);

   if (!value)
     return eina_value_error_new(EFL_MODEL_ERROR_NOT_FOUND);

   return eina_value_dup(value);
}

static Eina_Future *
_efl_model_item_efl_model_children_slice_get(Eo *obj, Efl_Model_Item_Data *pd, unsigned int start, unsigned int count)
{
   Eina_Value v;

   v = efl_model_list_value_get(pd->childrens, start, count);
   return eina_future_resolved(efl_loop_future_scheduler_get(obj), v);
}

static unsigned int
_efl_model_item_efl_model_children_count_get(const Eo *obj EINA_UNUSED, Efl_Model_Item_Data *pd)
{
   return eina_list_count(pd->childrens);
}

static Eo *
_efl_model_item_efl_model_child_add(Eo *obj, Efl_Model_Item_Data *sd)
{
   Efl_Model_Children_Event cevt;
   Efl_Model *child;

   child = efl_add(EFL_MODEL_ITEM_CLASS, obj);
   if (!child)
     {
        EINA_LOG_ERR("Could not allocate Efl.Model.Item");
        eina_error_set(EFL_MODEL_ERROR_UNKNOWN);
        return NULL;
     }

   cevt.index = eina_list_count(sd->childrens);
   sd->childrens = eina_list_append(sd->childrens, child);

   efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
   efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);

   return child;
}

static void
_efl_model_item_efl_model_child_del(Eo *obj, Efl_Model_Item_Data *sd, Eo *child)
{
   Efl_Model *data;
   Eina_List *l;
   unsigned int i = 0;

   EINA_LIST_FOREACH(sd->childrens, l, data)
     {
        if (data == child)
          {
             Efl_Model_Children_Event cevt;

             sd->childrens = eina_list_remove_list(sd->childrens, l);

             cevt.index = i;

             efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_REMOVED, &cevt);

             efl_parent_set(child, NULL);

             efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);

             break;
          }
        ++i;
     }
}

#include "efl_model_item.eo.c"
