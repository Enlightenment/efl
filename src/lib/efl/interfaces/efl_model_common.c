#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Efl.h"
#include "Efl_Model_Common.h"

EAPI Eina_Error EFL_MODEL_ERROR_UNKNOWN = 0;
EAPI Eina_Error EFL_MODEL_ERROR_NOT_SUPPORTED = 0;
EAPI Eina_Error EFL_MODEL_ERROR_NOT_FOUND = 0;
EAPI Eina_Error EFL_MODEL_ERROR_READ_ONLY = 0;
EAPI Eina_Error EFL_MODEL_ERROR_INIT_FAILED = 0;
EAPI Eina_Error EFL_MODEL_ERROR_PERMISSION_DENIED = 0;
EAPI Eina_Error EFL_MODEL_ERROR_INCORRECT_VALUE = 0;
EAPI Eina_Error EFL_MODEL_ERROR_INVALID_OBJECT = 0;

static const char EFL_MODEL_ERROR_UNKNOWN_STR[]           = "Unknown Error";
static const char EFL_MODEL_ERROR_NOT_SUPPORTED_STR[]     = "Operation not supported";
static const char EFL_MODEL_ERROR_NOT_FOUND_STR[]         = "Value not found";
static const char EFL_MODEL_ERROR_READ_ONLY_STR[]         = "Value read only";
static const char EFL_MODEL_ERROR_INIT_FAILED_STR[]       = "Init failed";
static const char EFL_MODEL_ERROR_PERMISSION_DENIED_STR[] = "Permission denied";
static const char EFL_MODEL_ERROR_INCORRECT_VALUE_STR[]   = "Incorrect value";
static const char EFL_MODEL_ERROR_INVALID_OBJECT_STR[]    = "Object is invalid";

#define _ERROR(Name) EFL_MODEL_ERROR_##Name = eina_error_msg_static_register(EFL_MODEL_ERROR_##Name##_STR);

EAPI int
efl_model_init(void)
{
   _ERROR(INCORRECT_VALUE);
   _ERROR(UNKNOWN);
   _ERROR(NOT_SUPPORTED);
   _ERROR(NOT_FOUND);
   _ERROR(READ_ONLY);
   _ERROR(INIT_FAILED);
   _ERROR(PERMISSION_DENIED);
   _ERROR(INVALID_OBJECT);

   return EINA_TRUE;
}

#undef _ERROR

EAPI void
_efl_model_properties_changed_internal(const Efl_Model *model, ...)
{
   Efl_Model_Property_Event ev = { 0 };
   Eina_Array *properties = eina_array_new(1);
   const char *property;
   va_list args;

   va_start(args, model);

   while ((property = (const char*) va_arg(args, const char*)))
     {
        eina_array_push(properties, property);
     }

   va_end(args);

   ev.changed_properties = properties;

   efl_event_callback_call((Efl_Model *) model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, &ev);

   eina_array_free(properties);
}

EAPI void
efl_model_property_invalidated_notify(Efl_Model *model, const char *property)
{
   Eina_Array *invalidated_properties = eina_array_new(1);
   EINA_SAFETY_ON_NULL_RETURN(invalidated_properties);

   Eina_Bool ret = eina_array_push(invalidated_properties, property);
   EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

   Efl_Model_Property_Event evt = {.invalidated_properties = invalidated_properties};
   efl_event_callback_call(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, &evt);

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
