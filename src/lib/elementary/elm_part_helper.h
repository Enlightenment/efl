#ifndef _ELM_PART_HELPER_H
#define _ELM_PART_HELPER_H

#include "elm_layout_internal_part.eo.h"

//#define ELM_PART_HOOK do { ERR("%p:%s [%d]", pd->obj, pd->part, (int) pd->temp); } while(0)
#define ELM_PART_HOOK

#define ELM_PART_REF(obj, pd) do { if (!(pd->temp++)) eo_ref(obj); } while(0)
#define ELM_PART_UNREF(obj, pd) do { if (pd->temp) { if (!(--pd->temp)) eo_unref(obj); } } while(0)
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
_ ## type ## _internal_part_eo_base_destructor(Eo *obj, partdata *pd) \
{ \
   ELM_PART_HOOK; \
   free(pd->part); \
   eo_data_xunref(pd->obj, pd->sd, obj); \
   eo_destructor(eo_super(obj, TYPE ## _INTERNAL_PART_CLASS)); \
} \

// Main part proxy implementation
#define ELM_PART_IMPLEMENT(type, TYPE, typedata, partdata) \
ELM_PART_IMPLEMENT_DESTRUCTOR(type, TYPE, typedata, partdata) \
static EOLIAN Eo_Base * \
_ ## type ## _efl_part_part(const Eo *obj, typedata *priv EINA_UNUSED, const char *part) \
{ \
   partdata *pd; \
   Eo *proxy; \
\
   proxy = eo_add(TYPE ## _INTERNAL_PART_CLASS, (Eo *) obj); \
   pd = eo_data_scope_get(proxy, TYPE ## _INTERNAL_PART_CLASS); \
   if (pd) \
     { \
        pd->obj = (Eo *) obj; \
        pd->sd = eo_data_xref(pd->obj, TYPE ## _CLASS, proxy); \
        pd->part = part ? strdup(part) : NULL; \
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

// For widgets that inherit from something with parts (eg. from Elm.Layout)
#define ELM_PART_OVERRIDE(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN Eo_Base * \
_ ## type ## _efl_part_part(const Eo *obj, typedata *priv EINA_UNUSED, const char *part) \
{ \
   partdata *pd; \
   Eo *proxy; \
\
   proxy = eo_add(TYPE ## _INTERNAL_PART_CLASS, (Eo *) obj); \
   pd = eo_data_scope_get(proxy, SUPER ## _INTERNAL_PART_CLASS); \
   if (pd) \
     { \
        pd->obj = (Eo *) obj; \
        pd->sd = eo_data_xref(pd->obj, SUPER ## _CLASS, proxy); \
        pd->part = part ? strdup(part) : NULL; \
        pd->temp = 1; \
     } \
\
   return proxy; \
}

#define ELM_PART_OVERRIDE_CONTENT_SET(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN Eina_Bool \
_ ## type ## _internal_part_efl_container_content_set(Eo *obj, void *_pd EINA_UNUSED, Efl_Gfx *content) \
{ \
   partdata *pd = eo_data_scope_get(obj, SUPER ## _INTERNAL_PART_CLASS); \
   ELM_PART_RETURN_VAL(_ ## type ## _content_set(pd->obj, pd->sd, pd->part, content)); \
}

#define ELM_PART_OVERRIDE_CONTENT_GET(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN Efl_Gfx * \
_ ## type ## _internal_part_efl_container_content_get(Eo *obj, void *_pd EINA_UNUSED) \
{ \
   partdata *pd = eo_data_scope_get(obj, SUPER ## _INTERNAL_PART_CLASS); \
   ELM_PART_RETURN_VAL(_ ## type ## _content_get(pd->obj, pd->sd, pd->part)); \
}

#define ELM_PART_OVERRIDE_CONTENT_UNSET(type, TYPE, SUPER, typedata, partdata) \
static EOLIAN Efl_Gfx * \
_ ## type ## _internal_part_efl_container_content_unset(Eo *obj, void *_pd EINA_UNUSED) \
{ \
   partdata *pd = eo_data_scope_get(obj, SUPER ## _INTERNAL_PART_CLASS); \
   ELM_PART_RETURN_VAL(_ ## type ## _content_unset(pd->obj, pd->sd, pd->part)); \
}

#endif
