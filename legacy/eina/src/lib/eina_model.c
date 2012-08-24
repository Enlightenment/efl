/* EINA - EFL data type library
 * Copyright (C) 2012 ProFUSION embedded systems
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_error.h"
#include "eina_log.h"
#include "eina_mempool.h"
#include "eina_lock.h"
#include "eina_inlist.h"
#include "eina_strbuf.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_value.h" /* eina-safety used in inline.x */
#include "eina_model.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static Eina_Mempool *_eina_model_mp = NULL;
static Eina_Hash *_eina_model_inner_mps = NULL;
static Eina_Lock _eina_model_inner_mps_lock;
static char *_eina_model_mp_choice = NULL;
static Eina_Hash *_eina_model_descriptions = NULL;
static Eina_Lock _eina_model_descriptions_lock;
static int _eina_model_log_dom = -1;
static enum {
  EINA_MODEL_DEBUG_NONE = 0,
  EINA_MODEL_DEBUG_CHECK = 1,
  EINA_MODEL_DEBUG_BACKTRACE = 2,
} _eina_model_debug = EINA_MODEL_DEBUG_NONE;
static Eina_Lock _eina_model_debug_list_lock;
static Eina_List *_eina_model_debug_list = NULL;

static const char _eina_model_str_deleted[] = "deleted";
static const char _eina_model_str_freed[] = "freed";
static const char _eina_model_str_property_set[] =  "property,set";
static const char _eina_model_str_property_del[] =  "property,deleted";
static const char _eina_model_str_children_changed[] =  "children,changed";
static const char _eina_model_str_child_inserted[] =  "child,inserted";
static const char _eina_model_str_child_set[] =  "child,set";
static const char _eina_model_str_child_del[] =  "child,deleted";
static const char _eina_model_str_loaded[] = "loaded";
static const char _eina_model_str_unloaded[] = "unloaded";
static const char _eina_model_str_properties_loaded[] = "properties,loaded";
static const char _eina_model_str_properties_unloaded[] = "properties,unloaded";
static const char _eina_model_str_children_loaded[] = "children,loaded";
static const char _eina_model_str_children_unloaded[] = "children,unloaded";

#ifdef CRITICAL
#undef CRITICAL
#endif
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_eina_model_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_model_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eina_model_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_model_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_model_log_dom, __VA_ARGS__)


/* convenience sort array of Eina_Model* giving compare Eina_Model* instead of
 * Eina_Model**
 */
static unsigned int
_eina_model_array_partition(Eina_Model **array, unsigned int start, unsigned int last, unsigned int pivot, Eina_Compare_Cb compare)
{
   Eina_Model **itr, **itr_end, *tmp, *pivot_value;

   pivot_value = tmp = array[pivot];
   array[pivot] = array[last];
   array[last] = tmp;

   pivot = start;
   itr = array + start;
   itr_end = array + last;
   for (; itr < itr_end; itr++)
     {
        if (compare(*itr, pivot_value) < 0)
          {
             tmp = *itr;
             *itr = array[pivot];
             array[pivot] = tmp;
             pivot++;
          }
     }

   tmp = array[last];
   array[last] = array[pivot];
   array[pivot] = tmp;

   return pivot;
}

static void
_eina_model_array_sort(Eina_Model **array, unsigned int start, unsigned int last, Eina_Compare_Cb compare)
{
   unsigned int pivot, new_pivot;

   if (last <= start)
     return;

   pivot = start + (last - start) / 2; /* avoid overflow */
   new_pivot = _eina_model_array_partition(array, start, last, pivot, compare);

   if (start + 1 < new_pivot)
     _eina_model_array_sort(array, start, new_pivot - 1, compare);

   if (new_pivot + 1 < last)
     _eina_model_array_sort(array, new_pivot + 1, last, compare);
}

/*
 * Most of inner allocations are made with internal mempools, types
 * and thus instace private data will repeat and it's good to use them.
 *
 * To save on the number of mempools, they are kept per size, not per
 * type.
 *
 * This is done by means of _eina_model_inner_alloc() and
 * _eina_model_inner_free(), both at thread safe.
 *
 */
typedef struct _Eina_Model_Inner_Mp Eina_Model_Inner_Mp;
struct _Eina_Model_Inner_Mp
{
   Eina_Mempool *mempool;
   int refcount;
};

static inline void
_eina_model_inner_mp_dispose(int size, Eina_Model_Inner_Mp *imp)
{
   EINA_SAFETY_ON_FALSE_RETURN(imp->refcount == 0);

   eina_hash_del_by_key(_eina_model_inner_mps, &size);
   eina_mempool_del(imp->mempool);
   free(imp);
}

static inline Eina_Model_Inner_Mp *
_eina_model_inner_mp_get(int size)
{
   Eina_Model_Inner_Mp *imp = eina_hash_find(_eina_model_inner_mps, &size);
   if (imp) return imp;

   imp = malloc(sizeof(Eina_Model_Inner_Mp));
   if (!imp)
     return NULL;

   imp->refcount = 0;

   imp->mempool = eina_mempool_add(_eina_model_mp_choice,
                                   "Eina_Model_Inner_Mp", NULL, size, 16);
   if (!imp->mempool)
     {
        free(imp);
        return NULL;
     }

   if (!eina_hash_add(_eina_model_inner_mps, &size, imp))
     {
        eina_mempool_del(imp->mempool);
        free(imp);
        return NULL;
     }

   return imp;
}

static inline void *
_eina_model_inner_alloc_internal(int size)
{
   Eina_Model_Inner_Mp *imp;
   void *mem;

   imp = _eina_model_inner_mp_get(size);
   if (!imp) return NULL;

   mem = eina_mempool_malloc(imp->mempool, size);
   if (mem) imp->refcount++;
   else if (imp->refcount == 0) _eina_model_inner_mp_dispose(size, imp);

   return mem;
}

static inline void
_eina_model_inner_free_internal(int size, void *mem)
{
   Eina_Model_Inner_Mp *imp = eina_hash_find(_eina_model_inner_mps, &size);
   EINA_SAFETY_ON_NULL_RETURN(imp);

   eina_mempool_free(imp->mempool, mem);

   imp->refcount--;
   if (imp->refcount > 0) return;
   _eina_model_inner_mp_dispose(size, imp);
}

static void *
_eina_model_inner_alloc(size_t size)
{
   void *mem;

   if (size > 512) return malloc(size);

   eina_lock_take(&_eina_model_inner_mps_lock);
   mem = _eina_model_inner_alloc_internal(size);
   eina_lock_release(&_eina_model_inner_mps_lock);

   return mem;
}

static void
_eina_model_inner_free(size_t size, void *mem)
{
   if (size > 512)
     {
        free(mem);
        return;
     }

   eina_lock_take(&_eina_model_inner_mps_lock);
   _eina_model_inner_free_internal(size, mem);
   eina_lock_release(&_eina_model_inner_mps_lock);
}


typedef union _Eina_Model_Provider Eina_Model_Provider;
union _Eina_Model_Provider
{
   const Eina_Model_Type *type;
   const Eina_Model_Interface *iface;
};

/* store event name to aid searching */
typedef struct _Eina_Model_Event_Description_Cache Eina_Model_Event_Description_Cache;
struct _Eina_Model_Event_Description_Cache
{
   const char *name;
   const Eina_Model_Event_Description *desc;
   Eina_Model_Provider provider;
};

/* description is an optimized structure for type. It's built at runtime
 * to avoid user input errors and help declaration.
 *
 * lookups (ifaces, events) are sorted for binary search.
 *
 * recursion is avoided by expansion of every possible value in "cache"
 * struct.
 *
 * the first usable operation is stopred for type at "ops" struct,
 * avoiding usage of _eina_model_type_find_offset().
 *
 * Get a model type description using _eina_model_description_get(),
 * when it's not used anymore use
 * _eina_model_description_dispose(). These operations are thread
 * safe.
 */
typedef struct _Eina_Model_Description Eina_Model_Description;
struct _Eina_Model_Description
{
   struct {
      const Eina_Model_Type **types; /* size = total.types */
      const Eina_Model_Interface **ifaces; /* sorted, size = total.ifaces */
      Eina_Model_Provider *privates; /* size = total.privates (types + ifaces) */
      Eina_Model_Event_Description_Cache *events; /* size = total.events */
   } cache;
   struct {
      /* ops are the topmost operation to use for type/interface */
      struct {
         Eina_Bool (*setup)(Eina_Model *model);
         Eina_Bool (*flush)(Eina_Model *model);
         Eina_Bool (*constructor)(Eina_Model *model);
         Eina_Bool (*destructor)(Eina_Model *model);
         Eina_Bool (*copy)(const Eina_Model *src, Eina_Model *dst);
         Eina_Bool (*deep_copy)(const Eina_Model *src, Eina_Model *dst);
         Eina_Bool (*compare)(const Eina_Model *a, const Eina_Model *b, int *cmp);
         Eina_Bool (*load)(Eina_Model *model);
         Eina_Bool (*unload)(Eina_Model *model);
         Eina_Bool (*property_get)(const Eina_Model *model, const char *name, Eina_Value *value);
         Eina_Bool (*property_set)(Eina_Model *model, const char *name, const Eina_Value *value);
         Eina_Bool (*property_del)(Eina_Model *model, const char *name);
         Eina_List *(*properties_names_list_get)(const Eina_Model *model);
         int (*child_count)(const Eina_Model *model);
         Eina_Model *(*child_get)(const Eina_Model *model, unsigned int position);
         Eina_Bool (*child_set)(Eina_Model *model, unsigned int position, Eina_Model *child);
         Eina_Bool (*child_del)(Eina_Model *model, unsigned int position);
         Eina_Bool (*child_insert_at)(Eina_Model *model, unsigned int position, Eina_Model *child);
         int (*child_find)(const Eina_Model *model, unsigned int start_position, const Eina_Model *other);
         int (*child_criteria_match)(const Eina_Model *model, unsigned int start_position, Eina_Each_Cb match, const void *data);
         void (*child_sort)(Eina_Model *model, Eina_Compare_Cb compare);
         Eina_Iterator *(*child_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count);
         Eina_Iterator *(*child_reversed_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count);
         Eina_Iterator *(*child_sorted_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count, Eina_Compare_Cb compare);
         Eina_Iterator *(*child_filtered_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count, Eina_Each_Cb match, const void *data);
         char *(*to_string)(const Eina_Model *model); /**< used to represent model as string, usually for debug purposes or user convenience */
         const void **extension;
      } type;
   } ops;
   struct {
      unsigned int types;
      unsigned int ifaces;
      unsigned int privates;
      unsigned int size; /* sum of all private sizes */
      unsigned int events;
   } total;
   int refcount;
};

static Eina_Bool
_eina_model_description_type_fill(Eina_Model_Description *desc, const Eina_Model_Type *type)
{
   const Eina_Model_Type *itr, *last_itr = NULL;
   unsigned int count, child_size = 0;

   for (count = 0, itr = type; itr != NULL; itr = itr->parent, count++)
     {
        if (itr->version != EINA_MODEL_TYPE_VERSION)
          {
             CRITICAL("Type %p version is %u, expected %u instead.",
                      itr, itr->version, EINA_MODEL_TYPE_VERSION);
             return EINA_FALSE;
          }
        if (!itr->name)
          {
             CRITICAL("Type %p provides no name!", itr);
             return EINA_FALSE;
          }
        if (itr->type_size < sizeof(Eina_Model_Type))
          {
             CRITICAL("Type %p %s size must be >= sizeof(Eina_Model_Type)!",
                      itr, itr->name);
             return EINA_FALSE;
          }
        if (child_size == 0) child_size = itr->type_size;
        else if (child_size < itr->type_size)
          {
             CRITICAL("Type %p %s size is bigger than its child type %p %s!",
                      itr, itr->name, last_itr, last_itr->name);
             return EINA_FALSE;
          }
        last_itr = itr;

#define DEF_METH(meth)                                                  \
        if (!desc->ops.type.meth) desc->ops.type.meth = itr->meth
        DEF_METH(setup);
        DEF_METH(flush);
        DEF_METH(constructor);
        DEF_METH(destructor);
        DEF_METH(copy);
        DEF_METH(deep_copy);
        DEF_METH(compare);
        DEF_METH(load);
        DEF_METH(unload);
        DEF_METH(property_get);
        DEF_METH(property_set);
        DEF_METH(property_del);
        DEF_METH(properties_names_list_get);
        DEF_METH(child_count);
        DEF_METH(child_get);
        DEF_METH(child_set);
        DEF_METH(child_del);
        DEF_METH(child_insert_at);
        DEF_METH(child_find);
        DEF_METH(child_criteria_match);
        DEF_METH(child_sort);
        DEF_METH(child_iterator_get);
        DEF_METH(child_reversed_iterator_get);
        DEF_METH(child_sorted_iterator_get);
        DEF_METH(child_filtered_iterator_get);
        DEF_METH(to_string);
#undef DEF_METH

        if ((!itr->parent) && (itr != EINA_MODEL_TYPE_BASE))
          {
             CRITICAL("Type %p (%s) does not inherit from EINA_MODEL_TYPE_BASE!",
                      type, type->name);
             return EINA_FALSE;
          }
     }

#define CK_METH(meth)                                           \
   if (!desc->ops.type.meth)                                    \
     {                                                          \
        CRITICAL("Mandatory method "#meth                       \
                 "() was not provided by type %p (%s).",        \
                 type, type->name);                             \
        return EINA_FALSE;                                      \
     }
   CK_METH(setup);
   CK_METH(flush);
   CK_METH(constructor);
   CK_METH(destructor);
   CK_METH(property_get);
#undef CK_METH

   if (child_size <= sizeof(Eina_Model_Type))
     desc->ops.type.extension = NULL;
   else
     {
        unsigned ext_size = child_size - sizeof(Eina_Model_Type);
        unsigned ext_count = ext_size / sizeof(void *);

        if (ext_size % sizeof(void *) != 0)
          {
             CRITICAL("Extension size %u is not multiple of sizeof(void*)",
                      ext_size);
             return EINA_FALSE;
          }

        desc->ops.type.extension = calloc(ext_count, sizeof(void *));
        EINA_SAFETY_ON_NULL_RETURN_VAL(desc->ops.type.extension, EINA_FALSE);

        for (itr = type; itr != NULL; itr = itr->parent)
          {
             unsigned cur_size = itr->type_size - sizeof(Eina_Model_Type);
             unsigned i, cur_count = cur_size / sizeof(void *);
             const void * const *ptr = (const void **)((const char *)itr + sizeof(Eina_Model_Type));

             if (cur_size == 0) break;

             for (i = 0; i < cur_count; i++)
               {
                  if (desc->ops.type.extension[i]) continue;
                  desc->ops.type.extension[i] = ptr[i];
               }
          }
     }

   desc->cache.types = malloc(count * sizeof(Eina_Model_Type *));
   EINA_SAFETY_ON_NULL_GOTO(desc->cache.types, cache_types_failed);
   desc->total.types = count;

   for (count = 0, itr = type; itr != NULL; itr = itr->parent, count++)
     desc->cache.types[count] = itr;

   return EINA_TRUE;

 cache_types_failed:
   free(desc->ops.type.extension);
   return EINA_FALSE;
}

static inline Eina_Bool
_eina_model_interface_implements(const Eina_Model_Interface *iface, const Eina_Model_Interface *query)
{
   const Eina_Model_Interface **itr;

   if (iface == query)
     return EINA_TRUE;

   if (!iface->interfaces)
     return EINA_FALSE;

   for (itr = iface->interfaces; *itr != NULL; itr++)
     if (_eina_model_interface_implements(*itr, query))
       return EINA_TRUE;

   return EINA_FALSE;
}

/* apply topological sort and remove duplicates */
/*
 * TODO: Topological sort will only work for linked interfaces, but
 *       will ignore original ordering provided by types. Consider the
 *       following:
 *
 *         - A_Type -> X_Iface (name: "MyIface")
 *         - B_Type -> Y_Iface (name: "MyIface")
 *
 *       Both X_Iface and Y_Iface are different implementations of the
 *       "MyIface".
 *
 *       B_Type inherits from A_Type, then Y_Iface must be looked up
 *       first, even though there is no link between Y_Iface and
 *       X_Iface.
 *
 *       However, the way the current topological sort behaves, the
 *       roots may come out in any order. We need a stable version
 *       that sorts roots before removing them from graph.
 *
 * Thanks to Tasn to report it :-)
 */
static Eina_Bool
_eina_model_description_ifaces_fix(Eina_Model_Description *desc)
{
   struct node {
      const Eina_Model_Interface *iface;
      unsigned int users;
      Eina_List *deps;
   } *nodes, **pending, **roots;
   unsigned int n_nodes = desc->total.ifaces, n_pending = 0, n_roots = 0, i, j;
   Eina_Bool ret = EINA_TRUE;

   nodes = alloca(n_nodes * sizeof(struct node));
   pending = alloca(n_nodes * sizeof(struct node *));
   roots = alloca(n_nodes * sizeof(struct node *));

   /* populate */
   for (i = 0, j = 0; i < n_nodes; i++)
     {
        unsigned int k;
        for (k = 0; k < j; k++)
          {
             if (nodes[k].iface == desc->cache.ifaces[i])
               break;
          }
        if (k < j)
          continue; /* already exists */

        nodes[j].iface = desc->cache.ifaces[i];
        nodes[j].users = 0;
        nodes[j].deps = NULL;
        j++;
     }
   n_nodes = j;

   for (i = 0; i < n_nodes; i++)
     {
        for (j = 0; j < n_nodes; j++)
          {
             if (i == j) continue;
             if (!_eina_model_interface_implements(nodes[j].iface,
                                                   nodes[i].iface))
               continue;

             nodes[i].users++;
             nodes[j].deps = eina_list_append(nodes[j].deps, nodes + i);
          }
     }
   for (i = 0; i < n_nodes; i++)
     {
        if (nodes[i].users == 0)
          {
             roots[n_roots] = nodes + i;
             n_roots++;
          }
        else
          {
             pending[n_pending] = nodes + i;
             n_pending++;
          }
     }

   /* topological sort */
   desc->total.ifaces = 0;
   while (n_roots > 0)
     {
        struct node *r, *d;

        /* TODO: sort roots using input order?  Or at least study if
         * it's enough to change roots append to prepend.
         *
         * See comments above.
         */
        n_roots--;
        r = roots[n_roots];

        desc->cache.ifaces[desc->total.ifaces] = r->iface;
        desc->total.ifaces++;

        EINA_LIST_FREE(r->deps, d)
          {
             d->users--;
             if (d->users > 0) continue;

             roots[n_roots] = d;
             n_roots++;

             /* remove node, it became a root */
             for (j = 0; j < n_pending; j++)
               {
                  if (pending[j] == d)
                    {
                       n_pending--;
                       if (j < n_pending)
                         pending[j] = pending[n_pending];
                       break;
                    }
               }
          }
     }

   if (n_pending > 0)
     {
        ERR("Dependency loop found for interfaces!");
        for (i = 0; i < n_pending; i++)
          ERR("%p (%s) is part of dependency loop!",
              pending[i]->iface, pending[i]->iface->name);
        CRITICAL("Cannot use type %p (%s) with broken interfaces!",
                 desc->cache.types[0], desc->cache.types[0]->name);
        free(desc->cache.ifaces);
        ret = EINA_FALSE;
     }

   /* likely from still pending (dependency loops) */
   for (i = 0; i < n_nodes; i++)
     eina_list_free(nodes[i].deps);

   return ret;
}

static Eina_Bool
_eina_model_description_ifaces_validate_and_count(const Eina_Model_Interface *iface, unsigned int *count)
{
   if (iface->version != EINA_MODEL_INTERFACE_VERSION)
     {
        CRITICAL("Interface %p version is %u, expected %u instead.",
                 iface, iface->version, EINA_MODEL_INTERFACE_VERSION);
        return EINA_FALSE;
     }

   if (!iface->name)
     {
        CRITICAL("Interface %p provides no name!", iface);
        return EINA_FALSE;
     }

   if (iface->interfaces)
     {
        const Eina_Model_Interface **itr = iface->interfaces;
        for (; *itr != NULL; itr++)
          if (!_eina_model_description_ifaces_validate_and_count(*itr, count))
            return EINA_FALSE;
     }

   (*count)++;
   return EINA_TRUE;
}

