#include "edje_private.h"
#define EFL_CANVAS_LAYOUT_INTERNAL_PROTECTED
#include "efl_canvas_layout_internal.eo.h"

struct _Edje_Part_Data
{
   Edje           *ed;
   Edje_Real_Part *rp;
   const char     *part;
   unsigned char   temp;
};

typedef struct _Edje_Part_Data Edje_Part_Data;

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

#define PROXY_IMPLEMENTATION(type, TYPE, datatype) \
typedef struct _Edje_Part_Data datatype; \
\
static Eo * _ ## type ## _proxy = NULL; \
\
static void \
type ## _del_cb(Eo *proxy) \
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
   Edje_Part_Data *pd; \
   Eo *proxy; \
   \
   pd = efl_data_scope_get(_ ## type ## _proxy, MY_CLASS); \
   if (!pd) \
     { \
        if (_ ## type ## _proxy) \
          { \
             ERR("Found invalid handle for efl_part. Reset."); \
             _ ## type ## _proxy = NULL; \
          } \
        return efl_add(MY_CLASS, ed->obj, \
                      _edje_real_part_set(efl_added, ed, rp, rp->part->name)); \
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
   return proxy; \
} \
\
EOLIAN static void \
_efl_canvas_layout_internal_ ## type ## _efl_canvas_layout_internal_real_part_set(Eo *obj, datatype *pd, void *ed, void *rp, const char *part) \
{ \
   pd->ed = ed; \
   pd->rp = rp; \
   pd->part = part; \
   pd->temp = 1; \
   efl_del_intercept_set(obj, type ## _del_cb); \
   efl_parent_set(obj, pd->ed->obj); \
} \
\
EOLIAN static Efl_Object * \
_efl_canvas_layout_internal_ ## type ## _efl_object_finalize(Eo *obj, datatype *pd) \
{ \
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->rp && pd->ed && pd->part, NULL); \
   return efl_finalize(efl_super(obj, MY_CLASS)); \
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
