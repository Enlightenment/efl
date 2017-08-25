#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined HAVE_DLADDR && ! defined _WIN32
# include <dlfcn.h>
#endif

#include <Eina.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
# include <mach/mach_time.h>
#endif

#include "Eo.h"
#include "eo_ptr_indirection.h"
#include "eo_private.h"
#include "eo_add_fallback.h"

#include "efl_object_override.eo.c"
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifdef HAVE_VALGRIND
# include <valgrind.h>
# include <memcheck.h>
#endif
static Eina_Bool _eo_trash_bypass = EINA_FALSE;

#define EO_CLASS_IDS_FIRST 1
#define EFL_OBJECT_OP_IDS_FIRST 1

/* Used inside the class_get functions of classes, see #EFL_DEFINE_CLASS */
EAPI Eina_Lock _efl_class_creation_lock;
EAPI unsigned int _efl_object_init_generation = 1;
int _eo_log_dom = -1;
Eina_Thread _efl_object_main_thread;

static inline void _eo_log_obj_init(void);
static inline void _eo_log_obj_shutdown(void);
static inline void _eo_log_obj_new(const _Eo_Object *obj);
static inline void _eo_log_obj_free(const _Eo_Object *obj);
#ifdef EO_DEBUG
static int _eo_log_objs_dom = -1;
static Eina_Bool _eo_log_objs_enabled = EINA_FALSE;
static Eina_Inarray _eo_log_objs_debug;
static Eina_Inarray _eo_log_objs_no_debug;
static double _eo_log_time_start;
#ifdef HAVE_BACKTRACE
static Eina_Array _eo_log_objs;
static Eina_Spinlock _eo_log_objs_lock;
#endif
#else
static inline void _eo_log_obj_init(void) { }
static inline void _eo_log_obj_shutdown(void) { }
static inline void _eo_log_obj_new(const _Eo_Object *obj EINA_UNUSED) { }
static inline void _eo_log_obj_free(const _Eo_Object *obj EINA_UNUSED) { }
void _eo_log_obj_report(const Eo_Id id EINA_UNUSED, int log_level EINA_UNUSED, const char *func_name EINA_UNUSED, const char *file EINA_UNUSED, int line EINA_UNUSED) { }
#endif

static _Efl_Class **_eo_classes = NULL;
static Eo_Id _eo_classes_last_id = 0;
static Eo_Id _eo_classes_alloc = 0;
static int _efl_object_init_count = 0;
static Efl_Object_Op _eo_ops_last_id = 0;
static Eina_Hash *_ops_storage = NULL;
static Eina_Spinlock _ops_storage_lock;

static const Efl_Object_Optional efl_object_optional_cow_default = {};
Eina_Cow *efl_object_optional_cow = NULL;

static size_t _eo_sz = 0;
static size_t _eo_class_sz = 0;

static void _eo_condtor_reset(_Eo_Object *obj);
static inline void *_efl_data_scope_get(const _Eo_Object *obj, const _Efl_Class *klass);
static inline void *_efl_data_xref_internal(const char *file, int line, _Eo_Object *obj, const _Efl_Class *klass, const _Eo_Object *ref_obj);
static inline void _efl_data_xunref_internal(_Eo_Object *obj, void *data, const _Eo_Object *ref_obj);
static void _vtable_init(Eo_Vtable *vtable, size_t size);

/* Start of Dich */


/* We are substracting the mask here instead of "AND"ing because it's a hot path,
 * it should be a valid class at this point, and this lets the compiler do 1
 * substraction at compile time. */
#define _UNMASK_ID(id) ((id) - MASK_CLASS_TAG)
#define ID_CLASS_GET(id) ({ \
      (_Efl_Class *) (((_UNMASK_ID(id) <= _eo_classes_last_id) && (_UNMASK_ID(id) > 0)) ? \
      (_eo_classes[_UNMASK_ID(id) - 1]) : NULL); \
      })

static inline void
_vtable_chain2_unref(Dich_Chain2 *chain)
{
   if (--(chain->refcount) == 0)
     {
        free(chain);
     }
}

static inline void
_vtable_chain_alloc(Dich_Chain1 *chain1)
{
   chain1->chain2 = calloc(1, sizeof(*(chain1->chain2)));
   chain1->chain2->refcount = 1;
}

static inline void _vtable_chain_write_prepare(Dich_Chain1 *dst);

static inline void
_vtable_chain_merge(Dich_Chain1 *dst, const Dich_Chain1 *src)
{
   size_t j;
   const op_type_funcs *sf = src->chain2->funcs;
   op_type_funcs *df = dst->chain2->funcs;

   if (df == sf)
     {
        /* Skip if the chain is the same. */
        return;
     }

   for (j = 0 ; j < DICH_CHAIN_LAST_SIZE ; j++, df++, sf++)
     {
        if (sf->func && memcmp(df, sf, sizeof(*df)))
          {
             _vtable_chain_write_prepare(dst);
             df = dst->chain2->funcs + j;
             memcpy(df, sf, sizeof(*df));
          }
     }
}

static inline void
_vtable_chain_write_prepare(Dich_Chain1 *dst)
{
   if (!dst->chain2)
     {
        _vtable_chain_alloc(dst);
        return;
     }
   else if (dst->chain2->refcount == 1)
     {
        /* We own it, no need to duplicate */
        return;
     }

   Dich_Chain1 old;
   old.chain2 = dst->chain2;

   _vtable_chain_alloc(dst);
   _vtable_chain_merge(dst, &old);

   _vtable_chain2_unref(old.chain2);
}

static inline void
_vtable_chain_copy_ref(Dich_Chain1 *dst, const Dich_Chain1 *src)
{
   if (dst->chain2)
     {
        _vtable_chain_merge(dst, src);
     }
   else
     {
        dst->chain2 = src->chain2;
        dst->chain2->refcount++;
     }
}

static inline void
_vtable_copy_all(Eo_Vtable *dst, const Eo_Vtable *src)
{
   Efl_Object_Op i;
   const Dich_Chain1 *sc1 = src->chain;
   Dich_Chain1 *dc1 = dst->chain;
   for (i = 0 ; i < src->size ; i++, sc1++, dc1++)
     {
        if (sc1->chain2)
          {
             _vtable_chain_copy_ref(dc1, sc1);
          }
     }
}

static inline const op_type_funcs *
_vtable_func_get(const Eo_Vtable *vtable, Efl_Object_Op op)
{
   size_t idx1 = DICH_CHAIN1(op);
   if (EINA_UNLIKELY(idx1 >= vtable->size))
      return NULL;
   Dich_Chain1 *chain1 = &vtable->chain[idx1];
   if (EINA_UNLIKELY(!chain1->chain2))
      return NULL;
   return &chain1->chain2->funcs[DICH_CHAIN_LAST(op)];
}

/* XXX: Only used for a debug message below. Doesn't matter that it's slow. */
static const _Efl_Class *
_eo_op_class_get(Efl_Object_Op op)
{
   _Efl_Class **itr = _eo_classes;
   int mid, max, min;

   min = 0;
   max = _eo_classes_last_id - 1;
   while (min <= max)
     {
        mid = (min + max) / 2;

        if (itr[mid]->base_id + itr[mid]->ops_count < op)
           min = mid + 1;
        else if (itr[mid]->base_id  > op)
           max = mid - 1;
        else
           return itr[mid];
     }

   return NULL;
}

static inline Eina_Bool
_vtable_func_set(Eo_Vtable *vtable, const _Efl_Class *klass,
                 const _Efl_Class *hierarchy_klass, Efl_Object_Op op,
                 Eo_Op_Func_Type func, Eina_Bool allow_same_override)
{
   op_type_funcs *fsrc;
   size_t idx1 = DICH_CHAIN1(op);
   Dich_Chain1 *chain1;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(idx1 < vtable->size, EINA_FALSE);
   chain1 = &vtable->chain[idx1];
   _vtable_chain_write_prepare(chain1);
   fsrc = &chain1->chain2->funcs[DICH_CHAIN_LAST(op)];
   if (hierarchy_klass)
     {
        if (!func)
          {
             op_type_funcs *fsrc_orig;
             Dich_Chain1 *chain1_orig;

             chain1_orig = &hierarchy_klass->vtable.chain[idx1];
             fsrc_orig = &chain1_orig->chain2->funcs[DICH_CHAIN_LAST(op)];
             func = fsrc_orig->func;
             klass = fsrc_orig->src;
          }
     }
   else
     {
        if (!allow_same_override && (fsrc->src == klass))
          {
             const _Efl_Class *op_kls = _eo_op_class_get(op);
             ERR("Class '%s': Overriding already set func %p for op %d (%s) with %p.",
                 klass->desc->name, fsrc->func, op, op_kls->desc->name, func);
             return EINA_FALSE;
          }
     }

   fsrc->func = func;
   fsrc->src = klass;

   return EINA_TRUE;
}

void
_vtable_func_clean_all(Eo_Vtable *vtable)
{
   size_t i;
   Dich_Chain1 *chain1 = vtable->chain;

   for (i = 0 ; i < vtable->size ; i++, chain1++)
     {
        if (chain1->chain2)
           _vtable_chain2_unref(chain1->chain2);
     }
   free(vtable->chain);
   vtable->chain = NULL;
}

/* END OF DICH */

#define _EO_ID_GET(Id) ((Eo_Id) (Id))


static inline Eina_Bool
_eo_is_a_obj(const Eo *eo_id)
{
   Eo_Id oid = (Eo_Id) _EO_ID_GET(eo_id);
   return !!(oid & MASK_OBJ_TAG);
}

static inline Eina_Bool
_eo_is_a_class(const Eo *eo_id)
{
   Eo_Id oid = (Eo_Id) _EO_ID_GET(eo_id);
   return !!(oid & MASK_CLASS_TAG);
}

static inline _Efl_Class *
_eo_class_pointer_get(const Efl_Class *klass_id)
{
   return ID_CLASS_GET((Eo_Id)klass_id);
}

static const char *
_eo_op_desc_name_get(const Efl_Op_Description *desc)
{
   static const char *fct_name = "unknown";

   if (!desc)
     {
        return fct_name;
     }

#ifndef _WIN32
# ifdef HAVE_DLADDR
   static Dl_info info;
   if (dladdr(desc->api_func, &info) != 0)
      fct_name = info.dli_sname;
# endif
#else
   fct_name = desc->api_func; /* Same on windows */
#endif
   return fct_name;
}

static inline const op_type_funcs *
_eo_kls_itr_next(const _Efl_Class *orig_kls, const _Efl_Class *cur_klass,
                 Efl_Object_Op op, Eina_Bool super)
{
   const _Efl_Class **kls_itr = NULL;

   /* Find the kls itr. */
   kls_itr = orig_kls->mro;
   while (*kls_itr && (*kls_itr != cur_klass))
      kls_itr++;

   if (*kls_itr)
     {
        if (super) kls_itr++;
        while (*kls_itr)
          {
             const op_type_funcs *fsrc = _vtable_func_get(&(*kls_itr)->vtable, op);
             if (!fsrc || !fsrc->func)
               {
                  kls_itr++;
                  continue;
               }
             return fsrc;
          }
     }

   return NULL;
}

/************************************ EO ************************************/

static EFL_FUNC_TLS _Efl_Class *_super_klass = NULL;

static Eo *
_efl_super_cast(const Eo *eo_id, const Efl_Class *cur_klass, Eina_Bool super)
{
   EO_CLASS_POINTER_GOTO(cur_klass, super_klass, err);

#ifdef EO_DEBUG
   if (EINA_UNLIKELY(!_eo_is_a_obj(eo_id) && !_eo_is_a_class(eo_id))) goto err_obj;
#endif

   if (EINA_UNLIKELY(!_eo_is_a_obj(eo_id)))
     goto do_klass;

#ifndef EO_DEBUG
   if (!super && EINA_UNLIKELY(!efl_isa(eo_id, cur_klass)))
#else
   if (EINA_UNLIKELY(!efl_isa(eo_id, cur_klass)))
#endif
     goto err_obj_hierarchy;

   EO_OBJ_POINTER_RETURN_VAL(eo_id, obj, NULL);
   obj->cur_klass = super_klass;
   obj->super = super;
   EO_OBJ_DONE(eo_id);

   return (Eo *) eo_id;

do_klass:
   // efl_super(Class) is extremely rarely used, so TLS write is fine
   EINA_SAFETY_ON_FALSE_RETURN_VAL(super, NULL);
   _super_klass = super_klass;
   return (Eo *) eo_id;

err:
   _EO_POINTER_ERR(cur_klass, "Class (%p) is an invalid ref.", cur_klass);
   return NULL;
#ifdef EO_DEBUG
err_obj:
   _EO_POINTER_ERR(eo_id, "Object (%p) is an invalid ref, class=%p (%s).", eo_id, cur_klass, efl_class_name_get(cur_klass));
   return NULL;
#endif
err_obj_hierarchy:
   _EO_POINTER_ERR(eo_id, "Object (%p) class=%p (%s) is not an instance of class=%p (%s).", eo_id, efl_class_get(eo_id), efl_class_name_get(eo_id), cur_klass, efl_class_name_get(cur_klass));
   return NULL;
}