static void
_eina_model_description_ifaces_populate(Eina_Model_Description *desc, const Eina_Model_Interface *iface)
{
   desc->cache.ifaces[desc->total.ifaces] = iface;
   desc->total.ifaces++;

   if (iface->interfaces)
     {
        const Eina_Model_Interface **itr = iface->interfaces;
        for (; *itr != NULL; itr++)
          _eina_model_description_ifaces_populate(desc, *itr);
     }
}

static Eina_Bool
_eina_model_description_ifaces_fill(Eina_Model_Description *desc)
{
   const Eina_Model_Type **titr, **titr_end;
   unsigned int count;

   titr = desc->cache.types;
   titr_end = titr + desc->total.types;

   /* naively count all interfaces, remove duplicates later */
   for (count = 0; titr < titr_end; titr++)
     {
        const Eina_Model_Type *type = *titr;
        const Eina_Model_Interface **iitr = type->interfaces;
        if (!type->interfaces) continue;

        for (; *iitr != NULL; iitr++)
          if (!_eina_model_description_ifaces_validate_and_count(*iitr, &count))
            return EINA_FALSE;
     }
   if (count == 0)
     {
        desc->cache.ifaces = NULL;
        desc->total.ifaces = 0;
        return EINA_TRUE;
     }

   desc->cache.ifaces = malloc(count * sizeof(Eina_Model_Interface *));
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->cache.ifaces, EINA_FALSE);

   titr = desc->cache.types;
   desc->total.ifaces = 0;
   for (; titr < titr_end; titr++)
     {
        const Eina_Model_Type *type = *titr;
        const Eina_Model_Interface **iitr = type->interfaces;

        if (!type->interfaces) continue;

        for (; *iitr != NULL; iitr++)
          _eina_model_description_ifaces_populate(desc, *iitr);
     }

   return _eina_model_description_ifaces_fix(desc);
}

static Eina_Bool
_eina_model_description_privates_fill(Eina_Model_Description *desc)
{
   unsigned int i;

   desc->total.privates = desc->total.types + desc->total.ifaces;
   desc->cache.privates = malloc(desc->total.privates *
                                 sizeof(Eina_Model_Provider));
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->cache.privates, EINA_FALSE);

   desc->total.size = 0;

   for (i = 0; i < desc->total.types; i++)
     {
        const Eina_Model_Type *type = desc->cache.types[i];
        desc->cache.privates[i].type = type;
        if (type->private_size > 0)
          {
             unsigned int size = type->private_size;
             if (size % sizeof(void *) != 0)
               size += sizeof(void *) - (size % sizeof(void *));
             desc->total.size += size;
          }
     }

   for (i = 0; i < desc->total.ifaces; i++)
     {
        const Eina_Model_Interface *iface = desc->cache.ifaces[i];
        desc->cache.privates[desc->total.types + i].iface = iface;
        if (iface->private_size > 0)
          {
             unsigned int size = iface->private_size;
             if (size % sizeof(void *) != 0)
               size += sizeof(void *) - (size % sizeof(void *));
             desc->total.size += size;
          }
     }

   return EINA_TRUE;
}

static int
_eina_model_description_events_cmp(const void *pa, const void *pb)
{
   const Eina_Model_Event_Description_Cache *a = pa, *b = pb;
   return strcmp(a->name, b->name);
}

static int
_eina_model_description_events_find(const Eina_Model_Description *desc, const Eina_Model_Event_Description *query)
{
   unsigned int i;
   for (i = 0; i < desc->total.events; i++)
     {
        const Eina_Model_Event_Description_Cache *itr = desc->cache.events + i;
        if ((itr->name == query->name) || (strcmp(itr->name, query->name) == 0))
          return i;
     }

   return -1;
}

/* warn and remove duplicates, sort items to speed up lookups */
static Eina_Bool
_eina_model_description_events_fill(Eina_Model_Description *desc)
{
   unsigned int i, count = 0, type_events;

   for (i = 0; i < desc->total.types; i++)
     {
        const Eina_Model_Event_Description *itr = desc->cache.types[i]->events;
        if (!itr) continue;
        for (; itr->name != NULL; itr++)
          {
             count++;
          }
     }
   type_events = count;

   for (i = 0; i < desc->total.ifaces; i++)
     {
        const Eina_Model_Event_Description *itr = desc->cache.ifaces[i]->events;
        if (!itr) continue;
        for (; itr->name != NULL; itr++)
          count++;
     }

   if (count == 0)
     {
        desc->cache.events = NULL;
        desc->total.events = 0;
        return EINA_TRUE;
     }

   desc->cache.events = malloc(count *
                               sizeof(Eina_Model_Event_Description_Cache));
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->cache.events, EINA_FALSE);
   desc->total.events = 0;

   for (i = 0; i < desc->total.types; i++)
     {
        const Eina_Model_Type *mtype = desc->cache.types[i];
        const Eina_Model_Event_Description *itr = mtype->events;
        if (!itr) continue;
        for (; itr->name != NULL; itr++)
          {
             int j = _eina_model_description_events_find(desc, itr);
             if (j >= 0)
               {
                  const Eina_Model_Event_Description_Cache *o = desc->cache.events + j;
                  const Eina_Model_Type *omtype = o->provider.type;
                  WRN("Ignored duplicated event '%s' (type: '%s') from "
                      "model type %p (%s): already exists with type '%s' "
                      "from model type %p (%s)",
                      itr->name,
                      itr->type ? itr->type : "",
                      mtype, mtype->name,
                      o->desc->type ? o->desc->type : "",
                      omtype, omtype->name);
                  continue;
               }

             desc->cache.events[desc->total.events].name = itr->name;
             desc->cache.events[desc->total.events].desc = itr;
             desc->cache.events[desc->total.events].provider.type = mtype;
             desc->total.events++;
          }
     }

   for (i = 0; i < desc->total.ifaces; i++)
     {
        const Eina_Model_Interface *miface = desc->cache.ifaces[i];
        const Eina_Model_Event_Description *itr = desc->cache.ifaces[i]->events;
        if (!itr) continue;
        for (; itr->name != NULL; itr++)
          {
             int j = _eina_model_description_events_find(desc, itr);
             if (j >= 0)
               {
                  const Eina_Model_Event_Description_Cache *o = desc->cache.events + j;
                  if ((unsigned)j < type_events)
                    {
                       const Eina_Model_Type *omtype = o->provider.type;
                       WRN("Ignored duplicated event '%s' (type: '%s') from "
                           "model interface %p (%s): already exists with "
                           "type '%s' from model interface %p (%s)",
                           itr->name,
                           itr->type ? itr->type : "",
                           miface, miface->name,
                           o->desc->type ? o->desc->type : "",
                           omtype, omtype->name);
                    }
                  else
                    {
                       const Eina_Model_Interface *omiface = o->provider.iface;
                       WRN("Ignored duplicated event '%s' (iface: '%s') from "
                           "model interface %p (%s): already exists with "
                           "interface '%s' from model interface %p (%s)",
                           itr->name,
                           itr->type ? itr->type : "",
                           miface, miface->name,
                           o->desc->type ? o->desc->type : "",
                           omiface, omiface->name);
                    }
                  continue;
               }

             desc->cache.events[desc->total.events].name = itr->name;
             desc->cache.events[desc->total.events].desc = itr;
             desc->cache.events[desc->total.events].provider.iface = miface;
             desc->total.events++;
          }
     }

   qsort(desc->cache.events, desc->total.events,
         sizeof(Eina_Model_Event_Description_Cache),
         _eina_model_description_events_cmp);

   return EINA_TRUE;
}

static const Eina_Model_Description *
_eina_model_description_get_internal(const Eina_Model_Type *type)
{
   Eina_Model_Description *desc;

   desc = eina_hash_find(_eina_model_descriptions, &type);
   if (desc)
     {
        desc->refcount++;
        return desc;
     }

   desc = calloc(1, sizeof(Eina_Model_Description));
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);

   if (!_eina_model_description_type_fill(desc, type)) goto failed_type;
   if (!_eina_model_description_ifaces_fill(desc)) goto failed_ifaces;
   if (!_eina_model_description_privates_fill(desc)) goto failed_privates;
   if (!_eina_model_description_events_fill(desc)) goto failed_events;
   if (!eina_hash_add(_eina_model_descriptions, &type, desc)) goto failed_hash;

   desc->refcount = 1;
   return desc;

 failed_hash:
   free(desc->cache.events);
 failed_events:
   free(desc->cache.privates);
 failed_privates:
   free(desc->cache.ifaces);
 failed_ifaces:
   free(desc->cache.types);
   free(desc->ops.type.extension);
 failed_type:
   free(desc);
   return NULL;
}

static void
_eina_model_description_dispose_internal(Eina_Model_Description *desc)
{
   const Eina_Model_Type *type;

   EINA_SAFETY_ON_FALSE_RETURN(desc->refcount > 0);
   desc->refcount--;
   if (desc->refcount > 0) return;

   type = desc->cache.types[0];
   if (!eina_hash_del_by_key(_eina_model_descriptions, &type))
     ERR("Cannot find type %p (%s) in descriptions hash!",
         type, type->name);

   INF("Disposed model description for type %p (%s)", type, type->name);

   free(desc->ops.type.extension);
   free(desc->cache.types);
   free(desc->cache.ifaces);
   free(desc->cache.privates);
   free(desc->cache.events);
   free(desc);
}

static const Eina_Model_Description *
_eina_model_description_get(const Eina_Model_Type *type)
{
   const Eina_Model_Description *desc;

   eina_lock_take(&_eina_model_descriptions_lock);
   desc = _eina_model_description_get_internal(type);
   eina_lock_release(&_eina_model_descriptions_lock);

   return desc;
}

static void
_eina_model_description_dispose(const Eina_Model_Description *desc)
{
   eina_lock_take(&_eina_model_descriptions_lock);
   _eina_model_description_dispose_internal((Eina_Model_Description *)desc);
   eina_lock_release(&_eina_model_descriptions_lock);
}

static inline int
_eina_model_description_event_id_find(const Eina_Model_Description *desc, const char *event_name)
{
   const Eina_Model_Event_Description_Cache *cache;
   Eina_Model_Event_Description_Cache criteria_match;

   criteria_match.name = event_name;
   cache = bsearch(&criteria_match, desc->cache.events, desc->total.events,
                   sizeof(Eina_Model_Event_Description_Cache),
                   _eina_model_description_events_cmp);
   if (!cache)
     {
        ERR("No event named %s for type %p (%s)", event_name,
            desc->cache.types[0], desc->cache.types[0]->name);
        return -1;
     }

   return cache - desc->cache.events;
}

/*
 * Model management and book keeping
 */
typedef struct _Eina_Model_Event_Listener Eina_Model_Event_Listener;
struct _Eina_Model_Event_Listener
{
   EINA_INLIST;
   Eina_Model_Event_Cb cb;
   const void *data;
   Eina_Bool deleted:1;
};

struct _Eina_Model
{
   const Eina_Model_Description *desc; /**< optimized model description */
   struct {
      Eina_Inlist **entries; /**< connected/listeners for each event, array of lists of Eina_Model_Event_Listener */
      Eina_List **deleted; /**< deleted listeners while was walking. array of lists of Eina_Model_Event_Listener with deleted flag */
      int *freeze; /**< freeze count for each event */
      int walking; /**< increased while walking entries lists */
   } listeners;
   void **privates; /**< private data per type and interface, each level gets its own stuff */
   Eina_Inlist *xrefs; /**< if EINA_MODEL_DEBUG and eina_model_xref() is used */
   int refcount; /**< number of users of this model instance */
   Eina_Bool deleted:1; /**< if deleted but still have references */
   EINA_MAGIC
};

