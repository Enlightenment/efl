#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined HAVE_DLADDR && ! defined _WIN32
# include <dlfcn.h>
#endif

#include <Eina.h>

#include "Eo.h"
#include "eo_ptr_indirection.h"
#include "eo_private.h"
#include "eo_add_fallback.h"

#include "efl_object_override.eo.c"

#define EO_CLASS_IDS_FIRST 1
#define EFL_OBJECT_OP_IDS_FIRST 1

/* Used inside the class_get functions of classes, see #EFL_DEFINE_CLASS */
EAPI Eina_Lock _efl_class_creation_lock;
EAPI unsigned int _efl_object_init_generation = 1;
int _eo_log_dom = -1;
Eina_Thread _efl_object_main_thread;

static _Efl_Class **_eo_classes = NULL;
static Eo_Id _eo_classes_last_id = 0;
static Eo_Id _eo_classes_alloc = 0;
static int _efl_object_init_count = 0;
static Efl_Object_Op _eo_ops_last_id = 0;
static Eina_Hash *_ops_storage = NULL;
static Eina_Spinlock _ops_storage_lock;

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
   Eina_Bool writeable = EINA_FALSE;
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
             if (!writeable)
               {
                  _vtable_chain_write_prepare(dst);
                  df = dst->chain2->funcs + j;
               }

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
_vtable_func_set(Eo_Vtable *vtable, const _Efl_Class *klass, Efl_Object_Op op, Eo_Op_Func_Type func)
{
   op_type_funcs *fsrc;
   size_t idx1 = DICH_CHAIN1(op);
   Dich_Chain1 *chain1 = &vtable->chain[idx1];
   _vtable_chain_write_prepare(chain1);
   fsrc = &chain1->chain2->funcs[DICH_CHAIN_LAST(op)];
   if (fsrc->src == klass)
     {
        const _Efl_Class *op_kls = _eo_op_class_get(op);
        ERR("Class '%s': Overriding already set func %p for op %d (%s) with %p.",
              klass->desc->name, fsrc->func, op, op_kls->desc->name, func);
        return EINA_FALSE;
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

#ifdef HAVE_EO_ID
# define _EO_ID_GET(Id) ((Eo_Id) (Id))
#else
# define _EO_ID_GET(Id) ((Eo_Id) ((Id) ? ((Eo_Header *) (Id))->id : 0))
#endif


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
#ifdef HAVE_EO_ID
   return ID_CLASS_GET((Eo_Id)klass_id);
#else
   return (_Efl_Class *) klass_id;
#endif
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
_eo_kls_itr_next(const _Efl_Class *orig_kls, const _Efl_Class *cur_klass, Efl_Object_Op op)
{
   const _Efl_Class **kls_itr = NULL;

   /* Find the kls itr. */
   kls_itr = orig_kls->mro;
   while (*kls_itr && (*kls_itr != cur_klass))
      kls_itr++;

   if (*kls_itr)
     {
        kls_itr++;
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

static const _Efl_Class *_super_class = NULL;
static Eina_Spinlock _super_class_lock;

EAPI Eo *
efl_super(const Eo *obj, const Efl_Class *cur_klass)
{
   EO_CLASS_POINTER_GOTO(cur_klass, klass, err);

   /* FIXME: Switch to atomic operations intead of lock. */
   eina_spinlock_take(&_super_class_lock);
   _super_class = klass;

   return (Eo *) ((Eo_Id) obj | MASK_SUPER_TAG);
err:
   _EO_POINTER_ERR("Class (%p) is an invalid ref.", cur_klass);
   return NULL;
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

   if (((Eo_Id) eo_id) & MASK_SUPER_TAG)
     {
        cur_klass = _super_class;
        _super_class = NULL;
        eina_spinlock_release(&_super_class_lock);

        eo_id = (Eo *) ((Eo_Id) eo_id & ~MASK_SUPER_TAG);
     }

   if (EINA_UNLIKELY(!eo_id)) return EINA_FALSE;

   call->eo_id = eo_id;

   is_obj = _eo_is_a_obj(eo_id);

   if (is_obj)
     {
        EO_OBJ_POINTER_RETURN_VAL(eo_id, _obj, EINA_FALSE);

        obj = _obj;
        klass = _obj->klass;
        vtable = obj->vtable;

        if (_obj_is_override(obj) && cur_klass &&
            (_eo_class_id_get(cur_klass) == EFL_OBJECT_OVERRIDE_CLASS))
          {
             /* Doing a efl_super(obj, EFL_OBJECT_OVERRIDE_CLASS) should result in calling
              * as if it's a normal class. */
             vtable = &klass->vtable;
             cur_klass = NULL;
          }

        is_override = _obj_is_override(obj) && (cur_klass == NULL);

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
        EINA_LIST_FOREACH(obj->composite_objects, itr, emb_obj_id)
          {
             EO_OBJ_POINTER(emb_obj_id, emb_obj);
             if (EINA_UNLIKELY(!emb_obj)) continue;

             func = _vtable_func_get(emb_obj->vtable, cache->op);
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
   func = _eo_kls_itr_next(klass, cur_klass, cache->op);
   if (!func) goto end;
   klass = func->src;
   goto ok_cur_klass_back;

ok_klass:
     {
        EO_CLASS_POINTER_GOTO(eo_id, _klass, err_klass);
        klass = _klass;
        vtable = &klass->vtable;
        call->obj = NULL;
        call->data = NULL;
     }
   goto ok_klass_back;

err_klass:
   _EO_POINTER_ERR("Class (%p) is an invalid ref.", eo_id);
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

/* klass is the klass we are working on. hierarchy_klass is the class whe should
 * use when validating. */
static Eina_Bool
_eo_class_funcs_set(Eo_Vtable *vtable, const Efl_Object_Ops *ops, const _Efl_Class *hierarchy_klass, const _Efl_Class *klass, Efl_Object_Op id_offset, Eina_Bool override_only)
{
   unsigned int i;
   Efl_Object_Op op_id;
   const void *last_api_func;
   const Efl_Op_Description *op_desc;
   const Efl_Op_Description *op_descs;

   op_id = hierarchy_klass->base_id + id_offset;
   op_descs = ops->descs;

   DBG("Set functions for class '%s':%p", klass->desc->name, klass);

   if (!op_descs) return EINA_TRUE;

   last_api_func = NULL;
   for (i = 0, op_desc = op_descs; i < ops->count; i++, op_desc++)
     {
        Efl_Object_Op op = EFL_NOOP;

        if (op_desc->api_func == NULL)
          {
             ERR("Class '%s': NULL API not allowed (NULL->%p '%s').",
                 klass->desc->name, op_desc->func, _eo_op_desc_name_get(op_desc));
             return EINA_FALSE;
          }

        /* Get the opid for the function. */
          {
             if (_eo_api_func_equal(op_desc->api_func, last_api_func))
               {
                  ERR("Class '%s': API previously defined (%p->%p '%s').",
                      klass->desc->name, op_desc->api_func, op_desc->func, _eo_op_desc_name_get(op_desc));
                  return EINA_FALSE;
               }

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
          }

        DBG("%p->%p '%s'", op_desc->api_func, op_desc->func, _eo_op_desc_name_get(op_desc));

        if (!_vtable_func_set(vtable, klass, op, op_desc->func))
          return EINA_FALSE;

        last_api_func = op_desc->api_func;
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
   _EO_POINTER_ERR("Class (%p) is an invalid ref.", klass_id);
   return EINA_FALSE;
}

EAPI Eo *
_efl_add_internal_start(const char *file, int line, const Efl_Class *klass_id, Eo *parent_id, Eina_Bool ref EINA_UNUSED, Eina_Bool is_fallback)
{
   _Eo_Object *obj;
   Eo_Stack_Frame *fptr = NULL;

   if (is_fallback) fptr = _efl_add_fallback_stack_push(NULL);

   EO_CLASS_POINTER_GOTO(klass_id, klass, err_klass);

   if (parent_id)
     {
        EO_OBJ_POINTER_GOTO(parent_id, parent, err_parent);
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

   obj->refcount++;
   obj->klass = klass;
   obj->vtable = &klass->vtable;

#ifndef HAVE_EO_ID
   EINA_MAGIC_SET((Eo_Header *) obj, EO_EINA_MAGIC);
#endif
   obj->header.id = _eo_id_allocate(obj, parent_id);
   Eo *eo_id = _eo_obj_id_get(obj);

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
        EO_OBJ_POINTER_GOTO(eo_id, new_obj, err_newid);
        /* We have two refs at this point. */
        _efl_unref(obj);
        efl_del((Eo *)obj->header.id);
        _efl_ref(new_obj);
        EO_OBJ_DONE(eo_id);
     }
   goto ok_nomatch_back;

err_noid:
   ERR("Object of class '%s' - Error while constructing object",
       klass->desc->name);
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
   _EO_POINTER_ERR("Class (%p) is an invalid ref.", klass_id);
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

   if (is_ref && efl_parent_get(eo_id))
     {
        efl_ref(eo_id);
     }

   if (is_fallback)
     {
        _efl_add_fallback_stack_pop();
     }

   return ret;
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
   _EO_POINTER_ERR("Class (%p) is an invalid ref.", eo_id);
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
   _EO_POINTER_ERR("Class (%p) is an invalid ref.", eo_id);
err_obj:
   return NULL;
}

static void
_vtable_init(Eo_Vtable *vtable, size_t size)
{
   vtable->size = size;
   vtable->chain = calloc(vtable->size, sizeof(*vtable->chain));
}

#ifdef EO_DEBUG
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
#endif

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
      free(data);

   EINA_TRASH_CLEAN(&klass->iterators.trash, data)
      free(data);

   eina_spinlock_free(&klass->objects.trash_lock);
   eina_spinlock_free(&klass->iterators.trash_lock);

   free(klass);
}

/* Not really called, just used for the ptr... */
static void
_eo_class_isa_func(Eo *eo_id EINA_UNUSED, void *class_data EINA_UNUSED)
{
   /* Do nonthing. */
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

   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->name, NULL);

   _Efl_Class *parent = _eo_class_pointer_get(parent_id);
#ifndef HAVE_EO_ID
   if (parent && !EINA_MAGIC_CHECK((Eo_Header *) parent, EO_CLASS_EINA_MAGIC))
     {
        EINA_MAGIC_FAIL((Eo_Header *) parent, EO_CLASS_EINA_MAGIC);
        return NULL;
     }
#endif

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
             switch (extn->desc->type)
               {
                case EFL_CLASS_TYPE_REGULAR_NO_INSTANT:
                case EFL_CLASS_TYPE_REGULAR:
                case EFL_CLASS_TYPE_INTERFACE:
                case EFL_CLASS_TYPE_MIXIN:
                   extn_list = eina_list_append(extn_list, extn);
                   break;
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
#ifndef HAVE_EO_ID
   EINA_MAGIC_SET((Eo_Header *) klass, EO_CLASS_EINA_MAGIC);
#endif
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
     {
        const _Efl_Class **extn_itr;

        for (extn_itr = klass->extensions ; *extn_itr ; extn_itr++)
          {
             const _Efl_Class *extn = *extn_itr;
             /* Set it in the dich. */
             _vtable_func_set(&klass->vtable, klass, extn->base_id +
                   extn->ops_count, _eo_class_isa_func);
          }

        _vtable_func_set(&klass->vtable, klass, klass->base_id + klass->ops_count,
              _eo_class_isa_func);

        if (klass->parent)
          {
             _vtable_func_set(&klass->vtable, klass,
                   klass->parent->base_id + klass->parent->ops_count,
                   _eo_class_isa_func);
          }
     }

   _eo_class_constructor(klass);

   DBG("Finished building class '%s'", klass->desc->name);

   return _eo_class_id_get(klass);
}

EAPI Eina_Bool
efl_object_override(Eo *eo_id, const Efl_Object_Ops *ops)
{
   EO_OBJ_POINTER_RETURN_VAL(eo_id, obj, EINA_FALSE);
   EO_CLASS_POINTER_GOTO(EFL_OBJECT_OVERRIDE_CLASS, klass, err_done);
   Eo_Vtable *previous = obj->vtable;

   if (ops)
     {
        if (obj->vtable == &obj->klass->vtable)
          {
             obj->vtable = calloc(1, sizeof(*obj->vtable));
             _vtable_init(obj->vtable, previous->size);
             _vtable_copy_all(obj->vtable, previous);
             // rare so move error handling to end to save l1 instruction cache
             if (!_eo_class_funcs_set(obj->vtable, ops, obj->klass,
                                      klass, 0, EINA_TRUE))
               goto err;
             goto done;
          }
        // rare so move error handling to end to save l1 instruction cache
        else goto err_already;
     }
   else
     {
        if (obj->vtable != &obj->klass->vtable)
          {
             free(obj->vtable);
             obj->vtable = (Eo_Vtable *) &obj->klass->vtable;
          }
     }
done:
   EO_OBJ_DONE(eo_id);
   return EINA_TRUE;

err_already:
   ERR("Function table already overridden, not allowed to override again. "
       "Call with NULL to reset the function table first.");
   goto err_done;
err:
   ERR("Failed to override functions for %p", eo_id);
err_done:
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

   domain = ((Eo_Id)eo_id >> SHIFT_DOMAIN) & MASK_DOMAIN;
   data = _eo_table_data_get();
   tdata = _eo_table_data_table_get(data, domain);
   if (!tdata) return EINA_FALSE;

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
          (obj->vtable, klass->base_id + klass->ops_count);

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
          (obj->vtable, klass->base_id + klass->ops_count);

        // Caching the result as we do a lot of serial efl_isa due to evas_object_image using it.
        tdata->cache.isa_id = eo_id;
        tdata->cache.klass = klass_id;
        // Currently implemented by reusing the LAST op id. Just marking it with
        // _eo_class_isa_func.
        isa = tdata->cache.isa = (func && (func->func == _eo_class_isa_func));
        eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
     }
   return isa;

err_shared_class:
   _EO_POINTER_ERR("Class (%p) is an invalid ref.", klass_id);
err_shared_obj:
   eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
   return EINA_FALSE;

err_class:
   _EO_POINTER_ERR("Class (%p) is an invalid ref.", klass_id);
err_obj:
   return EINA_FALSE;
}

EAPI Eo *
efl_xref_internal(const char *file, int line, Eo *obj_id, const Eo *ref_obj_id)
{
   efl_ref(obj_id);

#ifdef EO_DEBUG
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, obj_id);

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
        free(xref);
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
   obj->del_intercept = del_intercept_func;
   EO_OBJ_DONE(obj_id);
}

EAPI Efl_Del_Intercept
efl_del_intercept_get(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);
   Efl_Del_Intercept func;
   func = obj->del_intercept;
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
_efl_data_xunref_internal(_Eo_Object *obj, void *data, const _Eo_Object *ref_obj)
{
#ifdef EO_DEBUG
   const _Efl_Class *klass = obj->klass;
   Eina_Bool in_range = (((char *)data >= (((char *) obj) + _eo_sz)) &&
                         ((char *)data < (((char *) obj) + klass->obj_size)));
   if (!in_range)
     {
        ERR("Data %p is not in the data range of the object %p (%s).", data, (Eo *)obj->header.id, obj->klass->desc->name);
     }
#else
   (void) obj;
   (void) data;
#endif
#ifdef EO_DEBUG
   if (obj->datarefcount == 0)
     {
        ERR("Data for object %zx (%s) is already not referenced.", (size_t)_eo_obj_id_get(obj), obj->klass->desc->name);
     }
   else
     {
        (obj->datarefcount)--;
     }
   Eo_Xref_Node *xref = NULL;
   EINA_INLIST_FOREACH(obj->data_xrefs, xref)
     {
        if (xref->ref_obj == _eo_obj_id_get(ref_obj))
          break;
     }

   if (xref)
     {
        obj->data_xrefs = eina_inlist_remove(obj->data_xrefs, EINA_INLIST_GET(xref));
        free(xref);
     }
   else
     {
        ERR("ref_obj (0x%zx) does not reference data (%p) of obj (0x%zx).", (size_t)_eo_obj_id_get(ref_obj), data, (size_t)_eo_obj_id_get(obj));
     }
#else
   (void) ref_obj;
#endif
}

EAPI void *
efl_data_scope_get(const Eo *obj_id, const Efl_Class *klass_id)
{
   void *ret = NULL;
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);
   EO_CLASS_POINTER_GOTO(klass_id, klass, err_klass);

#ifdef EO_DEBUG
   if (_eo_class_mro_has(obj->klass, klass))
#endif
     ret = _efl_data_scope_safe_get(obj, klass);
#ifdef EO_DEBUG
   // rare to make it a goto to clear out instruction cache of rare code
   else goto err_mro;
   // rare to make it a goto to clear out instruction cache of rare code
   if (!ret && (klass->desc->data_size == 0)) goto err_ret;
   EO_OBJ_DONE(obj_id);
   return ret;

err_ret:
   ERR("Tried getting data of class '%s', but it has none.", klass->desc->name);
   goto err_klass;
err_mro:
   ERR("Tried getting data of class '%s' from object of class '%s', but the former is not a direct inheritance of the latter.", klass->desc->name, obj->klass->desc->name);
#endif
err_klass:
   EO_OBJ_DONE(obj_id);
   return ret;
}

EAPI void *
efl_data_xref_internal(const char *file, int line, const Eo *obj_id, const Efl_Class *klass_id, const Eo *ref_obj_id)
{
   void *ret = NULL;
   _Efl_Class *klass = NULL;
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);
   EO_OBJ_POINTER(ref_obj_id, ref_obj);
   if (ref_obj)
     {
        if (klass_id)
          {
             EO_CLASS_POINTER_GOTO(klass_id, klass2, err_klass);
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

EAPI Eina_Bool
efl_object_init(void)
{
   const char *log_dom = "eo";
   if (_efl_object_init_count++ > 0)
     return EINA_TRUE;

   eina_init();

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
        EINA_LOG_ERR("Could not init lock.");
        return EINA_FALSE;
     }

   if (!eina_spinlock_new(&_ops_storage_lock))
     {
        EINA_LOG_ERR("Could not init lock.");
        return EINA_FALSE;
     }

   if (!eina_spinlock_new(&_super_class_lock))
     {
        EINA_LOG_ERR("Could not init lock.");
        return EINA_FALSE;
     }

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
        EINA_LOG_ERR("Could not allocate shared table data");
        return EINA_FALSE;
     }
   _eo_table_data_shared_data = _eo_table_data_shared->tables[EFL_ID_DOMAIN_SHARED];

   // specially force eoid data to be creanted so we can switch it to domain 0
   Eo_Id_Data *data = _eo_table_data_new(EFL_ID_DOMAIN_MAIN);
   if (!data)
     {
        EINA_LOG_ERR("Could not allocate main table data");
        return EINA_FALSE;
     }
   if (!eina_tls_cb_new(&_eo_table_data, _eo_table_del_cb))
     {
        EINA_LOG_ERR("Could not allocate TLS for eo domain data");
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

   eina_spinlock_free(&_super_class_lock);
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
        ERR("Trying to adopt NULL domain data");
        return EFL_ID_DOMAIN_INVALID;
     }
   if (data_foreign->local_domain == data->local_domain)
     {
        ERR("Trying to adopt EO ID domain %i, is the same as the local %i",
            data_foreign->local_domain, data->local_domain);
        return EFL_ID_DOMAIN_INVALID;
     }
   if (data->tables[data_foreign->local_domain])
     {
        ERR("Trying to adopt an already adopted domain");
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
        ERR("Cannot return the local domain back to its owner");
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
   _eo_free(obj);
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

EAPI int
efl_callbacks_cmp(const Efl_Callback_Array_Item *a, const Efl_Callback_Array_Item *b)
{
   if (a->desc == b->desc) return 0;
   else if (a->desc > b->desc) return 1;
   else return -1;
}
