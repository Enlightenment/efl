#ifndef _ELM_PART_HELPER_H
#define _ELM_PART_HELPER_H

#include "Elementary.h"
#include "elm_priv.h"
#include "efl_ui_layout_part_legacy.eo.h"

//#define ELM_PART_HOOK do { ERR("%s@%p:%s [%d]", efl_class_name_get(pd->obj), pd->obj, pd->part, (int) pd->temp); } while(0)
#define ELM_PART_HOOK do {} while(0)

typedef struct _Elm_Part_Data Elm_Part_Data;
struct _Elm_Part_Data
{
   Eo             *obj;
   Eina_Tmpstr    *part;
};

// Note: this generic implementation can be improved to support part object
// caching or something...

// FIXME: Some widgets return the alias name, some return the real name
// alias doesn't work for frame, _elm_layout_part_aliasing_eval() fails for scroller
#define ELM_PART_CONTENT_DEFAULT_GET(type, part) \
   static const char * _ ## type ## _default_content_part_get(const Eo *obj EINA_UNUSED, void *sd EINA_UNUSED) { return part; }

#define ELM_PART_CONTENT_DEFAULT_OPS(type) \
   EFL_OBJECT_OP_FUNC(efl_ui_widget_default_content_part_get, _ ## type ## _default_content_part_get)

#define ELM_PART_TEXT_DEFAULT_GET(type, part) \
   static const char * _ ## type ## _default_text_part_get(const Eo *obj EINA_UNUSED, void *sd EINA_UNUSED) { return part; }

#define ELM_PART_TEXT_DEFAULT_OPS(type) \
   EFL_OBJECT_OP_FUNC(efl_ui_widget_default_text_part_get, _ ## type ## _default_text_part_get)

#define ELM_PART_CONTENT_DEFAULT_IMPLEMENT(type, typedata) \
   EOLIAN static Eina_Bool \
   _ ## type ## _efl_content_content_set(Eo *obj, typedata *sd EINA_UNUSED, Evas_Object *content) \
   { \
      return efl_content_set(efl_part(obj, efl_ui_widget_default_content_part_get(obj)), content); \
   } \
   \
   EOLIAN static Evas_Object* \
   _ ## type ## _efl_content_content_get(const Eo *obj, typedata *sd EINA_UNUSED) \
   { \
      return efl_content_get(efl_part(obj, efl_ui_widget_default_content_part_get(obj))); \
   } \
   \
   EOLIAN static Evas_Object* \
   _ ## type ## _efl_content_content_unset(Eo *obj, typedata *sd EINA_UNUSED) \
   { \
      return efl_content_unset(efl_part(obj, efl_ui_widget_default_content_part_get(obj))); \
   }

static inline Eo *
_elm_part_initialize(Eo *proxy, Eo *obj, const char *part)
{
   Elm_Part_Data *pd = efl_data_scope_get(proxy, EFL_UI_WIDGET_PART_CLASS);

   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd && obj && part, NULL);
   pd->part = eina_tmpstr_add(part);
   pd->obj = obj;

   return proxy;
}

static inline Eo *
ELM_PART_IMPLEMENT(const Efl_Class *part_klass, const Eo *obj, const char *part)
{
   return efl_add(part_klass, (Eo *) obj,
                  _elm_part_initialize(efl_added, (Eo *) obj, part));
}

#define ELM_PART_OVERRIDE_PARTIAL(type, TYPE, typedata, _is_part_cb) \
   EOLIAN static Efl_Object * \
   _ ## type ## _efl_part_part_get(const Eo *obj, typedata *priv EINA_UNUSED, const char *part) \
   { \
     EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL); \
     if (_is_part_cb(obj, part)) \
       return ELM_PART_IMPLEMENT(TYPE ## _PART_CLASS, obj, part); \
     return efl_part_get(efl_super(obj, TYPE ## _CLASS), part); \
   }

#define ELM_PART_OVERRIDE(type, TYPE, typedata) \
EOLIAN static Efl_Object * \
_ ## type ## _efl_part_part_get(const Eo *obj, typedata *priv EINA_UNUSED, const char *part) \
{ \
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL); \
   return ELM_PART_IMPLEMENT(TYPE ## _PART_CLASS, obj, part); \
}

#define ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata) \
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS); \
   typedata *sd = efl_data_scope_get(pd->obj, TYPE ## _CLASS);

#define ELM_PART_OVERRIDE_INTERNALS_FETCH_NO_SD() \
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS); \
   void *sd = NULL;

#define ELM_PART_OVERRIDE_CONTENT_SET_FULL(full, type, internals) \
EOLIAN static Eina_Bool \
_ ## full ## _efl_content_content_set(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx_Entity *content) \
{ \
   internals \
   return _ ## type ## _content_set(pd->obj, sd, pd->part, content); \
}

#define ELM_PART_OVERRIDE_CONTENT_GET_FULL(full, type, internals) \
EOLIAN static Efl_Gfx_Entity * \
_ ## full ## _efl_content_content_get(const Eo *obj, void *_pd EINA_UNUSED) \
{ \
   internals \
   return _ ## type ## _content_get(pd->obj, sd, pd->part); \
}

#define ELM_PART_OVERRIDE_CONTENT_UNSET_FULL(full, type, internals) \
EOLIAN static Efl_Gfx_Entity * \
_ ## full ## _efl_content_content_unset(Eo *obj, void *_pd EINA_UNUSED) \
{ \
   internals \
   return _ ## type ## _content_unset(pd->obj, sd, pd->part); \
}

#define ELM_PART_OVERRIDE_TEXT_TEXT_SET_FULL(full, type, internals) \
EOLIAN static void \
_ ## full ## _efl_text_text_set(Eo *obj, void *_pd EINA_UNUSED, const char *text) \
{ \
   internals \
   _ ## type ## _text_set(pd->obj, sd, pd->part, text); \
}

#define ELM_PART_OVERRIDE_TEXT_TEXT_GET_FULL(full, type, internals) \
EOLIAN static const char *\
_ ## full ## _efl_text_text_get(const Eo *obj, void *_pd EINA_UNUSED) \
{ \
   internals \
   return _ ## type ## _text_get(pd->obj, sd, pd->part); \
}

#define ELM_PART_OVERRIDE_TEXT_MARKUP_GET_FULL(full, type, internals) \
EOLIAN static const char *\
_ ## full ## _efl_text_markup_markup_get(const Eo *obj, void *_pd EINA_UNUSED) \
{ \
   internals \
   return _ ## type ## _text_markup_get(pd->obj, sd, pd->part); \
}

#define ELM_PART_OVERRIDE_TEXT_MARKUP_SET_FULL(full, type, internals) \
EOLIAN static void \
_ ## full ## _efl_text_markup_markup_set(Eo *obj, void *_pd EINA_UNUSED, const char *markup) \
{ \
   internals \
   _ ## type ## _text_markup_set(pd->obj, sd, pd->part, markup); \
}

#define ELM_PART_OVERRIDE_TEXT_FONT_SET_FULL(full, type, internals) \
EOLIAN static void \
_ ## full ## _efl_text_font_font_set(Eo *obj, void *_pd EINA_UNUSED, const char *font, Efl_Font_Size size) \
{ \
   internals \
   _ ## type ## _text_font_set(pd->obj, sd, pd->part, font, size); \
}

#define ELM_PART_OVERRIDE_TEXT_FONT_GET_FULL(full, type, internals) \
EOLIAN static void \
_ ## full ## _efl_text_font_font_get(const Eo *obj, void *_pd EINA_UNUSED, const char **font, Efl_Font_Size *size) \
{ \
   internals \
   _ ## type ## _text_font_get(pd->obj, sd, pd->part, font, size); \
}

#define ELM_PART_OVERRIDE_CONTENT_SET(type, TYPE, typedata) \
   ELM_PART_OVERRIDE_CONTENT_SET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata))