EAPI Eo *
efl_super(const Eo *eo_id, const Efl_Class *cur_klass)
{
   return _efl_super_cast(eo_id, cur_klass, EINA_TRUE);
}

EAPI Eo *
efl_cast(const Eo *eo_id, const Efl_Class *cur_klass)
{
   return _efl_super_cast(eo_id, cur_klass, EINA_FALSE);
}

EAPI Eina_Bool
_efl_object_call_resolve(Eo *eo_id, const char *func_name, Efl_Object_Op_Call_Data *call, Efl_Object_Call_Cache *cache, const char *file, int line)
{
   const _Efl_Class *klass, *inputklass, *main_klass;
   const _Efl_Class *cur_klass = NULL;
   _Eo_Object *obj = NULL;
   const Eo_Vtable *vtable = NULL;
   const op_type_funcs *func;
   Eina_Bool is_obj;
   Eina_Bool is_override = EINA_FALSE;
   Eina_Bool super = EINA_TRUE;

   if (EINA_UNLIKELY(!eo_id)) return EINA_FALSE;

   call->eo_id = eo_id;

   is_obj = _eo_is_a_obj(eo_id);

   if (is_obj)
     {
        EO_OBJ_POINTER_RETURN_VAL_PROXY(eo_id, _obj, EINA_FALSE);

        obj = _obj;
        klass = _obj->klass;
        vtable = EO_VTABLE(obj);
        is_override = _obj_is_override(obj);
        if (EINA_UNLIKELY(_obj->cur_klass != NULL))
          {
             // YES this is a goto with a label to return. this is a
             // micro-optimization to move infrequent code out of the
             // hot path of the function
             goto obj_super;
          }

obj_super_back:
        call->obj = obj;
        _efl_ref(_obj);
     }
   else
     {
        // YES this is a goto with a label to return. this is a
        // micro-optimization to move infrequent code out of the
        // hot path of the function
        goto ok_klass;
     }
ok_klass_back:

   inputklass = main_klass =  klass;

   if (!cache->op) goto err_cache_op;

   /* If we have a current class, we need to itr to the next. */
   if (cur_klass)
     {
        // YES this is a goto with a label to return. this is a
        // micro-optimization to move infrequent code out of the
        // hot path of the function
        goto ok_cur_klass;
     }
   else
     {
# if EFL_OBJECT_CALL_CACHE_SIZE > 0
        if (!is_override)
          {
# if EFL_OBJECT_CALL_CACHE_SIZE > 1
             int i;

             for (i = 0; i < EFL_OBJECT_CALL_CACHE_SIZE; i++)
# else
                const int i = 0;
# endif
               {
                  if ((const void *)inputklass == cache->index[i].klass)
                    {
                       func = (const op_type_funcs *)cache->entry[i].func;
                       call->func = func->func;
                       if (is_obj)
                         call->data = (char *)obj + cache->off[i].off;
                       return EINA_TRUE;
                    }
               }
          }
#endif
        func = _vtable_func_get(vtable, cache->op);
        // this is not very likely to happen - but may if its an invalid
        // call or a composite object, but either way, it's not very likely
        // so make it a goto to save on instruction cache
        if (!func) goto end;
     }
ok_cur_klass_back:

   if (EINA_LIKELY(func->func && func->src))
     {
        call->func = func->func;

        if (is_obj) call->data = _efl_data_scope_get(obj, func->src);

# if EFL_OBJECT_CALL_CACHE_SIZE > 0
        if (!cur_klass && !is_override)
          {
# if EFL_OBJECT_CALL_CACHE_SIZE > 1
             const int slot = cache->next_slot;
# else
             const int slot = 0;
# endif
             cache->index[slot].klass = (const void *)inputklass;
             cache->entry[slot].func = (const void *)func;
             cache->off[slot].off = (int)((long)((char *)call->data - (char *)obj));
# if EFL_OBJECT_CALL_CACHE_SIZE > 1
             cache->next_slot = (slot + 1) % EFL_OBJECT_CALL_CACHE_SIZE;
# endif
          }
#endif
        return EINA_TRUE;
     }

   // very unlikely in general to use a goto to move code out of l1 cache
   // ie instruction cache
   if (func->src != NULL) goto err_func_src;

end:
   /* Try composite objects */
   if (is_obj)
     {
        Eina_List *itr;
        Eo *emb_obj_id;
        EINA_LIST_FOREACH(obj->opt->composite_objects, itr, emb_obj_id)
          {
             EO_OBJ_POINTER_PROXY(emb_obj_id, emb_obj);
             if (EINA_UNLIKELY(!emb_obj)) continue;

             func = _vtable_func_get(EO_VTABLE(emb_obj), cache->op);
             if (func == NULL) goto composite_continue;

             if (EINA_LIKELY(func->func && func->src))
               {
                  call->eo_id = _eo_obj_id_get(emb_obj);
                  call->obj = _efl_ref(emb_obj);
                  call->func = func->func;
                  call->data = _efl_data_scope_get(emb_obj, func->src);
                  /* We reffed it above, but no longer need/use it. */
                  _efl_unref(obj);
                  EO_OBJ_DONE(emb_obj_id);
                  return EINA_TRUE;
               }
composite_continue:
             EO_OBJ_DONE(emb_obj_id);
          }
     }

   // all of this is error handling at the end so... rare-ish
   // If it's a do_super call.
   if (cur_klass)
     {
        ERR("in %s:%d: func '%s' (%d) could not be resolved for class '%s' for super of '%s'.",
            file, line, func_name, cache->op, main_klass->desc->name,
            cur_klass->desc->name);
        goto err;
     }
   else
     {
        /* we should not be able to take this branch */
        ERR("in %s:%d: func '%s' (%d) could not be resolved for class '%s'.",
            file, line, func_name, cache->op, main_klass->desc->name);
        goto err;
     }
err_cache_op:
   ERR("%s:%d: unable to resolve %s api func '%s' in class '%s'.",
       file, line, (!is_obj ? "class" : "regular"),
       func_name, klass->desc->name);
   goto err;
err_func_src:
   ERR("in %s:%d: you called a pure virtual func '%s' (%d) of class '%s'.",
       file, line, func_name, cache->op, klass->desc->name);
err:
   if (is_obj)
     {
        _efl_unref(obj);
        _eo_obj_pointer_done((Eo_Id)eo_id);
     }
   return EINA_FALSE;

   // yes - special "move out of hot path" code blobs with goto's for
   // speed reasons to have intr prefetches work better and miss less
ok_cur_klass:
   func = _eo_kls_itr_next(klass, cur_klass, cache->op, super);
   if (!func) goto end;
   klass = func->src;
   goto ok_cur_klass_back;

ok_klass:
     {
        EO_CLASS_POINTER_GOTO_PROXY(eo_id, _klass, err_klass);
        klass = _klass;
        vtable = &klass->vtable;
        cur_klass = _super_klass;
        if (cur_klass) _super_klass = NULL;
        call->obj = NULL;
        call->data = NULL;
     }
   goto ok_klass_back;

obj_super:
   {
      cur_klass = obj->cur_klass;
      super = obj->super;
      obj->cur_klass = NULL;

      if (_obj_is_override(obj) && cur_klass && super &&
          (_eo_class_id_get(cur_klass) == EFL_OBJECT_OVERRIDE_CLASS))
        {
           /* Doing a efl_super(obj, EFL_OBJECT_OVERRIDE_CLASS) should
            * result in calling as if it's a normal class. */
           vtable = &klass->vtable;
           cur_klass = NULL;
        }

      is_override = _obj_is_override(obj) && (cur_klass == NULL);
   }
   goto obj_super_back;

err_klass:
   _EO_POINTER_ERR(eo_id, "in %s:%d: func '%s': obj_id=%p is an invalid ref.", file, line, func_name, eo_id);
   return EINA_FALSE;
}

EAPI void
_efl_object_call_end(Efl_Object_Op_Call_Data *call)
{
   if (EINA_LIKELY(!!call->obj))
     {
        _efl_unref(call->obj);
        _eo_obj_pointer_done((Eo_Id)call->eo_id);
     }
}

static inline Eina_Bool
_eo_api_func_equal(const void *api_func1, const void *api_func2)
{
#ifndef _WIN32
   return (api_func1 == api_func2);
#else
   /* On Windows, DLL API's will be exported using the dllexport flag.
    * When used by another library or executable, they will be declared
    * using the dllimport flag. What happens really is that two symbols are
    * created, at two different addresses. So it's impossible to match
    * them. We fallback to plain string comparison based on the
    * function name itself. Slow, but this should rarely happen.
    */
   return (api_func2 && api_func1 && !strcmp(api_func2, api_func1));
#endif
}

static inline Efl_Object_Op
_efl_object_api_op_id_get_internal(const void *api_func)
{
   eina_spinlock_take(&_ops_storage_lock);
#ifndef _WIN32
   Efl_Object_Op op = (uintptr_t) eina_hash_find(_ops_storage, &api_func);
#else
   Efl_Object_Op op = (uintptr_t) eina_hash_find(_ops_storage, api_func);
#endif
   eina_spinlock_release(&_ops_storage_lock);

   return op;
}

/* LEGACY, should be removed before next release */
EAPI Efl_Object_Op
_efl_object_api_op_id_get(const void *api_func)
{
   Efl_Object_Op op = _efl_object_api_op_id_get_internal(api_func);

   if (op == EFL_NOOP)
     {
        ERR("Unable to resolve op for api func %p", api_func);
     }

   return op;
}

EAPI Efl_Object_Op
_efl_object_op_api_id_get(const void *api_func, const Eo *obj, const char *api_func_name, const char *file, int line)
{
   Efl_Object_Op op;

#ifndef EO_DEBUG
   if (!obj) return EFL_NOOP;
#endif
   op = _efl_object_api_op_id_get_internal(api_func);
   if (op == EFL_NOOP)
     {
        eina_log_print(_eo_log_dom, EINA_LOG_LEVEL_ERR,
                       file, api_func_name, line,
                       "Unable to resolve op for api func %p for obj=%p (%s)", api_func, obj, efl_class_name_get(obj));
     }

   return op;
}

/* klass is the klass we are working on. hierarchy_klass is the class whe should
 * use when validating. */
