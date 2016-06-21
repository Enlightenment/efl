#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

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

EOLIAN static Eo *
_efl_model_item_eo_base_constructor(Eo *obj, Efl_Model_Item_Data *sd)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
   if (!obj)
     return NULL;

   sd->properties = eina_hash_stringshared_new(_item_value_free_cb);
   sd->defined_properties = eina_array_new(8);

   return obj;
}

EOLIAN static void
_efl_model_item_eo_base_destructor(Eo *obj, Efl_Model_Item_Data *sd)
{
   Efl_Model *child;

   EINA_LIST_FREE(sd->children, child)
     {
        if (child)
          eo_parent_set(child, NULL);
     }

   eina_hash_foreach(sd->properties, _stringshared_keys_free, NULL);
   eina_hash_free(sd->properties);

   eina_array_free(sd->defined_properties);

   eo_destructor(eo_super(obj, MY_CLASS));
}

EOLIAN static const Eina_Array *
_efl_model_item_efl_model_properties_get(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd)
{
   return sd->defined_properties;
}


EOLIAN static void
_efl_model_item_efl_model_property_set(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd, const char *property, const Eina_Value *value, Eina_Promise_Owner *promise_owner)
{
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

   eina_promise_owner_value_set(promise_owner, p_v, NULL);
   return;

err4:
   eina_value_free(p_v);
err3:
   eina_array_pop(sd->defined_properties);
err2:
   eina_stringshare_del(sshared);
err1:
   eina_promise_owner_error_set(promise_owner, EFL_MODEL_ERROR_UNKNOWN);
}

EOLIAN static Eina_Promise *
_efl_model_item_efl_model_property_get(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd, const char *property)
{
   Eina_Promise_Owner *promise_owner = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise_owner);

   Eina_Stringshare *sshare = eina_stringshare_add(property);
   Eina_Value *p_v = eina_hash_find(sd->properties, sshare);
   eina_stringshare_del(sshare);
   if (p_v)
     eina_promise_owner_value_set(promise_owner, p_v, NULL);
   else
     eina_promise_owner_error_set(promise_owner, EFL_MODEL_ERROR_NOT_FOUND);

   return rpromise;
}

EOLIAN static Eina_Promise *
_efl_model_item_efl_model_children_slice_get(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd, unsigned int start, unsigned int count)
{
   Eina_Promise_Owner *promise_owner = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise_owner);

   Eina_Accessor* accessor = efl_model_list_slice(sd->children, start, count);
   eina_promise_owner_value_set(promise_owner, accessor, (Eina_Promise_Free_Cb)&eina_accessor_free);

   return rpromise;
}

EOLIAN static Eina_Promise *
_efl_model_item_efl_model_children_count_get(Eo *obj EINA_UNUSED, Efl_Model_Item_Data *sd)
{
   Eina_Promise_Owner *promise_owner = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise_owner);

   unsigned int *count = calloc(1, sizeof(unsigned int));
   *count = eina_list_count(sd->children);
   eina_promise_owner_value_set(promise_owner, count, &free);

   return rpromise;
}

EOLIAN static Eo *
_efl_model_item_efl_model_child_add(Eo *obj, Efl_Model_Item_Data *sd)
{
   Efl_Model_Children_Event cevt;
   Efl_Model *child = eo_add(EFL_MODEL_ITEM_CLASS, obj);
   if (!child)
     {
        EINA_LOG_ERR("Could not allocate Efl.Model.Item");
        eina_error_set(EFL_MODEL_ERROR_UNKNOWN);
        return NULL;
     }
   sd->children = eina_list_append(sd->children, child);
   cevt.child = child;
   cevt.index = eina_list_count(sd->children);
   eo_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
   eo_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, &cevt.index);
   return child;
}

EOLIAN static void
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

             cevt.child = eo_ref(child);
             cevt.index = i;

             eo_parent_set(child, NULL);

             eo_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_REMOVED, &cevt);
             eo_unref(child);

             i = eina_list_count(sd->children);
             eo_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, &i);
             break;
          }
        ++i;
     }
}

#include "interfaces/efl_model_item.eo.c"