#define ELM_PART_OVERRIDE_CONTENT_GET(type, TYPE, typedata) \
   ELM_PART_OVERRIDE_CONTENT_GET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata))

#define ELM_PART_OVERRIDE_CONTENT_UNSET(type, TYPE, typedata) \
   ELM_PART_OVERRIDE_CONTENT_UNSET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata))

#define ELM_PART_OVERRIDE_TEXT_SET(type, TYPE, typedata) \
   ELM_PART_OVERRIDE_TEXT_TEXT_SET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata))

#define ELM_PART_OVERRIDE_TEXT_GET(type, TYPE, typedata) \
   ELM_PART_OVERRIDE_TEXT_TEXT_GET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata))

#define ELM_PART_OVERRIDE_MARKUP_SET(type, TYPE, typedata) \
   ELM_PART_OVERRIDE_TEXT_MARKUP_SET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata))

#define ELM_PART_OVERRIDE_MARKUP_GET(type, TYPE, typedata) \
   ELM_PART_OVERRIDE_TEXT_MARKUP_GET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata))

#define ELM_PART_OVERRIDE_CONTENT_SET_NO_SD(type) \
   ELM_PART_OVERRIDE_CONTENT_SET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH_NO_SD())

#define ELM_PART_OVERRIDE_CONTENT_GET_NO_SD(type) \
   ELM_PART_OVERRIDE_CONTENT_GET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH_NO_SD())