static Eina_Bool
_eo_class_funcs_set(Eo_Vtable *vtable, const Efl_Object_Ops *ops, const _Efl_Class *hierarchy_klass, const _Efl_Class *klass, Efl_Object_Op id_offset, Eina_Bool override_only)
{
   unsigned int i, j;
   Efl_Object_Op op_id;
   const Efl_Op_Description *op_desc;
   const Efl_Op_Description *op_descs;
   const _Efl_Class *override_class;
   const void **api_funcs;
   Eina_Bool check_equal;

   op_id = hierarchy_klass->base_id + id_offset;
   op_descs = ops->descs;
   override_class = override_only ? hierarchy_klass : NULL;

   DBG("Set functions for class '%s':%p", klass->desc->name, klass);

   if (!op_descs || !ops->count) return EINA_TRUE;

#ifdef EO_DEBUG
   check_equal = EINA_TRUE;
#else
   check_equal = !override_only;
#endif
   api_funcs = alloca(ops->count * sizeof(api_funcs[0]));

   /* sanity checks */
   for (i = 0, op_desc = op_descs; i < ops->count; i++, op_desc++)
     {
        if (op_desc->api_func == NULL)
          {
             ERR("Class '%s': NULL API not allowed (NULL->%p '%s').",
                 klass->desc->name, op_desc->func, _eo_op_desc_name_get(op_desc));
             return EINA_FALSE;
          }

        if (check_equal)
          {
             for (j = 0; j < i; j++)
               {
                  if (_eo_api_func_equal(op_desc->api_func, api_funcs[j]))
                    {
                       ERR("Class '%s': API previously defined (%p->%p '%s').",
                           klass->desc->name, op_desc->api_func, op_desc->func, _eo_op_desc_name_get(op_desc));
                       return EINA_FALSE;
                    }
               }

             api_funcs[i] = op_desc->api_func;
          }
     }

   for (i = 0, op_desc = op_descs; i < ops->count; i++, op_desc++)
     {
        Efl_Object_Op op = EFL_NOOP;

        /* Get the opid for the function. */
        op = _efl_object_api_op_id_get_internal(op_desc->api_func);

        if (op == EFL_NOOP)
          {
             if (override_only)
               {
                  ERR("Class '%s': Tried overriding a previously undefined function.", klass->desc->name);
                  return EINA_FALSE;
               }

             op = op_id;
             eina_spinlock_take(&_ops_storage_lock);
#ifndef _WIN32
             eina_hash_add(_ops_storage, &op_desc->api_func, (void *) (uintptr_t) op);
#else
             eina_hash_add(_ops_storage, op_desc->api_func, (void *) (uintptr_t) op);
#endif
             eina_spinlock_release(&_ops_storage_lock);

             op_id++;
          }

        DBG("%p->%p '%s'", op_desc->api_func, op_desc->func, _eo_op_desc_name_get(op_desc));

        if (!_vtable_func_set(vtable, klass, override_class, op, op_desc->func, EINA_TRUE))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
efl_class_functions_set(const Efl_Class *klass_id, const Efl_Object_Ops *object_ops, const Efl_Object_Ops *class_ops)
{
   EO_CLASS_POINTER_GOTO(klass_id, klass, err_klass);
   Efl_Object_Ops empty_ops = { 0 };

   // not likely so use goto to alleviate l1 instruction cache of rare code
   if (klass->functions_set) goto err_funcs;
   klass->functions_set = EINA_TRUE;

   if (!object_ops) object_ops = &empty_ops;

   if (!class_ops) class_ops = &empty_ops;

   klass->ops_count = object_ops->count + class_ops->count;

   klass->base_id = _eo_ops_last_id;
   _eo_ops_last_id += klass->ops_count + 1;

   _vtable_init(&klass->vtable, DICH_CHAIN1(_eo_ops_last_id) + 1);

   /* Flatten the function array */
     {
        const _Efl_Class **mro_itr = klass->mro;
        for (  ; *mro_itr ; mro_itr++) ;

        /* Skip ourselves. */
        for ( mro_itr-- ; mro_itr > klass->mro ; mro_itr--)
          _vtable_copy_all(&klass->vtable, &(*mro_itr)->vtable);
     }

   return _eo_class_funcs_set(&klass->vtable, object_ops, klass, klass, 0, EINA_FALSE) &&
      _eo_class_funcs_set(&klass->vtable, class_ops, klass, klass, object_ops->count, EINA_FALSE);

err_funcs:
   ERR("Class %s already had its functions set..", klass->desc->name);
   return EINA_FALSE;
err_klass:
   _EO_POINTER_ERR(klass_id, "Class (%p) is an invalid ref.", klass_id);
   return EINA_FALSE;
}

EAPI Eo *
_efl_add_internal_start(const char *file, int line, const Efl_Class *klass_id, Eo *parent_id, Eina_Bool ref EINA_UNUSED, Eina_Bool is_fallback)
{
   const char *func_name = __FUNCTION__;
   _Eo_Object *obj;
   Eo_Stack_Frame *fptr = NULL;

   if (is_fallback) fptr = _efl_add_fallback_stack_push(NULL);

   EO_CLASS_POINTER_GOTO_PROXY(klass_id, klass, err_klass);

   if (parent_id)
     {
        EO_OBJ_POINTER_GOTO_PROXY(parent_id, parent, err_parent);
     }

   // not likely so use goto to alleviate l1 instruction cache of rare code
   if (EINA_UNLIKELY(klass->desc->type != EFL_CLASS_TYPE_REGULAR))
     goto err_noreg;

   eina_spinlock_take(&klass->objects.trash_lock);
   obj = eina_trash_pop(&klass->objects.trash);
   if (obj)
     {
        memset(obj, 0, klass->obj_size);
        klass->objects.trash_count--;
     }
   else
     {
        obj = calloc(1, klass->obj_size);
     }
   eina_spinlock_release(&klass->objects.trash_lock);

   obj->opt = eina_cow_alloc(efl_object_optional_cow);
   obj->refcount++;
   obj->klass = klass;

   obj->header.id = _eo_id_allocate(obj, parent_id);
   Eo *eo_id = _eo_obj_id_get(obj);

   _eo_log_obj_new(obj);

   _eo_condtor_reset(obj);

   efl_ref(eo_id);

   /* Reference for the parent if is_ref is done in _efl_add_end */
   efl_parent_set(eo_id, parent_id);

   /* eo_id can change here. Freeing is done on the resolved object. */
   eo_id = efl_constructor(eo_id);
   // not likely so use goto to alleviate l1 instruction cache of rare code
   if (!eo_id) goto err_noid;
   // not likely so use goto to alleviate l1 instruction cache of rare code
   else if (eo_id != _eo_obj_id_get(obj)) goto ok_nomatch;
ok_nomatch_back:
   if (is_fallback) fptr->obj = eo_id;
   if (parent_id) EO_OBJ_DONE(parent_id);
   return eo_id;

ok_nomatch:
     {
        EO_OBJ_POINTER_GOTO_PROXY(eo_id, new_obj, err_newid);
        /* We have two refs at this point. */
        _efl_unref(obj);
        efl_del((Eo *)obj->header.id);
        _efl_ref(new_obj);
        EO_OBJ_DONE(eo_id);
     }
   goto ok_nomatch_back;

err_noid:
   ERR("in %s:%d: Object of class '%s' - Error while constructing object",
       file, line, klass->desc->name);
   /* We have two refs at this point. */
   _efl_unref(obj);
   efl_del((Eo *) obj->header.id);
err_newid:
   if (parent_id) EO_OBJ_DONE(parent_id);
   return NULL;
err_noreg:
   ERR("in %s:%d: Class '%s' is not instantiate-able. Aborting.", file, line, klass->desc->name);
   if (parent_id) EO_OBJ_DONE(parent_id);
   return NULL;

err_klass:
   _EO_POINTER_ERR(klass_id, "in %s:%d: Class (%p) is an invalid ref.", file, line, klass_id);
err_parent:
   return NULL;
}

static Eo *
_efl_add_internal_end(Eo *eo_id, Eo *finalized_id)
{
   EO_OBJ_POINTER_RETURN_VAL(eo_id, obj, NULL);

   // rare so move error handling to end to save l1 instruction cache
   if (!obj->condtor_done) goto err_condtor;
   if (!finalized_id)
     {
        // XXX: Given EFL usage of objects, construction is a perfectly valid thing
        // to do. we shouldn't complain about it as handling a NULL obj creation is
        // the job of the caller. a perfect example here is ecore_con and ecore_ipc
        // where you create a con or ipc obj then set up type/destination/port and
        // the finalize of the constructor does the actual connect and thus this
        // fails or succeeds based on if service is there.
        //
        // until there is a better solution - don't complain here.
        // 
        //             ERR("Object of class '%s' - Finalizing the object failed.",
        //                   klass->desc->name);
        goto cleanup;
     }

   obj->finalized = EINA_TRUE;
   _efl_unref(obj);
   EO_OBJ_DONE(eo_id);
   return (Eo *)eo_id;

err_condtor:
     {
        const _Efl_Class *klass = obj->klass;
        ERR("Object of class '%s' - Not all of the object constructors have been executed.",
              klass->desc->name);
     }
cleanup:
   _efl_unref(obj);
   efl_del((Eo *) obj->header.id);
   EO_OBJ_DONE(eo_id);
   return NULL;
}

EAPI Eo *
_efl_add_end(Eo *eo_id, Eina_Bool is_ref, Eina_Bool is_fallback)
{
   Eo *ret = efl_finalize(eo_id);
   ret = _efl_add_internal_end(eo_id, ret);

   if (is_ref)
     {
        if (efl_parent_get(eo_id))
          {
             efl_ref(eo_id);
          }
        _efl_object_parent_sink_set(eo_id, EINA_TRUE);
     }

   if (is_fallback)
     {
        _efl_add_fallback_stack_pop();
     }

   return ret;
}

EAPI void
efl_reuse(const Eo *_obj)
{
   Eo *obj = (Eo *) _obj;
   efl_object_override(obj, NULL);
   _efl_object_parent_sink_set(obj, EINA_FALSE);
}

void
_eo_free(_Eo_Object *obj, Eina_Bool manual_free EINA_UNUSED)
{
   _Efl_Class *klass = (_Efl_Class*) obj->klass;

   _eo_log_obj_free(obj);

#ifdef EO_DEBUG
   if (manual_free)
     {
        Eo *obj_id = _eo_obj_id_get(obj);
        if (obj->datarefcount)
          {
             ERR("Object %p data still referenced %d time(s).", obj_id, obj->datarefcount);
          }
        while (obj->xrefs)
          {
             Eina_Inlist *nitr = obj->xrefs->next;
             Eo_Xref_Node *xref = EINA_INLIST_CONTAINER_GET(obj->data_xrefs, Eo_Xref_Node);
             ERR("Object %p is still referenced by object %p. Origin: %s:%d",
                 obj_id, xref->ref_obj, xref->file, xref->line);
             eina_freeq_ptr_main_add(xref, free, sizeof(*xref));
             obj->xrefs = nitr;
          }
        while (obj->data_xrefs)
          {
             Eina_Inlist *nitr = obj->data_xrefs->next;
             Eo_Xref_Node *xref = EINA_INLIST_CONTAINER_GET(obj->data_xrefs, Eo_Xref_Node);
             if (obj_id == xref->ref_obj)
               {
                  WRN("Object %p still has a reference to its own data (subclass: %s). Origin: %s:%d",
                      obj_id, xref->data_klass, xref->file, xref->line);
               }
             else
               {
                  ERR("Data of object %p (subclass: %s) is still referenced by object %p. Origin: %s:%d",
                      obj_id, xref->data_klass, xref->ref_obj, xref->file, xref->line);
               }

             eina_freeq_ptr_main_add(xref, free, sizeof(*xref));
             obj->data_xrefs = nitr;
          }
     }
#endif
   if (_obj_is_override(obj))
     {
        _vtable_func_clean_all(obj->opt->vtable);
        eina_freeq_ptr_main_add(obj->opt->vtable, free, 0);
        EO_OPTIONAL_COW_SET(obj, vtable, NULL);
     }

   _eo_id_release((Eo_Id) _eo_obj_id_get(obj));
   eina_cow_free(efl_object_optional_cow, (Eina_Cow_Data *) &obj->opt);

   eina_spinlock_take(&klass->objects.trash_lock);
   if ((klass->objects.trash_count <= 8) && (EINA_LIKELY(!_eo_trash_bypass)))
     {
        eina_trash_push(&klass->objects.trash, obj);
        klass->objects.trash_count++;
     }
   else
     {
        eina_freeq_ptr_main_add(obj, free, klass->obj_size);
     }
   eina_spinlock_release(&klass->objects.trash_lock);
}
/*****************************************************************************/

EAPI const Efl_Class *
efl_class_get(const Eo *eo_id)
{
   const Efl_Class *klass;

   if (_eo_is_a_class(eo_id))
     {
        EO_CLASS_POINTER_GOTO(eo_id, _klass, err_klass);
        return EFL_CLASS_CLASS;
     }

   EO_OBJ_POINTER_GOTO(eo_id, obj, err_obj);
   klass = _eo_class_id_get(obj->klass);
   EO_OBJ_DONE(eo_id);
   return klass;

err_klass:
   _EO_POINTER_ERR(eo_id, "Class (%p) is an invalid ref.", eo_id);
err_obj:
   return NULL;
}

EAPI const char *
efl_class_name_get(const Efl_Class *eo_id)
{
   const _Efl_Class *klass;

   if (_eo_is_a_class(eo_id))
     {
        EO_CLASS_POINTER_GOTO(eo_id, _klass, err_klass);
        klass = _klass;
     }
   else
     {
        EO_OBJ_POINTER_GOTO(eo_id, obj, err_obj);
        klass = obj->klass;
        EO_OBJ_DONE(eo_id);
     }
   return klass->desc->name;

err_klass:
   _EO_POINTER_ERR(eo_id, "Class (%p) is an invalid ref.", eo_id);
err_obj:
   return NULL;
}

static void
_vtable_init(Eo_Vtable *vtable, size_t size)
{
   vtable->size = size;
   vtable->chain = calloc(vtable->size, sizeof(*vtable->chain));
}

static void
_vtable_free(Eo_Vtable *vtable)
{
   if (!vtable) return;
   eina_freeq_ptr_main_add(vtable, EINA_FREE_CB(_vtable_func_clean_all), 0);
   eina_freeq_ptr_main_add(vtable, free, sizeof(*vtable));
}

static Eina_Bool
_eo_class_mro_has(const _Efl_Class *klass, const _Efl_Class *find)
{
   const _Efl_Class **itr;
   for (itr = klass->mro ; *itr ; itr++)
     {
        if (*itr == find)
          {
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static Eina_List *
_eo_class_list_remove_duplicates(Eina_List* list)
{
   Eina_List *itr1, *itr2, *itr2n;

   itr1 = eina_list_last(list);
   while (itr1)
     {
        itr2 = eina_list_prev(itr1);

        while (itr2)
          {
             itr2n = eina_list_prev(itr2);

             if (eina_list_data_get(itr1) == eina_list_data_get(itr2))
               {
                  list = eina_list_remove_list(list, itr2);
               }

             itr2 = itr2n;
          }

        itr1 = eina_list_prev(itr1);
     }

   return list;
}

static Eina_List *
_eo_class_mro_add(Eina_List *mro, const _Efl_Class *klass)
{
   if (!klass)
     return mro;

   mro = eina_list_append(mro, klass);

   /* Recursively add MIXINS extensions. */
     {
        const _Efl_Class **extn_itr;

        for (extn_itr = klass->extensions ; *extn_itr ; extn_itr++)
          {
             const _Efl_Class *extn = *extn_itr;
             if (extn->desc->type == EFL_CLASS_TYPE_MIXIN)
               mro = _eo_class_mro_add(mro, extn);
          }
     }

   mro = _eo_class_mro_add(mro, klass->parent);

   return mro;
}

static Eina_List *
_eo_class_mro_init(const Efl_Class_Description *desc, const _Efl_Class *parent, Eina_List *extensions)
{
   Eina_List *mro = NULL;
   Eina_List *extn_itr = NULL;
   Eina_List *extn_pos = NULL;
   const _Efl_Class *extn = NULL;

   /* Add MIXINS extensions. */
   EINA_LIST_FOREACH(extensions, extn_itr, extn)
     {
        if (extn->desc->type != EFL_CLASS_TYPE_MIXIN)
          continue;

        mro = _eo_class_mro_add(mro, extn);
        extn_pos = eina_list_append(extn_pos, eina_list_last(mro));
     }

   /* Check if we can create a consistent mro */
     {
        Eina_List *itr = extn_pos;
        EINA_LIST_FOREACH(extensions, extn_itr, extn)
          {
             if (extn->desc->type != EFL_CLASS_TYPE_MIXIN)
                continue;

             /* Get the first one after the extension. */
             Eina_List *extn_list = eina_list_next(eina_list_data_get(itr));

             /* If we found the extension again. */
             if (eina_list_data_find_list(extn_list, extn))
               {
                  eina_list_free(mro);
                  eina_list_free(extn_pos);
                  ERR("Cannot create a consistent method resolution order for class '%s' because of '%s'.", desc->name, extn->desc->name);
                  return NULL;
               }

             itr = eina_list_next(itr);
          }
     }

   eina_list_free(extn_pos);

   mro = _eo_class_mro_add(mro, parent);
   mro = _eo_class_list_remove_duplicates(mro);
   /* Will be replaced with the actual class pointer */
   mro = eina_list_prepend(mro, NULL);

   return mro;
}

static Eina_Bool
_eo_class_initializer(_Efl_Class *klass)
{
   if (klass->desc->class_initializer)
     return klass->desc->class_initializer(_eo_class_id_get(klass));

   return EINA_TRUE;
}

static void
_eo_class_constructor(_Efl_Class *klass)
{
   klass->constructed = EINA_TRUE;

   if (klass->desc->class_constructor)
     klass->desc->class_constructor(_eo_class_id_get(klass));
}

static void
eo_class_free(_Efl_Class *klass)
{
   void *data;

   if (klass->constructed)
     {
        if (klass->desc->class_destructor)
           klass->desc->class_destructor(_eo_class_id_get(klass));

        _vtable_func_clean_all(&klass->vtable);
     }

   EINA_TRASH_CLEAN(&klass->objects.trash, data)
      eina_freeq_ptr_main_add(data, free, klass->obj_size);

   EINA_TRASH_CLEAN(&klass->iterators.trash, data)
      eina_freeq_ptr_main_add(data, free, 0);

   eina_spinlock_free(&klass->objects.trash_lock);
   eina_spinlock_free(&klass->iterators.trash_lock);

   eina_freeq_ptr_main_add(klass, free, 0);
}

/* Not really called, just used for the ptr... */
static void
_eo_class_isa_func(Eo *eo_id EINA_UNUSED, void *class_data EINA_UNUSED)
{
   /* Do nonthing. */
}

static void
_eo_class_isa_recursive_set(_Efl_Class *klass, const _Efl_Class *cur)
{
   const _Efl_Class **extn_itr;

   _vtable_func_set(&klass->vtable, klass, NULL, cur->base_id + cur->ops_count,
                    _eo_class_isa_func, EINA_TRUE);

   for (extn_itr = cur->extensions ; *extn_itr ; extn_itr++)
     {
        _eo_class_isa_recursive_set(klass, *extn_itr);
     }

   if (cur->parent)
     {
        _eo_class_isa_recursive_set(klass, cur->parent);
     }
}

static inline void
_eo_classes_release(void)
{
#ifdef HAVE_MMAP
   size_t size;

   size = _eo_classes_alloc * sizeof(_Efl_Class *);
   if (_eo_classes) munmap(_eo_classes, size);
#else
   free(_eo_classes);
#endif
   _eo_classes = NULL;
   _eo_classes_last_id = 0;
   _eo_classes_alloc = 0;
}

static inline void
_eo_classes_expand(void)
{
   unsigned char *ptr;
   size_t newsize, psize;

   _eo_classes_last_id++;
   if (_eo_classes_last_id <= _eo_classes_alloc) return;
   psize = _eo_classes_alloc * sizeof(_Efl_Class *);
#ifdef HAVE_MMAP
   _eo_classes_alloc += (MEM_PAGE_SIZE / sizeof(_Efl_Class *));
   newsize = _eo_classes_alloc * sizeof(_Efl_Class *);
   ptr = mmap(NULL, newsize, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANON, -1, 0);
   if (ptr == MAP_FAILED)
     {
        ERR("mmap of eo class table region failed!");
        abort();
     }
   if (psize > 0) memcpy(ptr, _eo_classes, psize);
   if (_eo_classes) munmap(_eo_classes, psize);
#else
   _eo_classes_alloc += 128;
   newsize = _eo_classes_alloc * sizeof(_Efl_Class *);
   ptr = realloc(_eo_classes, newsize);
   if (!ptr)
     {
        ERR("realloc of eo class table region faile!!");
        abort();
     }
#endif
   memset(ptr + psize, 0, newsize - psize);
   _eo_classes = (_Efl_Class **)ptr;
}

EAPI const Efl_Class *
efl_class_new(const Efl_Class_Description *desc, const Efl_Class *parent_id, ...)
{
   _Efl_Class *klass;
   va_list p_list;
   size_t extn_sz, mro_sz, mixins_sz;
   Eina_List *extn_list, *mro, *mixins;
   _Efl_Class *parent = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->name, NULL);

   if (parent_id)
     {
        parent = _eo_class_pointer_get(parent_id);
        if (!parent)
          return NULL;
     }

   /* Check restrictions on Interface types. */
   if (desc->type == EFL_CLASS_TYPE_INTERFACE)
     {
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->data_size, NULL);
     }

   /* Check parent */
   if (parent)
     {
        /* Verify the inheritance is allowed. */
        switch (desc->type)
          {
           case EFL_CLASS_TYPE_REGULAR:
           case EFL_CLASS_TYPE_REGULAR_NO_INSTANT:
              if ((parent->desc->type != EFL_CLASS_TYPE_REGULAR) &&
                    (parent->desc->type != EFL_CLASS_TYPE_REGULAR_NO_INSTANT))
                {
                   ERR("Regular classes ('%s') aren't allowed to inherit from non-regular classes ('%s').",
                       desc->name, parent->desc->name);
                   return NULL;
                }
              break;
           case EFL_CLASS_TYPE_INTERFACE:
           case EFL_CLASS_TYPE_MIXIN:
              if ((parent->desc->type != EFL_CLASS_TYPE_INTERFACE) &&
                    (parent->desc->type != EFL_CLASS_TYPE_MIXIN))
                {
                   ERR("Non-regular classes ('%s') aren't allowed to inherit from regular classes ('%s').",
                       desc->name, parent->desc->name);
                   return NULL;
                }
              break;
          }
     }

   /* Build class extensions list */
     {
        DBG("Started building extensions list for class '%s'", desc->name);
        extn_list = NULL;
        const _Efl_Class *extn = NULL;
        const Eo_Id *extn_id = NULL;

        va_start(p_list, parent_id);

        extn_id = va_arg(p_list, Eo_Id *);
        while (extn_id)
          {
             extn = _eo_class_pointer_get((Efl_Class *)extn_id);
             if (EINA_LIKELY(extn != NULL))
               {
                  switch (extn->desc->type)
                    {
                     case EFL_CLASS_TYPE_REGULAR_NO_INSTANT:
                     case EFL_CLASS_TYPE_REGULAR:
                     case EFL_CLASS_TYPE_INTERFACE:
                     case EFL_CLASS_TYPE_MIXIN:
                       extn_list = eina_list_append(extn_list, extn);
                       break;
                    }
               }
             extn_id = va_arg(p_list, Eo_Id *);
          }

        va_end(p_list);

        extn_list = _eo_class_list_remove_duplicates(extn_list);

        extn_sz = sizeof(_Efl_Class *) * (eina_list_count(extn_list) + 1);

        DBG("Finished building extensions list for class '%s'", desc->name);
     }

   /* Prepare mro list */
     {
        DBG("Started building MRO list for class '%s'", desc->name);

        mro = _eo_class_mro_init(desc, parent, extn_list);
        if (!mro)
          {
             eina_list_free(extn_list);
             return NULL;
          }

        mro_sz = sizeof(_Efl_Class *) * (eina_list_count(mro) + 1);

        DBG("Finished building MRO list for class '%s'", desc->name);
     }

   /* Prepare mixins list */
     {
        Eina_List *itr;
        const _Efl_Class *kls_itr;

        DBG("Started building Mixins list for class '%s'", desc->name);

        mixins = NULL;
        EINA_LIST_FOREACH(mro, itr, kls_itr)
          {
             if ((kls_itr) && (kls_itr->desc->type == EFL_CLASS_TYPE_MIXIN) &&
                   (kls_itr->desc->data_size > 0))
               mixins = eina_list_append(mixins, kls_itr);
          }

        mixins_sz = sizeof(Eo_Extension_Data_Offset) * (eina_list_count(mixins) + 1);
        if ((desc->type == EFL_CLASS_TYPE_MIXIN) && (desc->data_size > 0))
          mixins_sz += sizeof(Eo_Extension_Data_Offset);

        DBG("Finished building Mixins list for class '%s'", desc->name);
     }

   klass = calloc(1, _eo_class_sz + extn_sz + mro_sz + mixins_sz);
   eina_spinlock_new(&klass->objects.trash_lock);
   eina_spinlock_new(&klass->iterators.trash_lock);
   klass->parent = parent;
   klass->desc = desc;
   klass->extensions = (const _Efl_Class **) ((char *) klass + _eo_class_sz);
   klass->mro = (const _Efl_Class **) ((char *) klass->extensions + extn_sz);
   klass->extn_data_off = (Eo_Extension_Data_Offset *) ((char *) klass->mro + mro_sz);

   if (klass->parent)
     {
        /* FIXME: Make sure this alignment is enough. */
        klass->data_offset = klass->parent->data_offset +
           EO_ALIGN_SIZE(klass->parent->desc->data_size);
     }
   else
     {
        /* Data starts after the object size. */
        klass->data_offset = _eo_sz;
     }

   mro = eina_list_remove(mro, NULL);
   mro = eina_list_prepend(mro, klass);
   if ((desc->type == EFL_CLASS_TYPE_MIXIN) && (desc->data_size > 0))
     mixins = eina_list_prepend(mixins, klass);

   /* Copy the extensions and free the list */
     {
        const _Efl_Class *extn = NULL;
        const _Efl_Class **extn_itr = klass->extensions;
        EINA_LIST_FREE(extn_list, extn)
          {
             *(extn_itr++) = extn;

             DBG("Added '%s' extension", extn->desc->name);
          }
        *(extn_itr) = NULL;
     }

   /* Copy the mro and free the list. */
     {
        const _Efl_Class *kls_itr = NULL;
        const _Efl_Class **mro_itr = klass->mro;
        EINA_LIST_FREE(mro, kls_itr)
          {
             *(mro_itr++) = kls_itr;

             DBG("Added '%s' to MRO", kls_itr->desc->name);
          }
        *(mro_itr) = NULL;
     }

   size_t extn_data_off = klass->data_offset;
   if (klass->desc->type != EFL_CLASS_TYPE_MIXIN)
      extn_data_off += EO_ALIGN_SIZE(klass->desc->data_size);

   /* Feed the mixins data offsets and free the mixins list. */
     {
        const _Efl_Class *kls_itr = NULL;
        Eo_Extension_Data_Offset *extn_data_itr = klass->extn_data_off;
        EINA_LIST_FREE(mixins, kls_itr)
          {
             extn_data_itr->klass = kls_itr;
             extn_data_itr->offset = extn_data_off;

             extn_data_off += EO_ALIGN_SIZE(extn_data_itr->klass->desc->data_size);
             extn_data_itr++;

             DBG("Added '%s' to Data Offset info", kls_itr->desc->name);
          }
        extn_data_itr->klass = 0;
        extn_data_itr->offset = 0;
     }

   klass->obj_size = extn_data_off;

     {
        Eo_Id new_id;

        eina_lock_take(&_efl_class_creation_lock);
        new_id = (_eo_classes_last_id + 1) | MASK_CLASS_TAG;
        _eo_classes_expand();
        _eo_classes[_UNMASK_ID(new_id) - 1] = klass;
        eina_lock_release(&_efl_class_creation_lock);

        klass->header.id = new_id;
     }

   if (!_eo_class_initializer(klass))
     {
        return NULL;
     }

   /* If functions haven't been set, invoke it with an empty ops structure. */
   if (!klass->functions_set)
     {
        efl_class_functions_set(_eo_class_id_get(klass), NULL, NULL);
     }

   /* Mark which classes we implement */
   if (klass->vtable.size)
     {
        _eo_class_isa_recursive_set(klass, klass);
     }

   _eo_class_constructor(klass);

   DBG("Finished building class '%s'", klass->desc->name);

   return _eo_class_id_get(klass);
}

EAPI Eina_Bool
efl_object_override(Eo *eo_id, const Efl_Object_Ops *ops)
{
   EO_OBJ_POINTER_RETURN_VAL(eo_id, obj, EINA_FALSE);
   EO_CLASS_POINTER_GOTO(EFL_OBJECT_OVERRIDE_CLASS, klass, err);

   if (ops)
     {
        Eo_Vtable *vtable = obj->opt->vtable;

        if (!vtable)
          {
             vtable = calloc(1, sizeof(*vtable));
             _vtable_init(vtable, obj->klass->vtable.size);
             _vtable_copy_all(vtable, &obj->klass->vtable);
          }

        if (!_eo_class_funcs_set(vtable, ops, obj->klass, klass, 0, EINA_TRUE))
          {
             ERR("Failed to override functions for %s@%p. All previous "
                 "overrides have been reset.", obj->klass->desc->name, eo_id);
             if (obj->opt->vtable == vtable)
               EO_OPTIONAL_COW_SET(obj, vtable, NULL);
             else
               _vtable_free(vtable);
             goto err;
          }

        EO_OPTIONAL_COW_SET(obj, vtable, vtable);
     }
   else
     {
        if (obj->opt->vtable)
          {
             _vtable_free(obj->opt->vtable);
             EO_OPTIONAL_COW_SET(obj, vtable, NULL);
          }
     }

   EO_OBJ_DONE(eo_id);
   return EINA_TRUE;

err:
   EO_OBJ_DONE(eo_id);
   return EINA_FALSE;
}

EAPI Eina_Bool
efl_isa(const Eo *eo_id, const Efl_Class *klass_id)
{
   Efl_Id_Domain domain;
   Eo_Id_Data *data;
   Eo_Id_Table_Data *tdata;
   Eina_Bool isa = EINA_FALSE;

   if (EINA_UNLIKELY(!eo_id)) return EINA_FALSE;
   domain = ((Eo_Id)eo_id >> SHIFT_DOMAIN) & MASK_DOMAIN;
   data = _eo_table_data_get();
   tdata = _eo_table_data_table_get(data, domain);
   if (EINA_UNLIKELY(!tdata)) goto err;

   if (EINA_LIKELY(domain != EFL_ID_DOMAIN_SHARED))
     {
        if ((tdata->cache.isa_id == eo_id) &&
            (tdata->cache.klass == klass_id))
          {
             isa = tdata->cache.isa;
             return isa;
          }

        EO_OBJ_POINTER_GOTO(eo_id, obj, err_obj);
        EO_CLASS_POINTER_GOTO(klass_id, klass, err_class);
        const op_type_funcs *func = _vtable_func_get
          (EO_VTABLE(obj), klass->base_id + klass->ops_count);

        // Caching the result as we do a lot of serial efl_isa due to evas_object_image using it.
        tdata->cache.isa_id = eo_id;
        tdata->cache.klass = klass_id;
        // Currently implemented by reusing the LAST op id. Just marking it with
        // _eo_class_isa_func.
        isa = tdata->cache.isa = (func && (func->func == _eo_class_isa_func));
     }
   else
     {
        eina_lock_take(&(_eo_table_data_shared_data->obj_lock));

        if ((tdata->cache.isa_id == eo_id) &&
            (tdata->cache.klass == klass_id))
          {
             isa = tdata->cache.isa;
             // since this is the cache we hope this gets a lot of hits and
             // thus lets assume the hit is the mot important thing thus
             // put the lock release and return here inline in the l1
             // instruction cache hopefully already fetched
             eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
             return isa;
          }

        EO_OBJ_POINTER_GOTO(eo_id, obj, err_shared_obj);
        EO_CLASS_POINTER_GOTO(klass_id, klass, err_shared_class);
        const op_type_funcs *func = _vtable_func_get
          (EO_VTABLE(obj), klass->base_id + klass->ops_count);

        // Caching the result as we do a lot of serial efl_isa due to evas_object_image using it.
        tdata->cache.isa_id = eo_id;
        tdata->cache.klass = klass_id;
        // Currently implemented by reusing the LAST op id. Just marking it with
        // _eo_class_isa_func.
        isa = tdata->cache.isa = (func && (func->func == _eo_class_isa_func));
        EO_OBJ_DONE(eo_id);
        eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
     }
   return isa;

err_shared_class:
   _EO_POINTER_ERR(klass_id, "Class (%p) is an invalid ref.", klass_id);
   EO_OBJ_DONE(eo_id);
err_shared_obj:
   eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
   return EINA_FALSE;

err_class:
   _EO_POINTER_ERR(klass_id, "Class (%p) is an invalid ref.", klass_id);
err_obj:
   return EINA_FALSE;

err:
   ERR("Object %p is not a valid object in this context: object domain: %d, "
       "current domain: %d, local domain: %d, available domains: [%s %s %s %s]."
       " Are you trying to access this object from another thread?",
       eo_id, (int)domain,
       (int)data->domain_stack[data->stack_top], (int)data->local_domain,
       (data->tables[0]) ? "0" : " ", (data->tables[1]) ? "1" : " ",
       (data->tables[2]) ? "2" : " ", (data->tables[3]) ? "3" : " ");
   return EINA_FALSE;
}

EAPI Eo *
efl_xref_internal(const char *file, int line, Eo *obj_id, const Eo *ref_obj_id)
{
   efl_ref(obj_id);

#ifdef EO_DEBUG
   const char *func_name = __FUNCTION__;
   EO_OBJ_POINTER_RETURN_VAL_PROXY(obj_id, obj, obj_id);

   Eo_Xref_Node *xref = calloc(1, sizeof(*xref));
   xref->ref_obj = ref_obj_id;
   xref->file = file;
   xref->line = line;

   obj->xrefs = eina_inlist_prepend(obj->xrefs, EINA_INLIST_GET(xref));
   EO_OBJ_DONE(obj_id);
#else
   (void) ref_obj_id;
   (void) file;
   (void) line;
#endif

   return obj_id;
}

EAPI void
efl_xunref(Eo *obj_id, const Eo *ref_obj_id)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);
#ifdef EO_DEBUG
   Eo_Xref_Node *xref = NULL;
   EINA_INLIST_FOREACH(obj->xrefs, xref)
     {
        if (xref->ref_obj == ref_obj_id)
          break;
     }

   if (xref)
     {
        obj->xrefs = eina_inlist_remove(obj->xrefs, EINA_INLIST_GET(xref));
        eina_freeq_ptr_main_add(xref, free, sizeof(*xref));
     }
   else
     {
        ERR("ref_obj (%p) does not reference obj (%p). Aborting unref.", ref_obj_id, obj_id);
        EO_OBJ_DONE(obj_id);
        return;
     }
   EO_OBJ_DONE(obj_id);
#else
   (void) ref_obj_id;
#endif
   efl_unref(obj_id);
}

EAPI Eo *
efl_ref(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, (Eo *)obj_id);

   ++(obj->user_refcount);
   if (EINA_UNLIKELY(obj->user_refcount == 1))
     _efl_ref(obj);
   EO_OBJ_DONE(obj_id);
   return (Eo *)obj_id;
}

