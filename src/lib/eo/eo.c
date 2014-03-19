#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "Eo.h"
#include "eo_ptr_indirection.h"
#include "eo_private.h"

#define EO_CLASS_IDS_FIRST 1
#define EO_OP_IDS_FIRST 1

/* Used inside the class_get functions of classes, see #EO_DEFINE_CLASS */
EAPI Eina_Spinlock _eo_class_creation_lock;
int _eo_log_dom = -1;

static _Eo_Class **_eo_classes;
static Eo_Id _eo_classes_last_id;
static Eina_Bool _eo_init_count = 0;
static Eo_Op _eo_ops_last_id = 0;

static size_t _eo_sz = 0;
static size_t _eo_class_sz = 0;

static void _eo_condtor_reset(_Eo_Object *obj);
static inline void *_eo_data_scope_get(const _Eo_Object *obj, const _Eo_Class *klass);
static inline void *_eo_data_xref_internal(const char *file, int line, _Eo_Object *obj, const _Eo_Class *klass, const _Eo_Object *ref_obj);
static inline void _eo_data_xunref_internal(_Eo_Object *obj, void *data, const _Eo_Object *ref_obj);
static const _Eo_Class *_eo_op_class_get(Eo_Op op);
static const Eo_Op_Description *_eo_op_id_desc_get(Eo_Op op);

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
_dich_chain_alloc(Dich_Chain1 *chain1)
{
   if (!chain1->funcs)
     {
        chain1->funcs = calloc(DICH_CHAIN_LAST_SIZE, sizeof(*(chain1->funcs)));
     }
}

