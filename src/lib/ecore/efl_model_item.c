#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>
#include <Ecore.h>

#define MY_CLASS EFL_MODEL_ITEM_CLASS

typedef struct _Efl_Model_Item_Data Efl_Model_Item_Data;
struct _Efl_Model_Item_Data
{
   Eina_Hash                        *properties;
   Eina_Array                       *defined_properties;
   Eina_List                        *children;
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
   sd->defined_properties = eina_array_new(8);

   return obj;
}

static void
_efl_model_item_efl_object_destructor(Eo *obj, Efl_Model_Item_Data *sd)
{
   Efl_Model *child;

   EINA_LIST_FREE(sd->children, child)
     {
        if (child)
          efl_parent_set(child, NULL);
     }

   eina_hash_foreach(sd->properties, _stringshared_keys_free, NULL);
   eina_hash_free(sd->properties);

   eina_array_free(sd->defined_properties);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static const Eina_Array *
_efl_model_item_efl_model_properties_get(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd)
{
   return sd->defined_properties;
}


static Efl_Future*
_efl_model_item_efl_model_property_set(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd, const char *property, const Eina_Value *value)
{
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future* future = efl_promise_future_get(promise);
   Efl_Model_Property_Event evt;

   Eina_Stringshare *sshared = eina_stringshare_add(property);
   Eina_Value *p_v = eina_hash_find(sd->properties, sshared);
   if (p_v)
     {
        eina_stringshare_del(sshared);
        if (!eina_value_copy(value, p_v))
          goto err1;
     }
   else
     {
        if (!eina_array_push(sd->defined_properties, sshared))
          goto err2;

        p_v = eina_value_new(eina_value_type_get(value));
        if (!p_v)
          goto err3;

        if (!eina_value_copy(value, p_v) ||
            !eina_hash_direct_add(sd->properties, sshared, p_v))
          goto err4;
     }

   efl_promise_value_set(promise, p_v, NULL);

   evt.changed_properties = eina_array_new(20);
   eina_array_push(evt.changed_properties, property);
   efl_event_callback_call(obj, EFL_MODEL_EVENT_PROPERTIES_CHANGED, &evt);
   eina_array_free(evt.changed_properties);

   return future;

err4:
   eina_value_free(p_v);
err3:
   eina_array_pop(sd->defined_properties);
err2:
   eina_stringshare_del(sshared);
err1:
   efl_promise_failed_set(promise, EFL_MODEL_ERROR_UNKNOWN);
   return future;
}

static Efl_Future *
_efl_model_item_efl_model_property_get(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd, const char *property)
{
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future *rfuture = efl_promise_future_get(promise);

   Eina_Stringshare *sshare = eina_stringshare_add(property);
   Eina_Value *p_v = eina_hash_find(sd->properties, sshare);
   eina_stringshare_del(sshare);
   if (p_v)
     efl_promise_value_set(promise, p_v, NULL);
   else
     efl_promise_failed_set(promise, EFL_MODEL_ERROR_NOT_FOUND);

   return rfuture;
}

static Efl_Future *
_efl_model_item_efl_model_children_slice_get(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd, unsigned int start, unsigned int count)
{
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future *rfuture = efl_promise_future_get(promise);

   Eina_Accessor* accessor = efl_model_list_slice(sd->children, start, count);
   efl_promise_value_set(promise, accessor, (Eina_Free_Cb)&eina_accessor_free);

   return rfuture;
}

static Efl_Future *
_efl_model_item_efl_model_children_count_get(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd)
{
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future *rfuture = efl_promise_future_get(promise);

   unsigned int *count = calloc(1, sizeof(unsigned int));
   *count = eina_list_count(sd->children);
   efl_promise_value_set(promise, count, &free);

   return rfuture;
}

static Eo *
_efl_model_item_efl_model_child_add(Eo *obj, Efl_Model_Item_Data *sd)
{
   Efl_Model_Children_Event cevt;
   Efl_Model *child = efl_add(EFL_MODEL_ITEM_CLASS, obj);
   if (!child)
     {
        EINA_LOG_ERR("Could not allocate Efl.Model.Item");
        eina_error_set(EFL_MODEL_ERROR_UNKNOWN);
        return NULL;
     }
   cevt.index = eina_list_count(sd->children);
   sd->children = eina_list_append(sd->children, child);
   cevt.child = child;
   efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
   efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, &cevt.index);
   return child;
}

static void
_efl_model_item_efl_model_child_del(Eo *obj, Efl_Model_Item_Data *sd, Eo *child)
{
   Eina_List *l;
   Efl_Model *data;
   unsigned int i = 0;
   EINA_LIST_FOREACH(sd->children, l, data)
     {
        if (data == child)
          {
             Efl_Model_Children_Event cevt;
             sd->children = eina_list_remove_list(sd->children, l);

             cevt.child = efl_ref(child);
             cevt.index = i;

             efl_parent_set(child, NULL);

             efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_REMOVED, &cevt);
             efl_unref(child);

             i = eina_list_count(sd->children);
             efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, &i);
             break;
          }
        ++i;
     }
}

#include "efl_model_item.eo.c"
