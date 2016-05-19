#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined HAVE_DLADDR && ! defined _WIN32
# include <dlfcn.h>
#endif

#include <Eina.h>

#define EO_BASE_BETA

#include "Eo.h"
#include "eo_ptr_indirection.h"
#include "eo_private.h"
#include "eo_add_fallback.h"

#include "eo_override.eo.c"

#define EO_CLASS_IDS_FIRST 1
#define EO_OP_IDS_FIRST 1

/* Used inside the class_get functions of classes, see #EO_DEFINE_CLASS */
EAPI Eina_Spinlock _eo_class_creation_lock;
EAPI unsigned int _eo_init_generation = 1;
int _eo_log_dom = -1;

static _Eo_Class **_eo_classes = NULL;
static Eo_Id _eo_classes_last_id = 0;
static Eo_Id _eo_classes_alloc = 0;
static int _eo_init_count = 0;
static Eo_Op _eo_ops_last_id = 0;
static Eina_Hash *_ops_storage = NULL;
static Eina_Spinlock _ops_storage_lock;

static size_t _eo_sz = 0;
static size_t _eo_class_sz = 0;

static void _eo_condtor_reset(_Eo_Object *obj);
static inline void *_eo_data_scope_get(const _Eo_Object *obj, const _Eo_Class *klass);
static inline void *_eo_data_xref_internal(const char *file, int line, _Eo_Object *obj, const _Eo_Class *klass, const _Eo_Object *ref_obj);
static inline void _eo_data_xunref_internal(_Eo_Object *obj, void *data, const _Eo_Object *ref_obj);

/* Start of Dich */

/* How we search and store the implementations in classes. */
#define DICH_CHAIN_LAST_BITS 5
#define DICH_CHAIN_LAST_SIZE (1 << DICH_CHAIN_LAST_BITS)
#define DICH_CHAIN1(x) ((x) >> DICH_CHAIN_LAST_BITS)
#define DICH_CHAIN_LAST(x) ((x) & ((1 << DICH_CHAIN_LAST_BITS) - 1))


/* We are substracting the mask here instead of "AND"ing because it's a hot path,
 * it should be a valid class at this point, and this lets the compiler do 1
 * substraction at compile time. */
#define _UNMASK_ID(id) ((id) - MASK_CLASS_TAG)
#define ID_CLASS_GET(id) ({ \
      (_Eo_Class *) (((_UNMASK_ID(id) <= _eo_classes_last_id) && (_UNMASK_ID(id) > 0)) ? \
      (_eo_classes[_UNMASK_ID(id) - 1]) : NULL); \
      })

static inline void
_vtable_chain_alloc(Dich_Chain1 *chain1)
{
   if (!chain1->funcs)
     {
        chain1->funcs = calloc(DICH_CHAIN_LAST_SIZE, sizeof(*(chain1->funcs)));
     }
}

static inline void
_vtable_copy_all(Eo_Vtable *dst, const Eo_Vtable *src)
{
   Eo_Op i;
   const Dich_Chain1 *sc1 = src->chain;
   Dich_Chain1 *dc1 = dst->chain;
   for (i = 0 ; i < src->size ; i++, sc1++, dc1++)
     {
        if (sc1->funcs)
          {
             size_t j;

             _vtable_chain_alloc(dc1);

             const op_type_funcs *sf = sc1->funcs;
             op_type_funcs *df = dc1->funcs;
             for (j = 0 ; j < DICH_CHAIN_LAST_SIZE ; j++, df++, sf++)
               {
                  if (sf->func)
                    {
                       memcpy(df, sf, sizeof(*df));
                    }
               }
          }
     }
}

static inline const op_type_funcs *
_vtable_func_get(const Eo_Vtable *vtable, Eo_Op op)
{
   size_t idx1 = DICH_CHAIN1(op);
   if (EINA_UNLIKELY(idx1 >= vtable->size))
      return NULL;
   Dich_Chain1 *chain1 = &vtable->chain[idx1];
   if (EINA_UNLIKELY(!chain1->funcs))
      return NULL;
   return &chain1->funcs[DICH_CHAIN_LAST(op)];
}

/* XXX: Only used for a debug message below. Doesn't matter that it's slow. */
static const _Eo_Class *
_eo_op_class_get(Eo_Op op)
{
   _Eo_Class **itr = _eo_classes;
   int mid, max, min;

   min = 0;
   max = _eo_classes_last_id - 1;
   while (min <= max)
     {
        mid = (min + max) / 2;

        if (itr[mid]->base_id + itr[mid]->desc->ops.count < op)
           min = mid + 1;
        else if (itr[mid]->base_id  > op)
           max = mid - 1;
        else
           return itr[mid];
     }

   return NULL;
}