static inline void
_dich_copy_all(_Eo_Class *dst, const _Eo_Class *src)
{
   Eo_Op i;
   const Dich_Chain1 *sc1 = src->chain;
   Dich_Chain1 *dc1 = dst->chain;
   for (i = 0 ; i < src->chain_size ; i++, sc1++, dc1++)
     {
        if (sc1->funcs)
          {
             size_t j;

             _dich_chain_alloc(dc1);

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
_dich_func_get(const _Eo_Class *klass, Eo_Op op)
{
   size_t idx1 = DICH_CHAIN1(op);
   if (EINA_UNLIKELY(idx1 >= klass->chain_size))
      return NULL;
   Dich_Chain1 *chain1 = &klass->chain[idx1];
   if (EINA_UNLIKELY(!chain1->funcs))
      return NULL;
   return &chain1->funcs[DICH_CHAIN_LAST(op)];
}

static inline void
_dich_func_set(_Eo_Class *klass, Eo_Op op, eo_op_func_type func)
{
   size_t idx1 = DICH_CHAIN1(op);
   Dich_Chain1 *chain1 = &klass->chain[idx1];
   _dich_chain_alloc(chain1);
   if (chain1->funcs[DICH_CHAIN_LAST(op)].src == klass)
     {
        const _Eo_Class *op_kls = _eo_op_class_get(op);
        const Eo_Op_Description *op_desc = _eo_op_id_desc_get(op);
        ERR("Already set function for op 0x%x (%s:%s). Overriding with func %p",
              op, op_kls->desc->name, op_desc->name, func);
     }

   chain1->funcs[DICH_CHAIN_LAST(op)].func = func;
   chain1->funcs[DICH_CHAIN_LAST(op)].src = klass;
}

static inline void
_dich_func_clean_all(_Eo_Class *klass)
{
   size_t i;
   Dich_Chain1 *chain1 = klass->chain;

   for (i = 0 ; i < klass->chain_size ; i++, chain1++)
     {
        if (chain1->funcs)
           free(chain1->funcs);
     }
   free(klass->chain);
   klass->chain = NULL;
}

/* END OF DICH */

static const Eo_Op_Description noop_desc =
        EO_OP_DESCRIPTION(EO_NOOP, "No operation.");


static inline Eina_Bool
_eo_is_a_class(const Eo *obj_id)
{
   Eo_Id oid;
#ifdef HAVE_EO_ID
   oid = (Eo_Id) obj_id;
#else
   /* fortunately EO_OBJ_POINTER_RETURN* will handle NULL obj_id */
   if (!obj_id) return EINA_FALSE;
   oid = ((Eo_Base *) obj_id)->id;
#endif
   return (((oid >> REF_TAG_SHIFT) & 0x1) == 0x0);
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

static const _Eo_Class *
_eo_op_class_get(Eo_Op op)
{
   /* FIXME: Make it fast. */
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

static const Eo_Op_Description *
_eo_op_id_desc_get(Eo_Op op)
{
   const _Eo_Class *klass;

   if (op == EO_NOOP)
      return &noop_desc;

   klass = _eo_op_class_get(op);

   if (klass)
     {
        Eo_Op sub_id = op - klass->base_id;
       if (sub_id < klass->desc->ops.count)
          return klass->desc->ops.descs + sub_id;
     }

   return NULL;
}

static const char *
_eo_op_id_name_get(Eo_Op op)
{
   const Eo_Op_Description *desc = _eo_op_id_desc_get(op);
   return (desc) ? desc->name : NULL;
}

static inline const _Eo_Class *
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
             const op_type_funcs *fsrc = _dich_func_get(*kls_itr, op);
             if (!fsrc || !fsrc->func)
               {
                  kls_itr++;
                  continue;
               }
             return fsrc->src;
          }
     }

   return NULL;
}

static inline const op_type_funcs *
_eo_kls_itr_func_get(const _Eo_Class *cur_klass, Eo_Op op)
{
   const _Eo_Class *klass = cur_klass;
   if (klass)
     {
        const op_type_funcs *func = _dich_func_get(klass, op);

        if (func && func->func)
          {
             return func;
          }
     }

   return NULL;
}

#define _EO_OP_ERR_NO_OP_PRINT(file, line, op, klass) \
   do \
      { \
         const _Eo_Class *op_klass = _eo_op_class_get(op); \
         const char *_dom_name = (op_klass) ? op_klass->desc->name : NULL; \
         ERR("in %s:%d: Can't execute function %s:%s (op 0x%x) for class '%s'. Aborting.", \
               file, line, _dom_name, _eo_op_id_name_get(op), op, \
               (klass) ? klass->desc->name : NULL); \
      } \
   while (0)

static inline Eina_Bool
_eo_op_internal(const char *file, int line, Eo_Base *eo_ptr, const _Eo_Class *cur_klass,
                Eo_Op_Type op_type, Eo_Op op, va_list *p_list)
{
#ifdef EO_DEBUG
   const Eo_Op_Description *op_desc = _eo_op_id_desc_get(op);

   if (op_desc && (op_type != op_desc->op_type))
     {
        if (op_type == EO_OP_TYPE_REGULAR)
          {
             ERR("in %s:%d: Tried calling a class op '%s' (0x%x) from a non-class context.",
                 file, line, op_desc->name, op);
          }
        else
          {
             ERR("in %s:%d: Tried calling an instance op '%s' (0x%x) from a class context.",
                 file, line, op_desc->name, op);
          }
        return EINA_FALSE;
     }
#endif

     {
        const op_type_funcs *func = _eo_kls_itr_func_get(cur_klass, op);
        if (EINA_LIKELY(func != NULL))
          {
             void *func_data = NULL;
             Eo *calling_obj;
             if (op_type == EO_OP_TYPE_REGULAR)
               {
                  func_data = _eo_data_scope_get((_Eo_Object *) eo_ptr, func->src);
                  calling_obj = _eo_id_get((_Eo_Object *) eo_ptr);
               }
             else
               {
                  calling_obj = _eo_class_id_get(cur_klass);
               }
             func->func(calling_obj, func_data, p_list);
             return EINA_TRUE;
          }
     }

   /* Try composite objects */
   if (op_type == EO_OP_TYPE_REGULAR)
     {
        const _Eo_Object **comp_itr = ((_Eo_Object *) eo_ptr)->composites;
        if (!comp_itr) return EINA_FALSE;

        for (unsigned int i = 0; i < ((_Eo_Object *) eo_ptr)->klass->composites_count; i++, comp_itr++)
          if (*comp_itr)
            {
               if (_eo_op_internal(file, line, (Eo_Base *) (*comp_itr), (*comp_itr)->klass, op_type, op, p_list))
                 {
                    return EINA_TRUE;
                 }
            }
     }

   return EINA_FALSE;
}

static inline Eina_Bool
_eo_dov_internal(const char *file, int line, Eo_Base *obj, const _Eo_Class *klass, Eo_Op_Type op_type, va_list *p_list)
{
   Eina_Bool ret = EINA_TRUE;
   Eo_Op op = EO_NOOP;

   op = va_arg(*p_list, Eo_Op);
   while (op)
     {
        if (!_eo_op_internal(file, line, obj, klass, op_type, op, p_list))
          {
             _EO_OP_ERR_NO_OP_PRINT(file, line, op, klass);
             ret = EINA_FALSE;
             break;
          }
        op = va_arg(*p_list, Eo_Op);
     }

   return ret;
}

static inline Eina_Bool
_eo_obj_dov_internal(const char *file, int line, _Eo_Object *obj, va_list *p_list)
{
   Eina_Bool prev_error;
   Eina_Bool ret = EINA_TRUE;

   prev_error = obj->do_error;
   _eo_ref(obj);

   ret = _eo_dov_internal(file, line, (Eo_Base *) obj, obj->klass, EO_OP_TYPE_REGULAR, p_list);

   if (obj->do_error)
     ret = EINA_FALSE;

   obj->do_error = prev_error;
   _eo_unref(obj);

   return ret;
}

static inline Eina_Bool
_eo_class_dov_internal(const char *file, int line, _Eo_Class *klass, va_list *p_list)
{
   return _eo_dov_internal(file, line, (Eo_Base *) klass, klass, EO_OP_TYPE_CLASS, p_list);
}

EAPI Eina_Bool
eo_do_internal(const char *file, int line, const Eo *obj_id, ...)
{
   Eina_Bool ret = EINA_TRUE;
   va_list p_list;
   Eina_Bool class_ref = _eo_is_a_class(obj_id);

   if (class_ref)
     {
        EO_CLASS_POINTER_RETURN_VAL(obj_id, klass, EINA_FALSE);

        va_start(p_list, obj_id);
        ret = _eo_class_dov_internal(file, line, klass, &p_list);
        va_end(p_list);
     }
   else
     {
        EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

        va_start(p_list, obj_id);
        ret = _eo_obj_dov_internal(file, line, obj, &p_list);
        va_end(p_list);
     }

   return ret;
}

EAPI Eina_Bool
eo_vdo_internal(const char *file, int line, const Eo *obj_id, va_list *ops)
{
   Eina_Bool class_ref = _eo_is_a_class(obj_id);

   if (class_ref)
     {
        EO_CLASS_POINTER_RETURN_VAL(obj_id, klass, EINA_FALSE);
        return _eo_class_dov_internal(file, line, klass, ops);
     }
   else
     {
        EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);
        return _eo_obj_dov_internal(file, line, obj, ops);
     }
}

EAPI Eina_Bool
eo_do_super_internal(const char *file, int line, const Eo *obj_id, const Eo_Class *cur_klass_id, Eo_Op op, ...)
{
   const _Eo_Class *nklass;
   Eina_Bool op_ret = EINA_TRUE;
   Eina_Bool ret = EINA_TRUE;
   va_list p_list;

   EO_CLASS_POINTER_RETURN_VAL(cur_klass_id, cur_klass, EINA_FALSE);

   if (_eo_is_a_class(obj_id))
     {
        EO_CLASS_POINTER_RETURN_VAL(obj_id, klass, EINA_FALSE);

        va_start(p_list, op);
        nklass = _eo_kls_itr_next(klass, cur_klass, op);
        op_ret = _eo_op_internal(file, line, (Eo_Base *) klass, nklass, EO_OP_TYPE_CLASS, op, &p_list);
        va_end(p_list);
     }
   else
     {
        EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

        va_start(p_list, op);
        nklass = _eo_kls_itr_next(obj->klass, cur_klass, op);
        op_ret = _eo_op_internal(file, line, (Eo_Base *) obj, nklass, EO_OP_TYPE_REGULAR, op, &p_list);
        if (obj->do_error)
          ret = EINA_FALSE;
        va_end(p_list);
     }

   if (!op_ret)
     _EO_OP_ERR_NO_OP_PRINT(file, line, op, nklass);

   return (ret & op_ret);
}

EAPI const Eo_Class *
eo_class_get(const Eo *obj_id)
{
   if (_eo_is_a_class(obj_id))
     {
        EO_CLASS_POINTER_RETURN_VAL(obj_id, _klass, NULL);
        return eo_class_class_get();
     }

   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);

   if (obj->klass)
      return _eo_class_id_get(obj->klass);
   return NULL;
}

