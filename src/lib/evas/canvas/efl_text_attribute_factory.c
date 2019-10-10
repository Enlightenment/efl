#include "evas_common_private.h"
#include "evas_private.h"

#include "efl_canvas_text_internal.h"

#define MY_CLASS EFL2_TEXT_ATTRIBUTE_FACTORY_CLASS

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
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   _efl2_text_attribute_factory_reset(obj, pd);
   return obj;
}

void
_efl2_text_attribute_factory_efl_object_destructor(Eo *obj, Efl2_Text_Attribute_Factory_Data *pd)
{
   efl_destructor(efl_super(obj, MY_CLASS));
   _efl2_text_attribute_factory_reset(obj, pd);
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
   _textblock_annotation_insert(efl2_text_cursor_handle_get(cur1), efl2_text_cursor_handle_get(cur2), handle);
   return handle;
}

EOLIAN static Eina_Iterator *
_efl2_text_attribute_factory_range_attributes_get(const Efl2_Text_Cursor *start, const Efl2_Text_Cursor *end)
{
   return _canvas_text_range_annotations_get(efl2_text_cursor_handle_get(start), efl2_text_cursor_handle_get(end));
}

EOLIAN static void
_efl2_text_attribute_factory_attribute_cursors_get(const Efl2_Text_Attribute_Handle *an, Efl2_Text_Cursor *start, Efl2_Text_Cursor *end)
{
   _evas_textblock_cursor_at_format_set(efl2_text_cursor_handle_get(start), an->start_node);
   _evas_textblock_cursor_at_format_set(efl2_text_cursor_handle_get(end), an->end_node);
}

EOLIAN static Efl2_Text_Attribute_Handle *
_efl2_text_attribute_factory_ref(Efl2_Text_Attribute_Handle *handle)
{
   handle->ref++;
   return handle;
}

EOLIAN static void
_efl2_text_attribute_factory_unref(Efl2_Text_Attribute_Handle *handle)
{
   if (handle->ref == 0)
     {
        ERR("Tried unreffing an attribute with 0 refs");
        return;
     }

   handle->ref--;
   _canvas_text_annotation_free_if_not_reffed(handle);
}

EOLIAN static void
_efl2_text_attribute_factory_remove(Efl2_Text_Attribute_Handle *handle)
{
   _canvas_text_annotation_remove(handle->obj, handle);
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