static inline Eina_Bool
_vtable_func_set(Eo_Vtable *vtable, const _Eo_Class *klass, Eo_Op op, eo_op_func_type func)
{
   op_type_funcs *fsrc;
   size_t idx1 = DICH_CHAIN1(op);
   Dich_Chain1 *chain1 = &vtable->chain[idx1];
   _vtable_chain_alloc(chain1);
   fsrc = &chain1->funcs[DICH_CHAIN_LAST(op)];
   if (fsrc->src == klass)
     {
        const _Eo_Class *op_kls = _eo_op_class_get(op);
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
        if (chain1->funcs)
           free(chain1->funcs);
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

static inline _Eo_Class *
_eo_class_pointer_get(const Eo_Class *klass_id)
{
#ifdef HAVE_EO_ID
   return ID_CLASS_GET((Eo_Id)klass_id);
#else
   return (_Eo_Class *) klass_id;
#endif
}

static const char *
_eo_op_desc_name_get(const Eo_Op_Description *desc)
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
_eo_kls_itr_next(const _Eo_Class *orig_kls, const _Eo_Class *cur_klass, Eo_Op op)
{
   const _Eo_Class **kls_itr = NULL;

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

static const Eo_Class *_super_class = NULL;
static Eina_Spinlock _super_class_lock;

EAPI Eo *
eo_super(const Eo *obj, const Eo_Class *cur_klass)
{
   /* FIXME: Switch to atomic operations intead of lock. */
   eina_spinlock_take(&_super_class_lock);
   _super_class = cur_klass;

   return (Eo *) ((Eo_Id) obj | MASK_SUPER_TAG);
}

EAPI Eina_Bool
_eo_call_resolve(Eo *eo_id, const char *func_name, Eo_Op_Call_Data *call, Eo_Call_Cache *cache, const char *file, int line)
{
   const _Eo_Class *klass, *inputklass, *main_klass;
   const _Eo_Class *cur_klass = NULL;
   _Eo_Object *obj = NULL;
   const Eo_Vtable *vtable = NULL;
   const op_type_funcs *func;
   Eina_Bool is_obj;
   Eina_Bool is_override = EINA_FALSE;

   if (((Eo_Id) eo_id) & MASK_SUPER_TAG)
     {
        const Eo_Class *tmp = _super_class;
        _super_class = NULL;
        eina_spinlock_release(&_super_class_lock);

        eo_id = (Eo *) ((Eo_Id) eo_id & ~MASK_SUPER_TAG);

        cur_klass = _eo_class_pointer_get(tmp);
        if (!cur_klass)
          {
             ERR("Invalid super class found. Aborting.");
             return EINA_FALSE;
          }
     }

   if (EINA_UNLIKELY(!eo_id))
      return EINA_FALSE;

   call->eo_id = eo_id;

   is_obj = _eo_is_a_obj(eo_id);

   if (is_obj)
     {
        EO_OBJ_POINTER_RETURN_VAL(eo_id, _obj, EINA_FALSE);
        obj = _obj;
        klass = _obj->klass;
        vtable = obj->vtable;

        if (_obj_is_override(obj) && cur_klass &&
              (_eo_class_id_get(cur_klass) == EO_OVERRIDE_CLASS))
          {
             /* Doing a eo_super(obj, EO_OVERRIDE_CLASS) should result in calling
              * as if it's a normal class. */
             vtable = &klass->vtable;
             cur_klass = NULL;
          }

        is_override = _obj_is_override(obj) && (cur_klass == NULL);

        call->obj = obj;
        _eo_ref(_obj);
     }
   else
     {
        EO_CLASS_POINTER_RETURN_VAL(eo_id, _klass, EINA_FALSE);
        klass = _klass;
        vtable = &klass->vtable;
        call->obj = NULL;
        call->data = NULL;
     }

   inputklass = main_klass =  klass;


   if (!cache->op)
     {
        ERR("%s:%d: unable to resolve %s api func '%s' in class '%s'.",
            file, line, (!is_obj ? "class" : "regular"),
            func_name, klass->desc->name);

        return EINA_FALSE;
     }

   /* If we have a current class, we need to itr to the next. */
   if (cur_klass)
     {
        func = _eo_kls_itr_next(klass, cur_klass, cache->op);

        if (!func)
          goto end;

        klass = func->src;
     }
   else
     {
# if EO_CALL_CACHE_SIZE > 0
        if (!is_override)
          {
# if EO_CALL_CACHE_SIZE > 1
             int i;

             for (i = 0; i < EO_CALL_CACHE_SIZE; i++)
# else
                const int i = 0;
# endif
               {
                  if ((const void *)inputklass == cache->index[i].klass)
                    {
                       func = (const op_type_funcs *)cache->entry[i].func;
                       call->func = func->func;
                       if (is_obj)
                         {
                            call->data = (char *) obj + cache->off[i].off;
                         }
                       return EINA_TRUE;
                    }
               }
          }
#endif

        func = _vtable_func_get(vtable, cache->op);

        if (!func)
          goto end;
     }

   if (EINA_LIKELY(func->func && func->src))
     {
        call->func = func->func;

        if (is_obj)
          {
             call->data = _eo_data_scope_get(obj, func->src);
          }

# if EO_CALL_CACHE_SIZE > 0
        if (!cur_klass && !is_override)
          {
# if EO_CALL_CACHE_SIZE > 1
             const int slot = cache->next_slot;
# else
             const int slot = 0;
# endif
             cache->index[slot].klass = (const void *)inputklass;
             cache->entry[slot].func = (const void *)func;
             cache->off[slot].off = (int)((long)((char *)call->data - (char *)obj));
# if EO_CALL_CACHE_SIZE > 1
             cache->next_slot = (slot + 1) % EO_CALL_CACHE_SIZE;
# endif
          }
#endif

        return EINA_TRUE;
     }

   if (func->src != NULL)
     {
        ERR("in %s:%d: you called a pure virtual func '%s' (%d) of class '%s'.",
            file, line, func_name, cache->op, klass->desc->name);
        return EINA_FALSE;
     }


end:
   /* Try composite objects */
   if (is_obj)
     {
        Eina_List *itr;
        Eo *emb_obj_id;
        EINA_LIST_FOREACH(obj->composite_objects, itr, emb_obj_id)
          {
             _Eo_Object *emb_obj = _eo_obj_pointer_get((Eo_Id)emb_obj_id);

             if (!emb_obj)
               continue;

             func = _vtable_func_get(emb_obj->vtable, cache->op);
             if (func == NULL)
               continue;

             if (EINA_LIKELY(func->func && func->src))
               {
                  call->eo_id = _eo_obj_id_get(emb_obj);
                  call->obj = obj; /* FIXME-eo4: Hack, we retain the previous object so we unref it... */
                  call->func = func->func;
                  call->data = _eo_data_scope_get(emb_obj, func->src);

                  return EINA_TRUE;
               }
          }
     }

     {
        /* If it's a do_super call. */
        if (cur_klass)
          {
             ERR("in %s:%d: func '%s' (%d) could not be resolved for class '%s' for super of '%s'.",
                 file, line, func_name, cache->op, main_klass->desc->name,
                 cur_klass->desc->name);
          }
        else
          {
             /* we should not be able to take this branch */
             ERR("in %s:%d: func '%s' (%d) could not be resolved for class '%s'.",
                 file, line, func_name, cache->op, main_klass->desc->name);
          }
     }
   return EINA_FALSE;
}

EAPI void
_eo_call_end(Eo_Op_Call_Data *call)
{
   if (EINA_LIKELY(!!call->obj))
     {
        _eo_unref(call->obj);
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

/* api_func should be the pointer to the function on all platforms except windows,
 * in which it should be the the name of the function (string).
 */
static inline const Eo_Op_Description *
_eo_api_desc_get(const void *api_func, const _Eo_Class *klass, const _Eo_Class **extns)
{
   const _Eo_Class *cur_klass;
   const _Eo_Class **kls_itr = NULL;
   const Eo_Op_Description *op_desc;
   const Eo_Op_Description *op_descs;

   if (klass)
     {
        for (kls_itr = klass->mro ; *kls_itr ; kls_itr++)
          {
             unsigned int i;
             cur_klass = *kls_itr;
             op_descs = cur_klass->desc->ops.descs;

             for (i = 0, op_desc = op_descs; i < cur_klass->desc->ops.count; i++, op_desc++)
               {
                  if (_eo_api_func_equal(op_desc->api_func, api_func))
                    {
                       return op_desc;
                    }
               }
          }
     }

   if (extns)
     {
        for (kls_itr = extns ; *kls_itr ; kls_itr++)
          {
             cur_klass = *kls_itr;
             op_desc = _eo_api_desc_get(api_func, cur_klass, NULL);
             if (op_desc) return op_desc;
          }
     }

   return NULL;
}

EAPI Eo_Op
_eo_api_op_id_get(const void *api_func)
{
   eina_spinlock_take(&_ops_storage_lock);
#ifndef _WIN32
   Eo_Op op = (uintptr_t) eina_hash_find(_ops_storage, &api_func);
#else
   Eo_Op op = (uintptr_t) eina_hash_find(_ops_storage, api_func);
#endif
   eina_spinlock_release(&_ops_storage_lock);

   if (op == EO_NOOP)
     {
        ERR("Unable to resolve op for api func %p", api_func);
     }

   return op;
}

/* klass is the klass we are working on. hierarchy_klass is the class whe should
 * use when validating. */
static Eina_Bool
_eo_class_funcs_set(Eo_Vtable *vtable, const Eo_Ops *ops, const _Eo_Class *hierarchy_klass, const _Eo_Class *klass, Eina_Bool override_only)
{
   unsigned int i;
   int op_id;
   const void *last_api_func;
   const Eo_Op_Description *op_desc;
   const Eo_Op_Description *op_descs;

   op_id = hierarchy_klass->base_id;
   op_descs = ops->descs;

   DBG("Set functions for class '%s':%p", klass->desc->name, klass);

   if (!op_descs) return EINA_TRUE;

   last_api_func = NULL;
   for (i = 0, op_desc = op_descs; i < ops->count; i++, op_desc++)
     {
        Eo_Op op = EO_NOOP;

        if (op_desc->api_func == NULL)
          {
             ERR("Class '%s': NULL API not allowed (NULL->%p '%s').",
                 klass->desc->name, op_desc->func, _eo_op_desc_name_get(op_desc));
             return EINA_FALSE;
          }

        if ((op_desc->op_type == EO_OP_TYPE_REGULAR) || (op_desc->op_type == EO_OP_TYPE_CLASS))
          {
             if (override_only)
               {
                  ERR("Creation of new functions is not allowed when overriding an object's vtable.");
                  return EINA_FALSE;
               }

             if (_eo_api_func_equal(op_desc->api_func, last_api_func))
               {
                  ERR("Class '%s': API previously defined (%p->%p '%s').",
                      klass->desc->name, op_desc->api_func, op_desc->func, _eo_op_desc_name_get(op_desc));
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
        else if ((op_desc->op_type == EO_OP_TYPE_REGULAR_OVERRIDE) || (op_desc->op_type == EO_OP_TYPE_CLASS_OVERRIDE))
          {
             const Eo_Op_Description *api_desc;
             if (override_only)
               {
                  api_desc = _eo_api_desc_get(op_desc->api_func, hierarchy_klass, NULL);
               }
             else
               {
                  api_desc = _eo_api_desc_get(op_desc->api_func, hierarchy_klass->parent, hierarchy_klass->extensions);
               }

             if (api_desc == NULL)
               {
                  ERR("Class '%s': Can't find api func description in class hierarchy (%p->%p) (%s).",
                      hierarchy_klass->desc->name, op_desc->api_func, op_desc->func, _eo_op_desc_name_get(op_desc));
                  return EINA_FALSE;
               }

             op = _eo_api_op_id_get(op_desc->api_func);
          }

        if (op == EO_NOOP)
          {
             ERR("Class '%s': Invalid op 'EO_NOOP' (%p->%p '%s').",
                 klass->desc->name, op_desc->api_func, op_desc->func, _eo_op_desc_name_get(op_desc));
             return EINA_FALSE;
          }

        DBG("%p->%p '%s'", op_desc->api_func, op_desc->func, _eo_op_desc_name_get(op_desc));

        if (!_vtable_func_set(vtable, klass, op, op_desc->func))
          return EINA_FALSE;

        last_api_func = op_desc->api_func;
     }

   return EINA_TRUE;
}

EAPI Eo *
_eo_add_internal_start(const char *file, int line, const Eo_Class *klass_id, Eo *parent_id, Eina_Bool ref, Eina_Bool is_fallback)
{
   _Eo_Object *obj;
   Eo_Stack_Frame *fptr = NULL;

   if (is_fallback)
     {
        fptr = _eo_add_fallback_stack_push(NULL);
     }

   EO_CLASS_POINTER_RETURN_VAL(klass_id, klass, NULL);

   if (parent_id)
     {
        EO_OBJ_POINTER_RETURN_VAL(parent_id, parent, NULL);
     }

   if (EINA_UNLIKELY(klass->desc->type != EO_CLASS_TYPE_REGULAR))
     {
        ERR("in %s:%d: Class '%s' is not instantiate-able. Aborting.", file, line, klass->desc->name);
        return NULL;
     }

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
   obj->header.id = _eo_id_allocate(obj);
   Eo *eo_id = _eo_obj_id_get(obj);

   _eo_condtor_reset(obj);

   _eo_ref(obj);

   eo_parent_set(eo_id, parent_id);

   /* If there's a parent. Ref. Eo_add should return an object with either a
    * parent ref, or with the lack of, just a ref. */
     {
        if (ref && eo_parent_get(eo_id))
          {
             _eo_ref(obj);
          }
     }

   /* eo_id can change here. Freeing is done on the resolved object. */
   eo_id = eo_constructor(eo_id);
   if (!eo_id)
     {
        ERR("Object of class '%s' - Error while constructing object",
            klass->desc->name);

        /* We have two refs at this point. */
        _eo_unref(obj);
        _eo_unref(obj);
        return NULL;
     }
   else if (eo_id != _eo_obj_id_get(obj))
     {
        /* We have two refs at this point. */
        _eo_unref(obj);
        _eo_unref(obj);

        eo_ref(eo_id);
     }

   if (is_fallback)
     {
        fptr->obj = eo_id;
     }

   return eo_id;
}

static Eo *
_eo_add_internal_end(Eo *eo_id, Eo *finalized_id)
{
   EO_OBJ_POINTER_RETURN_VAL(eo_id, obj, NULL);

   if (!obj->condtor_done)
     {
        const _Eo_Class *klass = obj->klass;

        ERR("Object of class '%s' - Not all of the object constructors have been executed.",
              klass->desc->name);
        goto cleanup;
     }

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

   _eo_unref(obj);

   return (Eo *)eo_id;

cleanup:
   _eo_unref(obj);
   _eo_unref(obj);
   return NULL;
}

EAPI Eo *
_eo_add_end(Eo *eo_id, Eina_Bool is_fallback)
{
   Eo *ret = eo_finalize(eo_id);
   ret = _eo_add_internal_end(eo_id, ret);

   if (is_fallback)
     {
        _eo_add_fallback_stack_pop();
     }

   return ret;
}

/*****************************************************************************/

EAPI const Eo_Class *
eo_class_get(const Eo *eo_id)
{
   if (_eo_is_a_class(eo_id))
     {
        EO_CLASS_POINTER_RETURN_VAL(eo_id, _klass, NULL);
        return EO_CLASS_CLASS;
     }

   EO_OBJ_POINTER_RETURN_VAL(eo_id, obj, NULL);

   return _eo_class_id_get(obj->klass);
}

EAPI const char *
eo_class_name_get(const Eo_Class *eo_id)
{
   const _Eo_Class *klass;

   if (_eo_is_a_class(eo_id))
     {
        EO_CLASS_POINTER_RETURN_VAL(eo_id, _klass, NULL);
        klass = _klass;
     }
   else
     {
        EO_OBJ_POINTER_RETURN_VAL(eo_id, obj, NULL);
        klass = obj->klass;
     }

   return klass->desc->name;
}

static void
_vtable_init(Eo_Vtable *vtable, size_t size)
{
   vtable->size = size;
   vtable->chain = calloc(vtable->size, sizeof(vtable->chain));
}

static void
_eo_class_base_op_init(_Eo_Class *klass)
{
   const Eo_Class_Description *desc = klass->desc;

   klass->base_id = _eo_ops_last_id;

   _eo_ops_last_id += desc->ops.count + 1;

   _vtable_init(&klass->vtable, DICH_CHAIN1(_eo_ops_last_id) + 1);
}

#ifdef EO_DEBUG
static Eina_Bool
_eo_class_mro_has(const _Eo_Class *klass, const _Eo_Class *find)
{
   const _Eo_Class **itr;
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
_eo_class_mro_add(Eina_List *mro, const _Eo_Class *klass)
{
   if (!klass)
     return mro;

   mro = eina_list_append(mro, klass);

   /* Recursively add MIXINS extensions. */
     {
        const _Eo_Class **extn_itr;

        for (extn_itr = klass->extensions ; *extn_itr ; extn_itr++)
          {
             const _Eo_Class *extn = *extn_itr;
             if (extn->desc->type == EO_CLASS_TYPE_MIXIN)
               mro = _eo_class_mro_add(mro, extn);
          }
     }

   mro = _eo_class_mro_add(mro, klass->parent);

   return mro;
}

static Eina_List *
_eo_class_mro_init(const Eo_Class_Description *desc, const _Eo_Class *parent, Eina_List *extensions)
{
   Eina_List *mro = NULL;
   Eina_List *extn_itr = NULL;
   Eina_List *extn_pos = NULL;
   const _Eo_Class *extn = NULL;

   /* Add MIXINS extensions. */
   EINA_LIST_FOREACH(extensions, extn_itr, extn)
     {
        if (extn->desc->type != EO_CLASS_TYPE_MIXIN)
          continue;

        mro = _eo_class_mro_add(mro, extn);
        extn_pos = eina_list_append(extn_pos, eina_list_last(mro));
     }

   /* Check if we can create a consistent mro */
     {
        Eina_List *itr = extn_pos;
        EINA_LIST_FOREACH(extensions, extn_itr, extn)
          {
             if (extn->desc->type != EO_CLASS_TYPE_MIXIN)
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

static void
_eo_class_constructor(_Eo_Class *klass)
{
   klass->constructed = EINA_TRUE;

   if (klass->desc->class_constructor)
     klass->desc->class_constructor(_eo_class_id_get(klass));
}

static void
eo_class_free(_Eo_Class *klass)
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
_eo_class_isa_func(Eo *eo_id EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   /* Do nonthing. */
}

static inline void
_eo_classes_release(void)
{
#ifdef HAVE_MMAP
   size_t size;

   size = _eo_classes_alloc * sizeof(_Eo_Class *);
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
   psize = _eo_classes_alloc * sizeof(_Eo_Class *);
#ifdef HAVE_MMAP
   _eo_classes_alloc += (MEM_PAGE_SIZE / sizeof(_Eo_Class *));
   newsize = _eo_classes_alloc * sizeof(_Eo_Class *);
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
   newsize = _eo_classes_alloc * sizeof(_Eo_Class *);
   ptr = realloc(_eo_classes, newsize);
   if (!ptr)
     {
        ERR("realloc of eo class table region faile!!");
        abort();
     }
#endif
   memset(ptr + psize, 0, newsize - psize);
   _eo_classes = (_Eo_Class **)ptr;
}

EAPI const Eo_Class *
eo_class_new(const Eo_Class_Description *desc, const Eo_Class *parent_id, ...)
{
   _Eo_Class *klass;
   va_list p_list;
   size_t extn_sz, mro_sz, mixins_sz;
   Eina_List *extn_list, *mro, *mixins;

   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->name, NULL);

   _Eo_Class *parent = _eo_class_pointer_get(parent_id);
#ifndef HAVE_EO_ID
   if (parent && !EINA_MAGIC_CHECK((Eo_Header *) parent, EO_CLASS_EINA_MAGIC))
     {
        EINA_MAGIC_FAIL((Eo_Header *) parent, EO_CLASS_EINA_MAGIC);
        return NULL;
     }
#endif

   /* Check restrictions on Interface types. */
   if (desc->type == EO_CLASS_TYPE_INTERFACE)
     {
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->data_size, NULL);
     }

   /* Check parent */
   if (parent)
     {
        /* Verify the inheritance is allowed. */
        switch (desc->type)
          {
           case EO_CLASS_TYPE_REGULAR:
           case EO_CLASS_TYPE_REGULAR_NO_INSTANT:
              if ((parent->desc->type != EO_CLASS_TYPE_REGULAR) &&
                    (parent->desc->type != EO_CLASS_TYPE_REGULAR_NO_INSTANT))
                {
                   ERR("Regular classes ('%s') aren't allowed to inherit from non-regular classes ('%s').",
                       desc->name, parent->desc->name);
                   return NULL;
                }
              break;
           case EO_CLASS_TYPE_INTERFACE:
           case EO_CLASS_TYPE_MIXIN:
              if ((parent->desc->type != EO_CLASS_TYPE_INTERFACE) &&
                    (parent->desc->type != EO_CLASS_TYPE_MIXIN))
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
        const _Eo_Class *extn = NULL;
        const Eo_Id *extn_id = NULL;

        va_start(p_list, parent_id);

        extn_id = va_arg(p_list, Eo_Id *);
        while (extn_id)
          {
             extn = _eo_class_pointer_get((Eo_Class *)extn_id);
             switch (extn->desc->type)
               {
                case EO_CLASS_TYPE_REGULAR_NO_INSTANT:
                case EO_CLASS_TYPE_REGULAR:
                case EO_CLASS_TYPE_INTERFACE:
                case EO_CLASS_TYPE_MIXIN:
                   extn_list = eina_list_append(extn_list, extn);
                   break;
               }

             extn_id = va_arg(p_list, Eo_Id *);
          }

        va_end(p_list);

        extn_list = _eo_class_list_remove_duplicates(extn_list);

        extn_sz = sizeof(_Eo_Class *) * (eina_list_count(extn_list) + 1);

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

        mro_sz = sizeof(_Eo_Class *) * (eina_list_count(mro) + 1);

        DBG("Finished building MRO list for class '%s'", desc->name);
     }

   /* Prepare mixins list */
     {
        Eina_List *itr;
        const _Eo_Class *kls_itr;

        DBG("Started building Mixins list for class '%s'", desc->name);

        mixins = NULL;
        EINA_LIST_FOREACH(mro, itr, kls_itr)
          {
             if ((kls_itr) && (kls_itr->desc->type == EO_CLASS_TYPE_MIXIN) &&
                   (kls_itr->desc->data_size > 0))
               mixins = eina_list_append(mixins, kls_itr);
          }

        mixins_sz = sizeof(Eo_Extension_Data_Offset) * (eina_list_count(mixins) + 1);
        if ((desc->type == EO_CLASS_TYPE_MIXIN) && (desc->data_size > 0))
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
   klass->extensions = (const _Eo_Class **) ((char *) klass + _eo_class_sz);
   klass->mro = (const _Eo_Class **) ((char *) klass->extensions + extn_sz);
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
   if ((desc->type == EO_CLASS_TYPE_MIXIN) && (desc->data_size > 0))
     mixins = eina_list_prepend(mixins, klass);

   /* Copy the extensions and free the list */
     {
        const _Eo_Class *extn = NULL;
        const _Eo_Class **extn_itr = klass->extensions;
        EINA_LIST_FREE(extn_list, extn)
          {
             *(extn_itr++) = extn;

             DBG("Added '%s' extension", extn->desc->name);
          }
        *(extn_itr) = NULL;
     }

   /* Copy the mro and free the list. */
     {
        const _Eo_Class *kls_itr = NULL;
        const _Eo_Class **mro_itr = klass->mro;
        EINA_LIST_FREE(mro, kls_itr)
          {
             *(mro_itr++) = kls_itr;

             DBG("Added '%s' to MRO", kls_itr->desc->name);
          }
        *(mro_itr) = NULL;
     }

   size_t extn_data_off = klass->data_offset;
   if (klass->desc->type != EO_CLASS_TYPE_MIXIN)
      extn_data_off += EO_ALIGN_SIZE(klass->desc->data_size);

   /* Feed the mixins data offsets and free the mixins list. */
     {
        const _Eo_Class *kls_itr = NULL;
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
   if (getenv("EO_DEBUG"))
     {
        fprintf(stderr, "Eo class '%s' will take %u bytes per object.\n",
                desc->name, klass->obj_size);
     }

   _eo_class_base_op_init(klass);

   /* Flatten the function array */
     {
        const _Eo_Class **mro_itr = klass->mro;
        for (  ; *mro_itr ; mro_itr++)
           ;

        /* Skip ourselves. */
        for ( mro_itr-- ; mro_itr > klass->mro ; mro_itr--)
          {
             _vtable_copy_all(&klass->vtable, &(*mro_itr)->vtable);
          }
     }

   /* Mark which classes we implement */
     {
        const _Eo_Class **extn_itr;

        for (extn_itr = klass->extensions ; *extn_itr ; extn_itr++)
          {
             const _Eo_Class *extn = *extn_itr;
             /* Set it in the dich. */
             _vtable_func_set(&klass->vtable, klass, extn->base_id +
                   extn->desc->ops.count, _eo_class_isa_func);
          }

        _vtable_func_set(&klass->vtable, klass, klass->base_id + klass->desc->ops.count,
              _eo_class_isa_func);

        if (klass->parent)
          {
             _vtable_func_set(&klass->vtable, klass,
                   klass->parent->base_id + klass->parent->desc->ops.count,
                   _eo_class_isa_func);
          }
     }

   if (!_eo_class_funcs_set(&klass->vtable, &(klass->desc->ops), klass, klass, EINA_FALSE))
     {
        eina_spinlock_free(&klass->objects.trash_lock);
        eina_spinlock_free(&klass->iterators.trash_lock);
        _vtable_func_clean_all(&klass->vtable);
        free(klass);
        return NULL;
     }

     {
        Eo_Id new_id;

        eina_spinlock_take(&_eo_class_creation_lock);
        new_id = (_eo_classes_last_id + 1) | MASK_CLASS_TAG;
        _eo_classes_expand();
        _eo_classes[_UNMASK_ID(new_id) - 1] = klass;
        eina_spinlock_release(&_eo_class_creation_lock);

        klass->header.id = new_id;
     }

   _eo_class_constructor(klass);

   DBG("Finished building class '%s'", klass->desc->name);

   return _eo_class_id_get(klass);
}

EAPI Eina_Bool
eo_override(Eo *eo_id, Eo_Ops ops)
{
   EO_OBJ_POINTER_RETURN_VAL(eo_id, obj, EINA_FALSE);
   EO_CLASS_POINTER_RETURN_VAL(EO_OVERRIDE_CLASS, klass, EINA_FALSE);
   Eo_Vtable *previous = obj->vtable;
   obj->vtable = calloc(1, sizeof(*obj->vtable));
   _vtable_init(obj->vtable, previous->size);
   _vtable_copy_all(obj->vtable, previous);

   if (!_eo_class_funcs_set(obj->vtable, &ops, obj->klass, klass, EINA_TRUE))
     {
        ERR("Failed to override functions for %p", eo_id);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
eo_isa(const Eo *eo_id, const Eo_Class *klass_id)
{
   EO_OBJ_POINTER_RETURN_VAL(eo_id, obj, EINA_FALSE);
   EO_CLASS_POINTER_RETURN_VAL(klass_id, klass, EINA_FALSE);
   const op_type_funcs *func = _vtable_func_get(obj->vtable,
         klass->base_id + klass->desc->ops.count);

   /* Currently implemented by reusing the LAST op id. Just marking it with
    * _eo_class_isa_func. */
   return (func && (func->func == _eo_class_isa_func));
}

EAPI Eo *
eo_xref_internal(const char *file, int line, Eo *obj_id, const Eo *ref_obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, obj_id);

   _eo_ref(obj);

#ifdef EO_DEBUG
   Eo_Xref_Node *xref = calloc(1, sizeof(*xref));
   xref->ref_obj = ref_obj_id;
   xref->file = file;
   xref->line = line;

   obj->xrefs = eina_inlist_prepend(obj->xrefs, EINA_INLIST_GET(xref));
#else
   (void) ref_obj_id;
   (void) file;
   (void) line;
#endif

   return obj_id;
}

EAPI void
eo_xunref(Eo *obj_id, const Eo *ref_obj_id)
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
        return;
     }
#else
   (void) ref_obj_id;
#endif
   _eo_unref(obj);
}

EAPI Eo *
eo_ref(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, (Eo *)obj_id);

   _eo_ref(obj);
   return (Eo *)obj_id;
}

EAPI void
eo_unref(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);

   _eo_unref(obj);
}

EAPI int
eo_ref_get(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, 0);

   return obj->refcount;
}

EAPI void
eo_del_intercept_set(Eo *obj_id, Eo_Del_Intercept del_intercept_func)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);

   obj->del_intercept = del_intercept_func;
}

EAPI Eo_Del_Intercept
eo_del_intercept_get(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);

   return obj->del_intercept;
}

void
_eo_condtor_done(Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);
   if (obj->condtor_done)
     {
        ERR("Object %p is already constructed at this point.", obj);
        return;
     }

   obj->condtor_done = EINA_TRUE;
}

static inline void *
_eo_data_scope_safe_get(const _Eo_Object *obj, const _Eo_Class *klass)
{
   if (EINA_LIKELY(klass->desc->data_size > 0))
     {
        return _eo_data_scope_get(obj, klass);
     }

   return NULL;
}

static inline void *
_eo_data_scope_get(const _Eo_Object *obj, const _Eo_Class *klass)
{
   if (EINA_LIKELY(klass->desc->type != EO_CLASS_TYPE_MIXIN))
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
_eo_data_xref_internal(const char *file, int line, _Eo_Object *obj, const _Eo_Class *klass, const _Eo_Object *ref_obj)
{
   void *data = NULL;
   if (klass != NULL)
     {
        data = _eo_data_scope_safe_get(obj, klass);
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
_eo_data_xunref_internal(_Eo_Object *obj, void *data, const _Eo_Object *ref_obj)
{
#ifdef EO_DEBUG
   const _Eo_Class *klass = obj->klass;
   Eina_Bool in_range = (((char *)data >= (((char *) obj) + _eo_sz)) &&
                         ((char *)data < (((char *) obj) + klass->obj_size)));
   if (!in_range)
     {
        ERR("Data %p is not in the data range of the object %p (%s).", data, (Eo *)obj->headr.id, obj->klass->desc->name);
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
eo_data_scope_get(const Eo *obj_id, const Eo_Class *klass_id)
{
   void *ret;
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);
   EO_CLASS_POINTER_RETURN_VAL(klass_id, klass, NULL);

#ifdef EO_DEBUG
   if (!_eo_class_mro_has(obj->klass, klass))
     {
        ERR("Tried getting data of class '%s' from object of class '%s', but the former is not a direct inheritance of the latter.", klass->desc->name, obj->klass->desc->name);
        return NULL;
     }
#endif

   ret = _eo_data_scope_safe_get(obj, klass);

#ifdef EO_DEBUG
   if (!ret && (klass->desc->data_size == 0))
     {
        ERR("Tried getting data of class '%s', but it has none.", klass->desc->name);
     }
#endif

   return ret;
}

EAPI void *
eo_data_xref_internal(const char *file, int line, const Eo *obj_id, const Eo_Class *klass_id, const Eo *ref_obj_id)
{
   void *ret;
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);
   EO_OBJ_POINTER_RETURN_VAL(ref_obj_id, ref_obj, NULL);
   _Eo_Class *klass = NULL;
   if (klass_id)
     {
        EO_CLASS_POINTER_RETURN_VAL(klass_id, klass2, NULL);
        klass = klass2;

#ifdef EO_DEBUG
        if (!_eo_class_mro_has(obj->klass, klass))
          {
             ERR("Tried getting data of class '%s' from object of class '%s', but the former is not a direct inheritance of the latter.", klass->desc->name, obj->klass->desc->name);
             return NULL;
          }
#endif
     }

   ret = _eo_data_xref_internal(file, line, obj, klass, ref_obj);

#ifdef EO_DEBUG
   if (klass && !ret && (klass->desc->data_size == 0))
     {
        ERR("Tried getting data of class '%s', but it has none.", klass->desc->name);
     }
#endif

   return ret;
}

EAPI void
eo_data_xunref_internal(const Eo *obj_id, void *data, const Eo *ref_obj_id)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);
   EO_OBJ_POINTER_RETURN(ref_obj_id, ref_obj);
   _eo_data_xunref_internal(obj, data, ref_obj);
}

EAPI Eina_Bool
eo_init(void)
{
   const char *log_dom = "eo";
   if (_eo_init_count++ > 0)
     return EINA_TRUE;

   eina_init();

   _eo_sz = EO_ALIGN_SIZE(sizeof(_Eo_Object));
   _eo_class_sz = EO_ALIGN_SIZE(sizeof(_Eo_Class));

   _eo_classes = NULL;
   _eo_classes_last_id = EO_CLASS_IDS_FIRST - 1;
   _eo_ops_last_id = EO_OP_IDS_FIRST;
   _eo_log_dom = eina_log_domain_register(log_dom, EINA_COLOR_LIGHTBLUE);
   if (_eo_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s.", log_dom);
        return EINA_FALSE;
     }

   if (!eina_spinlock_new(&_eo_class_creation_lock))
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

#ifndef _WIN32
   _ops_storage = eina_hash_pointer_new(NULL);
#else
   _ops_storage = eina_hash_string_superfast_new(NULL);
#endif

#ifdef EO_DEBUG
   /* Call it just for coverage purposes. Ugly I know, but I like it better than
    * casting everywhere else. */
   _eo_class_isa_func(NULL, NULL, NULL);
#endif

   _eo_add_fallback_init();

   eina_log_timing(_eo_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   /* bootstrap EO_CLASS_CLASS */
   (void) EO_CLASS_CLASS;

   return EINA_TRUE;
}

EAPI Eina_Bool
eo_shutdown(void)
{
   size_t i;
   _Eo_Class **cls_itr = _eo_classes;

   if (--_eo_init_count > 0)
     return EINA_TRUE;

   eina_log_timing(_eo_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   _eo_add_fallback_shutdown();

   for (i = 0 ; i < _eo_classes_last_id ; i++, cls_itr++)
     {
        if (*cls_itr)
          eo_class_free(*cls_itr);
     }

   eina_spinlock_take(&_eo_class_creation_lock);
   _eo_classes_release();
   eina_spinlock_release(&_eo_class_creation_lock);

   eina_hash_free(_ops_storage);

   eina_spinlock_free(&_super_class_lock);
   eina_spinlock_free(&_ops_storage_lock);
   eina_spinlock_free(&_eo_class_creation_lock);

   _eo_free_ids_tables();

   eina_log_domain_unregister(_eo_log_dom);
   _eo_log_dom = -1;

   ++_eo_init_generation;

   eina_shutdown();
   return EINA_FALSE;
}

EAPI Eina_Bool
eo_destructed_is(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

   return obj->destructed;
}

EAPI void
eo_manual_free_set(Eo *obj_id, Eina_Bool manual_free)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);
   obj->manual_free = manual_free;
}

EAPI Eina_Bool
eo_manual_free(Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

   if (EINA_FALSE == obj->manual_free)
     {
        ERR("Tried to manually free the object %p while the option has not been set; see eo_manual_free_set for more information.", obj);
        return EINA_FALSE;
     }

   if (!obj->destructed)
     {
        ERR("Tried deleting the object %p while still referenced(%d).", obj_id, obj->refcount);
        return EINA_FALSE;
     }

   _eo_free(obj);

   return EINA_TRUE;
}

