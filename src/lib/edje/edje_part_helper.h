#include "edje_private.h"
#include "efl_canvas_layout_internal.eo.h"

typedef struct _Efl_Canvas_Layout_Internal_Data Efl_Canvas_Layout_Internal_Data;

struct _Efl_Canvas_Layout_Internal_Data
{
   Edje           *ed;
   Edje_Real_Part *rp;
   const char     *part;
   unsigned char   temp;
};

struct _Part_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
};

#define PROXY_REF(obj, pd) do { if (!(pd->temp++)) efl_ref(obj); } while(0)
#define PROXY_UNREF(obj, pd) do { if (pd->temp) { if (!(--pd->temp)) efl_del(obj); } } while(0)
#define RETURN_VAL(a) do { typeof(a) _ret = a; PROXY_UNREF(obj, pd); return _ret; } while(0)
#define RETURN_VOID do { PROXY_UNREF(obj, pd); return; } while(0)
#define PROXY_CALL(a) ({ PROXY_REF(obj, pd); a; })

#ifndef PROXY_ADD_EXTRA_OP
# define PROXY_ADD_EXTRA_OP
#endif

void _edje_real_part_set(Eo *obj, void *ed, void *rp, const char *part);

/* ugly macros to avoid code duplication */

#define PROXY_RESET(type) \
   do { if (_ ## type ## _proxy) \
     { \
        efl_del_intercept_set(_ ## type ## _proxy, NULL); \
        efl_del(_ ## type ## _proxy); \
        _ ## type ## _proxy = NULL; \
     } } while (0)

#define PROXY_INIT(type) \
void \
_ ## type ## _shutdown(void); \

#define PROXY_DATA_GET(obj, pd) \
   Efl_Canvas_Layout_Internal_Data *pd = efl_data_scope_get(obj, EFL_CANVAS_LAYOUT_INTERNAL_CLASS)

#define PROXY_IMPLEMENTATION(type, TYPE, no_del_cb, ...) \
static Eo * _ ## type ## _proxy = NULL; \
\
static void \
_ ## type ## _del_cb(Eo *proxy) \
{ \
   if (_ ## type ## _proxy) \
     { \
        efl_del_intercept_set(proxy, NULL); \
        efl_del(proxy); \
        return; \
     } \
   if (efl_parent_get(proxy)) \
     { \
        efl_ref(proxy); \
        efl_parent_set(proxy, NULL); \
     } \
   efl_reuse(proxy); \
   _ ## type ## _proxy = proxy; \
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
   pd = efl_data_scope_get(_ ## type ## _proxy, EFL_CANVAS_LAYOUT_INTERNAL_CLASS); \
   if (!pd) \
     { \
        if (_ ## type ## _proxy) \
          { \
             ERR("Found invalid handle for efl_part. Reset."); \
             _ ## type ## _proxy = NULL; \
          } \
        proxy = efl_add(EFL_CANVAS_LAYOUT_##TYPE##_CLASS, ed->obj, \
                        _edje_real_part_set(efl_added, ed, rp, rp->part->name)); \
        __VA_ARGS__; \
        if (!no_del_cb) efl_del_intercept_set(proxy, _ ## type ## _del_cb); \
        return proxy; \
     } \
   \
   if (EINA_UNLIKELY(pd->temp)) \
     { \
        /* warn about misuse, since non-implemented functions may trigger this \
         * misuse by accident. */ \
        ERR("Misuse of efl_part detected. Handles returned by efl_part() are " \
            "valid for a single function call! Did you call a non implemented " \
            "function?"); \
     } \
   proxy = _ ## type ## _proxy; \
   _ ## type ## _proxy = NULL; \
   _edje_real_part_set(proxy, ed, rp, rp->part->name); \
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
