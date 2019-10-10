#include "evas_common_private.h"
#include "evas_private.h"

#include "efl_canvas_text_internal.h"

#define MY_CLASS EFL2_CANVAS_TEXT_STYLE_CLASS

typedef struct _Efl2_Canvas_Text_Style_Handle Efl2_Canvas_Text_Style_Data;

EOLIAN static void
_efl2_canvas_text_style_properties_set(Eo *obj EINA_UNUSED, Efl2_Canvas_Text_Style_Data *pd, const char *properties)
{
   eina_stringshare_replace(&pd->properties, properties);
}

EOLIAN static const char *
_efl2_canvas_text_style_properties_get(const Eo *obj EINA_UNUSED, Efl2_Canvas_Text_Style_Data *pd)
{
   return pd->properties;
}

static Efl_Canvas_Text_Style_Tag *
_free_tag(Efl2_Canvas_Text_Style_Data *pd, Efl_Canvas_Text_Style_Tag *tag)
{
   eina_stringshare_del(tag->tag.parent_tag);
   eina_stringshare_del(tag->tag.tag);
   eina_stringshare_del(tag->tag.replace);
   Efl_Canvas_Text_Style_Tag *ret = (void *) eina_inlist_remove(EINA_INLIST_GET(pd->tags), EINA_INLIST_GET(tag));
   free(tag);

   return ret;
}

EOLIAN static void
_efl2_canvas_text_style_tag_set(Eo *obj EINA_UNUSED, Efl2_Canvas_Text_Style_Data *pd, const char *tag_name, const char *parent_tag, const char *properties)
{
   Efl_Canvas_Text_Style_Tag *tag;
   EINA_INLIST_FOREACH(pd->tags, tag)
     {
        if (!strcmp(tag->tag.tag, tag_name))
          {
             break;
          }
     }

   if (tag)
     {
        pd->tags = _free_tag(pd, tag);
     }

   // The tag should exist, add back
   if (parent_tag || properties)
     {
        tag = calloc(1, sizeof(*tag));
        tag->tag.parent_tag = eina_stringshare_add(parent_tag);
        tag->tag.tag = eina_stringshare_add(tag_name);
        tag->tag.replace = eina_stringshare_add(properties);
        pd->tags = (Efl_Canvas_Text_Style_Tag *) eina_inlist_append(EINA_INLIST_GET(pd->tags), EINA_INLIST_GET(tag));
     }
}

EOLIAN static void
_efl2_canvas_text_style_tag_get(const Eo *obj EINA_UNUSED, Efl2_Canvas_Text_Style_Data *pd, const char *tag_name, const char **parent_tag, const char **properties)
{
   Efl_Canvas_Text_Style_Tag *tag;
   const char *ret_parent = NULL;
   const char *ret_properties = NULL;

   EINA_INLIST_FOREACH(pd->tags, tag)
     {
        if (!strcmp(tag->tag.tag, tag_name))
          {
             ret_parent = tag->tag.parent_tag;
             ret_properties = tag->tag.replace;
             break;
          }
     }

   if (parent_tag) *parent_tag = ret_parent;
   if (properties) *properties = ret_properties;
}

EOLIAN static Eo *
_efl2_canvas_text_style_efl_object_constructor(Eo *obj EINA_UNUSED, Efl2_Canvas_Text_Style_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->eo_obj = obj;
   return obj;
}

EOLIAN static void
_efl2_canvas_text_style_efl_object_destructor(Eo *obj EINA_UNUSED, Efl2_Canvas_Text_Style_Data *pd)
{
   Efl_Canvas_Text_Style_Tag *tag;

   efl_destructor(efl_super(obj, MY_CLASS));

   EINA_INLIST_FREE(pd->tags, tag)
     {
        pd->tags = _free_tag(pd, tag);
     }
   pd->tags = NULL;
   eina_stringshare_del(pd->properties);
   eina_list_free(pd->objects);
}

void
_canvas_text_style_object_add(Eo *obj, Efl2_Canvas_Text *canvas_text)
{
   Efl2_Canvas_Text_Style_Data *pd = efl_data_scope_get(obj, MY_CLASS);
   pd->objects = eina_list_append(pd->objects, canvas_text);
   efl_ref(obj);
}

void
_canvas_text_style_object_remove(Eo *obj, Efl2_Canvas_Text *canvas_text)
{
   Efl2_Canvas_Text_Style_Data *pd = efl_data_scope_get(obj, MY_CLASS);
   pd->objects = eina_list_remove(pd->objects, canvas_text);
   efl_unref(obj);
}

#include "canvas/efl2_canvas_text_style.eo.c"