static inline Eina_Bool
_eina_model_type_check(const Eina_Model_Type *type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(type, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(type->version == EINA_MODEL_TYPE_VERSION,
                                   EINA_FALSE);
   return EINA_TRUE;
}

/* find in type hierarchy the first one that the given offset is not a null
 * pointer. Use this to discover which method to call on a parent.
 */
static const void *
_eina_model_type_find_offset(const Eina_Model_Type *type, unsigned int offset)
{
   const unsigned char *ptr = (const unsigned char *)type;
   const void **addr = (const void **)(ptr + offset);

   if (*addr) return *addr;
   if (!type->parent) return NULL;
   return _eina_model_type_find_offset(type->parent, offset);
}

/* find in interface hierarchy the first one that the given offset is
 * not a null pointer. Use this to discover which method to call on a
 * parent.
 *
 * TODO: Keep Eina_Model_Interface_Description with topological sorted
 *       entries for each interface?
 *       I smell problems with the current code in more complex
 *       situations (k-s)
 *
 *             iface1
 *               ^
 *               |
 *     .---------+---------.
 *     |         |         |
 *   iface2    iface3    iface4
 *     ^         ^         ^
 *     |         |         |
 *     `---------+---------'
 *               |
 *             iface5
 *
 * It should look: iface5 -> iface2 -> iface3 -> iface4 -> iface1
 * Now it does: iface5 -> iface2 -> iface1 -> iface3 -> iface1 -> iface4 -> iface1
 *
 *
 *             iface1
 *               ^
 *               |
 *             iface2
 *               ^
 *               |
 *     .---------+---------.
 *     |                   |
 *   iface3              iface4
 *     ^                   ^
 *     |                   |
 *     `---------+---------'
 *               |
 *             iface5
 *
 * It should look: iface5 -> iface3 -> iface4 -> iface2 -> iface1
 * Now it does: iface5 -> iface3 -> iface2 -> iface1 -> iface4 -> iface2 -> iface1
 *
 *
 *   iface1              iface2
 *     ^                   ^
 *     |                   |
 *     `---------+---------'
 *               |
 *             iface3
 *
 * It should look: iface3 -> iface1 -> iface2
 * Now it does: iface3 -> iface1 -> iface2
 *
 * For the common case it should work, let's see.
 */
static const void *
_eina_model_interface_find_offset(const Eina_Model_Interface *iface, unsigned int offset)
{
   const Eina_Model_Interface **itr;
   const unsigned char *ptr = (const unsigned char *)iface;
   const void **addr = (const void **)(ptr + offset);

   if (offset + sizeof(void *) > iface->interface_size) return NULL;

   if (*addr) return *addr;
   if (!iface->interfaces) return NULL;

   for (itr = iface->interfaces; *itr != NULL; itr++)
     {
        const void *r = _eina_model_interface_find_offset(*itr, offset);
        if (r)
          return r;
     }

   return NULL;
}

static void
_eina_model_event_callback_free_deleted(Eina_Model *model)
{
   unsigned int i;

   for (i = 0; i < model->desc->total.events; i++)
     {
        Eina_Model_Event_Listener *el;
        EINA_LIST_FREE(model->listeners.deleted[i], el)
          {
             model->listeners.entries[i] = eina_inlist_remove
               (model->listeners.entries[i], EINA_INLIST_GET(el));
             _eina_model_inner_free(sizeof(Eina_Model_Event_Listener), el);
          }
     }

   _eina_model_inner_free(model->desc->total.events * sizeof(Eina_List *),
                          model->listeners.deleted);
   model->listeners.deleted = NULL;
}

static inline Eina_Bool
_eina_model_event_callback_call(Eina_Model *model, const char *name, const void *event_info)
{
   Eina_Inlist *lst;
   Eina_Model_Event_Listener *el;
   const Eina_Model_Event_Description *ev_desc;
   int event_id = _eina_model_description_event_id_find(model->desc, name);

   if (event_id < 0) return EINA_FALSE;
   if (!model->listeners.entries) return EINA_TRUE;

   if ((model->listeners.freeze) && (model->listeners.freeze[event_id]))
     {
        DBG("Ignored event callback '%s' of model %p (%s): frozen",
            name, model, model->desc->cache.types[0]->name);
        return EINA_TRUE;
     }

   lst = model->listeners.entries[event_id];
   if (!lst) return EINA_TRUE;

   ev_desc = model->desc->cache.events[event_id].desc;

   model->listeners.walking++;
   EINA_INLIST_FOREACH(lst, el)
     {
        if (el->deleted) continue;
        el->cb((void *)el->data, model, ev_desc, (void *)event_info);
     }
   model->listeners.walking--;

   if ((model->listeners.walking == 0) && (model->listeners.deleted))
     _eina_model_event_callback_free_deleted(model);

   return EINA_FALSE;
}

static const char EINA_ERROR_MODEL_FAILED_STR[] = "Model check failed.";
static const char EINA_ERROR_MODEL_METHOD_MISSING_STR[] = "Model method is missing.";
static const char EINA_MAGIC_MODEL_STR[] = "Eina Model";

static void _eina_model_unref(Eina_Model *model);

/**
 * @endcond
 */

/* EINA_MODEL_TYPE_BASE: base of all other types **********************/

static Eina_Bool
_eina_model_type_base_setup(Eina_Model *model)
{
   DBG("base setup of %p", model);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_base_flush(Eina_Model *model)
{
   DBG("base flush of %p", model);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_base_constructor(Eina_Model *model)
{
   DBG("base constructor of %p", model);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_base_destructor(Eina_Model *model)
{
   DBG("base destructor of %p", model);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_base_properties_copy(const Eina_Model *model, Eina_Model *copy)
{
   Eina_List *l, *props = eina_model_properties_names_list_get(model);
   const char *name;
   EINA_LIST_FOREACH(props, l, name)
     {
        Eina_Value tmp;
        if (!eina_model_property_get(model, name, &tmp))
          {
             ERR("Could not get property %s from model %p (%s)",
                 name, model, model->desc->cache.types[0]->name);
             eina_model_properties_names_list_free(props);
             return EINA_FALSE;
          }
        if (!eina_model_property_set(copy, name, &tmp))
          {
             ERR("Could not set property %s on model %p (%s)",
                 name, copy, copy->desc->cache.types[0]->name);
             eina_value_flush(&tmp);
             eina_model_properties_names_list_free(props);
             return EINA_FALSE;
          }
        eina_value_flush(&tmp);
     }
   eina_model_properties_names_list_free(props);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_base_children_copy(const Eina_Model *model, Eina_Model *copy)
{
   int i, count = eina_model_child_count(model);

   if (count < 0)
     {
        ERR("Could not get children count of model %p (%s)",
            model, model->desc->cache.types[0]->name);
        return EINA_FALSE;
     }

   for (i = 0; i < count; i++)
     {
        Eina_Model *child = eina_model_child_get(model, i);
        Eina_Bool ret;

        if (!child)
          {
             ERR("Could not get child #%d from model %p (%s)",
                 i, model, model->desc->cache.types[0]->name);
             return EINA_FALSE;
          }

        ret = eina_model_child_insert_at(copy, i, child);
        _eina_model_unref(child);

        if (!ret)
          {
             ERR("Could not set child #%d on model %p (%s)",
                 i, copy, copy->desc->cache.types[0]->name);
             return EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_base_copy(const Eina_Model *model, Eina_Model *copy)
{
   DBG("base copy of %p to %p", model, copy);

   return _eina_model_type_base_properties_copy(model, copy) &&
     _eina_model_type_base_children_copy(model, copy);
}

static Eina_Bool
_eina_model_type_base_children_deep_copy(const Eina_Model *model, Eina_Model *copy)
{
   int i, count = eina_model_child_count(model);

   if (count < 0)
     {
        ERR("Could not get children count of model %p (%s)",
            model, model->desc->cache.types[0]->name);
        return EINA_FALSE;
     }

   for (i = 0; i < count; i++)
     {
        Eina_Model *child_copy, *child = eina_model_child_get(model, i);
        Eina_Bool ret;

        if (!child)
          {
             ERR("Could not get child #%d from model %p (%s)",
                 i, model, model->desc->cache.types[0]->name);
             return EINA_FALSE;
          }

        child_copy = eina_model_deep_copy(child);
        if (!child_copy)
          {
             ERR("Could not deep copy child #%d %p (%s) from model %p (%s)", i,
                 child, child->desc->cache.types[0]->name,
                 model, model->desc->cache.types[0]->name);
             _eina_model_unref(child);
             return EINA_FALSE;
          }
        _eina_model_unref(child);

        ret = eina_model_child_insert_at(copy, i, child_copy);
        _eina_model_unref(child_copy);

        if (!ret)
          {
             ERR("Could not set child #%d on model %p (%s)",
                 i, copy, copy->desc->cache.types[0]->name);
             return EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_base_deep_copy(const Eina_Model *model, Eina_Model *copy)
{
   DBG("base deep copy of %p to %p", model, copy);

   return _eina_model_type_base_properties_copy(model, copy) &&
     _eina_model_type_base_children_deep_copy(model, copy);
}

static Eina_Bool
_eina_model_type_base_properties_compare(const Eina_Model *a, const Eina_Model *b, int *cmp)
{
   Eina_List *al, *aprops = eina_model_properties_names_list_get(a);
   Eina_List *bl, *bprops = eina_model_properties_names_list_get(b);
   Eina_List *l, *props = NULL;
   const char *aname, *bname, *name;
   Eina_Bool ret = EINA_TRUE;

   EINA_LIST_FOREACH(aprops, al, aname)
     {
        EINA_LIST_FOREACH(bprops, bl, bname)
          if (strcmp(aname, bname) == 0)
            {
               props = eina_list_append(props, aname);
               break;
            }
     }

   *cmp = 0;
   EINA_LIST_FOREACH(props, l, name)
     {
        Eina_Value atmp, btmp;

        if (!eina_model_property_get(a, name, &atmp))
          {
             ERR("Could not get property %s from model %p (%s)",
                 name, a, a->desc->cache.types[0]->name);
             ret = EINA_FALSE;
             *cmp = -1;
             break;
          }

        if (!eina_model_property_get(b, name, &btmp))
          {
             ERR("Could not get property %s from model %p (%s)",
                 name, b, b->desc->cache.types[0]->name);
             ret = EINA_FALSE;
             *cmp = -1;
             eina_value_flush(&atmp);
             break;
          }

        *cmp = eina_value_compare(&atmp, &btmp);
        if (eina_error_get() != 0)
          {
             char *astr = eina_value_to_string(&atmp);
             char *bstr = eina_value_to_string(&btmp);
             ERR("Could not compare property %s: %s=%s, %s=%s", name,
                 eina_value_type_name_get(eina_value_type_get(&atmp)), astr,
                 eina_value_type_name_get(eina_value_type_get(&btmp)), bstr);
             free(astr);
             free(bstr);
             ret = EINA_FALSE;
             *cmp = -1;
          }

        eina_value_flush(&atmp);
        eina_value_flush(&btmp);

        if ((!ret) || (*cmp != 0))
          break;
     }

   if ((ret) && (*cmp == 0))
     {
        int acount = eina_list_count(aprops);
        int bcount = eina_list_count(bprops);

        if (acount < bcount)
          *cmp = -1;
        else if (acount > bcount)
          *cmp = 1;
     }

   eina_model_properties_names_list_free(aprops);
   eina_model_properties_names_list_free(bprops);
   eina_list_free(props);
   return ret;
}

static Eina_Bool
_eina_model_type_base_children_compare(const Eina_Model *a, const Eina_Model *b, int *cmp)
{
   int acount = eina_model_child_count(a);
   int bcount = eina_model_child_count(b);
   int i, count;
   Eina_Bool ret = EINA_TRUE;

   if (acount < 0)
     {
        ERR("Could not get children count of model %p (%s)",
            a, a->desc->cache.types[0]->name);
        return EINA_FALSE;
     }
   if (bcount < 0)
     {
        ERR("Could not get children count of model %p (%s)",
            b, b->desc->cache.types[0]->name);
        return EINA_FALSE;
     }

   if (acount < bcount)
     count = acount;
   else
     count = bcount;

   for (i = 0; i < count; i++)
     {
        Eina_Model *achild, *bchild;

        achild = eina_model_child_get(a, i);
        if (!achild)
          {
             ERR("Could not get child #%d from model %p (%s)",
                 i, a, a->desc->cache.types[0]->name);
             *cmp = -1;
             return EINA_FALSE;
          }

        bchild = eina_model_child_get(b, i);
        if (!bchild)
          {
             ERR("Could not get child #%d from model %p (%s)",
                 i, b, b->desc->cache.types[0]->name);
             *cmp = -1;
             _eina_model_unref(achild);
             return EINA_FALSE;
          }

        *cmp = eina_model_compare(achild, bchild);
        if (eina_error_get())
          {
             ERR("Could not compare children #%d %p (%s) and %p (%s) "
                 "from models %p (%s) and %p (%s)", i,
                 achild,
                 eina_model_type_name_get(eina_model_type_get(achild)),
                 bchild,
                 eina_model_type_name_get(eina_model_type_get(bchild)),
                 a, a->desc->cache.types[0]->name,
                 b, b->desc->cache.types[0]->name);
             ret = EINA_FALSE;
          }
        _eina_model_unref(achild);
        _eina_model_unref(bchild);

        if ((!ret) || (*cmp != 0))
          break;
     }

   if ((ret) && (*cmp == 0))
     {
        if (acount < bcount)
          *cmp = -1;
        else if (acount > bcount)
          *cmp = 1;
     }

   return ret;
}

static Eina_Bool
_eina_model_type_base_compare(const Eina_Model *a, const Eina_Model *b, int *cmp)
{
   *cmp = 0;
   DBG("base compare of %p and %p", a, b);

   if (!_eina_model_type_base_properties_compare(a, b, cmp))
     return EINA_FALSE;

   if (*cmp != 0)
     return EINA_TRUE;

   return _eina_model_type_base_children_compare(a, b, cmp);
}

static int
_eina_model_type_base_child_count(const Eina_Model *model)
{
   DBG("base child_count of %p", model);
   return 0;
}

static int
_eina_model_type_base_child_find(const Eina_Model *model, unsigned int start_position, const Eina_Model *other)
{
   int x = eina_model_child_count(model);
   unsigned int i, count;

   DBG("base child_find of %p, %d children", model, x);

   if (x < 0)
     return -1;

   count = x;
   for (i = start_position; i < count; i++)
     {
        Eina_Model *current = eina_model_child_get(model, i);
        if (current)
          {
             _eina_model_unref(current); /* we'll not use it's value anyway */
             if (current == other)
               return i;
          }
     }

   return -1;
}

static int
_eina_model_type_base_child_criteria_match(const Eina_Model *model, unsigned int start_position, Eina_Each_Cb match, const void *user_data)
{
   int x = eina_model_child_count(model);
   unsigned int i, count;

   DBG("base child_criteria_match of %p, %d children", model, x);

   if (x < 0)
     return -1;

   count = x;
   for (i = start_position; i < count; i++)
     {
        Eina_Model *current = eina_model_child_get(model, i);
        if (current)
          {
             Eina_Bool r = match(model, current, (void *)user_data);
             _eina_model_unref(current);
             if (r)
               return i;
          }
     }

   return -1;
}

typedef struct _Eina_Iterator_Model_Base Eina_Iterator_Model_Base;
struct _Eina_Iterator_Model_Base
{
   Eina_Iterator base;
   Eina_Model *model;
   unsigned int current;
   unsigned int end;
};

static Eina_Bool
_eina_model_type_base_child_iterator_next(Eina_Iterator *base, void **data)
{
   Eina_Iterator_Model_Base *it;

   it = (Eina_Iterator_Model_Base *)base;
   if (it->current >= it->end)
     return EINA_FALSE;

   *data = eina_model_child_get(it->model, it->current);
   if (!*data)
     return EINA_FALSE;

   it->current++;
   return EINA_TRUE;
}

static void *
_eina_model_type_base_child_iterator_get_container(Eina_Iterator *base)
{
   Eina_Iterator_Model_Base *it;
   it = (Eina_Iterator_Model_Base *)base;
   return it->model;
}

static void
_eina_model_type_base_child_iterator_free(Eina_Iterator *base)
{
   Eina_Iterator_Model_Base *it;
   it = (Eina_Iterator_Model_Base *)base;
   eina_model_xunref(it->model, it);
   free(it);
}

static Eina_Iterator *
_eina_model_type_base_child_iterator_get(Eina_Model *model, unsigned int start, unsigned int count)
{
   Eina_Iterator_Model_Base *it = calloc(1, sizeof(*it));
   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);

   EINA_MAGIC_SET(&it->base, EINA_MAGIC_ITERATOR);
   it->base.version = EINA_ITERATOR_VERSION;
   it->base.next = _eina_model_type_base_child_iterator_next;
   it->base.get_container = _eina_model_type_base_child_iterator_get_container;
   it->base.free = _eina_model_type_base_child_iterator_free;

   it->model = eina_model_xref(model, it, "eina_model_child_slice_iterator_get");
   it->current = start;
   it->end = start + count;

   return &it->base;
}

typedef struct _Eina_Iterator_Model_Base_Reversed Eina_Iterator_Model_Base_Reversed;
struct _Eina_Iterator_Model_Base_Reversed
{
   Eina_Iterator base;
   Eina_Model *model;
   unsigned int current;
   unsigned int end;
};

static Eina_Bool
_eina_model_type_base_child_reversed_iterator_next(Eina_Iterator *base, void **data)
{
   Eina_Iterator_Model_Base_Reversed *it;

   it = (Eina_Iterator_Model_Base_Reversed *)base;
   if (it->current == it->end)
     return EINA_FALSE;

   it->current--;
   *data = eina_model_child_get(it->model, it->current);
   if (!*data)
     return EINA_FALSE;

   return EINA_TRUE;
}

static void *
_eina_model_type_base_child_reversed_iterator_get_container(Eina_Iterator *base)
{
   Eina_Iterator_Model_Base_Reversed *it;
   it = (Eina_Iterator_Model_Base_Reversed *)base;
   return it->model;
}

static void
_eina_model_type_base_child_reversed_iterator_free(Eina_Iterator *base)
{
   Eina_Iterator_Model_Base_Reversed *it;
   it = (Eina_Iterator_Model_Base_Reversed *)base;
   eina_model_xunref(it->model, it);
   free(it);
}

static Eina_Iterator *
_eina_model_type_base_child_reversed_iterator_get(Eina_Model *model, unsigned int start, unsigned int count)
{
   Eina_Iterator_Model_Base_Reversed *it;
   int children_count;

   children_count = eina_model_child_count(model);
   if (children_count < 0)
     return NULL;

   if (start + count > (unsigned int)children_count)
     {
        if (start >= (unsigned int)children_count)
          count = 0;
        else
          count = children_count - start;
     }

   it = calloc(1, sizeof(*it));
   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);
   EINA_MAGIC_SET(&it->base, EINA_MAGIC_ITERATOR);
   it->base.version = EINA_ITERATOR_VERSION;
   it->base.next = _eina_model_type_base_child_reversed_iterator_next;
   it->base.get_container = _eina_model_type_base_child_reversed_iterator_get_container;
   it->base.free = _eina_model_type_base_child_reversed_iterator_free;

   it->model = eina_model_xref(model, it, "eina_model_child_slice_reversed_iterator_get");
   it->current = start + count;
   it->end = start;

   return &it->base;
}

typedef struct _Eina_Iterator_Model_Base_Sorted Eina_Iterator_Model_Base_Sorted;
struct _Eina_Iterator_Model_Base_Sorted
{
   Eina_Iterator base;
   Eina_Model *model;
   unsigned int current;
   unsigned int count;
   Eina_Model *elements[];
};

static Eina_Bool
_eina_model_type_base_child_sorted_iterator_next(Eina_Iterator *base, void **data)
{
   Eina_Iterator_Model_Base_Sorted *it;

   it = (Eina_Iterator_Model_Base_Sorted *)base;
   if (it->current == it->count)
     return EINA_FALSE;

   *data = eina_model_ref(it->elements[it->current]);
   it->current++;
   return EINA_TRUE;
}

static void *
_eina_model_type_base_child_sorted_iterator_get_container(Eina_Iterator *base)
{
   Eina_Iterator_Model_Base_Sorted *it;
   it = (Eina_Iterator_Model_Base_Sorted *)base;
   return it->model;
}

static void
_eina_model_type_base_child_sorted_iterator_free(Eina_Iterator *base)
{
   Eina_Iterator_Model_Base_Sorted *it;
   unsigned int i;
   it = (Eina_Iterator_Model_Base_Sorted *)base;
   eina_model_xunref(it->model, it);

   for (i = 0; i < it->count; i++)
     _eina_model_unref(it->elements[i]);

   free(it);
}

static Eina_Iterator *
_eina_model_type_base_child_sorted_iterator_get(Eina_Model *model, unsigned int start, unsigned int count, Eina_Compare_Cb compare)
{
   Eina_Iterator_Model_Base_Sorted *it;
   int children_count;
   unsigned int i;

   children_count = eina_model_child_count(model);
   if (children_count < 0)
     return NULL;

   if (start + count > (unsigned int)children_count)
     {
        if (start >= (unsigned int)children_count)
          count = 0;
        else
          count = children_count - start;
     }

   it = calloc(1, sizeof(*it) + count * sizeof(Eina_Model *));
   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);
   EINA_MAGIC_SET(&it->base, EINA_MAGIC_ITERATOR);
   it->base.version = EINA_ITERATOR_VERSION;
   it->base.next = _eina_model_type_base_child_sorted_iterator_next;
   it->base.get_container = _eina_model_type_base_child_sorted_iterator_get_container;
   it->base.free = _eina_model_type_base_child_sorted_iterator_free;

   it->model = eina_model_xref(model, it, "eina_model_child_slice_sorted_iterator_get");
   it->current = 0;
   it->count = count;

   for (i = 0; i < count; i++)
     {
        it->elements[i] = eina_model_child_get(model, i + start);
        if (!it->elements[i])
          {
             ERR("Failed to get child %u of model %p (%s)",
                 i + start, model, model->desc->cache.types[0]->name);
             free(it);
             return NULL;
          }
     }

   if (count > 1)
     _eina_model_array_sort(it->elements, 0, count - 1, compare);

   return &it->base;
}

typedef struct _Eina_Iterator_Model_Base_Filtered Eina_Iterator_Model_Base_Filtered;
struct _Eina_Iterator_Model_Base_Filtered
{
   Eina_Iterator base;
   Eina_Model *model;
   Eina_Each_Cb match;
   const void *data;
   unsigned int current;
   unsigned int count;
};

static Eina_Bool
_eina_model_type_base_child_filtered_iterator_next(Eina_Iterator *base, void **data)
{
   Eina_Iterator_Model_Base_Filtered *it;
   unsigned int *ret;
   int i;

   it = (Eina_Iterator_Model_Base_Filtered *)base;
   if (it->count == 0) return EINA_FALSE;

   i = eina_model_child_criteria_match(it->model, it->current, it->match, it->data);
   if (i < 0) return EINA_FALSE;

   it->current = i + 1;
   it->count--;
   ret = (unsigned int *)data;
   *ret = i;
   return EINA_TRUE;
}

static void *
_eina_model_type_base_child_filtered_iterator_get_container(Eina_Iterator *base)
{
   Eina_Iterator_Model_Base_Filtered *it;
   it = (Eina_Iterator_Model_Base_Filtered *)base;
   return it->model;
}

static void
_eina_model_type_base_child_filtered_iterator_free(Eina_Iterator *base)
{
   Eina_Iterator_Model_Base_Filtered *it;
   it = (Eina_Iterator_Model_Base_Filtered *)base;
   eina_model_xunref(it->model, it);
   free(it);
}

static Eina_Iterator *
_eina_model_type_base_child_filtered_iterator_get(Eina_Model *model, unsigned int start, unsigned int count, Eina_Each_Cb match, const void *data)
{
   Eina_Iterator_Model_Base_Filtered *it = calloc(1, sizeof(*it));
   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);

   EINA_MAGIC_SET(&it->base, EINA_MAGIC_ITERATOR);
   it->base.version = EINA_ITERATOR_VERSION;
   it->base.next = _eina_model_type_base_child_filtered_iterator_next;
   it->base.get_container = _eina_model_type_base_child_filtered_iterator_get_container;
   it->base.free = _eina_model_type_base_child_filtered_iterator_free;

   it->model = eina_model_xref(model, it, "eina_model_child_slice_filtered_iterator_get");
   it->match = match;
   it->data = data;
   it->current = start;
   it->count = count;

   return &it->base;
}

static char *
_eina_model_type_base_to_string(const Eina_Model *model)
{
   Eina_List *l, *props;
   const char *name;
   Eina_Strbuf *str;
   Eina_Bool first;
   int i, count;
   char *ret;

   str = eina_strbuf_new();
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, NULL);

   eina_strbuf_append_printf(str, "%s({", model->desc->cache.types[0]->name);

   props = eina_model_properties_names_list_get(model);
   props = eina_list_sort(props, 0, EINA_COMPARE_CB(strcmp));

   first = EINA_TRUE;
   EINA_LIST_FOREACH(props, l, name)
     {
        Eina_Value val;

        if (!first)
          eina_strbuf_append_printf(str, ", %s: ", name);
        else
          {
             eina_strbuf_append_printf(str, "%s: ", name);
             first = EINA_FALSE;
          }

        if (!eina_model_property_get(model, name, &val))
          eina_strbuf_append_char(str, '?');
        else
          {
             char *tmp = eina_value_to_string(&val);
             eina_strbuf_append(str, tmp ? tmp : "?");
             free(tmp);
             eina_value_flush(&val);
          }
     }
   eina_list_free(props);

   eina_strbuf_append(str, "}, [");

   count = eina_model_child_count(model);
   first = EINA_TRUE;
   for (i = 0; i < count; i++)
     {
        Eina_Model *c = eina_model_child_get(model, i);
        if (!c)
          {
             if (!first)
               eina_strbuf_append(str, ", ?");
             else
               {
                  eina_strbuf_append_char(str, '?');
                  first = EINA_FALSE;
               }
          }
        else
          {
             char *tmp = eina_model_to_string(c);
             if (!first)
               eina_strbuf_append_printf(str, ", %s", tmp ? tmp : "?");
             else
               {
                  eina_strbuf_append(str, tmp ? tmp : "?");
                  first = EINA_FALSE;
               }
             free(tmp);
             _eina_model_unref(c);
          }
     }

   eina_strbuf_append(str, "])");

   ret = eina_strbuf_string_steal(str);
   eina_strbuf_free(str);

   return ret;
}

static const Eina_Model_Event_Description _eina_model_type_base_events[] = {
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_deleted, "", "model was deleted"),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_freed, "", "model memory was released"),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_property_set, "s", "model data was set, data name given as event information."),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_property_del, "s", "model data was deleted, data name given as event information."),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_children_changed, "", "model children changed (deleted, inserted)."),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_child_inserted, "u", "model child was inserted, child position is given."),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_child_set, "u", "model child was set, child position is given."),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_child_del, "u", "model child was deleted, child position is given."),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_loaded, "", "model was loaded"),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_unloaded, "", "model was unloaded"),
  EINA_MODEL_EVENT_DESCRIPTION_SENTINEL
};

static const Eina_Model_Type _EINA_MODEL_TYPE_BASE = {
  EINA_MODEL_TYPE_VERSION,
  0, /* there is no private data */
  sizeof(Eina_Model_Type),
  "Eina_Model_Type_Base",
  NULL, /* should be the only type with NULL here! */
  NULL, /* no interfaces implemented */
  _eina_model_type_base_events,
  _eina_model_type_base_setup,
  _eina_model_type_base_flush,
  _eina_model_type_base_constructor,
  _eina_model_type_base_destructor,
  _eina_model_type_base_copy,
  _eina_model_type_base_deep_copy,
  _eina_model_type_base_compare,
  NULL, /* no load */
  NULL, /* no unload */
  NULL, /* no property value get */
  NULL, /* no property value set */
  NULL, /* no property del */
  NULL, /* no properties names list */
  _eina_model_type_base_child_count,
  NULL, /* no child get */
  NULL, /* no child set */
  NULL, /* no child del */
  NULL, /* no child insert */
  _eina_model_type_base_child_find,
  _eina_model_type_base_child_criteria_match,
  NULL, /* no child sort */
  _eina_model_type_base_child_iterator_get,
  _eina_model_type_base_child_reversed_iterator_get,
  _eina_model_type_base_child_sorted_iterator_get,
  _eina_model_type_base_child_filtered_iterator_get,
  _eina_model_type_base_to_string,
  NULL, /* extension pointer */
  NULL, /* extension pointer */
  NULL, /* extension pointer */
  NULL  /* extension pointer */
};

/*
 * EINA_MODEL_TYPE_MIXIN:
 *
 * Mix-in is a type that uses 2 interfaces, one for properties,
 * another for children. Users should inherit this model and implement
 * at least onf of the interfaces to get an usable model without
 * defining the methods.
 */