EAPI void
efl_unref(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);

   --(obj->user_refcount);
   if (EINA_UNLIKELY(obj->user_refcount <= 0))
     {
        if (obj->user_refcount < 0)
          {
             ERR("Obj:%p. User refcount (%d) < 0. Too many unrefs.", obj, obj->user_refcount);
             _eo_log_obj_report((Eo_Id)obj_id, EINA_LOG_LEVEL_ERR, __FUNCTION__, __FILE__, __LINE__);
             EO_OBJ_DONE(obj_id);
             return;
          }
        _efl_unref(obj);
     }
   EO_OBJ_DONE(obj_id);
}

EAPI int
efl_ref_get(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, 0);
   int ref;
   ref = obj->user_refcount;
   EO_OBJ_DONE(obj_id);
   return ref;
}

EAPI int
___efl_ref2_get(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, 0);
   int ref;
   ref = obj->refcount;
   EO_OBJ_DONE(obj_id);
   return ref;
}

EAPI void
___efl_ref2_reset(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);
   obj->refcount = 0;
   EO_OBJ_DONE(obj_id);
}


EAPI void
efl_del_intercept_set(Eo *obj_id, Efl_Del_Intercept del_intercept_func)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);
   EO_OPTIONAL_COW_SET(obj, del_intercept, del_intercept_func);
   EO_OBJ_DONE(obj_id);
}

