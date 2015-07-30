#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Efl.h"

EAPI void
efl_model_load_set(Efl_Model_Base *model, Efl_Model_Load *load, Efl_Model_Load_Status status)
{
   Efl_Model_Load new_load = {.status = status};

   if ((load->status & (EFL_MODEL_LOAD_STATUS_LOADED | EFL_MODEL_LOAD_STATUS_LOADING)) &&
       (new_load.status & (EFL_MODEL_LOAD_STATUS_LOADED | EFL_MODEL_LOAD_STATUS_LOADING)))
     {
        // Merge status
        new_load.status = load->status | new_load.status;

        // Removes incompatible statuses (LOADING vs LOADED)
        switch (status)
          {
           case EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES:
             new_load.status &= ~EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES;
             break;
           case EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES:
             new_load.status &= ~EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES;
             break;
           case EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN:
             new_load.status &= ~EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN;
             break;
           case EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN:
             new_load.status &= ~EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN;
             break;
           case EFL_MODEL_LOAD_STATUS_LOADED:
             new_load.status &= ~EFL_MODEL_LOAD_STATUS_LOADING;
             break;
           case EFL_MODEL_LOAD_STATUS_LOADING:
             new_load.status &= ~EFL_MODEL_LOAD_STATUS_LOADED;
             break;
           default: break;
          }
     }

   if (load->status != new_load.status)
     {
        load->status = new_load.status;
        eo_do(model, eo_event_callback_call(EFL_MODEL_BASE_EVENT_LOAD_STATUS, load));
     }
}

EAPI Eina_Accessor *
efl_model_list_slice(Eina_List *list, unsigned start, unsigned count)
{
  fprintf(stderr, "efl_model_list_slice\n");
   if ((start == 0) && (count == 0))
     {
       fprintf(stderr, "efl_model_list_slice start == 0 count == 0\n");
       return eina_list_accessor_new(list);
     }

   Eina_List *nth_list = eina_list_nth_list(list, (start - 1));
   if (!nth_list)
     return NULL;

   Eina_List *it, *result = NULL;
   const void *data;
   EINA_LIST_FOREACH(nth_list, it, data)
     {
        result = eina_list_append(result, data);
        if (eina_list_count(result) == count)
          break;
     }

   return eina_list_accessor_new(result);
}

EAPI void
efl_model_error_notify(Efl_Model_Base *model)
{
   Efl_Model_Load load = {.status = EFL_MODEL_LOAD_STATUS_ERROR};
   eo_do(model, eo_event_callback_call(EFL_MODEL_BASE_EVENT_LOAD_STATUS, &load));
}

EAPI void
efl_model_property_changed_notify(Efl_Model_Base *model, const char *property)
{
   Eina_Array *changed_properties = eina_array_new(1);
   EINA_SAFETY_ON_NULL_RETURN(changed_properties);

   Eina_Bool ret = eina_array_push(changed_properties, property);
   EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

   Efl_Model_Property_Event evt = {.changed_properties = changed_properties};
   eo_do(model, eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));

on_error:
   eina_array_free(changed_properties);
}

EAPI void
efl_model_property_invalidated_notify(Efl_Model_Base *model, const char *property)
{
   Eina_Array *invalidated_properties = eina_array_new(1);
   EINA_SAFETY_ON_NULL_RETURN(invalidated_properties);

   Eina_Bool ret = eina_array_push(invalidated_properties, property);
   EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

   Efl_Model_Property_Event evt = {.invalidated_properties = invalidated_properties};
   eo_do(model, eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));

on_error:
   eina_array_free(invalidated_properties);
}

typedef struct _Efl_Model_Value_Struct_Desc Efl_Model_Value_Struct_Desc;

struct _Efl_Model_Value_Struct_Desc
{
   Eina_Value_Struct_Desc base;
   void *data;
   Eina_Value_Struct_Member members[];
};

EAPI Eina_Value_Struct_Desc *
efl_model_value_struct_description_new(unsigned int member_count, Efl_Model_Value_Struct_Member_Setup_Cb setup_cb, void *data)
{
   Efl_Model_Value_Struct_Desc *desc;
   unsigned int offset = 0;
   size_t i;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(member_count > 0, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(setup_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

   desc = malloc(sizeof(Efl_Model_Value_Struct_Desc) + member_count * sizeof(Eina_Value_Struct_Member));
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);

   desc->base.version = EINA_VALUE_STRUCT_DESC_VERSION;
   desc->base.ops = EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE;
   desc->base.members = desc->members;
   desc->base.member_count = member_count;
   desc->base.size = 0;
   desc->data = data;

   for (i = 0; i < member_count; ++i)
     {
        Eina_Value_Struct_Member *m = (Eina_Value_Struct_Member *)desc->members + i;
        unsigned int size;

        m->offset = offset;
        setup_cb(data, i, m);

        size = m->type->value_size;
        if (size % sizeof(void *) != 0)
          size += size - (size % sizeof(void *));

        offset += size;
     }

   desc->base.size = offset;
   return &desc->base;
}

EAPI void
efl_model_value_struct_description_free(Eina_Value_Struct_Desc *desc)
{
   size_t i;

   if (!desc) return;

   for (i = 0; i < desc->member_count; i++)
     eina_stringshare_del(desc->members[i].name);
   free(desc);
}
