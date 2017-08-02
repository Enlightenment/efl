#ifndef _ELM_PART_HELPER_H
#define _ELM_PART_HELPER_H

#include "elm_layout_internal_part.eo.h"

//#define ELM_PART_HOOK do { ERR("%p:%s [%d]", pd->obj, pd->part, (int) pd->temp); } while(0)
#define ELM_PART_HOOK do {} while(0)

#define ELM_PART_REF(obj, pd) do { if (!(pd->temp++)) efl_ref(obj); } while(0)
#define ELM_PART_UNREF(obj, pd) do { if (pd->temp) { if (!(--pd->temp)) efl_del(obj); } } while(0)
#define ELM_PART_RETURN_VAL(a) do { ELM_PART_HOOK; typeof(a) _ret = a; ELM_PART_UNREF(obj, pd); return _ret; } while(0)
#define ELM_PART_RETURN_VOID do { ELM_PART_HOOK; ELM_PART_UNREF(obj, pd); return; } while(0)
#define ELM_PART_CALL(a) ({ ELM_PART_REF(obj, pd); a; })

typedef struct _Elm_Part_Data Elm_Part_Data;
struct _Elm_Part_Data
{
   Eo             *obj;
   void           *sd;
   char           *part;
   unsigned char   temp;
};

// Note: this generic implementation can be improved to support part object
// caching or something...