static const char _EINA_MODEL_INTERFACE_NAME_PROPERTIES[] = "Eina_Model_Interface_Properties";
static const char _EINA_MODEL_INTERFACE_NAME_CHILDREN[] = "Eina_Model_Interface_Children";

typedef struct _Eina_Model_Type_Mixin_Data Eina_Model_Type_Mixin_Data;
struct _Eina_Model_Type_Mixin_Data
{
   /* just keep interfaces to avoid lookups */
   const Eina_Model_Interface *if_properties;
   const Eina_Model_Interface *if_children;
};

static Eina_Bool
_eina_model_type_mixin_setup(Eina_Model *model)
{
   DBG("mix-in setup of %p", model);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_mixin_flush(Eina_Model *model)
{
   DBG("mix-in flush of %p", model);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_mixin_constructor(Eina_Model *model)
{
   Eina_Model_Type_Mixin_Data *priv = eina_model_type_private_data_get
     (model, EINA_MODEL_TYPE_MIXIN);

   DBG("mix-in constructor of %p (priv=%p)", model, priv);

   priv->if_properties = eina_model_interface_get
     (model, EINA_MODEL_INTERFACE_NAME_PROPERTIES);
   if (priv->if_properties)
     {
        if (!eina_model_interface_constructor(priv->if_properties, model))
          {
             ERR("Could not construct properties interface %p of %p (%s)",
                 model, priv->if_properties, model->desc->cache.types[0]->name);
             return EINA_FALSE;
          }
     }

   priv->if_children = eina_model_interface_get
     (model, EINA_MODEL_INTERFACE_NAME_CHILDREN);
   if (priv->if_children)
     {
        if (!eina_model_interface_constructor(priv->if_children, model))
          {
             ERR("Could not construct children interface %p of %p (%s)",
                 model, priv->if_children, model->desc->cache.types[0]->name);
             return EINA_FALSE;
          }
     }

   if ((!priv->if_properties) && (!priv->if_children))
     {
        ERR("Mix-in model %p (%s) does not implement properties or children "
            "interfaces!",
            model, model->desc->cache.types[0]->name);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

#define EINA_MODEL_TYPE_MIXIN_GET(model)                                \
  Eina_Model_Type_Mixin_Data *priv = eina_model_type_private_data_get   \
    (model, EINA_MODEL_TYPE_MIXIN)

static Eina_Bool
_eina_model_type_mixin_destructor(Eina_Model *model)
{
   EINA_MODEL_TYPE_MIXIN_GET(model);

   DBG("mixin destructor of %p", model);

   if (priv->if_properties)
     eina_model_interface_destructor(priv->if_properties, model);

   if (priv->if_children)
     eina_model_interface_destructor(priv->if_children, model);

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_type_mixin_compare(const Eina_Model *a, const Eina_Model *b, int *cmp)
{
   Eina_Bool ret = EINA_TRUE, did_prop = EINA_FALSE, did_child = EINA_FALSE;

   *cmp = 0;

   EINA_MODEL_TYPE_MIXIN_GET(a);

   if (priv->if_properties)
     {
        Eina_Bool (*compare)(const Eina_Model*, const Eina_Model*, int *) =
          _eina_model_interface_find_offset
          (priv->if_properties,
           offsetof(Eina_Model_Interface_Properties, compare));

        if (compare)
          {
             ret &= compare(a, b, cmp);
             did_prop = EINA_TRUE;
          }
     }

   if ((ret) && (*cmp == 0))
     {
        if (priv->if_children)
          {
             Eina_Bool (*compare)(const Eina_Model*, const Eina_Model*, int *) =
               _eina_model_interface_find_offset
               (priv->if_children,
                offsetof(Eina_Model_Interface_Children, compare));

             if (compare)
               {
                  ret &= compare(a, b, cmp);
                  did_child = EINA_TRUE;
               }
          }
     }

   if ((!did_prop) && (!did_child))
     return eina_model_type_compare(EINA_MODEL_TYPE_BASE, a, b, cmp);

   return ret;
}

static Eina_Bool
_eina_model_type_mixin_load(Eina_Model *model)
{
   Eina_Bool ret = EINA_TRUE;

   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (priv->if_properties)
     ret &= eina_model_interface_properties_load(priv->if_properties, model);

   if (priv->if_children)
     ret &= eina_model_interface_children_load(priv->if_children, model);

   return ret;
}

static Eina_Bool
_eina_model_type_mixin_unload(Eina_Model *model)
{
   Eina_Bool ret = EINA_TRUE;

   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (priv->if_properties)
     ret &= eina_model_interface_properties_unload(priv->if_properties, model);

   if (priv->if_children)
     ret &= eina_model_interface_children_unload(priv->if_children, model);

   return ret;
}

static Eina_Bool
_eina_model_type_mixin_property_get(const Eina_Model *model, const char *name, Eina_Value *value)
{
   Eina_Bool ret = EINA_FALSE;

   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (priv->if_properties)
     ret = eina_model_interface_properties_get
       (priv->if_properties, model, name, value);

   return ret;
}

static Eina_Bool
_eina_model_type_mixin_property_set(Eina_Model *model, const char *name, const Eina_Value *value)
{
   Eina_Bool ret = EINA_FALSE;

   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (priv->if_properties)
     ret = eina_model_interface_properties_set
       (priv->if_properties, model, name, value);

   return ret;
}

static Eina_Bool
_eina_model_type_mixin_property_del(Eina_Model *model, const char *name)
{
   Eina_Bool ret = EINA_FALSE;

   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (priv->if_properties)
     ret = eina_model_interface_properties_del
       (priv->if_properties, model, name);

   return ret;
}

static Eina_List *
_eina_model_type_mixin_properties_names_list_get(const Eina_Model *model)
{
   Eina_List *ret = NULL;

   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (priv->if_properties)
     ret = eina_model_interface_properties_names_list_get
       (priv->if_properties, model);

   return ret;
}

static int
_eina_model_type_mixin_child_count(const Eina_Model *model)
{
   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (!priv->if_children)
     return 0;

   return eina_model_interface_children_count(priv->if_children, model);
}

static Eina_Model *
_eina_model_type_mixin_child_get(const Eina_Model *model, unsigned int position)
{
   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (!priv->if_children)
     return 0;

   return eina_model_interface_children_get(priv->if_children, model, position);
}

static Eina_Bool
_eina_model_type_mixin_child_set(Eina_Model *model, unsigned int position, Eina_Model *child)
{
   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (!priv->if_children)
     return 0;

   return eina_model_interface_children_set
     (priv->if_children, model, position, child);
}

static Eina_Bool
_eina_model_type_mixin_child_del(Eina_Model *model, unsigned int position)
{
   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (!priv->if_children)
     return 0;

   return eina_model_interface_children_del
     (priv->if_children, model, position);
}

static Eina_Bool
_eina_model_type_mixin_child_insert_at(Eina_Model *model, unsigned int position, Eina_Model *child)
{
   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (!priv->if_children)
     return 0;

   return eina_model_interface_children_insert_at
     (priv->if_children, model, position, child);
}

static void
_eina_model_type_mixin_child_sort(Eina_Model *model, Eina_Compare_Cb compare)
{
   EINA_MODEL_TYPE_MIXIN_GET(model);

   if (!priv->if_children)
     return;
   eina_model_interface_children_sort(priv->if_children, model, compare);
}

static const Eina_Model_Type _EINA_MODEL_TYPE_MIXIN = {
  EINA_MODEL_TYPE_VERSION,
  sizeof(Eina_Model_Type_Mixin_Data),
  sizeof(Eina_Model_Type),
  "Eina_Model_Type_Mixin",
  &_EINA_MODEL_TYPE_BASE,
  NULL, /* no interfaces implemented */
  NULL, /* no extra events */
  _eina_model_type_mixin_setup,
  _eina_model_type_mixin_flush,
  _eina_model_type_mixin_constructor,
  _eina_model_type_mixin_destructor,
  NULL, /* no copy, as interface is called automatically */
  NULL, /* no deep copy, as interface is called automatically */
  _eina_model_type_mixin_compare,
  _eina_model_type_mixin_load,
  _eina_model_type_mixin_unload,
  _eina_model_type_mixin_property_get,
  _eina_model_type_mixin_property_set,
  _eina_model_type_mixin_property_del,
  _eina_model_type_mixin_properties_names_list_get,
  _eina_model_type_mixin_child_count,
  _eina_model_type_mixin_child_get,
  _eina_model_type_mixin_child_set,
  _eina_model_type_mixin_child_del,
  _eina_model_type_mixin_child_insert_at,
  NULL, /* use default find */
  NULL, /* use default criteria_match */
  _eina_model_type_mixin_child_sort,
  NULL, /* use default iterator get */
  NULL, /* use default reversed iterator get */
  NULL, /* use default sorted iterator get */
  NULL, /* use default filtered iterator get */
  NULL, /* use default to string */
  NULL, /* extension pointer */
  NULL, /* extension pointer */
  NULL, /* extension pointer */
  NULL  /* extension pointer */
};
#undef EINA_MODEL_TYPE_MIXIN_GET

/* Events for all Properties interface */
static const Eina_Model_Event_Description _eina_model_interface_properties_events[] = {
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_properties_loaded, "", "model properties were loaded"),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_properties_unloaded, "", "model properties were unloaded"),
  EINA_MODEL_EVENT_DESCRIPTION_SENTINEL
};

/* EINA_MODEL_INTERFACE_PROPERTIES_HASH ******************************/

#define EINA_MODEL_INTERFACE_PROPERTIES_HASH_GET(model)                 \
  Eina_Hash *priv = *(Eina_Hash **)eina_model_interface_private_data_get \
    (model, EINA_MODEL_INTERFACE_PROPERTIES_HASH)

static Eina_Bool
_eina_model_interface_properties_hash_setup(Eina_Model *model)
{
   Eina_Hash **p_priv = eina_model_interface_private_data_get
     (model, EINA_MODEL_INTERFACE_PROPERTIES_HASH);

   DBG("setup interface properties (hash) at %p model %p (%s)",
       p_priv, model, model->desc->cache.types[0]->name);

   *p_priv = eina_hash_string_small_new(NULL);
   return !!*p_priv;
}

static Eina_Bool
_eina_model_interface_properties_hash_flush(Eina_Model *model)
{
   EINA_MODEL_INTERFACE_PROPERTIES_HASH_GET(model);

   DBG("flush interface properties (hash) at %p model %p (%s)",
       priv, model, model->desc->cache.types[0]->name);

   if (priv)
     {
        ERR("interface properties flushed with values! priv=%p, model %p (%s)",
            priv, model, model->desc->cache.types[0]->name);
        eina_hash_free(priv);
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_properties_hash_constructor(Eina_Model *model)
{
   EINA_MODEL_INTERFACE_PROPERTIES_HASH_GET(model);

   DBG("construct interface properties (hash) at %p model %p (%s)",
       priv, model, model->desc->cache.types[0]->name);

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_properties_hash_destructor_foreach(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   eina_value_free(data);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_properties_hash_destructor(Eina_Model *model)
{
   Eina_Hash **p_priv = eina_model_interface_private_data_get
     (model, EINA_MODEL_INTERFACE_PROPERTIES_HASH);
   int count = eina_hash_population(*p_priv);

   DBG("destroy interface properties (hash) at %p model %p (%s). %d values.",
       *p_priv, model, model->desc->cache.types[0]->name, count);

   eina_hash_foreach
     (*p_priv, _eina_model_interface_properties_hash_destructor_foreach, NULL);
   eina_hash_free(*p_priv);
   *p_priv = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_properties_hash_get(const Eina_Model *model, const char *name, Eina_Value *value)
{
   EINA_MODEL_INTERFACE_PROPERTIES_HASH_GET(model);
   const Eina_Value *prop = eina_hash_find(priv, name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop, EINA_FALSE);
   return eina_value_copy(prop, value);
}

static Eina_Bool
_eina_model_interface_properties_hash_set(Eina_Model *model, const char *name, const Eina_Value *value)
{
   EINA_MODEL_INTERFACE_PROPERTIES_HASH_GET(model);
   Eina_Value *prop, *old = eina_hash_find(priv, name);

   prop = eina_value_new(eina_value_type_get(value));
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop, EINA_FALSE);

   eina_value_flush(prop);
   if (!eina_value_copy(value, prop))
     {
        ERR("Could not copy value '%s' from %p to %p", name, value, prop);
        eina_value_free(prop);
        return EINA_FALSE;
     }

   if (!old)
     {
        if (!eina_hash_add(priv, name, prop))
          {
             ERR("Could not add value %p to hash as key '%s'", prop, name);
             eina_value_free(prop);
             return EINA_FALSE;
          }
     }
   else
     {
        eina_value_free(old);
        if (!eina_hash_modify(priv, name, prop))
          {
             ERR("Could not modify hash key '%s' value from %p to %p",
                 name, old, prop);
             eina_hash_del_by_key(priv, name);
             eina_value_free(prop);
             return EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_properties_hash_del(Eina_Model *model, const char *name)
{
   EINA_MODEL_INTERFACE_PROPERTIES_HASH_GET(model);
   Eina_Value *old = eina_hash_find(priv, name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(old, EINA_FALSE);
   eina_value_free(old);
   return eina_hash_del_by_key(priv, name);
}

static Eina_Bool
_eina_model_interface_properties_hash_names_list_foreach(const Eina_Hash *hash __UNUSED__, const void *key, void *data __UNUSED__, void *fdata)
{
   Eina_List **p_list = fdata;
   *p_list = eina_list_append(*p_list, eina_stringshare_add(key));
   return EINA_TRUE;
}

static Eina_List *
_eina_model_interface_properties_hash_names_list(const Eina_Model *model)
{
   EINA_MODEL_INTERFACE_PROPERTIES_HASH_GET(model);
   Eina_List *list = NULL;
   eina_hash_foreach
     (priv, _eina_model_interface_properties_hash_names_list_foreach, &list);
   return list;
}
#undef EINA_MODEL_INTERFACE_PROPERTIES_HASH_GET

static const Eina_Model_Interface_Properties _EINA_MODEL_INTERFACE_PROPERTIES_HASH = {
  {
    EINA_MODEL_INTERFACE_VERSION,
    sizeof(Eina_Hash *),
    sizeof(Eina_Model_Interface_Properties),
    _EINA_MODEL_INTERFACE_NAME_PROPERTIES,
    NULL, /* no parent interfaces */
    _eina_model_interface_properties_events,
    _eina_model_interface_properties_hash_setup,
    _eina_model_interface_properties_hash_flush,
    _eina_model_interface_properties_hash_constructor,
    _eina_model_interface_properties_hash_destructor,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
  },
  EINA_MODEL_INTERFACE_PROPERTIES_VERSION,
  NULL, /* no compare */
  NULL, /* no load */
  NULL, /* no unload */
  _eina_model_interface_properties_hash_get,
  _eina_model_interface_properties_hash_set,
  _eina_model_interface_properties_hash_del,
  _eina_model_interface_properties_hash_names_list
};

/* EINA_MODEL_INTERFACE_PROPERTIES_STRUCT ******************************/

static Eina_Value_Struct *
_eina_model_interface_properties_struct_private_get(const Eina_Model *model)
{
   Eina_Value *val = eina_model_interface_private_data_get
     (model, EINA_MODEL_INTERFACE_PROPERTIES_STRUCT);
   return eina_value_memory_get(val);
}

#define EINA_MODEL_INTERFACE_PROPERTIES_STRUCT_GET(model)         \
  Eina_Value_Struct *priv =                                       \
    _eina_model_interface_properties_struct_private_get(model)

static Eina_Bool
_eina_model_interface_properties_struct_setup(Eina_Model *model)
{
   Eina_Value *val = eina_model_interface_private_data_get
     (model, EINA_MODEL_INTERFACE_PROPERTIES_STRUCT);

   DBG("setup interface properties (struct) at %p model %p (%s)",
       val, model, model->desc->cache.types[0]->name);

   return eina_value_setup(val, EINA_VALUE_TYPE_STRUCT);
}

static Eina_Bool
_eina_model_interface_properties_struct_flush(Eina_Model *model)
{
   Eina_Value *val = eina_model_interface_private_data_get
     (model, EINA_MODEL_INTERFACE_PROPERTIES_STRUCT);

   DBG("flush interface properties (struct) at %p model %p (%s)",
       val, model, model->desc->cache.types[0]->name);

   if (val->type)
     {
        ERR("interface properties flushed with values! val=%p, model %p (%s)",
            val, model, model->desc->cache.types[0]->name);
        eina_value_flush(val);
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_properties_struct_constructor(Eina_Model *model)
{
   EINA_MODEL_INTERFACE_PROPERTIES_STRUCT_GET(model);

   DBG("construct interface properties (struct) at %p model %p (%s)",
       priv, model, model->desc->cache.types[0]->name);

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_properties_struct_destructor(Eina_Model *model)
{
   Eina_Value *val = eina_model_interface_private_data_get
     (model, EINA_MODEL_INTERFACE_PROPERTIES_STRUCT);

   DBG("destroy interface properties (struct) at %p model %p (%s)",
       val, model, model->desc->cache.types[0]->name);

   eina_value_flush(val);
   val->type = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_properties_struct_get(const Eina_Model *model, const char *name, Eina_Value *val)
{
   const Eina_Value *v = eina_model_interface_private_data_get
     (model, EINA_MODEL_INTERFACE_PROPERTIES_STRUCT);
   return eina_value_struct_value_get(v, name, val);
}

static Eina_Bool
_eina_model_interface_properties_struct_set(Eina_Model *model, const char *name, const Eina_Value *val)
{
   Eina_Value *v = eina_model_interface_private_data_get
     (model, EINA_MODEL_INTERFACE_PROPERTIES_STRUCT);
   return eina_value_struct_value_set(v, name, val);
}

static Eina_Bool
_eina_model_interface_properties_struct_del(Eina_Model *model __UNUSED__, const char *name __UNUSED__)
{
   return EINA_FALSE; /* not allowed */
}

static Eina_List *
_eina_model_interface_properties_struct_names_list(const Eina_Model *model)
{
   EINA_MODEL_INTERFACE_PROPERTIES_STRUCT_GET(model);
   const Eina_Value_Struct_Member *itr;
   Eina_List *list = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->desc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->desc->members, NULL);

   itr = priv->desc->members;
   if (priv->desc->member_count)
     {
        const Eina_Value_Struct_Member *end = itr + priv->desc->member_count;
        for (; itr < end; itr++)
          list = eina_list_append(list, eina_stringshare_add(itr->name));
     }
   else
     {
        for (; itr->name != NULL; itr++)
          list = eina_list_append(list, eina_stringshare_add(itr->name));
     }

   return list;
}
#undef EINA_MODEL_INTERFACE_PROPERTIES_STRUCT_GET

static const Eina_Model_Interface_Properties _EINA_MODEL_INTERFACE_PROPERTIES_STRUCT = {
  {
    EINA_MODEL_INTERFACE_VERSION,
    sizeof(Eina_Value),
    sizeof(Eina_Model_Interface_Properties),
    _EINA_MODEL_INTERFACE_NAME_PROPERTIES,
    NULL, /* no parent interfaces */
    _eina_model_interface_properties_events,
    _eina_model_interface_properties_struct_setup,
    _eina_model_interface_properties_struct_flush,
    _eina_model_interface_properties_struct_constructor,
    _eina_model_interface_properties_struct_destructor,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
  },
  EINA_MODEL_INTERFACE_PROPERTIES_VERSION,
  NULL, /* no compare */
  NULL, /* no load */
  NULL, /* no unload */
  _eina_model_interface_properties_struct_get,
  _eina_model_interface_properties_struct_set,
  _eina_model_interface_properties_struct_del,
  _eina_model_interface_properties_struct_names_list
};

/* Events for all Children interface */
static const Eina_Model_Event_Description _eina_model_interface_children_events[] = {
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_children_loaded, "", "model children were loaded"),
  EINA_MODEL_EVENT_DESCRIPTION(_eina_model_str_children_unloaded, "", "model children were unloaded"),
  EINA_MODEL_EVENT_DESCRIPTION_SENTINEL
};

/* EINA_MODEL_INTERFACE_CHILDREN_INARRAY ******************************/

#define EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model)        \
  Eina_Inarray *priv = eina_model_interface_private_data_get    \
    (model, EINA_MODEL_INTERFACE_CHILDREN_INARRAY)

static Eina_Bool
_eina_model_interface_children_inarray_setup(Eina_Model *model)
{
   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);

   DBG("setup interface children (inarray) at %p model %p (%s)",
       priv, model, model->desc->cache.types[0]->name);

   eina_inarray_step_set(priv, sizeof(Eina_Inarray), sizeof(Eina_Model *), 0);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_children_inarray_flush(Eina_Model *model)
{
   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);
   int count;

   DBG("flush interface children (inarray) at %p model %p (%s)",
       priv, model, model->desc->cache.types[0]->name);

   count = eina_inarray_count(priv);
   if (count > 0)
     ERR("interface children flushed with %d members! priv=%p, model %p (%s)",
         count, priv, model, model->desc->cache.types[0]->name);

   eina_inarray_flush(priv);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_children_inarray_constructor(Eina_Model *model)
{
   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);

   DBG("construct interface children (inarray) at %p model %p (%s)",
       priv, model, model->desc->cache.types[0]->name);

   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_children_inarray_destructor(Eina_Model *model)
{
   Eina_Model **itr, **itr_end;
   int count;

   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);

   count = eina_inarray_count(priv);

   DBG("destroy interface children (inarray) at %p model %p (%s). %d members.",
       priv, model, model->desc->cache.types[0]->name, count);

   itr = priv->members;
   itr_end = itr + count;
   for (; itr < itr_end; itr++)
     eina_model_xunref(*itr, EINA_MODEL_INTERFACE_CHILDREN_INARRAY);
   eina_inarray_flush(priv);

   return EINA_TRUE;
}

static int
_eina_model_interface_children_inarray_count(const Eina_Model *model)
{
   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);
   return eina_inarray_count(priv);
}

static Eina_Model *
_eina_model_interface_children_inarray_get(const Eina_Model *model, unsigned int position)
{
   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);
   Eina_Model **child = eina_inarray_nth(priv, position);
   if (!child)
     return NULL;
   return eina_model_ref(*child);
}

static Eina_Bool
_eina_model_interface_children_inarray_set(Eina_Model *model, unsigned int position, Eina_Model *child)
{
   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);
   Eina_Model **p_old = eina_inarray_nth(priv, position);
   Eina_Model *old;

   if (!p_old)
     return EINA_FALSE;

   old = *p_old;
   if (!eina_inarray_replace_at(priv, position, &child))
     return EINA_FALSE;

   eina_model_xref(child, EINA_MODEL_INTERFACE_CHILDREN_INARRAY,
                   "eina_model_child_set");
   eina_model_xunref(old, EINA_MODEL_INTERFACE_CHILDREN_INARRAY);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_children_inarray_del(Eina_Model *model, unsigned int position)
{
   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);
   Eina_Model **p_old = eina_inarray_nth(priv, position);
   Eina_Model *old;

   if (!p_old)
     return EINA_FALSE;

   old = *p_old;
   if (!eina_inarray_remove_at(priv, position))
     return EINA_FALSE;

   eina_model_xunref(old, EINA_MODEL_INTERFACE_CHILDREN_INARRAY);
   return EINA_TRUE;
}

static Eina_Bool
_eina_model_interface_children_inarray_insert_at(Eina_Model *model, unsigned int position, Eina_Model *child)
{
   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);

   if (!eina_inarray_insert_at(priv, position, &child))
     return EINA_FALSE;

   eina_model_xref(child, EINA_MODEL_INTERFACE_CHILDREN_INARRAY,
                   "eina_model_child_insert_at");
   return EINA_TRUE;
}

static void
_eina_model_interface_children_inarray_sort(Eina_Model *model, Eina_Compare_Cb compare)
{
   EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET(model);
   int count = eina_inarray_count(priv);
   EINA_SAFETY_ON_FALSE_RETURN(count >= 0);

   if (count > 1)
     _eina_model_array_sort(priv->members, 0, count - 1, compare);
}
#undef EINA_MODEL_INTERFACE_CHILDREN_INARRAY_GET

static const Eina_Model_Interface_Children _EINA_MODEL_INTERFACE_CHILDREN_INARRAY = {
  {
    EINA_MODEL_INTERFACE_VERSION,
    sizeof(Eina_Inarray),
    sizeof(Eina_Model_Interface_Children),
    _EINA_MODEL_INTERFACE_NAME_CHILDREN,
    NULL, /* no parent interfaces */
    _eina_model_interface_children_events,
    _eina_model_interface_children_inarray_setup,
    _eina_model_interface_children_inarray_flush,
    _eina_model_interface_children_inarray_constructor,
    _eina_model_interface_children_inarray_destructor,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
  },
  EINA_MODEL_INTERFACE_CHILDREN_VERSION,
  NULL, /* no compare */
  NULL, /* no load */
  NULL, /* no unload */
  _eina_model_interface_children_inarray_count,
  _eina_model_interface_children_inarray_get,
  _eina_model_interface_children_inarray_set,
  _eina_model_interface_children_inarray_del,
  _eina_model_interface_children_inarray_insert_at,
  _eina_model_interface_children_inarray_sort
};

/* EINA_MODEL_TYPE_GENERIC ********************************************/

static const Eina_Model_Interface *_EINA_MODEL_TYPE_GENERIC_IFACES[] = {
  &_EINA_MODEL_INTERFACE_PROPERTIES_HASH.base,
  &_EINA_MODEL_INTERFACE_CHILDREN_INARRAY.base,
  NULL
};

static const Eina_Model_Type _EINA_MODEL_TYPE_GENERIC =
  EINA_MODEL_TYPE_INIT_NOPRIVATE("Eina_Model_Type_Generic",
                                 Eina_Model_Type,
                                 &_EINA_MODEL_TYPE_MIXIN,
                                 _EINA_MODEL_TYPE_GENERIC_IFACES,
                                 NULL);

/* EINA_MODEL_TYPE_STRUCT ********************************************/

static const Eina_Model_Interface *_EINA_MODEL_TYPE_STRUCT_IFACES[] = {
  &_EINA_MODEL_INTERFACE_PROPERTIES_STRUCT.base,
  &_EINA_MODEL_INTERFACE_CHILDREN_INARRAY.base,
  NULL
};

static const Eina_Model_Type _EINA_MODEL_TYPE_STRUCT =
  EINA_MODEL_TYPE_INIT_NOPRIVATE("Eina_Model_Type_Struct",
                                 Eina_Model_Type,
                                 &_EINA_MODEL_TYPE_MIXIN,
                                 _EINA_MODEL_TYPE_STRUCT_IFACES,
                                 NULL);

/**
 */

/**
 * @internal
 * @brief Initialize the model module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the model module of Eina. It is called
 * by eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_model_init(void)
{
   const char *choice, *tmp;

   _eina_model_log_dom = eina_log_domain_register("eina_model",
                                                  EINA_LOG_COLOR_DEFAULT);
   if (_eina_model_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_model");
        return EINA_FALSE;
     }

   choice = getenv("EINA_MODEL_DEBUG");
   if (choice)
     {
        if (strcmp(choice, "1") == 0)
          _eina_model_debug = EINA_MODEL_DEBUG_CHECK;
        else if (strcmp(choice, "backtrace") == 0)
          _eina_model_debug = EINA_MODEL_DEBUG_BACKTRACE;
     }

#ifdef EINA_DEFAULT_MEMPOOL
   choice = "pass_through";
#else
   choice = "chained_mempool";
#endif
   tmp = getenv("EINA_MEMPOOL");
   if (tmp && tmp[0])
     choice = tmp;

   if (choice)
     _eina_model_mp_choice = strdup(choice);

   _eina_model_mp = eina_mempool_add
     (_eina_model_mp_choice, "model", NULL, sizeof(Eina_Model), 32);
   if (!_eina_model_mp)
     {
        ERR("Mempool for model cannot be allocated in model init.");
        goto on_init_fail_mp;
     }

   if (!eina_lock_new(&_eina_model_inner_mps_lock))
     {
        ERR("Cannot create inner mempools lock in model init.");
        goto on_init_fail_lock_mp;
     }
   _eina_model_inner_mps = eina_hash_int32_new(NULL);
   if (!_eina_model_inner_mps)
     {
        ERR("Cannot create hash for inner mempools in model init.");
        goto on_init_fail_hash_mp;
     }

   if (!eina_lock_new(&_eina_model_descriptions_lock))
     {
        ERR("Cannot create model descriptions lock in model init.");
        goto on_init_fail_lock_desc;
     }
   _eina_model_descriptions = eina_hash_pointer_new(NULL);
   if (!_eina_model_descriptions)
     {
        ERR("Cannot create model descriptions hash in model init.");
        goto on_init_fail_hash_desc;
     }

   if (!eina_lock_new(&_eina_model_debug_list_lock))
     {
        ERR("Cannot create model debug list lock in model init.");
        goto on_init_fail_lock_debug;
     }

   EINA_ERROR_MODEL_FAILED = eina_error_msg_static_register(
                                                            EINA_ERROR_MODEL_FAILED_STR);
   EINA_ERROR_MODEL_METHOD_MISSING = eina_error_msg_static_register(
                                                                    EINA_ERROR_MODEL_METHOD_MISSING_STR);

   EINA_MODEL_TYPE_BASE = &_EINA_MODEL_TYPE_BASE;
   EINA_MODEL_TYPE_MIXIN = &_EINA_MODEL_TYPE_MIXIN;
   EINA_MODEL_TYPE_GENERIC = &_EINA_MODEL_TYPE_GENERIC;
   EINA_MODEL_TYPE_STRUCT = &_EINA_MODEL_TYPE_STRUCT;

   EINA_MODEL_INTERFACE_PROPERTIES_HASH = &_EINA_MODEL_INTERFACE_PROPERTIES_HASH.base;
   EINA_MODEL_INTERFACE_PROPERTIES_STRUCT = &_EINA_MODEL_INTERFACE_PROPERTIES_STRUCT.base;

   EINA_MODEL_INTERFACE_CHILDREN_INARRAY = &_EINA_MODEL_INTERFACE_CHILDREN_INARRAY.base;

   EINA_MODEL_INTERFACE_NAME_PROPERTIES = _EINA_MODEL_INTERFACE_NAME_PROPERTIES;
   EINA_MODEL_INTERFACE_NAME_CHILDREN = _EINA_MODEL_INTERFACE_NAME_CHILDREN;

   eina_magic_string_static_set(EINA_MAGIC_MODEL, EINA_MAGIC_MODEL_STR);

   return EINA_TRUE;

 on_init_fail_lock_debug:
   eina_hash_free(_eina_model_descriptions);
 on_init_fail_hash_desc:
   eina_lock_free(&_eina_model_descriptions_lock);
 on_init_fail_lock_desc:
   eina_hash_free(_eina_model_inner_mps);
   _eina_model_inner_mps = NULL;
 on_init_fail_hash_mp:
   eina_lock_free(&_eina_model_inner_mps_lock);
 on_init_fail_lock_mp:
   eina_mempool_del(_eina_model_mp);
 on_init_fail_mp:
   free(_eina_model_mp_choice);
   _eina_model_mp_choice = NULL;
   eina_log_domain_unregister(_eina_model_log_dom);
   _eina_model_log_dom = -1;
   return EINA_FALSE;
}

/**
 * @internal
 * @brief Shut down the model module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the model module set up by
 * eina_model_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_model_shutdown(void)
{
   eina_lock_take(&_eina_model_debug_list_lock);
   if (eina_list_count(_eina_model_debug_list) > 0)
     ERR("%d models are still alive!", eina_list_count(_eina_model_debug_list));
   eina_lock_release(&_eina_model_debug_list_lock);
   eina_lock_free(&_eina_model_debug_list_lock);

   eina_lock_take(&_eina_model_inner_mps_lock);
   if (eina_hash_population(_eina_model_inner_mps) != 0)
     ERR("Cannot free eina_model internal memory pools -- still in use!");
   else
     eina_hash_free(_eina_model_inner_mps);
   eina_lock_release(&_eina_model_inner_mps_lock);
   eina_lock_free(&_eina_model_inner_mps_lock);

   eina_lock_take(&_eina_model_descriptions_lock);
   if (eina_hash_population(_eina_model_descriptions) != 0)
     ERR("Cannot free eina_model internal descriptions -- still in use!");
   else
     eina_hash_free(_eina_model_descriptions);
   eina_lock_release(&_eina_model_descriptions_lock);
   eina_lock_free(&_eina_model_descriptions_lock);

   free(_eina_model_mp_choice);
   _eina_model_mp_choice = NULL;
   eina_mempool_del(_eina_model_mp);
   eina_log_domain_unregister(_eina_model_log_dom);
   _eina_model_log_dom = -1;
   return EINA_TRUE;
}

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/


EAPI Eina_Error EINA_ERROR_MODEL_FAILED = 0;
EAPI Eina_Error EINA_ERROR_MODEL_METHOD_MISSING = 0;

EAPI const Eina_Model_Type *EINA_MODEL_TYPE_BASE = NULL;
EAPI const Eina_Model_Type *EINA_MODEL_TYPE_MIXIN = NULL;
EAPI const Eina_Model_Type *EINA_MODEL_TYPE_GENERIC = NULL;
EAPI const Eina_Model_Type *EINA_MODEL_TYPE_STRUCT = NULL;

EAPI const Eina_Model_Interface *EINA_MODEL_INTERFACE_PROPERTIES_HASH = NULL;
EAPI const Eina_Model_Interface *EINA_MODEL_INTERFACE_PROPERTIES_STRUCT = NULL;
EAPI const Eina_Model_Interface *EINA_MODEL_INTERFACE_CHILDREN_INARRAY = NULL;

EAPI const char *EINA_MODEL_INTERFACE_NAME_PROPERTIES = "Eina_Model_Interface_Properties";
EAPI const char *EINA_MODEL_INTERFACE_NAME_CHILDREN = "Eina_Model_Interface_Children";

EAPI Eina_Model *
eina_model_new(const Eina_Model_Type *type)
{
   const Eina_Model_Description *desc;
   Eina_Model *model;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(type, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_model_type_check(type), NULL);

   desc = _eina_model_description_get(type);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);

   model = eina_mempool_malloc(_eina_model_mp, sizeof(Eina_Model));
   EINA_SAFETY_ON_NULL_GOTO(model, failed_model);

   model->desc = desc;
   model->listeners.entries = NULL;
   model->listeners.deleted = NULL;
   model->listeners.freeze = NULL;
   model->listeners.walking = 0;

   if (desc->total.size == 0)
     model->privates = NULL;
   else
     {
        unsigned char *ptr;

        model->privates = _eina_model_inner_alloc
          (desc->total.privates * sizeof(void *) +
           desc->total.size);
        EINA_SAFETY_ON_NULL_GOTO(model->privates, failed_privates);

        ptr = (unsigned char *)(model->privates + desc->total.privates);
        for (i = 0; i < desc->total.privates; i++)
          {
             unsigned int size;
             if (i < desc->total.types)
               size = desc->cache.privates[i].type->private_size;
             else
               size = desc->cache.privates[i].iface->private_size;

             if (size == 0)
               {
                  model->privates[i] = NULL;
                  continue;
               }

             model->privates[i] = ptr;
             memset(ptr, 0, size);

             if (size % sizeof(void *) != 0)
               size += sizeof(void *) - (size % sizeof(void *));
             ptr += size;
          }
     }

   model->refcount = 1;
   model->xrefs = NULL;
   model->deleted = EINA_FALSE;
   EINA_MAGIC_SET(model, EINA_MAGIC_MODEL);

   /* call setup of every type in the reverse order,
    * they should not call parent's setup.
    */
   for (i = desc->total.types; i > 0; i--)
     {
        if (desc->cache.types[i - 1]->setup)
          {
             if (!desc->cache.types[i - 1]->setup(model))
               {
                  ERR("Failed to setup model %p at type %p (%s)",
                      model, desc->cache.types[i - 1],
                      desc->cache.types[i - 1]->name);
                  goto failed_types;
               }
          }
     }

   /* call setup of every interface in the reverse order,
    * they should not call parent's setup.
    */
   for (i = desc->total.ifaces; i > 0; i--)
     {
        if (desc->cache.ifaces[i - 1]->setup)
          {
             if (!desc->cache.ifaces[i - 1]->setup(model))
               {
                  ERR("Failed to setup model %p at interface %p (%s)",
                      model, desc->cache.ifaces[i - 1],
                      desc->cache.ifaces[i - 1]->name);
                  goto failed_ifaces;
               }
          }
     }

   if (!desc->ops.type.constructor(model))
     {
        ERR("Failed to construct model %p, type %p (%s)",
            model, desc->cache.types[0], desc->cache.types[0]->name);
        goto failed_constructor;
     }

   if (EINA_UNLIKELY(_eina_model_debug))
     {
        eina_lock_take(&_eina_model_debug_list_lock);
        _eina_model_debug_list = eina_list_append
          (_eina_model_debug_list, model);
        eina_lock_release(&_eina_model_debug_list_lock);
     }

   return model;

 failed_constructor:
   i = 0;
 failed_ifaces:
   /* flush every setup interface, natural order */
   for (; i < desc->total.ifaces; i++)
     desc->cache.ifaces[i]->flush(model);
   i = 0;
 failed_types:
   /* flush every setup type, natural order */
   for (; i < desc->total.types; i++)
     desc->cache.types[i]->flush(model);

   if (model->privates)
     _eina_model_inner_free(desc->total.privates * sizeof(void *) +
                            desc->total.size,
                            model->privates);

 failed_privates:
   EINA_MAGIC_SET(model, EINA_MAGIC_NONE);
   eina_mempool_free(_eina_model_mp, model);
 failed_model:
   _eina_model_description_dispose(desc);
   return NULL;
}

static void
_eina_model_free(Eina_Model *model)
{
   const Eina_Model_Description *desc = model->desc;
   unsigned int i;

   DBG("model %p (%s) refcount=%d deleted=%hhu",
       model, model->desc->cache.types[0]->name,
       model->refcount, model->deleted);

   if (EINA_UNLIKELY(_eina_model_debug))
     {
        if (model->xrefs)
          {
             ERR("Model %p (%s) released with references pending:",
                 model, model->desc->cache.types[0]->name);
             while (model->xrefs)
               {
                  Eina_Model_XRef *ref = (Eina_Model_XRef *)model->xrefs;
                  model->xrefs = eina_inlist_remove(model->xrefs, model->xrefs);

                  ERR("xref: %p '%s'", ref->id, ref->label);
                  free(ref);
               }
          }

        eina_lock_take(&_eina_model_debug_list_lock);
        _eina_model_debug_list = eina_list_remove
          (_eina_model_debug_list, model);
        eina_lock_release(&_eina_model_debug_list_lock);
     }

   /* flush every interface, natural order */
   for (i = 0; i < desc->total.ifaces; i++)
     if (desc->cache.ifaces[i]->flush)
       desc->cache.ifaces[i]->flush(model);

   /* flush every type, natural order */
   for (i = 0; i < desc->total.types; i++)
     if (desc->cache.types[i]->flush)
       desc->cache.types[i]->flush(model);

   model->refcount--;
   _eina_model_event_callback_call(model, _eina_model_str_freed, NULL);

   if (model->privates)
     _eina_model_inner_free(desc->total.privates * sizeof(void *) +
                            desc->total.size,
                            model->privates);

   if (model->listeners.deleted)
     _eina_model_event_callback_free_deleted(model);

   if (model->listeners.entries)
     {
        for (i = 0; i < desc->total.events; i++)
          {
             Eina_Inlist *lst = model->listeners.entries[i];
             while (lst)
               {
                  void *tmp = lst;
                  lst = lst->next;
                  _eina_model_inner_free(sizeof(Eina_Model_Event_Listener),
                                         tmp);
               }
          }

        _eina_model_inner_free(desc->total.events * sizeof(Eina_Inlist *),
                               model->listeners.entries);
     }

   if (model->listeners.freeze)
     _eina_model_inner_free(model->desc->total.events * sizeof(int),
                            model->listeners.freeze);

   EINA_MAGIC_SET(model, EINA_MAGIC_NONE);
   eina_mempool_free(_eina_model_mp, model);

   _eina_model_description_dispose(desc);
}

static void
_eina_model_del(Eina_Model *model)
{
   const Eina_Model_Description *desc = model->desc;

   DBG("model %p (%s) refcount=%d deleted=%hhu",
       model, model->desc->cache.types[0]->name,
       model->refcount, model->deleted);

   EINA_SAFETY_ON_TRUE_RETURN(model->deleted);

   model->deleted = EINA_TRUE;
   _eina_model_event_callback_call(model, _eina_model_str_deleted, NULL);

   if (!desc->ops.type.destructor(model))
     ERR("Failed to destroy model %p, type %p (%s)",
         model, desc->cache.types[0], desc->cache.types[0]->name);
}

static void
_eina_model_unref(Eina_Model *model)
{
   DBG("model %p (%s) refcount=%d deleted=%hhu",
       model, model->desc->cache.types[0]->name,
       model->refcount, model->deleted);

   if (model->refcount > 1)
     {
        model->refcount--;
        return;
     }

   if (!model->deleted) _eina_model_del(model);
   _eina_model_free(model);
}

#define EINA_MODEL_INSTANCE_CHECK_VAL(inst, retval)                     \
  do                                                                    \
    {                                                                   \
       if (!EINA_MAGIC_CHECK(inst, EINA_MAGIC_MODEL))                   \
         {                                                              \
            EINA_MAGIC_FAIL(inst, EINA_MAGIC_MODEL);                    \
            return retval;                                              \
         }                                                              \
       EINA_SAFETY_ON_NULL_RETURN_VAL(inst->desc, retval);              \
       EINA_SAFETY_ON_FALSE_RETURN_VAL(inst->refcount > 0, retval);     \
       EINA_SAFETY_ON_FALSE_RETURN_VAL(inst->desc->refcount > 0, retval); \
    }                                                                   \
  while (0)

#define EINA_MODEL_INSTANCE_CHECK(inst)                         \
  do                                                            \
    {                                                           \
       if (!EINA_MAGIC_CHECK(inst, EINA_MAGIC_MODEL))           \
         {                                                      \
            EINA_MAGIC_FAIL(inst, EINA_MAGIC_MODEL);            \
            return;                                             \
         }                                                      \
       EINA_SAFETY_ON_NULL_RETURN(inst->desc);                  \
       EINA_SAFETY_ON_FALSE_RETURN(inst->refcount > 0);         \
       EINA_SAFETY_ON_FALSE_RETURN(inst->desc->refcount > 0);   \
    }                                                           \
  while (0)

#define EINA_MODEL_TYPE_CALL_OPTIONAL_RETURN(model, method, def_retval, ...) \
  do                                                                    \
    {                                                                   \
       eina_error_set(0);                                               \
       if (model->desc->ops.type.method)                                \
         return model->desc->ops.type.method(model, ## __VA_ARGS__);    \
       DBG("Optional method" # method "() not implemented for model %p (%s)", \
           model, model->desc->cache.types[0]->name);                   \
       return def_retval;                                               \
    }                                                                   \
  while (0)

#define EINA_MODEL_TYPE_CALL_OPTIONAL(model, method, ...)               \
  do                                                                    \
    {                                                                   \
       eina_error_set(0);                                               \
       if (model->desc->ops.type.method)                                \
         model->desc->ops.type.method(model, ## __VA_ARGS__);           \
       else                                                             \
         DBG("Optional method" # method "() not implemented for model %p (%s)", \
             model, model->desc->cache.types[0]->name);                 \
    }                                                                   \
  while (0)

#define EINA_MODEL_TYPE_CALL_MANDATORY_RETURN(model, method, def_retval, ...) \
  do                                                                    \
    {                                                                   \
       eina_error_set(0);                                               \
       if (model->desc->ops.type.method)                                \
         return model->desc->ops.type.method(model, ## __VA_ARGS__);    \
       eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);                 \
       CRITICAL("Mandatory method" # method "() not implemented for model %p (%s)", \
                model, model->desc->cache.types[0]->name);              \
       return def_retval;                                               \
    }                                                                   \
  while (0)

#define EINA_MODEL_TYPE_CALL_MANDATORY(model, method, ...)              \
  do                                                                    \
    {                                                                   \
       eina_error_set(0);                                               \
       if (model->desc->ops.type.method)                                \
         model->desc->ops.type.method(model, ## __VA_ARGS__);           \
       else                                                             \
         {                                                              \
            eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);            \
            CRITICAL("Mandatory method" # method "() not implemented for model %p (%s)", \
                     model, model->desc->cache.types[0]->name);         \
         }                                                              \
    }                                                                   \
  while (0)


#define EINA_MODEL_TYPE_CALL_RETURN(model, method, def_retval, ...)     \
  do                                                                    \
    {                                                                   \
       eina_error_set(0);                                               \
       if (model->desc->ops.type.method)                                \
         return model->desc->ops.type.method(model, ## __VA_ARGS__);    \
       eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);                 \
       ERR("Method" # method "() not implemented for model %p (%s)",    \
           model, model->desc->cache.types[0]->name);                   \
       return def_retval;                                               \
    }                                                                   \
  while (0)

#define EINA_MODEL_TYPE_CALL(model, method, ...)                        \
  do                                                                    \
    {                                                                   \
       eina_error_set(0);                                               \
       if (model->desc->ops.type.method)                                \
         model->desc->ops.type.method(model, ## __VA_ARGS__);           \
       else                                                             \
         {                                                              \
            eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);            \
            ERR("Method" # method "() not implemented for model %p (%s)", \
                model, model->desc->cache.types[0]->name);              \
         }                                                              \
    }                                                                   \
  while (0)

EAPI void
eina_model_del(Eina_Model *model)
{
   if (!model)
     return;

   EINA_MODEL_INSTANCE_CHECK(model);
   _eina_model_del(model);
   _eina_model_unref(model);
}

EAPI const Eina_Model_Type *
eina_model_type_get(const Eina_Model *model)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   return model->desc->cache.types[0];
}

EAPI const Eina_Model_Interface *
eina_model_interface_get(const Eina_Model *model, const char *name)
{
   const Eina_Model_Description *desc;
   const Eina_Model_Interface **itr, **itr_end;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   desc = model->desc;
   itr = desc->cache.ifaces;
   itr_end = itr + desc->total.ifaces;

   /* fallback to strcmp if user is lazy about speed */
   for (; itr < itr_end; itr++)
     if (strcmp((*itr)->name, name) == 0)
       return *itr;

   return NULL;
}

static Eina_Bool
_eina_model_instance_check(const Eina_Model *model, const Eina_Model_Type *type)
{
   const Eina_Model_Type **itr, **itr_end;

   itr = model->desc->cache.types;
   itr_end = itr + model->desc->total.types;

   for (; itr < itr_end; itr++)
     if (*itr == type)
       return EINA_TRUE;

   return EINA_FALSE;
}

EAPI Eina_Bool
eina_model_instance_check(const Eina_Model *model, const Eina_Model_Type *type)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(type, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_type_check(type), EINA_FALSE);
   return _eina_model_instance_check(model, type);
}

EAPI Eina_Model *
eina_model_ref(Eina_Model *model)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   DBG("model %p (%s) refcount=%d deleted=%hhu",
       model, model->desc->cache.types[0]->name,
       model->refcount, model->deleted);
   model->refcount++;
   return model;
}

static Eina_Model *
_eina_model_xref_add(Eina_Model *model, const void *id, const char *label)
{
   Eina_Model_XRef *ref;
   void *bt[256];
   int btlen, labellen;

   labellen = label ? strlen(label): 0;
   btlen = 0;

#ifdef HAVE_BACKTRACE
   if (_eina_model_debug == EINA_MODEL_DEBUG_BACKTRACE)
     btlen = backtrace(bt, EINA_C_ARRAY_LENGTH(bt));
#endif

   ref = calloc(1, sizeof(*ref) + (btlen * sizeof(void *)) + (labellen + 1));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ref, NULL);

   ref->id = id;
   memcpy(ref->label, label, labellen);
   ref->label[labellen] = '\0';
   ref->backtrace.count = btlen;
   if (btlen == 0) ref->backtrace.symbols = NULL;
   else
     {
        void *ptr = (unsigned char *)ref + sizeof(*ref) + (labellen + 1);
        ref->backtrace.symbols = ptr;
        memcpy(ptr, bt, btlen * sizeof(void *));
     }

   model->xrefs = eina_inlist_append(model->xrefs, EINA_INLIST_GET(ref));
   return model;
}

EAPI Eina_Model *
eina_model_xref(Eina_Model *model, const void *id, const char *label)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   DBG("model %p (%s) refcount=%d deleted=%hhu id=%p label=%s",
       model, model->desc->cache.types[0]->name,
       model->refcount, model->deleted, id, label ? label : "");

   model->refcount++;

   if (EINA_LIKELY(!_eina_model_debug))
     return model;

   return _eina_model_xref_add(model, id, label);
}

EAPI void
eina_model_unref(Eina_Model *model)
{
   EINA_MODEL_INSTANCE_CHECK(model);
   _eina_model_unref(model);
}

EAPI void
eina_model_xunref(Eina_Model *model, const void *id)
{
   Eina_Model_XRef *ref;
   EINA_MODEL_INSTANCE_CHECK(model);

   if (EINA_LIKELY(!_eina_model_debug))
     {
        _eina_model_unref(model);
        return;
     }

   EINA_INLIST_FOREACH(model->xrefs, ref)
     {
        if (ref->id != id) continue;

        model->xrefs = eina_inlist_remove(model->xrefs, EINA_INLIST_GET(ref));
        free(ref);
        _eina_model_unref(model);
        return;
     }

   ERR("Could not find existing reference %p to model %p", id, model);
}

EAPI int
eina_model_refcount(const Eina_Model *model)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, -1);
   return model->refcount;
}

EAPI const Eina_Inlist *
eina_model_xrefs_get(const Eina_Model *model)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   return model->xrefs;
}

EAPI Eina_Bool
eina_model_event_callback_add(Eina_Model *model, const char *event_name, Eina_Model_Event_Cb cb, const void *data)
{
   const Eina_Model_Description *desc;
   Eina_Model_Event_Listener *el;
   int event_id;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(event_name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, EINA_FALSE);

   desc = model->desc;
   event_id = _eina_model_description_event_id_find(desc, event_name);
   if (event_id < 0)
     {
        ERR("No event named %s for model %p (%s)",
            event_name, model, model->desc->cache.types[0]->name);
        return EINA_FALSE;
     }

   if (!model->listeners.entries)
     {
        model->listeners.entries = _eina_model_inner_alloc
          (desc->total.events * sizeof(Eina_Inlist *));
        EINA_SAFETY_ON_NULL_RETURN_VAL(model->listeners.entries, EINA_FALSE);
        memset(model->listeners.entries, 0,
               desc->total.events * sizeof(Eina_Inlist *));
     }

   el = _eina_model_inner_alloc(sizeof(Eina_Model_Event_Listener));
   EINA_SAFETY_ON_NULL_RETURN_VAL(el, EINA_FALSE);

   el->cb = cb;
   el->data = data;
   el->deleted = EINA_FALSE;
   model->listeners.entries[event_id] = eina_inlist_append
     (model->listeners.entries[event_id], EINA_INLIST_GET(el));

   return EINA_TRUE;
}

EAPI Eina_Bool
eina_model_event_callback_del(Eina_Model *model, const char *event_name, Eina_Model_Event_Cb cb, const void *data)
{
   int event_id;
   Eina_Inlist *lst;
   Eina_Model_Event_Listener *el;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(event_name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, EINA_FALSE);

   if (!model->listeners.entries)
     {
        ERR("No event callbacks for model %p (%s)",
            model, model->desc->cache.types[0]->name);
        return EINA_FALSE;
     }

   event_id = _eina_model_description_event_id_find(model->desc, event_name);
   if (event_id < 0)
     {
        ERR("No event named %s for model %p (%s)",
            event_name, model, model->desc->cache.types[0]->name);
        return EINA_FALSE;
     }

   lst = model->listeners.entries[event_id];
   EINA_INLIST_FOREACH(lst, el)
     {
        if (el->cb != cb) continue;
        if ((data) && (el->data != data)) continue;

        if (model->listeners.walking == 0)
          {
             model->listeners.entries[event_id] = eina_inlist_remove
               (model->listeners.entries[event_id], EINA_INLIST_GET(el));
             _eina_model_inner_free(sizeof(Eina_Model_Event_Listener), el);
          }
        else
          {
             el->deleted = EINA_TRUE;
             if (!model->listeners.deleted)
               {
                  model->listeners.deleted = _eina_model_inner_alloc
                    (model->desc->total.events * sizeof(Eina_List *));
                  EINA_SAFETY_ON_NULL_RETURN_VAL(model->listeners.deleted,
                                                 EINA_FALSE);

                  memset(model->listeners.deleted, 0,
                         model->desc->total.events * sizeof(Eina_List *));

               }
             model->listeners.deleted[event_id] = eina_list_append
               (model->listeners.deleted[event_id], el);
          }
        return EINA_TRUE;
     }

   ERR("No callback %p data %p found for event named %s for model %p (%s)",
       cb, data, event_name, model, model->desc->cache.types[0]->name);
   return EINA_FALSE;
}

EAPI const Eina_Model_Event_Description *
eina_model_event_description_get(const Eina_Model *model, const char *event_name)
{
   const Eina_Model_Description *desc;
   int event_id;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(event_name, NULL);

   desc = model->desc;
   event_id = _eina_model_description_event_id_find(desc, event_name);
   if (event_id < 0)
     return NULL;

   return desc->cache.events[event_id].desc;
}

EAPI Eina_List *
eina_model_event_names_list_get(const Eina_Model *model)
{
   const Eina_Model_Event_Description_Cache *itr, *itr_end;
   Eina_List *lst = NULL;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);

   itr = model->desc->cache.events;
   itr_end = itr + model->desc->total.events;

   for (; itr < itr_end; itr++)
     lst = eina_list_append(lst, eina_stringshare_add(itr->name));

   return lst;
}

