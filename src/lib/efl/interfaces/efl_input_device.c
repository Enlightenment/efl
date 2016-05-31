#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

#define EFL_INTERNAL_UNSTABLE
#include "efl_common_internal.h"

/* Efl Input Device = Evas Device */

EOLIAN static Eo_Base *
_efl_input_device_eo_base_constructor(Eo *obj, Efl_Input_Device_Data *pd)
{
   obj = eo_constructor(eo_super(obj, EFL_INPUT_DEVICE_CLASS));
   pd->eo = obj;
   return obj;
}

EOLIAN static void
_efl_input_device_eo_base_destructor(Eo *obj, Efl_Input_Device_Data *pd)
{
   Eo *eo_child;

   eina_stringshare_del(pd->name);
   eina_stringshare_del(pd->desc);
   EINA_LIST_FREE(pd->children, eo_child)
     {
        Efl_Input_Device_Data *child = eo_data_scope_get(eo_child, EFL_INPUT_DEVICE_CLASS);
        child->parent = NULL;
        eo_unref(eo_child);
     }
   eo_unref(pd->source);

   return eo_destructor(eo_super(obj, EFL_INPUT_DEVICE_CLASS));
}

EOLIAN static void
_efl_input_device_device_type_set(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd, Efl_Input_Device_Class klass)
{
   pd->klass= klass;
}

EOLIAN static Efl_Input_Device_Class
_efl_input_device_device_type_get(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd)
{
   return pd->klass;
}

EOLIAN static void
_efl_input_device_device_subtype_set(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd, Efl_Input_Device_Sub_Class klass)
{
   pd->subclass = klass;
}

EOLIAN static Efl_Input_Device_Sub_Class
_efl_input_device_device_subtype_get(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd)
{
   return pd->subclass;
}

EOLIAN static void
_efl_input_device_source_set(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd, Efl_Input_Device *src)
{
   if (pd->source == src) return;
   eo_unref(pd->source);
   pd->source = eo_ref(src);
}

EOLIAN static Efl_Input_Device *
_efl_input_device_source_get(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd)
{
   return pd->source;
}

EOLIAN static void
_efl_input_device_name_set(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd, const char *name)
{
   eina_stringshare_replace(&pd->name, name);
}

EOLIAN static const char *
_efl_input_device_name_get(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd)
{
   return pd->name;
}

EOLIAN static void
_efl_input_device_description_set(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd, const char *desc)
{
   eina_stringshare_replace(&pd->desc, desc);
}

EOLIAN static const char *
_efl_input_device_description_get(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd)
{
   return pd->desc;
}

EOLIAN static Efl_Input_Device *
_efl_input_device_parent_get(Eo *obj EINA_UNUSED, Efl_Input_Device_Data *pd)
{
   return pd->parent;
}

#include "interfaces/efl_input_device.eo.c"