EAPI const char *
eo_class_name_get(const Eo_Class *obj_id)
{
   const _Eo_Class *klass;

   if (_eo_is_a_class(obj_id))
     {
        EO_CLASS_POINTER_RETURN_VAL(obj_id, _klass, NULL);
        klass = _klass;
     }
     else
     {
        EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);
        klass = obj->klass;
     }

   return klass->desc->name;
}

static void
_eo_class_base_op_init(_Eo_Class *klass)
{
   const Eo_Class_Description *desc = klass->desc;

   klass->base_id = _eo_ops_last_id;

   if (desc->ops.base_op_id)
      *(desc->ops.base_op_id) = klass->base_id;

   _eo_ops_last_id += desc->ops.count + 1;

   klass->chain_size = DICH_CHAIN1(_eo_ops_last_id) + 1;
   klass->chain = calloc(klass->chain_size, sizeof(*klass->chain));
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
   if (klass->constructed)
      return;

   klass->constructed = EINA_TRUE;

   if (klass->desc->class_constructor)
      klass->desc->class_constructor(_eo_class_id_get(klass));
}

EAPI void
eo_class_funcs_set(Eo_Class *klass_id, const Eo_Op_Func_Description *func_descs)
{
   EO_CLASS_POINTER_RETURN(klass_id, klass);

   const Eo_Op_Func_Description *itr;
   itr = func_descs;
   if (itr)
     {
        for ( ; itr->op_type != EO_OP_TYPE_INVALID ; itr++)
          {
             const Eo_Op_Description *op_desc = _eo_op_id_desc_get(itr->op);

             if (EINA_UNLIKELY(!op_desc || (itr->op == EO_NOOP)))
               {
                  ERR("Setting implementation for non-existent op 0x%x for class '%s'. Func index: %lu", itr->op, klass->desc->name, (unsigned long) (itr - func_descs));
               }
             else if (EINA_LIKELY(itr->op_type == op_desc->op_type))
               {
                  _dich_func_set(klass, itr->op, itr->func);
               }
             else
               {
                  ERR("Set function's op type (0x%x) is different than the one in the op description (%d) for op '%s:%s'. Func index: %lu",
                        itr->op_type,
                        (op_desc) ? op_desc->op_type : EO_OP_TYPE_REGULAR,
                        klass->desc->name,
                        (op_desc) ? op_desc->name : NULL,
                        (unsigned long) (itr - func_descs));
               }
          }
     }
}