EAPI void
eina_model_event_names_list_free(Eina_List *list)
{
   const char *str;
   EINA_LIST_FREE(list, str)
     eina_stringshare_del(str);
}

EAPI Eina_Bool
eina_model_event_callback_call(Eina_Model *model, const char *name, const void *event_info)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   return _eina_model_event_callback_call(model, name, event_info);
}

EAPI int
eina_model_event_callback_freeze(Eina_Model *model, const char *name)
{
   int event_id;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, -1);

   event_id = _eina_model_description_event_id_find(model->desc, name);
   if (event_id < 0) return -1;

   if (!model->listeners.freeze)
     {
        model->listeners.freeze = _eina_model_inner_alloc
          (model->desc->total.events * sizeof(int));
        EINA_SAFETY_ON_NULL_RETURN_VAL(model->listeners.freeze, -1);

        memset(model->listeners.freeze, 0,
               model->desc->total.events * sizeof(int));
     }

   if (model->listeners.freeze[event_id] == 0)
     DBG("model %p (%s) event %s frozen",
         model, model->desc->cache.types[0]->name, name);

   model->listeners.freeze[event_id]++;
   return model->listeners.freeze[event_id];
}

EAPI int
eina_model_event_callback_thaw(Eina_Model *model, const char *name)
{
   int event_id;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(model->listeners.freeze, -1);

   event_id = _eina_model_description_event_id_find(model->desc, name);
   if (event_id < 0) return -1;

   model->listeners.freeze[event_id]--;
   if (model->listeners.freeze[event_id] == 0)
     DBG("model %p (%s) event %s unfrozen",
         model, model->desc->cache.types[0]->name, name);
   return model->listeners.freeze[event_id];
}