EAPI Efl_Del_Intercept
efl_del_intercept_get(const Eo *obj_id)
{
   Efl_Del_Intercept func;

   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);
   func = obj->opt->del_intercept;
   EO_OBJ_DONE(obj_id);
   return func;
}

void
_eo_condtor_done(Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);
   if (obj->condtor_done)
     {
        ERR("Object %p is already constructed at this point.", obj);
        EO_OBJ_DONE(obj_id);
        return;
     }
   obj->condtor_done = EINA_TRUE;
   EO_OBJ_DONE(obj_id);
}

static inline void *
_efl_data_scope_safe_get(const _Eo_Object *obj, const _Efl_Class *klass)
{
   if (EINA_LIKELY(klass->desc->data_size > 0))
     {
        return _efl_data_scope_get(obj, klass);
     }

   return NULL;
}

static inline void *
_efl_data_scope_get(const _Eo_Object *obj, const _Efl_Class *klass)
{
   if (EINA_LIKELY(klass->desc->type != EFL_CLASS_TYPE_MIXIN))
     return ((char *) obj) + klass->data_offset;

   if (EINA_UNLIKELY(klass->desc->data_size == 0))
     {
        return NULL;
     }
   else
     {
        Eo_Extension_Data_Offset *doff_itr = obj->klass->extn_data_off;

        if (!doff_itr)
          return NULL;

        while (doff_itr->klass)
          {
             if (doff_itr->klass == klass)
               return ((char *) obj) + doff_itr->offset;
             doff_itr++;
          }
     }

   return NULL;
}

static inline void *
_efl_data_xref_internal(const char *file, int line, _Eo_Object *obj, const _Efl_Class *klass, const _Eo_Object *ref_obj)
{
   void *data = NULL;
   if (klass != NULL)
     {
        data = _efl_data_scope_safe_get(obj, klass);
        if (data == NULL) return NULL;
     }
#ifdef EO_DEBUG
   (obj->datarefcount)++;
   Eo_Xref_Node *xref = calloc(1, sizeof(*xref));
   xref->ref_obj = _eo_obj_id_get(ref_obj);
   xref->data_klass = klass ? klass->desc->name : NULL;
   xref->file = file;
   xref->line = line;

   obj->data_xrefs = eina_inlist_prepend(obj->data_xrefs, EINA_INLIST_GET(xref));
#else
   (void) ref_obj;
   (void) file;
   (void) line;
#endif
   return data;
}

static inline void
_efl_data_xunref_internal(_Eo_Object *obj EINA_UNUSED, void *data EINA_UNUSED, const _Eo_Object *ref_obj EINA_UNUSED)
{
#ifdef EO_DEBUG
   const _Efl_Class *klass = obj->klass;
   Eo_Xref_Node *xref = NULL;
   Eina_Bool in_range = (((char *)data >= (((char *) obj) + _eo_sz)) &&
                         ((char *)data < (((char *) obj) + klass->obj_size)));
   if (!in_range)
     {
        ERR("Data %p is not in the data range of the object %p (%s).",
            data, _eo_obj_id_get(obj), obj->klass->desc->name);
     }
   if (obj->datarefcount == 0)
     {
        ERR("Data for object %p (%s) is already not referenced.",
            _eo_obj_id_get(obj), obj->klass->desc->name);
     }
   else
     {
        (obj->datarefcount)--;
     }
   EINA_INLIST_FOREACH(obj->data_xrefs, xref)
     {
        if (xref->ref_obj == _eo_obj_id_get(ref_obj))
          break;
     }
   if (xref)
     {
        obj->data_xrefs = eina_inlist_remove(obj->data_xrefs, EINA_INLIST_GET(xref));
        eina_freeq_ptr_main_add(xref, free, sizeof(*xref));
     }
   else
     {
        ERR("ref_obj %p (%s) does not reference data %p of obj %p (%s).",
            _eo_obj_id_get(ref_obj), ref_obj->klass->desc->name, data,
            _eo_obj_id_get(obj), obj->klass->desc->name);
     }
#endif
}

EAPI void *
efl_data_scope_get(const Eo *obj_id, const Efl_Class *klass_id)
{
   void *ret = NULL;
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);
   EO_CLASS_POINTER_GOTO(klass_id, klass, err_klass);

#ifndef EO_DEBUG
   ret = _efl_data_scope_safe_get(obj, klass);
#else
   if (_eo_class_mro_has(obj->klass, klass))
     {
        ret = _efl_data_scope_safe_get(obj, klass);
        if (!ret && (klass->desc->data_size == 0))
          ERR("Tried getting data of class '%s', but it has none.", klass->desc->name);
     }
   else
     {
        ERR("Tried getting data of class '%s' from object of class '%s', but the former is not a direct inheritance of the latter.",
            klass->desc->name, obj->klass->desc->name);
     }
#endif

err_klass:
   EO_OBJ_DONE(obj_id);
   return ret;
}

EAPI void *
efl_data_scope_safe_get(const Eo *obj_id, const Efl_Class *klass_id)
{
#ifndef EO_DEBUG
   void *ret = NULL;

   if (!obj_id) return NULL;
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);
   EO_CLASS_POINTER_GOTO(klass_id, klass, err_klass);
   if (obj->destructed)
     {
        goto err_klass;
     }

   if (_eo_class_mro_has(obj->klass, klass))
     ret = _efl_data_scope_safe_get(obj, klass);

err_klass:
   EO_OBJ_DONE(obj_id);
   return ret;