static void
eo_class_free(_Eo_Class *klass)
{
   void *data;

   if (klass->constructed)
     {
        if (klass->desc->class_destructor)
           klass->desc->class_destructor(_eo_class_id_get(klass));

        _dich_func_clean_all(klass);
     }

   EINA_TRASH_CLEAN(&klass->objects.trash, data)
      free(data);

   EINA_TRASH_CLEAN(&klass->iterators.trash, data)
      free(data);

   eina_spinlock_free(&klass->objects.trash_lock);
   eina_spinlock_free(&klass->iterators.trash_lock);

   free(klass);
}

/* DEVCHECK */
static Eina_Bool
_eo_class_check_op_descs(const Eo_Class_Description *desc)
{
   const Eo_Op_Description *itr;
   size_t i;

   if (desc->ops.count > 0)
     {
        if (!desc->ops.base_op_id)
          {
             ERR("Class '%s' has a non-zero ops count, but base_id is NULL.",
                   desc->name);
             return EINA_FALSE;
          }

        if (!desc->ops.descs)
          {
             ERR("Class '%s' has a non-zero ops count, but there are no descs.",
                   desc->name);
             return EINA_FALSE;
          }
     }

   itr = desc->ops.descs;
   for (i = 0 ; i < desc->ops.count ; i++, itr++)
     {
        if (itr->sub_op != i)
          {
             if (itr->name)
               {
                  ERR("Wrong order in Ops description for class '%s'. Expected 0x%lx and got 0x%lx", desc->name, (unsigned long) i, (unsigned long) itr->sub_op);
               }
             else
               {
                  ERR("Found too few Ops description for class '%s'. Expected 0x%lx descriptions, but found 0x%lx.", desc->name, (unsigned long) desc->ops.count, (unsigned long) i);
               }
             return EINA_FALSE;
          }
     }

   if (itr && itr->name)
     {
        ERR("Found extra Ops description for class '%s'. Expected %lu descriptions, but found more.", desc->name, (unsigned long) desc->ops.count);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/* Not really called, just used for the ptr... */
static void
_eo_class_isa_func(Eo *obj_id EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   /* Do nonthing. */
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

   DBG("Started building class '%s'", desc->name);

   if (!_eo_class_check_op_descs(desc))
     return NULL;

   _Eo_Class *parent = _eo_class_pointer_get(parent_id);
#ifndef HAVE_EO_ID
   if (parent && !EINA_MAGIC_CHECK((Eo_Base *) parent, EO_CLASS_EINA_MAGIC))
     {
        EINA_MAGIC_FAIL((Eo_Base *) parent, EO_CLASS_EINA_MAGIC);
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
                   ERR("Regular non-instantiable classes ('%s') aren't allowed in class ('%s') extension list",
                       extn->desc->name, desc->name);
                   va_end(p_list);
                   return NULL;
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
   EINA_MAGIC_SET((Eo_Base *) klass, EO_CLASS_EINA_MAGIC);
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
             if (extn->desc->type == EO_CLASS_TYPE_REGULAR)
               klass->composites_count += 1;

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

   klass->obj_size = _eo_sz + extn_data_off;
   if (klass->composites_count > 0)
     klass->obj_size += (klass->composites_count * sizeof(_Eo_Object *));
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
             _dich_copy_all(klass, *mro_itr);
          }
     }

   /* Mark which classes we implement */
     {
        const _Eo_Class **extn_itr;

        for (extn_itr = klass->extensions ; *extn_itr ; extn_itr++)
          {
             const _Eo_Class *extn = *extn_itr;
             /* Set it in the dich. */
             _dich_func_set(klass, extn->base_id +
                   extn->desc->ops.count, _eo_class_isa_func);
          }

        _dich_func_set(klass, klass->base_id + klass->desc->ops.count,
              _eo_class_isa_func);

        if (klass->parent)
          {
             _dich_func_set(klass,
                   klass->parent->base_id + klass->parent->desc->ops.count,
                   _eo_class_isa_func);
          }
     }

   eina_spinlock_take(&_eo_class_creation_lock);
   klass->header.id = ++_eo_classes_last_id | MASK_CLASS_TAG;
     {
        /* FIXME: Handle errors. */
        size_t arrsize = _eo_classes_last_id * sizeof(*_eo_classes);
        _Eo_Class **tmp;
        tmp = realloc(_eo_classes, arrsize);

        /* If it's the first allocation, memset. */
        if (!_eo_classes)
           memset(tmp, 0, arrsize);

        _eo_classes = tmp;
        _eo_classes[klass->header.id - 1] = klass;
     }
   eina_spinlock_release(&_eo_class_creation_lock);

   _eo_class_constructor(klass);

   DBG("Finished building class '%s'", klass->desc->name);

   return _eo_class_id_get(klass);
}

EAPI Eina_Bool
eo_isa(const Eo *obj_id, const Eo_Class *klass_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);
   EO_CLASS_POINTER_RETURN_VAL(klass_id, klass, EINA_FALSE);
   const op_type_funcs *func = _dich_func_get(obj->klass,
         klass->base_id + klass->desc->ops.count);

   /* Currently implemented by reusing the LAST op id. Just marking it with
    * _eo_class_isa_func. */
   return (func && (func->func == _eo_class_isa_func));
}