EAPI Eina_Model *
eina_model_copy(const Eina_Model *model)
{
   const Eina_Model_Description *desc;
   Eina_Model *copy;
   unsigned int i;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   desc = model->desc;
   copy = eina_model_new(desc->cache.types[0]);
   EINA_SAFETY_ON_NULL_RETURN_VAL(copy, NULL);

   /* call copy of every type in the reverse order,
    * they should not call parent's copy.
    */
   for (i = desc->total.types; i > 0; i--)
     {
        if (desc->cache.types[i - 1]->copy)
          {
             if (!desc->cache.types[i - 1]->copy(model, copy))
               goto failed;
          }
     }

   /* call copy of every interface in the reverse order,
    * they should not call parent's copy.
    */
   for (i = desc->total.ifaces; i > 0; i--)
     {
        if (desc->cache.ifaces[i - 1]->copy)
          {
             if (!desc->cache.ifaces[i - 1]->copy(model, copy))
               goto failed;
          }
     }

   return copy;

 failed:
   ERR("Failed to copy model %p %s", model, desc->cache.types[0]->name);
   eina_model_del(copy);
   return NULL;
}

EAPI Eina_Model *
eina_model_deep_copy(const Eina_Model *model)
{
   const Eina_Model_Description *desc;
   Eina_Model *deep_copy;
   unsigned int i;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   desc = model->desc;
   deep_copy = eina_model_new(desc->cache.types[0]);
   EINA_SAFETY_ON_NULL_RETURN_VAL(deep_copy, NULL);

   /* call deep_copy of every type in the reverse order,
    * they should not call parent's deep_copy.
    */
   for (i = desc->total.types; i > 0; i--)
     {
        if (desc->cache.types[i - 1]->deep_copy)
          {
             if (!desc->cache.types[i - 1]->deep_copy(model, deep_copy))
               goto failed;
          }
     }

   /* call deep_copy of every interface in the reverse order,
    * they should not call parent's deep_copy.
    */
   for (i = desc->total.ifaces; i > 0; i--)
     {
        if (desc->cache.ifaces[i - 1]->deep_copy)
          {
             if (!desc->cache.ifaces[i - 1]->deep_copy(model, deep_copy))
               goto failed;
          }
     }

   return deep_copy;

 failed:
   ERR("Failed to deep copy model %p %s", model, desc->cache.types[0]->name);
   eina_model_del(deep_copy);
   return NULL;
}

EAPI int
eina_model_compare(const Eina_Model *a, const Eina_Model *b)
{
   const Eina_Model_Description *desc_a, *desc_b;
   Eina_Bool ok;
   int cmp = -1;

   EINA_MODEL_INSTANCE_CHECK_VAL(a, -1);
   EINA_MODEL_INSTANCE_CHECK_VAL(b, -1);
   desc_a = a->desc;
   desc_b = b->desc;

   if ((!desc_a->ops.type.compare) && (!desc_b->ops.type.compare))
     {
        ERR("Models %p (%s) and %p (%s) can't compare",
            a, desc_a->cache.types[0]->name,
            b, desc_b->cache.types[0]->name);
        eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);
        return -1;
     }
   else if ((desc_a->ops.type.compare) && (desc_b->ops.type.compare))
     {
        ok = desc_a->ops.type.compare(a, b, &cmp);
        if (!ok)
          {
             ok = desc_b->ops.type.compare(b, a, &cmp);
             if (ok)
               cmp = -cmp; /* swapped sides! */
          }
     }
   else if (desc_a->ops.type.compare)
     ok = desc_a->ops.type.compare(a, b, &cmp);
   else
     {
        ok = desc_b->ops.type.compare(b, a, &cmp);
        if (ok)
          cmp = -cmp; /* swapped sides! */
     }

   if (!ok)
     {
        ERR("Could not compare models %p (%s) and %p (%s)",
            a, desc_a->cache.types[0]->name,
            b, desc_b->cache.types[0]->name);
        eina_error_set(EINA_ERROR_MODEL_FAILED);
        return -1;
     }

   return cmp;
}

EAPI Eina_Bool
eina_model_load(Eina_Model *model)
{
   Eina_Bool ret;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);

   eina_error_set(0);
   if (model->desc->ops.type.load)
     {
        ret = model->desc->ops.type.load(model);
        if (ret)
          _eina_model_event_callback_call(model, _eina_model_str_loaded, NULL);
     }
   else
     {
        eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);
        ret = EINA_FALSE;
        ERR("Method load() not implemented for model %p (%s)",
            model, model->desc->cache.types[0]->name);
     }

   return ret;
}