#define ELM_PART_IMPLEMENT_DESTRUCTOR(type, TYPE, typedata, partdata) \
static EOLIAN void \
_ ## type ## _internal_part_efl_object_destructor(Eo *obj, partdata *pd) \
{ \
   ELM_PART_HOOK; \
   free(pd->part); \
   efl_data_xunref(pd->obj, pd->sd, obj); \
   efl_destructor(efl_super(obj, TYPE ## _INTERNAL_PART_CLASS)); \
} \

// Main part proxy implementation
#define ELM_PART_IMPLEMENT(type, TYPE, typedata, partdata) \
ELM_PART_IMPLEMENT_DESTRUCTOR(type, TYPE, typedata, partdata) \
static EOLIAN Efl_Object * \
_ ## type ## _efl_part_part(const Eo *obj, typedata *priv EINA_UNUSED, const char *part) \
{ \
   partdata *pd; \
   Eo *proxy; \
\
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL); \
   proxy = efl_add(TYPE ## _INTERNAL_PART_CLASS, (Eo *) obj); \
   pd = efl_data_scope_get(proxy, TYPE ## _INTERNAL_PART_CLASS); \
   if (pd) \
     { \
        pd->obj = (Eo *) obj; \
        pd->sd = efl_data_xref(pd->obj, TYPE ## _CLASS, proxy); \
        pd->part = strdup(part); \
        pd->temp = 1; \
     } \
\
   return proxy; \
}

#define ELM_PART_IMPLEMENT_CONTENT_SET(type, TYPE, typedata, partdata) \
static EOLIAN Eina_Bool \
_ ## type ## _internal_part_efl_container_content_set(Eo *obj, partdata *pd, Efl_Gfx *content) \
{ \
   ELM_PART_RETURN_VAL(_ ## type ## _content_set(pd->obj, pd->sd, pd->part, content)); \
}

#define ELM_PART_IMPLEMENT_CONTENT_GET(type, TYPE, typedata, partdata) \
static EOLIAN Efl_Gfx * \
_ ## type ## _internal_part_efl_container_content_get(Eo *obj, partdata *pd) \
{ \
   ELM_PART_RETURN_VAL(_ ## type ## _content_get(pd->obj, pd->sd, pd->part)); \
}

#define ELM_PART_IMPLEMENT_CONTENT_UNSET(type, TYPE, typedata, partdata) \
static EOLIAN Efl_Gfx * \
_ ## type ## _internal_part_efl_container_content_unset(Eo *obj, partdata *pd) \
{ \
   ELM_PART_RETURN_VAL(_ ## type ## _content_unset(pd->obj, pd->sd, pd->part)); \
}

#define ELM_PART_IMPLEMENT_TEXT_SET(type, TYPE, typedata, partdata) \
static EOLIAN void \
_ ## type ## _internal_part_efl_text_text_set(Eo *obj EINA_UNUSED, partdata *pd, const char *text) \
{ \
  _ ## type ## _text_set(pd->obj, pd->sd, pd->part, text); \
}

#define ELM_PART_IMPLEMENT_TEXT_GET(type, TYPE, typedata, partdata) \
static EOLIAN const char * \
_ ## type ## _internal_part_efl_text_text_get(Eo *obj, partdata *pd) \
{ \
   ELM_PART_RETURN_VAL(_ ## type ## _text_get(pd->obj, pd->sd, pd->part)); \
}

#define ELM_PART_CONTENT_DEFAULT_SET(type, part) \
   static const char * _ ## type ## _default_content_part_get(const Eo *obj EINA_UNUSED, void *sd EINA_UNUSED) { return part; }

#define ELM_PART_CONTENT_DEFAULT_OPS(type) \
   EFL_OBJECT_OP_FUNC(elm_widget_default_content_part_get, _ ## type ## _default_content_part_get)

#define ELM_PART_TEXT_DEFAULT_GET(type, part) \
   static const char * _ ## type ## _default_text_part_get(const Eo *obj EINA_UNUSED, void *sd EINA_UNUSED) { return part; }

#define ELM_PART_TEXT_DEFAULT_OPS(type) \
   EFL_OBJECT_OP_FUNC(elm_widget_default_text_part_get, _ ## type ## _default_text_part_get)

// For widgets that inherit from something with parts (eg. from Elm.Layout)
#define ELM_PART_OVERRIDE(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN Efl_Object * \
_ ## type ## _efl_part_part(const Eo *obj, typedata *priv EINA_UNUSED, const char *part) \
{ \
   partdata *pd; \
   Eo *proxy; \
\
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL); \
   proxy = efl_add(TYPE ## _INTERNAL_PART_CLASS, (Eo *) obj); \
   pd = efl_data_scope_get(proxy, SUPER ## _INTERNAL_PART_CLASS); \
   if (pd) \
     { \
        pd->obj = (Eo *) obj; \
        pd->sd = efl_data_xref(pd->obj, SUPER ## _CLASS, proxy); \
        pd->part = strdup(part); \
        pd->temp = 1; \
     } \
\
   return proxy; \
}

#define ELM_PART_OVERRIDE_CONTENT_SET(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN Eina_Bool \
_ ## type ## _internal_part_efl_container_content_set(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx *content) \
{ \
   partdata *pd = efl_data_scope_get(obj, SUPER ## _INTERNAL_PART_CLASS); \
   typedata *sd = efl_data_scope_get(pd->obj, TYPE ## _CLASS); \
   ELM_PART_RETURN_VAL(_ ## type ## _content_set(pd->obj, sd, pd->part, content)); \
}

#define ELM_PART_OVERRIDE_CONTENT_GET(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN Efl_Gfx * \
_ ## type ## _internal_part_efl_container_content_get(Eo *obj, void *_pd EINA_UNUSED) \
{ \
   partdata *pd = efl_data_scope_get(obj, SUPER ## _INTERNAL_PART_CLASS); \
   typedata *sd = efl_data_scope_get(pd->obj, TYPE ## _CLASS); \
   ELM_PART_RETURN_VAL(_ ## type ## _content_get(pd->obj, sd, pd->part)); \
}

#define ELM_PART_OVERRIDE_CONTENT_UNSET(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN Efl_Gfx * \
_ ## type ## _internal_part_efl_container_content_unset(Eo *obj, void *_pd EINA_UNUSED) \
{ \
   partdata *pd = efl_data_scope_get(obj, SUPER ## _INTERNAL_PART_CLASS); \
   typedata *sd = efl_data_scope_get(pd->obj, TYPE ## _CLASS); \
   ELM_PART_RETURN_VAL(_ ## type ## _content_unset(pd->obj, sd, pd->part)); \
}

#define ELM_PART_OVERRIDE_TEXT_SET(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN void \
_ ## type ## _internal_part_efl_text_text_set(Eo *obj, void *_pd EINA_UNUSED, const char *text) \
{ \
   partdata *pd = efl_data_scope_get(obj, SUPER ## _INTERNAL_PART_CLASS); \
   typedata *sd = efl_data_scope_get(pd->obj, TYPE ## _CLASS); \
   ELM_PART_CALL(_ ## type ## _text_set(pd->obj, sd, pd->part, text)); \
}

#define ELM_PART_OVERRIDE_TEXT_GET(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN const char *\
_ ## type ## _internal_part_efl_text_text_get(Eo *obj, void *_pd EINA_UNUSED) \
{ \
   partdata *pd = efl_data_scope_get(obj, SUPER ## _INTERNAL_PART_CLASS); \
   typedata *sd = efl_data_scope_get(pd->obj, TYPE ## _CLASS); \
   ELM_PART_RETURN_VAL(_ ## type ## _text_get(pd->obj, sd, pd->part)); \
}

#define EFL_TEXT_PART_DEFAULT_IMPLEMENT(type, Type) \
EOLIAN static void \
_ ## type ## _efl_text_text_set(Eo *obj, Type *pd EINA_UNUSED, const char *text) \
{ \
   efl_text_set(efl_part(efl_super(obj, MY_CLASS), "elm.text"), text); \
} \
EOLIAN static const char * \
_ ## type ## _efl_text_text_get(Eo *obj, Type *pd EINA_UNUSED) \
{ \
  return efl_text_get(efl_part(efl_super(obj, MY_CLASS), "elm.text")); \
}

#endif