// A little bit hacky, but does the job
static void
_eo_parent_internal_set(_Eo_Object *obj, ...)
{
   va_list p_list;

   va_start(p_list, obj);
   _eo_op_internal(__FILE__, __LINE__, (Eo_Base *) obj, obj->klass,
                   EO_OP_TYPE_REGULAR, EO_BASE_ID(EO_BASE_SUB_ID_PARENT_SET),
                   &p_list);
   va_end(p_list);
}

EAPI Eo *
eo_add_internal(const char *file, int line, const Eo_Class *klass_id, Eo *parent_id, ...)
{
   Eina_Bool do_err;
   _Eo_Object *obj;
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
   if (klass->composites_count == 0)
     obj->composites = NULL;
   else
     obj->composites = (const _Eo_Object **)
        ((char *) obj + klass->obj_size - (klass->composites_count * sizeof(_Eo_Object *)));

#ifndef HAVE_EO_ID
   EINA_MAGIC_SET((Eo_Base *) obj, EO_EINA_MAGIC);
#endif
   Eo_Id obj_id = _eo_id_allocate(obj);
   obj->header.id = obj_id;

   _eo_condtor_reset(obj);

   _eo_ref(obj);

   _eo_parent_internal_set(obj, parent_id);

   /* Run the relevant do stuff. */
     {
        va_list p_list;
        va_start(p_list, parent_id);
        do_err = !_eo_obj_dov_internal(file, line, obj, &p_list);
        va_end(p_list);
     }

   if (EINA_UNLIKELY(do_err))
     {
        ERR("in %s:%d, Object of class '%s' - One of the object constructors have failed.",
            file, line, klass->desc->name);
        goto fail;
     }

   if (!obj->condtor_done)
     {
        ERR("in %s:%d: Object of class '%s' - Not all of the object constructors have been executed.",
            file, line, klass->desc->name);
        goto fail;
     }

   _eo_unref(obj);

   return _eo_id_get(obj);

fail:
   /* Unref twice, once for the ref above, and once for the basic object ref. */
   _eo_unref(obj);
   _eo_unref(obj);
   return NULL;
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

EAPI void
eo_del(const Eo *obj)
{
   eo_do((Eo *) obj, eo_parent_set(NULL));
   eo_unref(obj);
}

EAPI int
eo_ref_get(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, 0);

   return obj->refcount;
}