#else
   return efl_data_scope_get(obj_id, klass_id);
#endif
}

EAPI void *
efl_data_xref_internal(const char *file, int line, const Eo *obj_id, const Efl_Class *klass_id, const Eo *ref_obj_id)
{
   void *ret = NULL;
   _Efl_Class *klass = NULL;
   const char *func_name = __FUNCTION__;
   EO_OBJ_POINTER_RETURN_VAL_PROXY(obj_id, obj, NULL);
   EO_OBJ_POINTER_PROXY(ref_obj_id, ref_obj);
   if (ref_obj)
     {
        if (klass_id)
          {
             EO_CLASS_POINTER_GOTO_PROXY(klass_id, klass2, err_klass);
             klass = klass2;
#ifdef EO_DEBUG
             // rare to use goto to keep instruction cache cleaner
             if (!_eo_class_mro_has(obj->klass, klass)) goto err_mro;
#endif
          }

        ret = _efl_data_xref_internal(file, line, obj, klass, ref_obj);
#ifdef EO_DEBUG
        // rare to use goto to keep instruction cache cleaner
        if (klass && !ret && (klass->desc->data_size == 0)) goto err_ret;
#endif
err_klass:
        EO_OBJ_DONE(ref_obj_id);
     }
   EO_OBJ_DONE(obj_id);
   return ret;
#ifdef EO_DEBUG
err_ret:
   ERR("Tried getting data of class '%s', but it has none.", klass->desc->name);
   goto err;
err_mro:
   ERR("Tried getting data of class '%s' from object of class '%s', but the former is not a direct inheritance of the latter.", klass->desc->name, obj->klass->desc->name);
err:
   EO_OBJ_DONE(obj_id);
   EO_OBJ_DONE(ref_obj_id);
   return NULL;
#endif
}

EAPI void
efl_data_xunref_internal(const Eo *obj_id, void *data, const Eo *ref_obj_id)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);
   EO_OBJ_POINTER(ref_obj_id, ref_obj);
   if (ref_obj)
     {
        _efl_data_xunref_internal(obj, data, ref_obj);
        EO_OBJ_DONE(ref_obj_id);
     }
   EO_OBJ_DONE(obj_id);
}

static void
_eo_table_del_cb(void *in)
{
   Eo_Id_Data *data = in;
   _eo_free_ids_tables(data);
}

/* FIXME: Support other domains and tables, at the moment only the main
 * domain and table.
 * This is used by the gdb debug helper script */
Eo_Id_Data *_eo_gdb_main_domain = NULL;

EAPI Eina_Bool
efl_object_init(void)
{
   const char *log_dom = "eo";
   if (_efl_object_init_count++ > 0)
     return EINA_TRUE;

   eina_init();

#if HAVE_VALGRIND
   _eo_trash_bypass = RUNNING_ON_VALGRIND;
#endif

   _efl_object_main_thread = eina_thread_self();

   _eo_sz = EO_ALIGN_SIZE(sizeof(_Eo_Object));
   _eo_class_sz = EO_ALIGN_SIZE(sizeof(_Efl_Class));

   _eo_classes = NULL;
   _eo_classes_last_id = EO_CLASS_IDS_FIRST - 1;
   _eo_ops_last_id = EFL_OBJECT_OP_IDS_FIRST;
   _eo_log_dom = eina_log_domain_register(log_dom, EINA_COLOR_LIGHTBLUE);
   if (_eo_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s.", log_dom);
        return EINA_FALSE;
     }

   if (!eina_lock_recursive_new(&_efl_class_creation_lock))
     {
        ERR("Could not init lock.");
        return EINA_FALSE;
     }

   if (!eina_spinlock_new(&_ops_storage_lock))
     {
        ERR("Could not init lock.");
        return EINA_FALSE;
     }

   _eo_log_obj_init();

   eina_magic_string_static_set(EO_EINA_MAGIC, EO_EINA_MAGIC_STR);
   eina_magic_string_static_set(EO_FREED_EINA_MAGIC,
                                EO_FREED_EINA_MAGIC_STR);
   eina_magic_string_static_set(EO_CLASS_EINA_MAGIC,
                                EO_CLASS_EINA_MAGIC_STR);
   efl_future_init();

#ifndef _WIN32
   _ops_storage = eina_hash_pointer_new(NULL);
#else
   _ops_storage = eina_hash_string_superfast_new(NULL);
#endif

   _eo_table_data_shared = _eo_table_data_new(EFL_ID_DOMAIN_SHARED);
   if (!_eo_table_data_shared)
     {
        ERR("Could not allocate shared table data");
        return EINA_FALSE;
     }
   _eo_table_data_shared_data = _eo_table_data_shared->tables[EFL_ID_DOMAIN_SHARED];

   // specially force eoid data to be creanted so we can switch it to domain 0
   Eo_Id_Data *data = _eo_table_data_new(EFL_ID_DOMAIN_MAIN);
   _eo_gdb_main_domain = data;
   if (!data)
     {
        ERR("Could not allocate main table data");
        return EINA_FALSE;
     }
   if (!eina_tls_cb_new(&_eo_table_data, _eo_table_del_cb))
     {
        ERR("Could not allocate TLS for eo domain data");
        _eo_table_del_cb(data);
        return EINA_FALSE;
     }
   eina_tls_set(_eo_table_data, data);
   _efl_object_main_thread = eina_thread_self();

#ifdef EO_DEBUG
   /* Call it just for coverage purposes. Ugly I know, but I like it better than
    * casting everywhere else. */
   _eo_class_isa_func(NULL, NULL);
#endif

   efl_object_optional_cow =
         eina_cow_add("Efl Object Optional Data", sizeof(Efl_Object_Optional),
                      64, &efl_object_optional_cow_default, EINA_TRUE);

   _efl_add_fallback_init();

   eina_log_timing(_eo_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   /* bootstrap EFL_CLASS_CLASS */
   (void) EFL_CLASS_CLASS;

   return EINA_TRUE;
}

EAPI Eina_Bool
efl_object_shutdown(void)
{
   size_t i;
   _Efl_Class **cls_itr = _eo_classes + _eo_classes_last_id - 1;

   if (--_efl_object_init_count > 0)
     return EINA_TRUE;

   eina_log_timing(_eo_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   _efl_add_fallback_shutdown();

   efl_future_shutdown();

   for (i = 0 ; i < _eo_classes_last_id ; i++, cls_itr--)
     {
        if (*cls_itr)
          eo_class_free(*cls_itr);
     }

   eina_lock_take(&_efl_class_creation_lock);
   _eo_classes_release();
   eina_lock_release(&_efl_class_creation_lock);

   eina_hash_free(_ops_storage);

   eina_spinlock_free(&_ops_storage_lock);
   eina_lock_free(&_efl_class_creation_lock);

   _eo_free_ids_tables(_eo_table_data_get());
   eina_tls_free(_eo_table_data);
   if (_eo_table_data_shared)
     {
        _eo_free_ids_tables(_eo_table_data_shared);
        _eo_table_data_shared = NULL;
        _eo_table_data_shared_data = NULL;
     }

   eina_cow_del(efl_object_optional_cow);
   efl_object_optional_cow = NULL;

   _eo_log_obj_shutdown();

   eina_log_domain_unregister(_eo_log_dom);
   _eo_log_dom = -1;

   ++_efl_object_init_generation;

   eina_shutdown();
   return EINA_FALSE;
}


EAPI Efl_Id_Domain
efl_domain_get(void)
{
   Eo_Id_Data *data = _eo_table_data_get();
   return data->local_domain;
}

EAPI Efl_Id_Domain
efl_domain_current_get(void)
{
   Eo_Id_Data *data = _eo_table_data_get();
   return data->domain_stack[data->stack_top];
}

EAPI Eina_Bool
efl_domain_switch(Efl_Id_Domain domain)
{
   Eo_Id_Data *data = _eo_table_data_get();
   if ((domain < EFL_ID_DOMAIN_MAIN) || (domain > EFL_ID_DOMAIN_THREAD) ||
       (domain == EFL_ID_DOMAIN_SHARED))
     {
        ERR("Invalid domain %i being switched to", domain);
        return EINA_FALSE;
     }
   if (data)
     {
        if (data->local_domain == domain) return EINA_TRUE;
        _eo_free_ids_tables(data);
     }
   data = _eo_table_data_new(domain);
   data->local_domain = domain;
   data->domain_stack[data->stack_top] = domain;
   eina_tls_set(_eo_table_data, data);
   return EINA_TRUE;
}

static inline Eina_Bool
_efl_domain_push(Eo_Id_Data *data, Efl_Id_Domain domain)
{
   if (data->stack_top >= (sizeof(data->domain_stack) - 1))
     {
        ERR("Failed to push domain %i on stack. Out of stack space at %i",
            domain, data->stack_top);
        return EINA_FALSE;
     }
   data->stack_top++;
   data->domain_stack[data->stack_top] = domain;
   return EINA_TRUE;
}

static inline void
_efl_domain_pop(Eo_Id_Data *data)
{
   if (data->stack_top > 0) data->stack_top--;
}

EAPI Eina_Bool
efl_domain_current_push(Efl_Id_Domain domain)
{
   Eo_Id_Data *data = _eo_table_data_get();
   return _efl_domain_push(data, domain);
}

EAPI void
efl_domain_current_pop(void)
{
   Eo_Id_Data *data = _eo_table_data_get();
   _efl_domain_pop(data);
}

EAPI Eina_Bool
efl_domain_current_set(Efl_Id_Domain domain)
{
   Eo_Id_Data *data = _eo_table_data_get();
   if ((domain < EFL_ID_DOMAIN_MAIN) || (domain > EFL_ID_DOMAIN_THREAD))
     {
        ERR("Invalid domain %i being set", domain);
        return EINA_FALSE;
     }
   data->domain_stack[data->stack_top] = domain;
   return EINA_TRUE;
}

EAPI Efl_Domain_Data *
efl_domain_data_get(void)
{
   Eo_Id_Data *data = _eo_table_data_get();
   return (Efl_Domain_Data *)data;
}

EAPI Efl_Id_Domain
efl_domain_data_adopt(Efl_Domain_Data *data_in)
{
   Eo_Id_Data *data = _eo_table_data_get();
   Eo_Id_Data *data_foreign = (Eo_Id_Data *)data_in;

   if (!data_foreign)
     {
        ERR("Trying to adopt NULL domain data [data=%p in=%p]", data, data_in);
        return EFL_ID_DOMAIN_INVALID;
     }
   if (data_foreign->local_domain == data->local_domain)
     {
        ERR("Trying to adopt EO ID domain %i, is the same as the local %i [data=%p in=%p foreign=%p]",
            data_foreign->local_domain, data->local_domain, data, data_in, data_foreign);
        return EFL_ID_DOMAIN_INVALID;
     }
   if (data->tables[data_foreign->local_domain])
     {
        ERR("Trying to adopt an already adopted domain [data=%p in=%p foreign=%p]", data, data_in, data_foreign);
        return EFL_ID_DOMAIN_INVALID;
     }
   data->tables[data_foreign->local_domain] =
     data_foreign->tables[data_foreign->local_domain];
   _efl_domain_push(data, data_foreign->local_domain);
   return data->domain_stack[data->stack_top];
}

EAPI Eina_Bool
efl_domain_data_return(Efl_Id_Domain domain)
{
   Eo_Id_Data *data = _eo_table_data_get();

   if ((domain < EFL_ID_DOMAIN_MAIN) || (domain > EFL_ID_DOMAIN_THREAD))
     {
        ERR("Invalid domain %i being returned to owning thread", domain);
        return EINA_FALSE;
     }
   if (domain == data->local_domain)
     {
        ERR("Cannot return the local domain %i back to its owner [data=%p]", domain, data);
        return EINA_FALSE;
     }
   data->tables[domain] = NULL;
   _efl_domain_pop(data);
   return EINA_TRUE;
}

EAPI Eina_Bool
efl_compatible(const Eo *obj, const Eo *obj_target)
{
   Efl_Id_Domain domain1 = ((Eo_Id)obj >> SHIFT_DOMAIN) & MASK_DOMAIN;
   Efl_Id_Domain domain2 = ((Eo_Id)obj_target >> SHIFT_DOMAIN) & MASK_DOMAIN;
   if (domain1 == domain2) return EINA_TRUE;
   DBG("Object %p and %p are not compatible. Domain %i and %i do not match",
       obj, obj_target, domain1, domain2);
   return EINA_FALSE;
}

EAPI Eina_Bool
efl_destructed_is(const Eo *obj_id)
{
   Eina_Bool is;
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);
   is = obj->destructed;
   EO_OBJ_DONE(obj_id);
   return is;
}

EAPI void
efl_manual_free_set(Eo *obj_id, Eina_Bool manual_free)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);
   obj->manual_free = manual_free;
   EO_OBJ_DONE(obj_id);
}

EAPI Eina_Bool
efl_manual_free(Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

   // rare to use goto to keep instruction cache cleaner
   if (obj->manual_free == EINA_FALSE) goto err_manual_free;
   // rare to use goto to keep instruction cache cleaner
   if (!obj->destructed) goto err_not_destructed;
   _eo_free(obj, EINA_TRUE);
   EO_OBJ_DONE(obj_id);
   return EINA_TRUE;

err_manual_free:
   ERR("Tried to manually free the object %p while the option has not been set; see efl_manual_free_set for more information.", obj);
   goto err;
err_not_destructed:
   ERR("Tried deleting the object %p while still referenced(%d).", obj_id, obj->refcount);
   goto err;
err:
   EO_OBJ_DONE(obj_id);
   return EINA_FALSE;
}