#define ELM_PART_OVERRIDE_CONTENT_UNSET_NO_SD(type) \
   ELM_PART_OVERRIDE_CONTENT_UNSET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH_NO_SD())

#define ELM_PART_OVERRIDE_TEXT_SET_NO_SD(type) \
   ELM_PART_OVERRIDE_TEXT_TEXT_SET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH_NO_SD())

#define ELM_PART_OVERRIDE_TEXT_GET_NO_SD(type) \
   ELM_PART_OVERRIDE_TEXT_TEXT_GET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH_NO_SD())

#define ELM_PART_OVERRIDE_MARKUP_SET_NO_SD(type) \
   ELM_PART_OVERRIDE_TEXT_MARKUP_SET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH_NO_SD())

#define ELM_PART_OVERRIDE_MARKUP_GET_NO_SD(type) \
   ELM_PART_OVERRIDE_TEXT_MARKUP_GET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH_NO_SD())

#define ELM_PART_OVERRIDE_FONT_SET(type, TYPE, typedata) \
   ELM_PART_OVERRIDE_TEXT_FONT_SET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata))

#define ELM_PART_OVERRIDE_FONT_GET(type, TYPE, typedata) \
   ELM_PART_OVERRIDE_TEXT_FONT_GET_FULL(type ## _part, type, ELM_PART_OVERRIDE_INTERNALS_FETCH(TYPE, typedata))


#define ELM_PART_TEXT_DEFAULT_IMPLEMENT(type, typedata) \
EOLIAN static void \
_ ## type ## _efl_text_text_set(Eo *obj, typedata *sd EINA_UNUSED, const char *text) \
{ \
   efl_text_set(efl_part(obj, efl_ui_widget_default_text_part_get(obj)), text); \
} \
EOLIAN static const char * \
_ ## type ## _efl_text_text_get(const Eo *obj, typedata *sd EINA_UNUSED) \
{ \
  return efl_text_get(efl_part(obj, efl_ui_widget_default_text_part_get(obj))); \
} \
EOLIAN static void \
_ ## type ## _efl_ui_translatable_translatable_text_set(Eo *obj, typedata *sd EINA_UNUSED, const char *label, const char *domain) \
{ \
   efl_ui_translatable_text_set(efl_part(obj, efl_ui_widget_default_text_part_get(obj)), label, domain); \
} \
EOLIAN static const char * \
_ ## type ## _efl_ui_translatable_translatable_text_get(const Eo *obj, typedata *sd EINA_UNUSED, const char **domain) \
{ \
  return efl_ui_translatable_text_get(efl_part(obj, efl_ui_widget_default_text_part_get(obj)), domain); \
}

#define ELM_PART_MARKUP_DEFAULT_IMPLEMENT(type, typedata) \
EOLIAN static const char * \
_ ## type ## _efl_text_markup_markup_get(const Eo *obj, typedata *sd EINA_UNUSED) \
{ \
  return efl_text_markup_get(efl_part(obj, efl_ui_widget_default_text_part_get(obj))); \
} \
EOLIAN static void \
_ ## type ## _efl_text_markup_markup_set(Eo *obj, typedata *sd EINA_UNUSED, const char *markup) \
{ \
  efl_text_markup_set(efl_part(obj, efl_ui_widget_default_text_part_get(obj)), markup); \
}

#endif
