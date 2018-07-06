#include "edje_private.h"

typedef struct _Efl_Canvas_Layout_Part_Data Efl_Canvas_Layout_Part_Data;

struct _Efl_Canvas_Layout_Part_Data
{
   Edje           *ed;
   Edje_Real_Part *rp;
   const char     *part;
   Eo             *obj;
};

struct _Part_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
};

void _part_reuse_error(Efl_Canvas_Layout_Part_Data *pd);
const char * _part_type_to_string(unsigned char type);

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
   Efl_Canvas_Layout_Part_Data *pd = efl_data_scope_get(obj, EFL_CANVAS_LAYOUT_PART_CLASS);

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
_edje_ ## type ## _internal_proxy_get(Edje_Object *obj EINA_UNUSED, Edje *ed, Edje_Real_Part *rp, const char *part) \
{ \
   Efl_Canvas_Layout_Part_Data *pd; \
   Eo *proxy = PROXY_STATIC_VAR(type); \
   pd = proxy ? efl_data_scope_get(proxy, EFL_CANVAS_LAYOUT_PART_CLASS) : NULL; \
   if (!pd) \
     { \
        if (EINA_UNLIKELY(proxy != NULL)) \
          ERR("Found invalid handle for efl_part. Reset."); \
        proxy = efl_add(KLASS, ed->obj, _edje_real_part_set(efl_added, ed, rp, part)); \
     } \
   else \
     { \
        PROXY_STATIC_VAR(type) = NULL; \
        _edje_real_part_set(proxy, ed, rp, part); \
     } \
   __VA_ARGS__; \
   if (!no_del_cb) efl_del_intercept_set(proxy, _ ## type ## _del_cb); \
   efl_allow_parent_unref_set(proxy, 1); \
   ___efl_auto_unref_set(proxy, 1); \
   return proxy; \
}

#ifdef DEBUG
#define PART_TABLE_GET(obj, part, ...) ({ \
   Eo *__table = efl_part(obj, part); \
   if (!__table || !efl_isa(__table, EFL_CANVAS_LAYOUT_PART_TABLE_CLASS)) \
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