EAPI const char *
efl_debug_name_get(const Eo *obj_id)
{
   const char *override = "";
   const char *name, *clsname;
   Eina_Strbuf *sb;

   if (!obj_id) return "(null)";

   if (_eo_is_a_class(obj_id))
     {
        const char *clstype;

        EO_CLASS_POINTER(obj_id, klass);
        if (!klass || !klass->desc)
          return eina_slstr_printf("Invalid_Class_ID(invalid)@%p", obj_id);

        switch (klass->desc->type)
          {
           case EFL_CLASS_TYPE_REGULAR: clstype = "regular"; break;
           case EFL_CLASS_TYPE_REGULAR_NO_INSTANT: clstype = "abstract"; break;
           case EFL_CLASS_TYPE_INTERFACE: clstype = "interface"; break;
           case EFL_CLASS_TYPE_MIXIN: clstype = "mixin"; break;
           default: clstype = "invalid"; break;
          }

        return eina_slstr_printf("%s(%s)@%p", klass->desc->name, clstype, obj_id);
     }

   EO_OBJ_POINTER(obj_id, obj);
   if (!obj) return eina_slstr_printf("Invalid_Object_ID@%p", obj_id);

   sb = eina_strbuf_new();
   name = efl_name_get(obj_id);
   clsname = obj->klass->desc->name;
   if (_obj_is_override(obj)) override = "(override)";

   if (name)
     eina_strbuf_append_printf(sb, "%s%s@%p[%d]:'%s'", clsname, override, obj_id, (int) obj->refcount, name);
   else
     eina_strbuf_append_printf(sb, "%s%s@%p[%d]", clsname, override, obj_id, (int) obj->refcount);

   if (!obj->cur_klass)
     {
        sb = efl_debug_name_override((Eo *) obj_id, sb);
     }
   else
     {
        if (obj->super)
          sb = efl_debug_name_override(efl_super(obj_id, (Efl_Class *) obj->cur_klass->header.id), sb);
        else
          sb = efl_debug_name_override(efl_cast(obj_id, (Efl_Class *) obj->cur_klass->header.id), sb);
        obj->super = EINA_FALSE;
        obj->cur_klass = NULL;
     }

   EO_OBJ_DONE(obj_id);
   return eina_slstr_strbuf_new(sb);
}

EAPI int
efl_callbacks_cmp(const Efl_Callback_Array_Item *a, const Efl_Callback_Array_Item *b)
{
   if (a->desc == b->desc) return 0;
   else if (a->desc > b->desc) return 1;
   else return -1;
}

#ifdef EO_DEBUG
/* NOTE: cannot use ecore_time_get()! */
static inline double
_eo_log_time_now(void)
{
#ifdef HAVE_EVIL
   return evil_time_get();
#elif defined(__APPLE__) && defined(__MACH__)
   static double clk_conv = -1.0;

   if (EINA_UNLIKELY(clk_conv < 0))
     {
        mach_timebase_info_data_t info;
        kern_return_t err = mach_timebase_info(&info);
        if (err == 0)
          clk_conv = 1e-9 * (double)info.numer / (double)info.denom;
        else
          clk_conv = 1e-9;
     }

   return clk_conv * mach_absolute_time();
#else
#if defined (HAVE_CLOCK_GETTIME) || defined (EXOTIC_PROVIDE_CLOCK_GETTIME)
   struct timespec t;
   static int clk_id = -1;

   if (EINA_UNLIKELY(clk_id == -2)) goto try_gettimeofday;
   if (EINA_UNLIKELY(clk_id == -1))
     {
     retry_clk_id:
        clk_id = CLOCK_MONOTONIC;
        if (EINA_UNLIKELY(clock_gettime(clk_id, &t)))
          {
             WRN("CLOCK_MONOTONIC failed!");
             clk_id = CLOCK_REALTIME;
             if (EINA_UNLIKELY(clock_gettime(clk_id, &t)))
               {
                  WRN("CLOCK_REALTIME failed!");
                  clk_id = -2;
                  goto try_gettimeofday;
               }
          }
     }
   else
     {
        if (EINA_UNLIKELY(clock_gettime(clk_id, &t)))
          {
             WRN("clk_id=%d previously ok, now failed... retry", clk_id);
             goto retry_clk_id;
          }
     }
   return (double)t.tv_sec + (((double)t.tv_nsec) / 1000000000.0);

 try_gettimeofday:
#endif
   {
      struct timeval timev;

      gettimeofday(&timev, NULL);
      return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
   }
#endif
}

#ifdef HAVE_BACKTRACE
typedef struct _Eo_Log_Obj_Entry {
   Eo_Id id;
   const _Eo_Object *obj;
   const _Efl_Class *klass;
   double timestamp;
   Eina_Bool is_free;
   uint8_t bt_size;
   void *bt[];
} Eo_Log_Obj_Entry;

static void
_eo_log_obj_find(const Eo_Id id, const Eo_Log_Obj_Entry **added, const Eo_Log_Obj_Entry **deleted)
{
   const Eo_Log_Obj_Entry *entry;
   Eina_Array_Iterator it;
   unsigned int idx;

   *added = NULL;
   *deleted = NULL;

   eina_spinlock_take(&_eo_log_objs_lock);
   EINA_ARRAY_ITER_NEXT(&_eo_log_objs, idx, entry, it)
     {
        if (EINA_UNLIKELY(id == entry->id))
          {
             if (entry->is_free)
               *deleted = entry;
             else
               {
                  *added = entry;
                  *deleted = NULL; /* forget previous add, if any */
               }
          }
     }
   eina_spinlock_release(&_eo_log_objs_lock);
}

static void
_eo_log_obj_entry_show(const Eo_Log_Obj_Entry *entry, int log_level, const char *func_name, const char *file, int line, double now)
{
   uint8_t i;

   eina_log_print(_eo_log_objs_dom, log_level, file, func_name, line,
                  "obj_id=%p %s obj=%p, class=%p (%s) [%0.4fs, %0.4f ago]:",
                  (void *)entry->id,
                  entry->is_free ? "deleted" : "created",
                  entry->obj,
                  entry->klass,
                  entry->klass->desc->name,
                  entry->timestamp - _eo_log_time_start, now - entry->timestamp);

   for (i = 0; i < entry->bt_size; i++)
     {
#ifdef HAVE_DLADDR
        Dl_info info;

        if (dladdr(entry->bt[i], &info))
          {
             if (info.dli_sname)
               {
                  eina_log_print(_eo_log_objs_dom, log_level, file, func_name, line,
                                 "   0x%016llx: %s+%llu (in %s 0x%llx)",
                                 (unsigned long long)(uintptr_t)entry->bt[i],
                                 info.dli_sname,
                                 (unsigned long long)(uintptr_t)((char *)entry->bt[i] - (char *)info.dli_saddr),
                                 info.dli_fname ? info.dli_fname : "??",
                                 (unsigned long long)(uintptr_t)info.dli_fbase);
                  continue;
               }
             else if (info.dli_fname)
               {
                  const char *fname;

#ifdef HAVE_EVIL
                  fname = strrchr(info.dli_fname, '\\');
#else
                  fname = strrchr(info.dli_fname, '/');
#endif
                  if (!fname) fname = info.dli_fname;
                  else fname++;

                  eina_log_print(_eo_log_objs_dom, log_level, file, func_name, line,
                                 "   0x%016llx: %s+%llu (in %s 0x%llx)",
                                 (unsigned long long)(uintptr_t)entry->bt[i],
                                 fname,
                                 (unsigned long long)(uintptr_t)((char *)entry->bt[i] - (char *)info.dli_fbase),
                                 info.dli_fname,
                                 (unsigned long long)(uintptr_t)info.dli_fbase);
                  continue;
               }
          }
#endif

        eina_log_print(_eo_log_objs_dom, log_level, func_name, file, line,
                       "   0x%016llx", (unsigned long long)(uintptr_t)entry->bt[i]);
     }
}
#endif

inline void
_eo_log_obj_report(const Eo_Id id, int log_level, const char *func_name, const char *file, int line)
{
#ifdef HAVE_BACKTRACE
   const Eo_Log_Obj_Entry *added, *deleted;
   double now;

   if (EINA_LIKELY(!_eo_log_objs_enabled)) return;

   _eo_log_obj_find(id, &added, &deleted);

   if ((!added) && (!deleted))
     {
        if ((!_eo_log_objs_debug.len) && (!_eo_log_objs_no_debug.len))
          {
             eina_log_print(_eo_log_objs_dom, log_level, file, func_name, line,
                            "obj_id=%p was neither created or deleted.", (void *)id);
          }
        else if ((_eo_log_objs_debug.len) && (_eo_log_objs_no_debug.len))
          {
             eina_log_print(_eo_log_objs_dom, log_level, file, func_name, line,
                            "obj_id=%p was neither created or deleted (EO_LIFECYCLE_DEBUG='%s', EO_LIFECYCLE_NO_DEBUG='%s').",
                            (void *)id, getenv("EO_LIFECYCLE_DEBUG"), getenv("EO_LIFECYCLE_NO_DEBUG"));
          }
        else if (_eo_log_objs_debug.len)
          {
             eina_log_print(_eo_log_objs_dom, log_level, file, func_name, line,
                            "obj_id=%p was neither created or deleted (EO_LIFECYCLE_DEBUG='%s').",
                            (void *)id, getenv("EO_LIFECYCLE_DEBUG"));
          }
        else
          {
             eina_log_print(_eo_log_objs_dom, log_level, file, func_name, line,
                            "obj_id=%p was neither created or deleted (EO_LIFECYCLE_NO_DEBUG='%s').",
                            (void *)id, getenv("EO_LIFECYCLE_NO_DEBUG"));
          }
        return;
     }

   now = _eo_log_time_now();

   if (added)
     _eo_log_obj_entry_show(added, log_level, func_name, file, line, now);

   if (deleted)
     {
        _eo_log_obj_entry_show(deleted, log_level, func_name, file, line, now);
        eina_log_print(_eo_log_objs_dom, log_level, file, func_name, line,
                       "obj_id=%p was already deleted %0.4f seconds ago!",
                       (void *)id, now - deleted->timestamp);
     }

#else
   (void)id;
   (void)log_level;
   (void)func_name;
   (void)file;
   (void)line;
#endif
}

#ifdef HAVE_BACKTRACE
static Eo_Log_Obj_Entry *
_eo_log_obj_entry_new_and_add(const _Eo_Object *obj, Eina_Bool is_free, uint8_t size, void *const *bt)
{
   Eo_Log_Obj_Entry *entry;
   Eina_Bool ret;

   entry = malloc(sizeof(Eo_Log_Obj_Entry) + size * sizeof(void *));
   if (EINA_UNLIKELY(!entry)) return NULL;

   entry->id = (Eo_Id)_eo_obj_id_get(obj);
   entry->timestamp = _eo_log_time_now();
   entry->obj = obj;
   entry->klass = obj->klass;
   entry->is_free = is_free;
   entry->bt_size = size;
   memcpy(entry->bt, bt, size * sizeof(void *));

   if (EINA_UNLIKELY(!entry)) return NULL;
   eina_spinlock_take(&_eo_log_objs_lock);
   ret = eina_array_push(&_eo_log_objs, entry);
   eina_spinlock_release(&_eo_log_objs_lock);
   if (!ret)
     {
        free(entry);
        return NULL;
     }

   return entry;
}

static inline void
_eo_log_obj_entry_free(Eo_Log_Obj_Entry *entry)
{
   free(entry);
}
#endif

static int
_eo_class_name_slice_cmp(const void *pa, const void *pb)
{
   const Eina_Slice *a = pa;
   const Eina_Slice *b = pb;

   if (a->len < b->len) return -1;
   if (a->len > b->len) return 1;
   return memcmp(a->mem, b->mem, a->len);
}