EAPI void
eo_error_set_internal(const Eo *obj_id, const char *file, int line)
{
   EO_OBJ_POINTER_RETURN(obj_id, obj);

   ERR("Error with obj '%p' at %s:%d", obj, file, line);

   obj->do_error = EINA_TRUE;
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
_eo_data_scope_get(const _Eo_Object *obj, const _Eo_Class *klass)
{
   if (EINA_LIKELY((klass->desc->data_size > 0) && (klass->desc->type != EO_CLASS_TYPE_MIXIN)))
     return ((char *) obj) + _eo_sz + klass->data_offset;

   if (EINA_UNLIKELY(klass->desc->data_size == 0))
     return NULL;
   else

     {
        Eo_Extension_Data_Offset *doff_itr = obj->klass->extn_data_off;

        if (!doff_itr)
          return NULL;

        while (doff_itr->klass)
          {
             if (doff_itr->klass == klass)
               return ((char *) obj) + _eo_sz + doff_itr->offset;
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
        data = _eo_data_scope_get(obj, klass);
        if (data == NULL) return NULL;
     }
   (obj->datarefcount)++;
#ifdef EO_DEBUG
   Eo_Xref_Node *xref = calloc(1, sizeof(*xref));
   xref->ref_obj = _eo_id_get(ref_obj);
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
   (void) data;
#endif
   if (obj->datarefcount == 0)
     {
        ERR("Data for object %lx (%s) is already not referenced.", (unsigned long) _eo_id_get(obj), obj->klass->desc->name);
     }
   else
     {
        (obj->datarefcount)--;
     }
#ifdef EO_DEBUG
   Eo_Xref_Node *xref = NULL;
   EINA_INLIST_FOREACH(obj->data_xrefs, xref)
     {
        if (xref->ref_obj == _eo_id_get(ref_obj))
          break;
     }

   if (xref)
     {
        obj->data_xrefs = eina_inlist_remove(obj->data_xrefs, EINA_INLIST_GET(xref));
        free(xref);
     }
   else
     {
        ERR("ref_obj (0x%lx) does not reference data (%p) of obj (0x%lx).", (unsigned long) _eo_id_get(ref_obj), data, (unsigned long)_eo_id_get(obj));
     }
#else
   (void) ref_obj;
#endif
}

EAPI void *
eo_data_get(const Eo *obj_id, const Eo_Class *klass_id)
{
   return eo_data_scope_get(obj_id, klass_id);
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

   ret = _eo_data_scope_get(obj, klass);

#ifdef EO_DEBUG
   if (!ret && (klass->desc->data_size == 0))
     {
        ERR("Tried getting data of class '%s', but it has none..", klass->desc->name);
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
        ERR("Tried getting data of class '%s', but it has none..", klass->desc->name);
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
        EINA_LOG_ERR("Could not register log domain: %s", log_dom);
        return EINA_FALSE;
     }

   if (!eina_spinlock_new(&_eo_class_creation_lock))
     {
        EINA_LOG_ERR("Could not init lock.");
        return EINA_FALSE;
     }

   eina_magic_string_static_set(EO_EINA_MAGIC, EO_EINA_MAGIC_STR);
   eina_magic_string_static_set(EO_FREED_EINA_MAGIC,
                                EO_FREED_EINA_MAGIC_STR);
   eina_magic_string_static_set(EO_CLASS_EINA_MAGIC,
                                EO_CLASS_EINA_MAGIC_STR);

#ifdef EO_DEBUG
   /* Call it just for coverage purposes. Ugly I know, but I like it better than
    * casting everywhere else. */
   _eo_class_isa_func(NULL, NULL, NULL);
#endif

   eina_log_timing(_eo_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   /* bootstrap EO_CLASS_CLASS */
   (void) eo_class_class_get();

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

   for (i = 0 ; i < _eo_classes_last_id ; i++, cls_itr++)
     {
        if (*cls_itr)
          eo_class_free(*cls_itr);
     }

   if (_eo_classes)
     free(_eo_classes);

   eina_spinlock_free(&_eo_class_creation_lock);

   _eo_free_ids_tables();

   eina_log_domain_unregister(_eo_log_dom);
   _eo_log_dom = -1;

   eina_shutdown();
   return EINA_TRUE;
}

EAPI Eina_Bool
eo_composite_attach(Eo *comp_obj_id, Eo *parent_id)
{
   const _Eo_Object **comp_itr;
   const _Eo_Object **comp_dst;

   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);
   EO_OBJ_POINTER_RETURN_VAL(parent_id, parent, EINA_FALSE);

   if (!parent->composites) return EINA_FALSE;
   if (comp_obj->klass->desc->type != EO_CLASS_TYPE_REGULAR) return EINA_FALSE;
   if (!eo_isa(parent_id, _eo_class_id_get(comp_obj->klass))) return EINA_FALSE;

   comp_dst = NULL;
   comp_itr = parent->composites;
   for (unsigned int i = 0; i < parent->klass->composites_count; i++, comp_itr++)
     {
        if (*comp_itr)
          {
             if ((*comp_itr)->klass == comp_obj->klass)
               return EINA_FALSE;
          }
        else if (!comp_dst)
          comp_dst = comp_itr;
     }

   if (!comp_dst)
     return EINA_FALSE;

   comp_obj->composite = EINA_TRUE;
   *comp_dst = comp_obj;
   eo_do(comp_obj_id, eo_parent_set(parent_id));

   return EINA_TRUE;
}

EAPI Eina_Bool
eo_composite_detach(Eo *comp_obj_id, Eo *parent_id)
{
   const _Eo_Object **comp_itr;

   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);
   EO_OBJ_POINTER_RETURN_VAL(parent_id, parent, EINA_FALSE);

   if (!parent->composites) return EINA_FALSE;

   comp_itr = parent->composites;
   for (unsigned int i = 0; i < parent->klass->composites_count; i++, comp_itr++)
     {
        if (*comp_itr == comp_obj)
          {
             comp_obj->composite = EINA_FALSE;
             *comp_itr = NULL;
             eo_do(comp_obj_id, eo_parent_set(NULL));
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
eo_composite_is(const Eo *comp_obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);

   return comp_obj->composite;
}

EAPI Eina_Bool
eo_destructed_is(const Eo *obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

   return obj->del;
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

   if (!obj->del)
     {
        ERR("Tried deleting the object %p while still referenced(%d).", obj_id, obj->refcount);
        return EINA_FALSE;
     }

   _eo_free(obj);

   return EINA_TRUE;
}