EAPI Eina_Bool
eina_model_unload(Eina_Model *model)
{
   Eina_Bool ret;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);

   eina_error_set(0);
   if (model->desc->ops.type.unload)
     {
        ret = model->desc->ops.type.unload(model);
        if (ret)
          _eina_model_event_callback_call
            (model, _eina_model_str_unloaded, NULL);
     }
   else
     {
        eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);
        ret = EINA_FALSE;
        ERR("Method unload() not implemented for model %p (%s)",
            model, model->desc->cache.types[0]->name);
     }

   return ret;
}

EAPI Eina_Bool
eina_model_property_get(const Eina_Model *model, const char *name, Eina_Value *value)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);
   EINA_MODEL_TYPE_CALL_MANDATORY_RETURN(model, property_get, EINA_FALSE,
                                         name, value);
}

EAPI Eina_Bool
eina_model_property_set(Eina_Model *model, const const char *name, const Eina_Value *value)
{
   Eina_Bool ret;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(value->type), EINA_FALSE);

   eina_error_set(0);
   if (model->desc->ops.type.property_set)
     {
        ret = model->desc->ops.type.property_set(model, name, value);
        if (ret)
          _eina_model_event_callback_call
            (model, _eina_model_str_property_set, name);
     }
   else
     {
        eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);
        ret = EINA_FALSE;
        ERR("Method property_set() not implemented for model %p (%s)",
            model, model->desc->cache.types[0]->name);
     }

   return ret;
}

EAPI Eina_Bool
eina_model_property_del(Eina_Model *model, const char *name)
{
   Eina_Bool ret;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);

   eina_error_set(0);
   if (model->desc->ops.type.property_del)
     {
        ret = model->desc->ops.type.property_del(model, name);
        if (ret)
          _eina_model_event_callback_call
            (model, _eina_model_str_property_del, name);
     }
   else
     {
        eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);
        ret = EINA_FALSE;
        ERR("Method property_del() not implemented for model %p (%s)",
            model, model->desc->cache.types[0]->name);
     }

   return ret;
}

EAPI Eina_List *
eina_model_properties_names_list_get(const Eina_Model *model)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_MODEL_TYPE_CALL_OPTIONAL_RETURN(model, properties_names_list_get, NULL);
}

EAPI void
eina_model_properties_names_list_free(Eina_List *list)
{
   const char *str;
   EINA_LIST_FREE(list, str)
     eina_stringshare_del(str);
}

EAPI int
eina_model_child_count(const Eina_Model *model)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, -1);
   EINA_MODEL_TYPE_CALL_OPTIONAL_RETURN(model, child_count, 0);
}

EAPI Eina_Model *
eina_model_child_get(const Eina_Model *model, unsigned int position)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_MODEL_TYPE_CALL_RETURN(model, child_get, NULL, position);
}

EAPI Eina_Bool
eina_model_child_set(Eina_Model *model, unsigned int position, Eina_Model *child)
{
   Eina_Bool ret;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_MODEL_INSTANCE_CHECK_VAL(child, EINA_FALSE);

   eina_error_set(0);
   if (model->desc->ops.type.child_set)
     {
        ret = model->desc->ops.type.child_set(model, position, child);
        if (ret)
          _eina_model_event_callback_call
            (model, _eina_model_str_child_set, &position);
     }
   else
     {
        eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);
        ret = EINA_FALSE;
        ERR("Method child_set() not implemented for model %p (%s)",
            model, model->desc->cache.types[0]->name);
     }

   return ret;
}

EAPI Eina_Bool
eina_model_child_del(Eina_Model *model, unsigned int position)
{
   Eina_Bool ret;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);

   eina_error_set(0);
   if (model->desc->ops.type.child_del)
     {
        ret = model->desc->ops.type.child_del(model, position);
        if (ret)
          {
             _eina_model_event_callback_call
               (model, _eina_model_str_child_del, &position);
             _eina_model_event_callback_call
               (model, _eina_model_str_children_changed, NULL);
          }
     }
   else
     {
        eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);
        ret = EINA_FALSE;
        ERR("Method child_del() not implemented for model %p (%s)",
            model, model->desc->cache.types[0]->name);
     }

   return ret;
}

EAPI Eina_Bool
eina_model_child_insert_at(Eina_Model *model, unsigned int position, Eina_Model *child)
{
   Eina_Bool ret;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EINA_FALSE);

   eina_error_set(0);
   if (model->desc->ops.type.child_insert_at)
     {
        ret = model->desc->ops.type.child_insert_at(model, position, child);
        if (ret)
          {
             _eina_model_event_callback_call
               (model, _eina_model_str_child_inserted, &position);
             _eina_model_event_callback_call
               (model, _eina_model_str_children_changed, NULL);
          }
     }
   else
     {
        eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);
        ret = EINA_FALSE;
        ERR("Method child_insert_at() not implemented for model %p (%s)",
            model, model->desc->cache.types[0]->name);
     }

   return ret;
}

EAPI int
eina_model_child_append(Eina_Model *model, Eina_Model *child)
{
   Eina_Bool ret;
   int position;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, -1);

   position = eina_model_child_count(model);
   if (position < 0)
     return -1;

   eina_error_set(0);
   if (model->desc->ops.type.child_insert_at)
     {
        ret = model->desc->ops.type.child_insert_at(model, position, child);
        if (ret)
          {
             _eina_model_event_callback_call
               (model, _eina_model_str_child_inserted, &position);
             _eina_model_event_callback_call
               (model, _eina_model_str_children_changed, NULL);
          }
     }
   else
     {
        eina_error_set(EINA_ERROR_MODEL_METHOD_MISSING);
        ret = EINA_FALSE;
        ERR("Method child_insert_at() not implemented for model %p (%s)",
            model, model->desc->cache.types[0]->name);
     }

   return ret ? position : -1;
}

EAPI int
eina_model_child_find(const Eina_Model *model, unsigned int start_position, const Eina_Model *other)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(other, -1);
   EINA_MODEL_TYPE_CALL_RETURN(model, child_find, -1, start_position, other);
}

EAPI int
eina_model_child_criteria_match(const Eina_Model *model, unsigned int start_position, Eina_Each_Cb match, const void *data)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(match, -1);
   EINA_MODEL_TYPE_CALL_RETURN(model, child_criteria_match, -1,
                               start_position, match, data);
}

EAPI Eina_Bool
eina_model_child_sort(Eina_Model *model, Eina_Compare_Cb compare)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, EINA_FALSE);
   EINA_MODEL_TYPE_CALL(model, child_sort, compare);
   _eina_model_event_callback_call
     (model, _eina_model_str_children_changed, NULL);
   return EINA_TRUE;
}

EAPI Eina_Iterator *
eina_model_child_iterator_get(Eina_Model *model)
{
   int count;
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   count = eina_model_child_count(model);
   if (count < 0)
     return NULL;
   EINA_MODEL_TYPE_CALL_RETURN(model, child_iterator_get, NULL, 0, count);
}

EAPI Eina_Iterator *
eina_model_child_slice_iterator_get(Eina_Model *model, unsigned int start, unsigned int count)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_MODEL_TYPE_CALL_RETURN(model, child_iterator_get, NULL, start, count);
}

EAPI Eina_Iterator *
eina_model_child_reversed_iterator_get(Eina_Model *model)
{
   int count;
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   count = eina_model_child_count(model);
   if (count < 0)
     return NULL;
   EINA_MODEL_TYPE_CALL_RETURN(model, child_reversed_iterator_get, NULL,
                               0, count);
}

EAPI Eina_Iterator *
eina_model_child_slice_reversed_iterator_get(Eina_Model *model, unsigned int start, unsigned int count)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_MODEL_TYPE_CALL_RETURN(model, child_reversed_iterator_get, NULL,
                               start, count);
}

EAPI Eina_Iterator *
eina_model_child_sorted_iterator_get(Eina_Model *model, Eina_Compare_Cb compare)
{
   int count;
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, NULL);
   count = eina_model_child_count(model);
   if (count < 0)
     return NULL;
   EINA_MODEL_TYPE_CALL_RETURN(model, child_sorted_iterator_get, NULL,
                               0, count, compare);
}

EAPI Eina_Iterator *
eina_model_child_slice_sorted_iterator_get(Eina_Model *model, unsigned int start, unsigned int count, Eina_Compare_Cb compare)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, NULL);
   EINA_MODEL_TYPE_CALL_RETURN(model, child_sorted_iterator_get, NULL,
                               start, count, compare);
}

EAPI Eina_Iterator *
eina_model_child_filtered_iterator_get(Eina_Model *model, Eina_Each_Cb match, const void *data)
{
   int count;
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(match, NULL);
   count = eina_model_child_count(model);
   if (count < 0)
     return NULL;
   EINA_MODEL_TYPE_CALL_RETURN(model, child_filtered_iterator_get, NULL,
                               0, count, match, data);
}

EAPI Eina_Iterator *
eina_model_child_slice_filtered_iterator_get(Eina_Model *model, unsigned int start, unsigned int count, Eina_Each_Cb match, const void *data)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(match, NULL);
   EINA_MODEL_TYPE_CALL_RETURN(model, child_filtered_iterator_get, NULL,
                               start, count, match, data);
}

EAPI char *
eina_model_to_string(const Eina_Model *model)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_MODEL_TYPE_CALL_RETURN(model, to_string, NULL);
}

/* type functions *****************************************************/

EAPI Eina_Bool
eina_model_type_check(const Eina_Model_Type *type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(type, EINA_FALSE);
   return _eina_model_type_check(type);
}

EAPI const char *
eina_model_type_name_get(const Eina_Model_Type *type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(type, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_type_check(type), NULL);
   return type->name;
}

EAPI const Eina_Model_Type *
eina_model_type_parent_get(const Eina_Model_Type *type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(type, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_type_check(type), NULL);
   return type->parent;
}

#define EINA_MODEL_TYPE_INSTANCE_CHECK(type, model)                     \
  EINA_SAFETY_ON_NULL_RETURN(type);                                     \
  EINA_SAFETY_ON_FALSE_RETURN(_eina_model_type_check(type));            \
  EINA_MODEL_INSTANCE_CHECK(model);                                     \
  EINA_SAFETY_ON_FALSE_RETURN(_eina_model_instance_check(model, type));

#define EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, retval)         \
  EINA_SAFETY_ON_NULL_RETURN_VAL(type, retval);                         \
  EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_type_check(type), retval); \
  EINA_MODEL_INSTANCE_CHECK_VAL(model, retval);                         \
  EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_instance_check(model, type), retval);

EAPI Eina_Bool
eina_model_type_constructor(const Eina_Model_Type *type, Eina_Model *model)
{
   Eina_Bool (*constructor)(Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);

   constructor = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, constructor));
   EINA_SAFETY_ON_NULL_RETURN_VAL(constructor, EINA_FALSE);

   return constructor(model);
}

EAPI Eina_Bool
eina_model_type_destructor(const Eina_Model_Type *type, Eina_Model *model)
{
   Eina_Bool (*destructor)(Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);

   destructor = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, destructor));
   EINA_SAFETY_ON_NULL_RETURN_VAL(destructor, EINA_FALSE);

   return destructor(model);
}

EAPI Eina_Bool
eina_model_type_copy(const Eina_Model_Type *type, const Eina_Model *src, Eina_Model *dst)
{
   Eina_Bool (*copy)(const Eina_Model *, Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, src, EINA_FALSE);
   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, dst, EINA_FALSE);

   copy = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, copy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(copy, EINA_FALSE);

   return copy(src, dst);
}

EAPI Eina_Bool
eina_model_type_deep_copy(const Eina_Model_Type *type, const Eina_Model *src, Eina_Model *dst)
{
   Eina_Bool (*deep_copy)(const Eina_Model *, Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, src, EINA_FALSE);
   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, dst, EINA_FALSE);

   deep_copy = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, deep_copy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(deep_copy, EINA_FALSE);

   return deep_copy(src, dst);
}

EAPI Eina_Bool
eina_model_type_compare(const Eina_Model_Type *type, const Eina_Model *a, const Eina_Model *b, int *cmp)
{
   Eina_Bool (*compare)(const Eina_Model *, const Eina_Model *, int *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(cmp, EINA_FALSE);
   *cmp = 0;

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, a, EINA_FALSE);
   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, b, EINA_FALSE);

   compare = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, compare));
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, EINA_FALSE);

   return compare(a, b, cmp);
}

EAPI Eina_Bool
eina_model_type_load(const Eina_Model_Type *type, Eina_Model *model)
{
   Eina_Bool (*load)(Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);

   load = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, load));
   EINA_SAFETY_ON_NULL_RETURN_VAL(load, EINA_FALSE);

   return load(model);
}

EAPI Eina_Bool
eina_model_type_unload(const Eina_Model_Type *type, Eina_Model *model)
{
   Eina_Bool (*unload)(Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);

   unload = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, unload));
   EINA_SAFETY_ON_NULL_RETURN_VAL(unload, EINA_FALSE);

   return unload(model);
}

EAPI Eina_Bool
eina_model_type_property_get(const Eina_Model_Type *type, const Eina_Model *model, const char *name, Eina_Value *value)
{
   Eina_Bool (*property_get)(const Eina_Model *, const char *, Eina_Value *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);

   property_get = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, property_get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(property_get, EINA_FALSE);

   return property_get(model, name, value);
}

EAPI Eina_Bool
eina_model_type_property_set(const Eina_Model_Type *type, Eina_Model *model, const char *name, const Eina_Value *value)
{
   Eina_Bool (*property_set)(Eina_Model *, const char *, const Eina_Value *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(value->type), EINA_FALSE);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);

   property_set = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, property_set));
   EINA_SAFETY_ON_NULL_RETURN_VAL(property_set, EINA_FALSE);

   return property_set(model, name, value);
}

EAPI Eina_Bool
eina_model_type_property_del(const Eina_Model_Type *type, Eina_Model *model, const char *name)
{
   Eina_Bool (*property_del)(const Eina_Model *, const char *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);

   property_del = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, property_del));
   EINA_SAFETY_ON_NULL_RETURN_VAL(property_del, EINA_FALSE);

   return property_del(model, name);
}

EAPI Eina_List *
eina_model_type_properties_names_list_get(const Eina_Model_Type *type, const Eina_Model *model)
{
   Eina_List *(*properties_names_list_get)(const Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, NULL);

   properties_names_list_get = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, properties_names_list_get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(properties_names_list_get, NULL);

   return properties_names_list_get(model);
}

EAPI int
eina_model_type_child_count(const Eina_Model_Type *type, const Eina_Model *model)
{
   int (*child_count)(const Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, -1);

   child_count = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_count));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_count, -1);

   return child_count(model);
}

EAPI Eina_Model *
eina_model_type_child_get(const Eina_Model_Type *type, const Eina_Model *model, unsigned int position)
{
   Eina_Model *(*child_get)(const Eina_Model *, unsigned int);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, NULL);

   child_get = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_get, NULL);

   return child_get(model, position);
}

EAPI Eina_Bool
eina_model_type_child_set(const Eina_Model_Type *type, Eina_Model *model, unsigned int position, Eina_Model *child)
{
   Eina_Bool (*child_set)(Eina_Model *, unsigned int, Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);
   EINA_MODEL_INSTANCE_CHECK_VAL(child, EINA_FALSE);

   child_set = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_set));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_set, EINA_FALSE);

   return child_set(model, position, child);
}

EAPI Eina_Bool
eina_model_type_child_del(const Eina_Model_Type *type, Eina_Model *model, unsigned int position)
{
   Eina_Bool (*child_del)(Eina_Model *, unsigned int);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);

   child_del = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_del));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_del, EINA_FALSE);

   return child_del(model, position);
}

EAPI Eina_Bool
eina_model_type_child_insert_at(const Eina_Model_Type *type, Eina_Model *model, unsigned int position, Eina_Model *child)
{
   Eina_Bool (*child_insert_at)(Eina_Model *, unsigned int, Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, EINA_FALSE);
   EINA_MODEL_INSTANCE_CHECK_VAL(child, EINA_FALSE);

   child_insert_at = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_insert_at));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_insert_at, EINA_FALSE);

   return child_insert_at(model, position, child);
}

EAPI int
eina_model_type_child_find(const Eina_Model_Type *type, const Eina_Model *model, unsigned int start_position, const Eina_Model *other)
{
   int (*child_find)(const Eina_Model *, unsigned int, const Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, -1);
   EINA_MODEL_INSTANCE_CHECK_VAL(other, -1);

   child_find = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_find));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_find, -1);

   return child_find(model, start_position, other);
}

EAPI int
eina_model_type_child_criteria_match(const Eina_Model_Type *type, const Eina_Model *model, unsigned int start_position, Eina_Each_Cb match, const void *data)
{
   int (*child_criteria_match)(const Eina_Model *, unsigned int, Eina_Each_Cb, const void *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(match, -1);
   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, -1);

   child_criteria_match = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_criteria_match));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_criteria_match, -1);

   return child_criteria_match(model, start_position, match, data);
}

EAPI void
eina_model_type_child_sort(const Eina_Model_Type *type, Eina_Model *model, Eina_Compare_Cb compare)
{
   void (*child_sort)(Eina_Model *, Eina_Compare_Cb);

   EINA_SAFETY_ON_NULL_RETURN(compare);
   EINA_MODEL_TYPE_INSTANCE_CHECK(type, model);

   child_sort = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_sort));
   EINA_SAFETY_ON_NULL_RETURN(child_sort);

   return child_sort(model, compare);
}

EAPI Eina_Iterator *
eina_model_type_child_iterator_get(const Eina_Model_Type *type, Eina_Model *model, unsigned int start, unsigned int count)
{
   Eina_Iterator *(*child_iterator_get)(const Eina_Model *, unsigned int, unsigned int);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, NULL);

   child_iterator_get = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_iterator_get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_iterator_get, NULL);

   return child_iterator_get(model, start, count);
}

EAPI Eina_Iterator *
eina_model_type_child_reversed_iterator_get(const Eina_Model_Type *type, Eina_Model *model, unsigned int start, unsigned int count)
{
   Eina_Iterator *(*child_reversed_iterator_get)(const Eina_Model *, unsigned int, unsigned int);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, NULL);

   child_reversed_iterator_get = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_reversed_iterator_get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_reversed_iterator_get, NULL);

   return child_reversed_iterator_get(model, start, count);
}

EAPI Eina_Iterator *
eina_model_type_child_sorted_iterator_get(const Eina_Model_Type *type, Eina_Model *model, unsigned int start, unsigned int count, Eina_Compare_Cb compare)
{
   Eina_Iterator *(*child_sorted_iterator_get)(const Eina_Model *, unsigned int, unsigned int, Eina_Compare_Cb);

   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, NULL);
   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, NULL);

   child_sorted_iterator_get = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_sorted_iterator_get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_sorted_iterator_get, NULL);

   return child_sorted_iterator_get(model, start, count, compare);
}

EAPI Eina_Iterator *
eina_model_type_child_filtered_iterator_get(const Eina_Model_Type *type, Eina_Model *model, unsigned int start, unsigned int count, Eina_Each_Cb match, const void *data)
{
   Eina_Iterator *(*child_filtered_iterator_get)(const Eina_Model *, unsigned int, unsigned int, Eina_Each_Cb, const void *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(match, NULL);
   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, NULL);

   child_filtered_iterator_get = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, child_filtered_iterator_get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_filtered_iterator_get, NULL);

   return child_filtered_iterator_get(model, start, count, match, data);
}

EAPI char *
eina_model_type_to_string(const Eina_Model_Type *type, const Eina_Model *model)
{
   char *(*to_string)(const Eina_Model *);

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, NULL);

   to_string = _eina_model_type_find_offset
     (type, offsetof(Eina_Model_Type, to_string));
   EINA_SAFETY_ON_NULL_RETURN_VAL(to_string, NULL);

   return to_string(model);
}