static Eina_Bool
_eo_log_obj_desired(const _Eo_Object *obj)
{
   Eina_Slice cls_name;

   if (EINA_LIKELY((_eo_log_objs_debug.len == 0) &&
                   (_eo_log_objs_no_debug.len == 0)))
     return EINA_TRUE;

   cls_name.mem = obj->klass->desc->name;
   cls_name.len = strlen(cls_name.mem);

   if (_eo_log_objs_no_debug.len)
     {
        if (eina_inarray_search_sorted(&_eo_log_objs_no_debug, &cls_name, _eo_class_name_slice_cmp) >= 0)
          return EINA_FALSE;
     }

   if (!_eo_log_objs_debug.len)
     return EINA_TRUE;

   if (eina_inarray_search_sorted(&_eo_log_objs_debug, &cls_name, _eo_class_name_slice_cmp) >= 0)
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline void
_eo_log_obj_new(const _Eo_Object *obj)
{
#ifdef HAVE_BACKTRACE
   void *bt[64];
   int size;
#endif

   if (EINA_LIKELY(!_eo_log_objs_enabled)) return;
   if (EINA_LIKELY(!_eo_log_obj_desired(obj))) return;

#ifdef HAVE_BACKTRACE
   size = backtrace(bt, sizeof(bt)/sizeof(bt[0]));
   if (EINA_UNLIKELY(size < 1)) return;

   _eo_log_obj_entry_new_and_add(obj, EINA_FALSE, size, bt);
#endif
   EINA_LOG_DOM_DBG(_eo_log_objs_dom,
                    "new obj=%p obj_id=%p class=%p (%s) [%0.4f]",
                    obj, _eo_obj_id_get(obj), obj->klass, obj->klass->desc->name,
                    _eo_log_time_now() - _eo_log_time_start);
}

static inline void
_eo_log_obj_free(const _Eo_Object *obj)
{
#ifdef HAVE_BACKTRACE
   void *bt[64];
   int size;
#endif

   if (EINA_LIKELY(!_eo_log_objs_enabled)) return;
   if (EINA_LIKELY(!_eo_log_obj_desired(obj))) return;

#ifdef HAVE_BACKTRACE
   size = backtrace(bt, sizeof(bt)/sizeof(bt[0]));
   if (EINA_UNLIKELY(size < 1)) return;

   _eo_log_obj_entry_new_and_add(obj, EINA_TRUE, size, bt);
#endif
   EINA_LOG_DOM_DBG(_eo_log_objs_dom,
                    "free obj=%p obj_id=%p class=%p (%s) [%0.4f]",
                    obj, _eo_obj_id_get(obj), obj->klass, obj->klass->desc->name,
                    _eo_log_time_now() - _eo_log_time_start);
}

static inline void
_eo_log_obj_init(void)
{
   const char *s;

   _eo_log_objs_dom = eina_log_domain_register("eo_lifecycle", EINA_COLOR_BLUE);
   _eo_log_time_start = _eo_log_time_now();

#ifdef HAVE_BACKTRACE
   eina_array_step_set(&_eo_log_objs, sizeof(Eina_Array), 4096);
   eina_spinlock_new(&_eo_log_objs_lock);
#endif
   eina_inarray_step_set(&_eo_log_objs_debug, sizeof(Eina_Inarray), sizeof(Eina_Slice), 0);
   eina_inarray_step_set(&_eo_log_objs_no_debug, sizeof(Eina_Inarray), sizeof(Eina_Slice), 0);

   s = getenv("EO_LIFECYCLE_DEBUG");
   if ((s) && (s[0] != '\0'))
     {
        _eo_log_objs_enabled = EINA_TRUE;
        if ((strcmp(s, "*") == 0) || (strcmp(s, "1") == 0))
          {
             EINA_LOG_DOM_DBG(_eo_log_objs_dom,
                              "will log all object allocation and free");
          }
        else
          {
             Eina_Slice slice;
             const Eina_Slice *itr;
             do
               {
                  char *p = strchr(s, ',');
                  slice.mem = s;
                  if (p)
                    {
                       slice.len = p - s;
                       s = p + 1;
                    }
                  else
                    {
                       slice.len = strlen(s);
                       s = NULL;
                    }
                  eina_inarray_push(&_eo_log_objs_debug, &slice);
               }
             while (s);
             eina_inarray_sort(&_eo_log_objs_debug, _eo_class_name_slice_cmp);

             EINA_INARRAY_FOREACH(&_eo_log_objs_debug, itr)
               {
                  EINA_LOG_DOM_DBG(_eo_log_objs_dom,
                                "will log class '" EINA_SLICE_STR_FMT "'",
                                   EINA_SLICE_STR_PRINT(*itr));
               }
          }
#ifndef HAVE_BACKTRACE
        WRN("EO_LIFECYCLE_DEBUG='%s' but your system has no backtrace()!", s);
#endif
     }

   if (EINA_LIKELY(!_eo_log_objs_enabled)) return;

   DBG("logging object allocation and free, use EINA_LOG_LEVELS=eo_lifecycle:4");

   s = getenv("EO_LIFECYCLE_NO_DEBUG");
   if ((s) && (s[0] != '\0'))
     {
        if ((strcmp(s, "*") == 0) || (strcmp(s, "1") == 0))
          {
             EINA_LOG_DOM_ERR(_eo_log_objs_dom,
                              "expected class names to not log allocation and free, got '%s'", s);
          }
        else
          {
             Eina_Slice slice;
             const Eina_Slice *itr;
             do
               {
                  char *p = strchr(s, ',');
                  slice.mem = s;
                  if (p)
                    {
                       slice.len = p - s;
                       s = p + 1;
                    }
                  else
                    {
                       slice.len = strlen(s);
                       s = NULL;
                    }
                  eina_inarray_push(&_eo_log_objs_no_debug, &slice);
               }
             while (s);
             eina_inarray_sort(&_eo_log_objs_no_debug, _eo_class_name_slice_cmp);

             EINA_INARRAY_FOREACH(&_eo_log_objs_no_debug, itr)
               {
                  EINA_LOG_DOM_DBG(_eo_log_objs_dom,
                                   "will NOT log class '" EINA_SLICE_STR_FMT "'",
                                   EINA_SLICE_STR_PRINT(*itr));
               }
          }
     }
}

static inline void
_eo_log_obj_shutdown(void)
{
#ifdef HAVE_BACKTRACE
   Eo_Log_Obj_Entry *entry;
   Eina_Array_Iterator it;
   unsigned int idx;

   eina_spinlock_take(&_eo_log_objs_lock);
   if (eina_log_domain_level_check(_eo_log_objs_dom, EINA_LOG_LEVEL_INFO))
     {
        void * const *itr = _eo_log_objs.data;
        void * const *itr_end = itr + _eo_log_objs.count;
        double now = _eo_log_time_now();
        size_t leaks = 0;

        for (; itr < itr_end; itr++)
          {
             void * const *cur;
             entry = *itr;
             if (entry->is_free) continue;
             for (cur = itr + 1; cur < itr_end; cur++)
               {
                  const Eo_Log_Obj_Entry *cur_entry = *cur;
                  if (EINA_UNLIKELY((cur_entry->id == entry->id) && (cur_entry->is_free)))
                    break;
               }
             if (EINA_UNLIKELY(cur == itr_end))
               {
                  EINA_LOG_DOM_INFO(_eo_log_objs_dom,
                                    "leaking obj_id=%p obj=%p class=%p (%s) [%0.4fs, %0.4f ago]",
                                    (void *)entry->id,
                                    entry->obj,
                                    entry->klass,
                                    entry->klass->desc->name,
                                    entry->timestamp - _eo_log_time_start, now - entry->timestamp);
                  _eo_log_obj_entry_show(entry, EINA_LOG_LEVEL_DBG, __FUNCTION__, __FILE__, __LINE__, now);
                  leaks++;
               }
          }
        if (leaks)
          EINA_LOG_DOM_WARN(_eo_log_objs_dom, "Leaked %zd objects! Check details with EINA_LOG_LEVELS=eo_lifecycle:4", leaks);
        else
          EINA_LOG_DOM_INFO(_eo_log_objs_dom, "No leaked objects!");
     }
   EINA_ARRAY_ITER_NEXT(&_eo_log_objs, idx, entry, it)
     _eo_log_obj_entry_free(entry);
   eina_array_flush(&_eo_log_objs);
   eina_spinlock_release(&_eo_log_objs_lock);
   eina_spinlock_free(&_eo_log_objs_lock);
#endif

   eina_inarray_flush(&_eo_log_objs_debug);
   eina_inarray_flush(&_eo_log_objs_no_debug);
}
#endif

typedef struct
{
   Eina_Iterator iterator;
   unsigned int cur_kl_id;
} _Eo_Classes_Iterator;

static Eina_Bool
_eo_classes_iterator_next(Eina_Iterator *it, void **data)
{
   _Eo_Classes_Iterator *eo_it = (_Eo_Classes_Iterator *)it;

   if (eo_it->cur_kl_id == _eo_classes_last_id) return EINA_FALSE;
   *data = _eo_class_id_get(_eo_classes[eo_it->cur_kl_id]);
   eo_it->cur_kl_id++;
   return EINA_TRUE;
}

static void
_eo_classes_iterator_free(Eina_Iterator *it)
{
   EINA_MAGIC_SET(it, EINA_MAGIC_NONE);
   free(it);
}

EAPI Eina_Iterator *
eo_classes_iterator_new(void)
{
   _Eo_Classes_Iterator *it;

   it = calloc(1, sizeof (*it));
   if (!it) return NULL;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = _eo_classes_iterator_next;
   it->iterator.free = _eo_classes_iterator_free;
   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   return (Eina_Iterator *)it;
}

typedef struct
{
   Eina_Iterator iterator;
   Eo_Id_Table_Data *tdata;
   Table_Index mid_table_id;
   Table_Index table_id;
   Table_Index entry_id;
} _Eo_Objects_Iterator;

static Eina_Bool
_eo_objects_iterator_next(Eina_Iterator *it, void **data)
{
   Table_Index mid_table_id, table_id, entry_id;
   Eo_Id_Table_Data *tdata;
   _Eo_Objects_Iterator *eo_it = (_Eo_Objects_Iterator *)it;
   if (!eo_it->tdata) return EINA_FALSE;

   tdata = eo_it->tdata;
   mid_table_id = eo_it->mid_table_id;
   table_id = eo_it->table_id;
   entry_id = eo_it->entry_id;
   while (mid_table_id < MAX_MID_TABLE_ID)
     {
        if (tdata->eo_ids_tables[mid_table_id])
          {
             while (table_id < MAX_TABLE_ID)
               {
                  if (TABLE_FROM_IDS)
                    {
                       while (entry_id < MAX_ENTRY_ID)
                         {
                            _Eo_Id_Entry *entry = &(TABLE_FROM_IDS->entries[entry_id]);
                            if (entry->active)
                              {
                                 Eo *obj = _eo_header_id_get((Eo_Header *) entry->ptr);
                                 *data = obj;
                                 eo_it->mid_table_id = mid_table_id;
                                 eo_it->table_id = table_id;
                                 eo_it->entry_id = entry_id + 1;
                                 return EINA_TRUE;
                              }
                            entry_id++;
                         }
                       entry_id = 0;
                    }
                  table_id++;
               }
             table_id = 0;
          }
        mid_table_id++;
     }
   return EINA_FALSE;
}

static void
_eo_objects_iterator_free(Eina_Iterator *it)
{
   EINA_MAGIC_SET(it, EINA_MAGIC_NONE);
   free(it);
}

EAPI Eina_Iterator *
eo_objects_iterator_new(void)
{
   _Eo_Objects_Iterator *it;
   Eo_Id_Table_Data *tdata = _eo_table_data_table_get(_eo_table_data_get(), EFL_ID_DOMAIN_MAIN);

   if (!tdata) return NULL;

   it = calloc(1, sizeof (*it));
   if (!it) return NULL;

   it->tdata = tdata;
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = _eo_objects_iterator_next;
   it->iterator.free = _eo_objects_iterator_free;
   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   return (Eina_Iterator *)it;
}

static Eina_Bool
_eo_value_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Eo **tmem = mem;
   *tmem = NULL;
   return EINA_TRUE;
}

static Eina_Bool
_eo_value_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Eo **tmem = mem;
   if (*tmem)
     {
        efl_unref(*tmem);
        *tmem = NULL;
     }
   return EINA_TRUE;
}

static void
_eo_value_replace(Eo **dst, Eo * const *src)
{
   if (*src == *dst) return;
   //ref *src first, since efl_unref(*dst) may trigger *src unref()
   efl_ref(*src);
   efl_unref(*dst);
   *dst = *src;
}

static Eina_Bool
_eo_value_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   Eo **dst = mem;
   Eo **src = va_arg(args, Eo **);
   _eo_value_replace(dst, src);
   return EINA_TRUE;
}

static Eina_Bool
_eo_value_pset(const Eina_Value_Type *type EINA_UNUSED,
              void *mem, const void *ptr)
{
   Eo **dst = mem;
   Eo * const *src = ptr;
   _eo_value_replace(dst, src);
   return EINA_TRUE;
}

static Eina_Bool
_eo_value_pget(const Eina_Value_Type *type EINA_UNUSED,
              const void *mem, void *ptr)
{
   Eo * const *src = mem;
   Eo **dst = ptr;
   *dst = *src;
   return EINA_TRUE;
}

static Eina_Bool
_eo_value_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   Eo * const *eo = type_mem;

   if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
       convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[256];
        snprintf(buf, sizeof(buf), "Object id: %p, class: %s, name: %s",
                 *eo, efl_class_name_get(efl_class_get(*eo)),
                 efl_debug_name_get(*eo));
        other_mem = buf;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   return EINA_FALSE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_OBJECT = {
  .version = EINA_VALUE_TYPE_VERSION,
  .value_size = sizeof(Eo *),
  .name = "Efl_Object",
  .setup = _eo_value_setup,
  .flush = _eo_value_flush,
  .copy = NULL,
  .compare = NULL,
  .convert_to = _eo_value_convert_to,
  .convert_from = NULL,
  .vset = _eo_value_vset,
  .pset = _eo_value_pset,
  .pget = _eo_value_pget
};

EOAPI const Eina_Value_Type *EINA_VALUE_TYPE_OBJECT = &_EINA_VALUE_TYPE_OBJECT;
