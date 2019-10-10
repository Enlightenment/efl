#include "evas_common_private.h"
#include "evas_private.h"

#include "efl_canvas_text_internal.h"

#define MY_CLASS EFL2_TEXT_ATTRIBUTE_FACTORY

struct _Efl2_Text_Attribute_Factory_Data
{
   Efl2_Text_Attribute_Handle handle;
};

typedef struct _Efl2_Text_Attribute_Factory_Data Efl2_Text_Attribute_Factory_Data;


static void
_handle_copy(Efl2_Text_Attribute_Handle *dst, const Efl2_Text_Attribute_Handle *src)
{
   eina_stringshare_replace(&dst->tag, src->tag);
   eina_stringshare_replace(&dst->properties, src->properties);
   dst->ephemeral = src->ephemeral;
}

EOLIAN static void
_efl2_text_attribute_factory_reset(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   eina_stringshare_replace(&pd->handle.tag, NULL);
   eina_stringshare_replace(&pd->handle.properties, NULL);
   pd->handle.ephemeral = EINA_TRUE;
}

Efl_Object *
_efl2_text_attribute_factory_efl_object_constructor(Eo *obj, Efl2_Text_Attribute_Factory_Data *pd)
{
   _efl2_text_attribute_factory_reset(obj, pd);
   return obj;
}

EOLIAN static void
_efl2_text_attribute_factory_load(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, const Efl2_Text_Attribute_Handle *handle)
{
   _handle_copy(&pd->handle, handle);
}

EOLIAN static void
_efl2_text_attribute_factory_update(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Attribute_Handle *handle)
{
   _handle_copy(handle, &pd->handle);
}

EOLIAN static Efl2_Text_Attribute_Handle *
_efl2_text_attribute_factory_create(Eo *obj, Efl2_Text_Attribute_Factory_Data *pd)
{
   Efl2_Text_Attribute_Handle *handle = calloc(1, sizeof(*handle));
   _efl2_text_attribute_factory_update(obj, pd, handle);
   return handle;
}

EOLIAN static Efl2_Text_Attribute_Handle *
_efl2_text_attribute_factory_insert(Eo *obj, Efl2_Text_Attribute_Factory_Data *pd, const Efl2_Text_Cursor *cur1, const Efl2_Text_Cursor *cur2)
{
   Efl2_Text_Attribute_Handle *handle = _efl2_text_attribute_factory_create(obj, pd);
   // FIME: insert;
   return handle;
}

EOLIAN static Eina_Iterator *
_efl2_text_attribute_factory_range_attributes_get(const Efl2_Text_Cursor *start, const Efl2_Text_Cursor *end)
{
   // FIXME: implement
   return NULL;
}

EOLIAN static void
_efl2_text_attribute_factory_attribute_cursors_get(Efl2_Text_Cursor *start, Efl2_Text_Cursor *end)
{
   // FIXME: implement
}

EOLIAN static Efl2_Text_Attribute_Handle *
_efl2_text_attribute_factory_ref(Efl2_Text_Attribute_Handle *handle)
{
   // FIXME: implement
   return handle;
}

EOLIAN static void
_efl2_text_attribute_factory_unref(Efl2_Text_Attribute_Handle *handle)
{
   // FIXME: implement
   (void) handle;
}

EOLIAN static void
_efl2_text_attribute_factory_remove(Efl2_Text_Attribute_Handle *handle)
{
   // FIXME: implement
   (void) handle;
}

EOLIAN static void
_efl2_text_attribute_factory_tag_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, const char *name)
{
   eina_stringshare_replace(&pd->handle.tag, name);
}

EOLIAN static const char *
_efl2_text_attribute_factory_tag_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->handle.tag;
}

EOLIAN static void
_efl2_text_attribute_factory_properties_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, const char *properties)
{
   eina_stringshare_replace(&pd->handle.properties, properties);
}

EOLIAN static const char *
_efl2_text_attribute_factory_properties_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->handle.properties;
}

EOLIAN static void
_efl2_text_attribute_factory_ephemeral_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Eina_Bool ephemeral)
{
   pd->handle.ephemeral = ephemeral;
}

EOLIAN static Eina_Bool
_efl2_text_attribute_factory_ephemeral_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->handle.ephemeral;
}

#include "canvas/efl2_text_attribute_factory.eo.c"
