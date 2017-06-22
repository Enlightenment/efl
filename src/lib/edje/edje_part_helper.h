#include "edje_private.h"
#include "efl_canvas_layout_internal.eo.h"

typedef struct _Efl_Canvas_Layout_Internal_Data Efl_Canvas_Layout_Internal_Data;

struct _Efl_Canvas_Layout_Internal_Data
{
   Edje           *ed;
   Edje_Real_Part *rp;
   const char     *part;
   Eo             *obj;
   unsigned char   temp, in_call;
};

struct _Part_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
};

void _part_reuse_error(Efl_Canvas_Layout_Internal_Data *pd);

#define PROXY_CALL_BEGIN(pd) do { pd->in_call = 1; } while (0)
#define PROXY_CALL_END(pd) do { pd->in_call = 0; } while (0)
#define PROXY_REF(obj, pd) do { if (!(pd->temp++)) efl_ref(obj); } while(0)
#define PROXY_UNREF(obj, pd) do { if (pd->temp) { if (!(--pd->temp)) efl_del(obj); } } while(0)
#define RETURN_VAL(a) do { typeof(a) _ret = a; PROXY_CALL_END(pd); PROXY_UNREF(obj, pd); return _ret; } while(0)
#define RETURN_VOID do { PROXY_CALL_END(pd); PROXY_UNREF(obj, pd); return; } while(0)
#define PROXY_CALL(a) ({ PROXY_REF(obj, pd); a; })
#define PROXY_STATIC_VAR(type) _##type##_proxy

#ifndef PROXY_ADD_EXTRA_OP
# define PROXY_ADD_EXTRA_OP
#endif

void _edje_real_part_set(Eo *obj, Edje *ed, Edje_Real_Part *rp, const char *part);

static inline void
_part_proxy_del_cb(Eo *proxy, Eo **static_var)
{
   if (*static_var)
     {
        // FIXME: Enable debug checks only in debug mode
        Efl_Canvas_Layout_Internal_Data *pd = efl_data_scope_get
              (*static_var, EFL_CANVAS_LAYOUT_INTERNAL_CLASS);
        if (pd && pd->temp && !pd->in_call)
          _part_reuse_error(pd);
        if (*static_var != proxy)
          efl_del_intercept_set(*static_var, NULL);
     }
   if (efl_parent_get(proxy))
     {
        efl_ref(proxy);
        efl_parent_set(proxy, NULL);
     }
   efl_reuse(proxy);
   *static_var = proxy;
}

/* ugly macros to avoid code duplication */

#define PROXY_RESET(type) \
   do { if (PROXY_STATIC_VAR(type)) \
     { \
        efl_del_intercept_set(PROXY_STATIC_VAR(type), NULL); \
        efl_del(PROXY_STATIC_VAR(type)); \
        PROXY_STATIC_VAR(type) = NULL; \
     } } while (0)

#define PROXY_INIT(type) \
void \
_ ## type ## _shutdown(void); \

#define PROXY_DATA_GET(obj, pd) \
   Efl_Canvas_Layout_Internal_Data *pd = efl_data_scope_get(obj, EFL_CANVAS_LAYOUT_INTERNAL_CLASS); \
   PROXY_CALL_BEGIN(pd)

#define PROXY_IMPLEMENTATION(type, KLASS, no_del_cb, ...) \
static Eo * PROXY_STATIC_VAR(type) = NULL; \
\
static void \
_ ## type ## _del_cb(Eo *proxy) \
{ \
   _part_proxy_del_cb(proxy, &(PROXY_STATIC_VAR(type))); \
} \
\
void \
_ ## type ## _shutdown(void) \
{ \
   PROXY_RESET(type); \
} \
\
Eo * \
_edje_ ## type ## _internal_proxy_get(Edje_Object *obj EINA_UNUSED, Edje *ed, Edje_Real_Part *rp) \
{ \
   Efl_Canvas_Layout_Internal_Data *pd; \
   Eo *proxy; \
   \
   pd = PROXY_STATIC_VAR(type) ? efl_data_scope_get(PROXY_STATIC_VAR(type), EFL_CANVAS_LAYOUT_INTERNAL_CLASS) : NULL; \
   if (!pd) \
     { \
        if (PROXY_STATIC_VAR(type)) \
          ERR("Found invalid handle for efl_part. Reset."); \
        PROXY_STATIC_VAR(type) = efl_add(KLASS, ed->obj, _edje_real_part_set(efl_added, ed, rp, rp->part->name)); \
        goto end ; \
     } \
   \
   if (EINA_UNLIKELY(pd->temp)) \
     { \
        /* if (!pd->in_call) _part_reuse_error(pd); */ \
        PROXY_STATIC_VAR(type) = efl_add(KLASS, ed->obj, _edje_real_part_set(efl_added, ed, rp, rp->part->name)); \
     } \
   else \
     { \
        _edje_real_part_set(PROXY_STATIC_VAR(type), ed, rp, rp->part->name); \
     } \
   \
end: \
   proxy = PROXY_STATIC_VAR(type); \
   __VA_ARGS__; \
   if (!no_del_cb) efl_del_intercept_set(proxy, _ ## type ## _del_cb); \
   return proxy; \
}

#ifdef DEBUG
#define PART_TABLE_GET(obj, part, ...) ({ \
   Eo *__table = efl_part(obj, part); \
   if (!__table || !efl_isa(__table, EFL_CANVAS_LAYOUT_INTERNAL_TABLE_CLASS)) \
     { \
        ERR("No such table part '%s' in layout %p", part, obj); \
        return __VA_ARGS__; \
     } \
   __table; })
#else
#define PART_TABLE_GET(obj, part, ...) ({ \
   Eo *__table = efl_part(obj, part); \
   if (!__table) return __VA_ARGS__; \
   __table; })
#endif