EAPI Eina_Bool
eina_model_type_subclass_setup(Eina_Model_Type *type, const Eina_Model_Type *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(type, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_type_check(parent), EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(type->version == EINA_MODEL_TYPE_VERSION,
                                   EINA_FALSE);

   type->parent = parent;
   type->type_size = parent->type_size;
   type->interfaces = NULL;
   type->events = NULL;

   type->setup = NULL;
   type->flush = NULL;
   type->constructor = NULL;
   type->destructor = NULL;
   type->copy = NULL;
   type->deep_copy = NULL;
   type->compare = NULL;
   type->load = NULL;
   type->unload = NULL;
   type->property_get = NULL;
   type->property_set = NULL;
   type->property_del = NULL;
   type->properties_names_list_get = NULL;
   type->child_count = NULL;
   type->child_get = NULL;
   type->child_set = NULL;
   type->child_del = NULL;
   type->child_insert_at = NULL;
   type->child_find = NULL;
   type->child_criteria_match = NULL;
   type->child_sort = NULL;
   type->child_iterator_get = NULL;
   type->child_reversed_iterator_get = NULL;
   type->child_sorted_iterator_get = NULL;
   type->child_filtered_iterator_get = NULL;
   type->to_string = NULL;
   type->__extension_ptr0 = NULL;
   type->__extension_ptr1 = NULL;
   type->__extension_ptr2 = NULL;
   type->__extension_ptr3 = NULL;

   if (type->type_size > sizeof(Eina_Model_Type))
     {
        unsigned char *p = (unsigned char *)type;
        p += sizeof(Eina_Model_Type);
        memset(p, 0, type->type_size - sizeof(Eina_Model_Type));
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
eina_model_type_subclass_check(const Eina_Model_Type *type, const Eina_Model_Type *self_or_parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(type, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_type_check(type), EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(self_or_parent, EINA_FALSE);

   for (; type != NULL; type = type->parent)
     {
        if (type == self_or_parent)
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

static inline const Eina_Model_Interface *
_eina_model_type_interface_get(const Eina_Model_Type *type, const char *name, Eina_Bool ptr_cmp __UNUSED__)
{
   const Eina_Model_Interface **itr;

   if (!type)
     return NULL;

   if (!type->interfaces)
     return _eina_model_type_interface_get(type->parent, name, ptr_cmp);

     {
        for (itr = type->interfaces ; itr != NULL ; itr++)
          if (strcmp((*itr)->name, name) == 0)
            return *itr;
     }

   return NULL;
}

static inline Eina_Bool
_eina_model_interface_check(const Eina_Model_Interface *iface)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (iface->version == EINA_MODEL_INTERFACE_VERSION, EINA_FALSE);
   return EINA_TRUE;
}

EAPI const Eina_Model_Interface *
eina_model_type_interface_get(const Eina_Model_Type *type, const char *name)
{
   const Eina_Model_Interface *iface;

   EINA_SAFETY_ON_NULL_RETURN_VAL(type, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_type_check(type), NULL);

   /* search for pointer, make speed-aware users fast */
   iface = _eina_model_type_interface_get(type, name, EINA_TRUE);

   if (!iface)
     {
        /* search using strcmp(), slow users don't care */
        iface = _eina_model_type_interface_get(type, name, EINA_FALSE);
     }
   else if (!_eina_model_interface_check(iface))
     iface = NULL;

   return iface;
}

EAPI void *
eina_model_type_private_data_get(const Eina_Model *model, const Eina_Model_Type *type)
{
   const Eina_Model_Description *desc;
   unsigned int i;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(type, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_type_check(type), NULL);

   desc = model->desc;

   for (i = 0; i < desc->total.types; i++)
     if (desc->cache.types[i] == type)
       return model->privates[i];

   CRITICAL("Model %p (%s) is not an instance of type %p (%s)",
            model, desc->cache.types[0]->name,
            type, type->name);
   return NULL;
}

EAPI const void *
eina_model_method_offset_resolve(const Eina_Model *model, unsigned int offset)
{
   const Eina_Model_Description *desc;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(offset >= sizeof(Eina_Model_Type), NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(offset % sizeof(void *) == 0, NULL);

   desc = model->desc;
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (offset + sizeof(void *) <= desc->cache.types[0]->type_size, NULL);

   offset -= sizeof(Eina_Model_Type);
   offset /= sizeof(void *);
   return desc->ops.type.extension[offset];
}

EAPI const void *
eina_model_type_method_offset_resolve(const Eina_Model_Type *type, const Eina_Model *model, unsigned int offset)
{
   const Eina_Model_Description *desc;

   EINA_MODEL_TYPE_INSTANCE_CHECK_VAL(type, model, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(offset >= sizeof(Eina_Model_Type), NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(offset % sizeof(void *) == 0, NULL);

   desc = model->desc;
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (offset + sizeof(void *) <= desc->cache.types[0]->type_size, NULL);

   return _eina_model_type_find_offset(type, offset);
}

/* interface functions ************************************************/

EAPI Eina_Bool
eina_model_interface_check(const Eina_Model_Interface *iface)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);
   return _eina_model_interface_check(iface);
}

EAPI void *
eina_model_interface_private_data_get(const Eina_Model *model, const Eina_Model_Interface *iface)
{
   const Eina_Model_Description *desc;
   unsigned int i;

   EINA_MODEL_INSTANCE_CHECK_VAL(model, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_interface_check(iface), NULL);

   desc = model->desc;

   for (i = 0; i < desc->total.ifaces; i++)
     if (desc->cache.ifaces[i] == iface)
       return model->privates[desc->total.types + i];

   CRITICAL("Model %p (%s) does not implement interface %p (%s)",
            model, desc->cache.types[0]->name,
            iface, iface->name);
   return NULL;
}

static Eina_Bool
_eina_model_interface_implemented(const Eina_Model *model, const Eina_Model_Interface *iface)
{
   const Eina_Model_Interface **itr, **itr_end;

   itr = model->desc->cache.ifaces;
   itr_end = itr + model->desc->total.ifaces;

   for (; itr < itr_end; itr++)
     if (*itr == iface)
       return EINA_TRUE;

   return EINA_FALSE;
}

EAPI Eina_Bool
eina_model_interface_implemented(const Eina_Model *model, const Eina_Model_Interface *iface)
{
   EINA_MODEL_INSTANCE_CHECK_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_interface_check(iface),
                                   EINA_FALSE);
   return _eina_model_interface_implemented(model, iface);
}

#define EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK(iface, model)            \
  EINA_SAFETY_ON_NULL_RETURN(iface);                                    \
  EINA_SAFETY_ON_FALSE_RETURN(_eina_model_interface_check(iface));      \
  EINA_MODEL_INSTANCE_CHECK(model);                                     \
  EINA_SAFETY_ON_FALSE_RETURN(_eina_model_interface_implemented(model, iface));

#define EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, retval) \
  EINA_SAFETY_ON_NULL_RETURN_VAL(iface, retval);                        \
  EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_interface_check(iface), retval); \
  EINA_MODEL_INSTANCE_CHECK_VAL(model, retval);                         \
  EINA_SAFETY_ON_FALSE_RETURN_VAL(_eina_model_interface_implemented(model, iface), retval);


EAPI Eina_Bool
eina_model_interface_constructor(const Eina_Model_Interface *iface, Eina_Model *model)
{
   Eina_Bool (*constructor)(Eina_Model *);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   constructor = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface, constructor));
   EINA_SAFETY_ON_NULL_RETURN_VAL(constructor, EINA_FALSE);
   return constructor(model);
}

EAPI Eina_Bool
eina_model_interface_destructor(const Eina_Model_Interface *iface, Eina_Model *model)
{
   Eina_Bool (*destructor)(Eina_Model *);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   destructor = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface, destructor));
   EINA_SAFETY_ON_NULL_RETURN_VAL(destructor, EINA_FALSE);
   return destructor(model);
}

EAPI Eina_Bool
eina_model_interface_copy(const Eina_Model_Interface *iface, const Eina_Model *src, Eina_Model *dst)
{
   Eina_Bool (*copy)(const Eina_Model *, Eina_Model *);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, src, EINA_FALSE);
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, dst, EINA_FALSE);

   copy = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface, copy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(copy, EINA_FALSE);
   return copy(src, dst);
}

EAPI Eina_Bool
eina_model_interface_deep_copy(const Eina_Model_Interface *iface, const Eina_Model *src, Eina_Model *dst)
{
   Eina_Bool (*deep_copy)(const Eina_Model *, Eina_Model *);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, src, EINA_FALSE);
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, dst, EINA_FALSE);

   deep_copy = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface, deep_copy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(deep_copy, EINA_FALSE);
   return deep_copy(src, dst);
}

EAPI const void
*eina_model_interface_method_offset_resolve(const Eina_Model_Interface *iface, const Eina_Model *model, unsigned int offset)
{
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(offset >= sizeof(Eina_Model_Interface), NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(offset % sizeof(void *) == 0, NULL);
   return _eina_model_interface_find_offset(iface, offset);
}


/* Eina_Model_Interface_Properties ************************************/

EAPI Eina_Bool
eina_model_interface_properties_compare(const Eina_Model_Interface *iface, const Eina_Model *a, const Eina_Model *b, int *cmp)
{
   Eina_Bool (*compare)(const Eina_Model *, const Eina_Model *, int *cmp);

   EINA_SAFETY_ON_NULL_RETURN_VAL(cmp, EINA_FALSE);

   *cmp = 0;
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, a, EINA_FALSE);
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, b, EINA_FALSE);

   compare = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Properties, compare));
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, EINA_FALSE);
   return compare(a, b, cmp);
}

EAPI Eina_Bool
eina_model_interface_properties_load(const Eina_Model_Interface *iface, Eina_Model *model)
{
   Eina_Bool (*load)(Eina_Model *);
   Eina_Bool ret;

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   load = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Properties, load));
   EINA_SAFETY_ON_NULL_RETURN_VAL(load, EINA_FALSE);
   ret = load(model);

   if (ret)
     _eina_model_event_callback_call
       (model, _eina_model_str_properties_loaded, NULL);

   return ret;
}

EAPI Eina_Bool
eina_model_interface_properties_unload(const Eina_Model_Interface *iface, Eina_Model *model)
{
   Eina_Bool (*unload)(Eina_Model *);
   Eina_Bool ret;

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   unload = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Properties, unload));
   EINA_SAFETY_ON_NULL_RETURN_VAL(unload, EINA_FALSE);
   ret = unload(model);

   if (ret)
     _eina_model_event_callback_call
       (model, _eina_model_str_properties_unloaded, NULL);

   return ret;
}

EAPI Eina_Bool
eina_model_interface_properties_get(const Eina_Model_Interface *iface, const Eina_Model *model, const char *name, Eina_Value *value)
{
   Eina_Bool (*get)(const Eina_Model *, const char *, Eina_Value *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   get = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Properties, get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(get, EINA_FALSE);
   return get(model, name, value);
}

EAPI Eina_Bool
eina_model_interface_properties_set(const Eina_Model_Interface *iface, Eina_Model *model, const char *name, const Eina_Value *value)
{
   Eina_Bool (*set)(Eina_Model *, const char *, const Eina_Value *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(value->type), EINA_FALSE);
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   set = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Properties, set));
   EINA_SAFETY_ON_NULL_RETURN_VAL(set, EINA_FALSE);
   return set(model, name, value);
}

EAPI Eina_Bool
eina_model_interface_properties_del(const Eina_Model_Interface *iface, Eina_Model *model, const char *name)
{
   Eina_Bool (*del)(Eina_Model *, const char *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   del = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Properties, del));
   EINA_SAFETY_ON_NULL_RETURN_VAL(del, EINA_FALSE);
   return del(model, name);
}

EAPI Eina_List *
eina_model_interface_properties_names_list_get(const Eina_Model_Interface *iface, const Eina_Model *model)
{
   Eina_List *(*names_list_get)(const Eina_Model *);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, NULL);

   names_list_get = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Properties, names_list_get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(names_list_get, NULL);
   return names_list_get(model);
}

/* Eina_Model_Interface_Children **************************************/

EAPI Eina_Bool
eina_model_interface_children_compare(const Eina_Model_Interface *iface, const Eina_Model *a, const Eina_Model *b, int *cmp)
{
   Eina_Bool (*compare)(const Eina_Model *, const Eina_Model *, int *);

   EINA_SAFETY_ON_NULL_RETURN_VAL(cmp, EINA_FALSE);

   *cmp = 0;

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, a, EINA_FALSE);
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, b, EINA_FALSE);

   compare = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Children, compare));
   EINA_SAFETY_ON_NULL_RETURN_VAL(compare, EINA_FALSE);
   return compare(a, b, cmp);
}

EAPI Eina_Bool
eina_model_interface_children_load(const Eina_Model_Interface *iface, Eina_Model *model)
{
   Eina_Bool (*load)(Eina_Model *);
   Eina_Bool ret;

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   load = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Children, load));
   EINA_SAFETY_ON_NULL_RETURN_VAL(load, EINA_FALSE);
   ret = load(model);

   if (ret)
     _eina_model_event_callback_call
       (model, _eina_model_str_children_loaded, NULL);

   return ret;
}

EAPI Eina_Bool
eina_model_interface_children_unload(const Eina_Model_Interface *iface, Eina_Model *model)
{
   Eina_Bool (*unload)(Eina_Model *);
   Eina_Bool ret;

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   unload = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Children, unload));
   EINA_SAFETY_ON_NULL_RETURN_VAL(unload, EINA_FALSE);
   ret = unload(model);

   if (ret)
     _eina_model_event_callback_call
       (model, _eina_model_str_children_unloaded, NULL);

   return ret;
}

EAPI int
eina_model_interface_children_count(const Eina_Model_Interface *iface, const Eina_Model *model)
{
   int (*count)(const Eina_Model *);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, -1);

   count = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Children, count));
   EINA_SAFETY_ON_NULL_RETURN_VAL(count, -1);
   return count(model);
}

EAPI Eina_Model *
eina_model_interface_children_get(const Eina_Model_Interface *iface, const Eina_Model *model, unsigned int position)
{
   Eina_Model *(*get)(const Eina_Model *, unsigned int);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, NULL);

   get = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Children, get));
   EINA_SAFETY_ON_NULL_RETURN_VAL(get, NULL);
   return get(model, position);
}

EAPI Eina_Bool eina_model_interface_children_set(const Eina_Model_Interface *iface, Eina_Model *model, unsigned int position, Eina_Model *child)
{
   Eina_Bool (*set)(const Eina_Model *, unsigned int, Eina_Model *);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);
   EINA_MODEL_INSTANCE_CHECK_VAL(child, EINA_FALSE);

   set = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Children, set));
   EINA_SAFETY_ON_NULL_RETURN_VAL(set, EINA_FALSE);
   return set(model, position, child);
}

EAPI Eina_Bool
eina_model_interface_children_del(const Eina_Model_Interface *iface, Eina_Model *model, unsigned int position)
{
   Eina_Bool (*del)(Eina_Model *, unsigned int);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);

   del = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Children, del));
   EINA_SAFETY_ON_NULL_RETURN_VAL(del, EINA_FALSE);
   return del(model, position);
}


EAPI Eina_Bool
eina_model_interface_children_insert_at(const Eina_Model_Interface *iface, Eina_Model *model, unsigned int position, Eina_Model *child)
{
   Eina_Bool (*insert_at)(const Eina_Model *, unsigned int, Eina_Model *);

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL(iface, model, EINA_FALSE);
   EINA_MODEL_INSTANCE_CHECK_VAL(child, EINA_FALSE);

   insert_at = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Children, insert_at));
   EINA_SAFETY_ON_NULL_RETURN_VAL(insert_at, EINA_FALSE);
   return insert_at(model, position, child);
}

EAPI void
eina_model_interface_children_sort(const Eina_Model_Interface *iface, Eina_Model *model, Eina_Compare_Cb compare)
{
   void (*sort)(const Eina_Model *, Eina_Compare_Cb);

   EINA_SAFETY_ON_NULL_RETURN(compare);
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK(iface, model);

   sort = _eina_model_interface_find_offset
     (iface, offsetof(Eina_Model_Interface_Children, sort));
   EINA_SAFETY_ON_NULL_RETURN(sort);
   return sort(model, compare);
}

static Eina_Bool
_eina_model_struct_set(Eina_Model *m, const Eina_Value_Struct_Desc *desc, void *memory)
{
   Eina_Value_Struct st = {desc, memory};
   Eina_Value *val = eina_model_interface_private_data_get
     (m, &_EINA_MODEL_INTERFACE_PROPERTIES_STRUCT.base);
   return eina_value_pset(val, &st);
}

EAPI Eina_Model *
eina_model_struct_new(const Eina_Value_Struct_Desc *desc)
{
   Eina_Model *m;

   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (desc->version == EINA_VALUE_STRUCT_DESC_VERSION, NULL);

   m = eina_model_new(EINA_MODEL_TYPE_STRUCT);
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   EINA_SAFETY_ON_FALSE_GOTO(_eina_model_struct_set(m, desc, NULL), error);
   return m;

 error:
   eina_model_del(m);
   return NULL;
}

EAPI Eina_Model *
eina_model_type_struct_new(const Eina_Model_Type *type, const Eina_Value_Struct_Desc *desc)
{
   Eina_Model *m;

   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (eina_model_type_subclass_check(type, EINA_MODEL_TYPE_STRUCT), NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (desc->version == EINA_VALUE_STRUCT_DESC_VERSION, NULL);

   m = eina_model_new(type);
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   EINA_SAFETY_ON_FALSE_GOTO(_eina_model_struct_set(m, desc, NULL), error);
   return m;

 error:
   eina_model_del(m);
   return NULL;
}

EAPI Eina_Bool
eina_model_struct_set(Eina_Model *model, const Eina_Value_Struct_Desc *desc, void *memory)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (desc->version == EINA_VALUE_STRUCT_DESC_VERSION, EINA_FALSE);
   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL
     (&_EINA_MODEL_INTERFACE_PROPERTIES_STRUCT.base, model, EINA_FALSE);

   return _eina_model_struct_set(model, desc, memory);
}

EAPI Eina_Bool
eina_model_struct_get(const Eina_Model *model, const Eina_Value_Struct_Desc **p_desc, void **p_memory)
{
   const Eina_Value *val;
   Eina_Value_Struct st;

   EINA_SAFETY_ON_NULL_RETURN_VAL(p_desc, EINA_FALSE);

   *p_desc = NULL;
   if (p_memory) *p_memory = NULL;

   EINA_MODEL_INTERFACE_IMPLEMENTED_CHECK_VAL
     (&_EINA_MODEL_INTERFACE_PROPERTIES_STRUCT.base, model, EINA_FALSE);

   val = eina_model_interface_private_data_get
     (model, &_EINA_MODEL_INTERFACE_PROPERTIES_STRUCT.base);

   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_pget(val, &st), EINA_FALSE);

   *p_desc = st.desc;
   if (p_memory) *p_memory = st.memory;
   return EINA_FALSE;
}

EAPI void
eina_models_usage_dump(void)
{
   const Eina_List *l;
   const Eina_Model *m;

   eina_lock_take(&_eina_model_debug_list_lock);

   puts("DDD: model          refs           info (type, holders, backtrace)");
   puts("DDD: -------------- -------------- ---------------------------------");

   EINA_LIST_FOREACH(_eina_model_debug_list, l, m)
     {
        Eina_Model_XRef *ref;

        printf("DDD: %14p %14d %s\n",
               m, m->refcount, m->desc->cache.types[0]->name);

        EINA_INLIST_FOREACH(m->xrefs, ref)
          {
             printf("DDD:                               id: %p '%s'\n",
                    ref->id, ref->label);
             if (ref->backtrace.count)
               {
                  char **symbols;
                  unsigned int i;

#ifdef HAVE_BACKTRACE_SYMBOLS
                  symbols = backtrace_symbols((void * const *)ref->backtrace.symbols,
                                              ref->backtrace.count);
#else
                  symbols = NULL;
#endif

                  printf("DDD:            Backtrace: Address        Symbol\n");
                  for (i = 0; i < ref->backtrace.count; i++)
                    printf("DDD:                       %14p %s\n",
                           ref->backtrace.symbols[i],
                           symbols ? symbols[i] : "???");

                  free(symbols);
                  puts("DDD:");
               }
          }
     }

   eina_lock_release(&_eina_model_debug_list_lock);
}

EAPI Eina_List *
eina_models_list_get(void)
{
   const Eina_List *l;
   Eina_Model *m;
   Eina_List *ret = NULL;

   eina_lock_take(&_eina_model_debug_list_lock);

   EINA_LIST_FOREACH(_eina_model_debug_list, l, m)
     {
        ret = eina_list_append
          (ret, eina_model_xref
           (m, eina_models_list_get, "eina_models_list_get"));
     }

   eina_lock_release(&_eina_model_debug_list_lock);

   return ret;
}

EAPI void
eina_models_list_free(Eina_List *list)
{
   Eina_Model *m;

   EINA_LIST_FREE(list, m)
     eina_model_xunref(m, eina_models_list_get);
}
