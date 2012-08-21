#include <Eina.h>

#include "Eo.h"
#include "eo_private.h"

#include "config.h"

/* The last id that should be reserved for statically allocated classes. */
#define EO_CLASS_IDS_FIRST 1
#define EO_OP_IDS_FIRST 1

/* Used inside the class_get functions of classes, see #EO_DEFINE_CLASS */
EAPI Eina_Lock _eo_class_creation_lock;
int _eo_log_dom = -1;

static Eo_Class **_eo_classes;
static Eo_Class_Id _eo_classes_last_id;
static Eina_Bool _eo_init_count = 0;
static Eo_Op _eo_ops_last_id = 0;

static void _eo_condtor_reset(Eo *obj);
static inline void *_eo_data_get(const Eo *obj, const Eo_Class *klass);
static inline Eo *_eo_ref(Eo *obj);
static inline void _eo_unref(Eo *obj);
static const Eo_Class *_eo_op_class_get(Eo_Op op);
static const Eo_Op_Description *_eo_op_id_desc_get(Eo_Op op);

typedef struct
{
   const Eo_Class *kls;
} Eo_Kls_Itr;

struct _Eo {
     EINA_MAGIC
     EINA_INLIST;
     Eo *parent;
     Eina_Inlist *children;
     const Eo_Class *klass;
     int refcount;
#ifndef NDEBUG
     Eina_Inlist *xrefs;
#endif

     Eina_List *composite_objects;

     Eo_Kls_Itr mro_itr;

     Eina_Bool do_error:1;
     Eina_Bool condtor_done:1;

     Eina_Bool composite:1;
     Eina_Bool del:1;
     Eina_Bool manual_free:1;
};

/* Start of Dich */

/* How we search and store the implementations in classes. */
#define DICH_CHAIN_LAST_BITS 5
#define DICH_CHAIN_LAST_SIZE (1 << DICH_CHAIN_LAST_BITS)
#define DICH_CHAIN1(x) ((x) / DICH_CHAIN_LAST_SIZE)
#define DICH_CHAIN_LAST(x) ((x) % DICH_CHAIN_LAST_SIZE)

#define OP_CLASS_OFFSET_GET(x) (((x) >> EO_OP_CLASS_OFFSET) & 0xffff)

#define ID_CLASS_GET(id) ({ \
      (Eo_Class *) ((id <= _eo_classes_last_id) && (id > 0)) ? \
      (_eo_classes[id - 1]) : NULL; \
      })

#define EO_ALIGN_SIZE(size) \
        ((size) + (sizeof(void *) - ((size) % sizeof(void *))))

typedef struct _Dich_Chain1 Dich_Chain1;

typedef struct
{
   eo_op_func_type func;
   const Eo_Class *src;
} op_type_funcs;

struct _Dich_Chain1
{
   op_type_funcs *funcs;
};

typedef struct
{
   const Eo_Class *klass;
   size_t offset;
} Eo_Extension_Data_Offset;

struct _Eo_Class
{
   EINA_MAGIC
   Eo_Class_Id class_id;
   const Eo_Class *parent;
   const Eo_Class_Description *desc;
   Dich_Chain1 *chain; /**< The size is chain size */
   size_t chain_size;
   size_t base_id;

   const Eo_Class **extensions;

   Eo_Extension_Data_Offset *extn_data_off;
   size_t extn_data_size;

   const Eo_Class **mro;
   Eo_Kls_Itr mro_itr;

   size_t data_offset; /* < Offset of the data within object data. */

   Eina_Bool constructed : 1;
};

static inline void
_dich_chain_alloc(Dich_Chain1 *chain1)
{
   if (!chain1->funcs)
     {
        chain1->funcs = calloc(DICH_CHAIN_LAST_SIZE, sizeof(*(chain1->funcs)));
     }
}

static inline void
_dich_copy_all(Eo_Class *dst, const Eo_Class *src)
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
_dich_func_get(const Eo_Class *klass, Eo_Op op)
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
_dich_func_set(Eo_Class *klass, Eo_Op op, eo_op_func_type func)
{
   size_t idx1 = DICH_CHAIN1(op);
   Dich_Chain1 *chain1 = &klass->chain[idx1];
   _dich_chain_alloc(chain1);
   if (chain1->funcs[DICH_CHAIN_LAST(op)].src == klass)
     {
        const Eo_Class *op_kls = _eo_op_class_get(op);
        const Eo_Op_Description *op_desc = _eo_op_id_desc_get(op);
        ERR("Already set function for op %x (%s:%s). Overriding with func %p",
              op, op_kls->desc->name, op_desc->name, func);
     }

   chain1->funcs[DICH_CHAIN_LAST(op)].func = func;
   chain1->funcs[DICH_CHAIN_LAST(op)].src = klass;
}

static inline void
_dich_func_clean_all(Eo_Class *klass)
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

static const Eo_Class *
_eo_op_class_get(Eo_Op op)
{
   /* FIXME: Make it fast. */
   const Eo_Class *klass = NULL;
   Eo_Class **itr = _eo_classes;
   Eo_Class_Id i;
   for (i = 0 ; i < _eo_classes_last_id ; i++, itr++)
     {
        if (*itr && ((*itr)->base_id <= op) &&
              (op <= (*itr)->base_id + (*itr)->desc->ops.count))
          {
             klass = *itr;
             return klass;
          }
     }

   return klass;
}

static const Eo_Op_Description *
_eo_op_id_desc_get(Eo_Op op)
{
   const Eo_Class *klass;

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

static inline void
_eo_kls_itr_init(const Eo_Class *obj_klass, Eo_Kls_Itr *cur, Eo_Kls_Itr *prev_state)
{
   memcpy(prev_state, cur, sizeof(*cur));
   cur->kls = *obj_klass->mro;
}

static inline void
_eo_kls_itr_end(Eo_Kls_Itr *cur, Eo_Kls_Itr *prev_state)
{
   memcpy(cur, prev_state, sizeof(*cur));
}

static inline const Eo_Class *
_eo_kls_itr_get(Eo_Kls_Itr *cur)
{
   return cur->kls;
}

static inline void
_eo_kls_itr_set(Eo_Kls_Itr *cur, const Eo_Class *kls)
{
   cur->kls = kls;
}

static inline const Eo_Class *
_eo_kls_itr_next(const Eo_Class *orig_kls, Eo_Kls_Itr *cur, Eo_Kls_Itr *prev_state, Eo_Op op)
{
   const Eo_Class **kls_itr = NULL;
   memcpy(prev_state, cur, sizeof(*cur));

   /* Find the kls itr. */
   kls_itr = orig_kls->mro;
   while (*kls_itr && (*kls_itr != cur->kls))
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
             cur->kls = fsrc->src;
             return cur->kls;
          }
     }

   cur->kls = NULL;
   return NULL;
}

static inline const op_type_funcs *
_eo_kls_itr_func_get(Eo_Kls_Itr *mro_itr, Eo_Op op)
{
   const Eo_Class *klass = _eo_kls_itr_get(mro_itr);
   if (klass)
     {
        const op_type_funcs *func = _dich_func_get(klass, op);

        if (func && func->func)
          {
             _eo_kls_itr_set(mro_itr, func->src);
             return func;
          }
     }

   _eo_kls_itr_set(mro_itr, NULL);
   return NULL;
}

#define _EO_OP_ERR_NO_OP_PRINT(op, klass) \
   do \
      { \
         const Eo_Class *op_klass = _eo_op_class_get(op); \
         const char *_dom_name = (op_klass) ? op_klass->desc->name : NULL; \
         ERR("Can't find func for op %x (%s:%s) for class '%s'. Aborting.", \
               op, _dom_name, _eo_op_id_name_get(op), \
               (klass) ? klass->desc->name : NULL); \
      } \
   while (0)

static Eina_Bool
_eo_op_internal(Eo *obj, Eo_Op_Type op_type, Eo_Op op, va_list *p_list)
{
#ifndef NDEBUG
   const Eo_Op_Description *op_desc = _eo_op_id_desc_get(op);

   if (op_desc)
     {
        if (op_desc->op_type == EO_OP_TYPE_CLASS)
          {
             ERR("Tried calling a class op '%s' (%x) from a non-class context.", (op_desc) ? op_desc->name : NULL, op);
             return EINA_FALSE;
          }
     }
#endif

     {
        const op_type_funcs *func =
           _eo_kls_itr_func_get(&obj->mro_itr, op);
        if (func)
          {
             func->func(obj, _eo_data_get(obj, func->src), p_list);
             return EINA_TRUE;
          }
     }

   /* Try composite objects */
     {
        Eina_List *itr;
        Eo *emb_obj;
        EINA_LIST_FOREACH(obj->composite_objects, itr, emb_obj)
          {
             /* FIXME: Clean this up a bit. */
             Eo_Kls_Itr prev_state;
             _eo_kls_itr_init(emb_obj->klass, &emb_obj->mro_itr, &prev_state);
             if (_eo_op_internal(emb_obj, op_type, op, p_list))
               {
                  _eo_kls_itr_end(&emb_obj->mro_itr, &prev_state);
                  return EINA_TRUE;
               }
             _eo_kls_itr_end(&emb_obj->mro_itr, &prev_state);
          }
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
eo_do_internal(Eo *obj, Eo_Op_Type op_type, ...)
{
   Eina_Bool prev_error;
   Eina_Bool ret = EINA_TRUE;
   Eo_Op op = EO_NOOP;
   Eo_Kls_Itr prev_state;
   va_list p_list;

   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, EINA_FALSE);

   prev_error = obj->do_error;
   _eo_ref(obj);

   va_start(p_list, op_type);

   op = va_arg(p_list, Eo_Op);
   while (op)
     {
        _eo_kls_itr_init(obj->klass, &obj->mro_itr, &prev_state);
        if (!_eo_op_internal(obj, op_type, op, &p_list))
          {
             _EO_OP_ERR_NO_OP_PRINT(op, obj->klass);
             ret = EINA_FALSE;
             _eo_kls_itr_end(&obj->mro_itr, &prev_state);
             break;
          }
        op = va_arg(p_list, Eo_Op);
        _eo_kls_itr_end(&obj->mro_itr, &prev_state);
     }

   va_end(p_list);

   _eo_unref(obj);

   if (obj->do_error)
      ret = EINA_FALSE;

   obj->do_error = prev_error;

   return ret;
}

EAPI Eina_Bool
eo_do_super_internal(Eo *obj, Eo_Op_Type op_type, Eo_Op op, ...)
{
   const Eo_Class *nklass;
   Eina_Bool ret = EINA_TRUE;
   va_list p_list;
   Eo_Kls_Itr prev_state;
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, EINA_FALSE);

   /* Advance the kls itr. */
   nklass = _eo_kls_itr_next(obj->klass, &obj->mro_itr, &prev_state, op);

   va_start(p_list, op);
   if (!_eo_op_internal(obj, op_type, op, &p_list))
     {
        _EO_OP_ERR_NO_OP_PRINT(op, nklass);
        ret = EINA_FALSE;
     }
   va_end(p_list);

   if (obj->do_error)
      ret = EINA_FALSE;

   _eo_kls_itr_end(&obj->mro_itr, &prev_state);
   return ret;
}

static Eina_Bool
_eo_class_op_internal(Eo_Class *klass, Eo_Op op, va_list *p_list)
{
#ifndef NDEBUG
   const Eo_Op_Description *op_desc = _eo_op_id_desc_get(op);

   if (op_desc)
     {
        if (op_desc->op_type != EO_OP_TYPE_CLASS)
          {
             ERR("Tried calling an instance op '%s' (%x) from a class context.", (op_desc) ? op_desc->name : NULL, op);
             return EINA_FALSE;
          }
     }
#endif

     {
        const op_type_funcs *func =
           _eo_kls_itr_func_get(&klass->mro_itr, op);
        if (func)
          {
             ((eo_op_func_type_class) func->func)(klass, p_list);
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
eo_class_do_internal(const Eo_Class *klass, ...)
{
   Eina_Bool ret = EINA_TRUE;
   Eo_Op op = EO_NOOP;
   Eo_Kls_Itr prev_state;
   va_list p_list;

   EO_MAGIC_RETURN_VAL(klass, EO_CLASS_EINA_MAGIC, EINA_FALSE);

   va_start(p_list, klass);

   op = va_arg(p_list, Eo_Op);
   while (op)
     {
        _eo_kls_itr_init(klass, &((Eo_Class *) klass)->mro_itr, &prev_state);
        if (!_eo_class_op_internal((Eo_Class *) klass, op, &p_list))
          {
             _EO_OP_ERR_NO_OP_PRINT(op, klass);
             ret = EINA_FALSE;
             _eo_kls_itr_end(&((Eo_Class *) klass)->mro_itr, &prev_state);
             break;
          }
        _eo_kls_itr_end(&((Eo_Class *) klass)->mro_itr, &prev_state);
        op = va_arg(p_list, Eo_Op);
     }

   va_end(p_list);

   return ret;
}

EAPI Eina_Bool
eo_class_do_super_internal(const Eo_Class *klass, Eo_Op op, ...)
{
   const Eo_Class *nklass;
   Eina_Bool ret = EINA_TRUE;
   va_list p_list;
   Eo_Kls_Itr prev_state;
   EO_MAGIC_RETURN_VAL(klass, EO_CLASS_EINA_MAGIC, EINA_FALSE);

   /* Advance the kls itr. */
   nklass = _eo_kls_itr_next(klass, &((Eo_Class *) klass)->mro_itr, &prev_state, op);

   va_start(p_list, op);
   if (!_eo_class_op_internal((Eo_Class *) klass, op, &p_list))
     {
        _EO_OP_ERR_NO_OP_PRINT(op, nklass);
        ret = EINA_FALSE;
     }
   va_end(p_list);

   _eo_kls_itr_end(&((Eo_Class *) klass)->mro_itr, &prev_state);
   return ret;
}

EAPI const Eo_Class *
eo_class_get(const Eo *obj)
{
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, EINA_FALSE);

   return obj->klass;
}

EAPI const char *
eo_class_name_get(const Eo_Class *klass)
{
   EO_MAGIC_RETURN_VAL(klass, EO_CLASS_EINA_MAGIC, NULL);

   return klass->desc->name;
}

static void
_eo_class_base_op_init(Eo_Class *klass)
{
   const Eo_Class_Description *desc = klass->desc;

   klass->base_id = _eo_ops_last_id;

   if (desc && desc->ops.base_op_id)
      *(desc->ops.base_op_id) = klass->base_id;

   _eo_ops_last_id += desc->ops.count + 1;

   klass->chain_size = DICH_CHAIN1(_eo_ops_last_id) + 1;
   klass->chain = calloc(klass->chain_size, sizeof(*klass->chain));
}

#ifndef NDEBUG
static Eina_Bool
_eo_class_mro_has(const Eo_Class *klass, const Eo_Class *find)
{
   const Eo_Class **itr;
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
_eo_class_mro_add(Eina_List *mro, const Eo_Class *klass)
{
   Eina_List *extn_pos = NULL;
   Eina_Bool check_consistency = !mro;
   if (!klass)
      return mro;

   mro = eina_list_append(mro, klass);

   /* ONLY ADD MIXINS! */

   /* Recursively add extenions. */
     {
        const Eo_Class **extn_itr;

        for (extn_itr = klass->extensions ; *extn_itr ; extn_itr++)
          {
             const Eo_Class *extn = *extn_itr;
             if (extn->desc->type != EO_CLASS_TYPE_MIXIN)
                continue;

             mro = _eo_class_mro_add(mro, extn);
             /* Not possible: if (!mro) return NULL; */

             if (check_consistency)
               {
                  extn_pos = eina_list_append(extn_pos, eina_list_last(mro));
               }
          }
     }

   /* Check if we can create a consistent mro. We only do it for the class
    * we are working on (i.e no parents). */
   if (check_consistency)
     {
        const Eo_Class **extn_itr;

        Eina_List *itr = extn_pos;
        for (extn_itr = klass->extensions ; *extn_itr ; extn_itr++)
          {
             const Eo_Class *extn = *extn_itr;
             if (extn->desc->type != EO_CLASS_TYPE_MIXIN)
                continue;

             /* Get the first one after the extension. */
             Eina_List *extn_list = eina_list_next(eina_list_data_get(itr));

             /* If we found the extension again. */
             if (eina_list_data_find_list(extn_list, extn))
               {
                  eina_list_free(mro);
                  ERR("Cannot create a consistent method resolution order for class '%s' because of '%s'.", klass->desc->name, extn->desc->name);
                  return NULL;
               }

             itr = eina_list_next(itr);
          }
     }


   mro = _eo_class_mro_add(mro, klass->parent);

   return mro;
}

static Eina_Bool
_eo_class_mro_init(Eo_Class *klass)
{
   Eina_List *mro = NULL;

   DBG("Started creating MRO for class '%s'", klass->desc->name);
   mro = _eo_class_mro_add(mro, klass);

   if (!mro)
      return EINA_FALSE;

   /* Remove duplicates and make them the right order. */
     {
        Eina_List *itr1, *itr2, *itr2n;

        itr1 = eina_list_last(mro);
        while (itr1)
          {
             itr2 = eina_list_prev(itr1);

             while (itr2)
               {
                  itr2n = eina_list_prev(itr2);

                  if (eina_list_data_get(itr1) == eina_list_data_get(itr2))
                    {
                       mro = eina_list_remove_list(mro, itr2);
                    }

                  itr2 = itr2n;
               }

             itr1 = eina_list_prev(itr1);
          }
     }

   /* Copy the mro and free the list. */
     {
        const Eo_Class *kls_itr;
        const Eo_Class **mro_itr;
        klass->mro = calloc(sizeof(*klass->mro), eina_list_count(mro) + 1);

        mro_itr = klass->mro;

        EINA_LIST_FREE(mro, kls_itr)
          {
             *(mro_itr++) = kls_itr;

             DBG("Added '%s' to MRO", kls_itr->desc->name);
          }
        *(mro_itr) = NULL;
     }

   DBG("Finished creating MRO for class '%s'", klass->desc->name);

   return EINA_TRUE;
}

static void
_eo_class_constructor(Eo_Class *klass)
{
   if (klass->constructed)
      return;

   klass->constructed = EINA_TRUE;

   if (klass->desc->class_constructor)
      klass->desc->class_constructor(klass);
}

EAPI void
eo_class_funcs_set(Eo_Class *klass, const Eo_Op_Func_Description *func_descs)
{
   EO_MAGIC_RETURN(klass, EO_CLASS_EINA_MAGIC);

   const Eo_Op_Func_Description *itr;
   itr = func_descs;
   if (itr)
     {
        for ( ; itr->op_type != EO_OP_TYPE_INVALID ; itr++)
          {
             const Eo_Op_Description *op_desc = _eo_op_id_desc_get(itr->op);

             if (EINA_UNLIKELY(!op_desc || (itr->op == EO_NOOP)))
               {
                  ERR("Setting implementation for non-existent op %x for class '%s'. Func index: %d", itr->op, klass->desc->name, itr - func_descs);
               }
             else if (EINA_LIKELY(itr->op_type == op_desc->op_type))
               {
                  _dich_func_set(klass, itr->op, itr->func);
               }
             else
               {
                  ERR("Set function's op type (%x) is different than the one in the op description (%d) for op '%s:%s'. Func index: %d",
                        itr->op_type,
                        (op_desc) ? op_desc->op_type : EO_OP_TYPE_REGULAR,
                        klass->desc->name,
                        (op_desc) ? op_desc->name : NULL,
                        itr - func_descs);
               }
          }
     }
}

static void
eo_class_free(Eo_Class *klass)
{
   if (klass->constructed)
     {
        if (klass->desc->class_destructor)
           klass->desc->class_destructor(klass);

        _dich_func_clean_all(klass);
     }

   free(klass->extensions);

   if (klass->mro)
      free(klass->mro);

   if (klass->extn_data_off)
      free(klass->extn_data_off);

   free(klass);
}

/* DEVCHECK */
static Eina_Bool
_eo_class_check_op_descs(const Eo_Class *klass)
{
   const Eo_Class_Description *desc = klass->desc;
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
                  ERR("Wrong order in Ops description for class '%s'. Expected %x and got %x", desc->name, i, itr->sub_op);
               }
             else
               {
                  ERR("Found too few Ops description for class '%s'. Expected %x descriptions, but found %x.", desc->name, desc->ops.count, i);
               }
             return EINA_FALSE;
          }
     }

   if (itr && itr->name)
     {
        ERR("Found extra Ops description for class '%s'. Expected %d descriptions, but found more.", desc->name, desc->ops.count);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/* Not really called, just used for the ptr... */
static void
_eo_class_isa_func(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   /* Do nonthing. */
}

EAPI const Eo_Class *
eo_class_new(const Eo_Class_Description *desc, const Eo_Class *parent, ...)
{
   Eo_Class *klass;
   va_list p_list;

   if (parent && !EINA_MAGIC_CHECK(parent, EO_CLASS_EINA_MAGIC))
     {
        EINA_MAGIC_FAIL(parent, EO_CLASS_EINA_MAGIC);
        return NULL;
     }

   va_start(p_list, parent);

   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->name, NULL);

   /* Check restrictions on Interface types. */
   if (desc->type == EO_CLASS_TYPE_INTERFACE)
     {
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->data_size, NULL);
     }

   klass = calloc(1, sizeof(Eo_Class));
   klass->parent = parent;

   /* Handle class extensions */
     {
        Eina_List *extn_list = NULL;
        const Eo_Class *extn = NULL;
        const Eo_Class **extn_itr = NULL;

        extn = va_arg(p_list, Eo_Class *);
        while (extn)
          {
             switch (extn->desc->type)
               {
                case EO_CLASS_TYPE_REGULAR:
                case EO_CLASS_TYPE_REGULAR_NO_INSTANT:
                case EO_CLASS_TYPE_INTERFACE:
                case EO_CLASS_TYPE_MIXIN:
                   extn_list = eina_list_append(extn_list, extn);
                   break;
               }

             extn = va_arg(p_list, Eo_Class *);
          }

        klass->extensions = calloc(sizeof(*klass->extensions),
              eina_list_count(extn_list) + 1);

        extn_itr = klass->extensions;
        EINA_LIST_FREE(extn_list, extn)
          {
             *(extn_itr++) = extn;
          }
     }

   klass->desc = desc;

   /* Handle the inheritance */
   if (klass->parent)
     {
        /* Verify the inheritance is allowed. */
        switch (klass->desc->type)
          {
           case EO_CLASS_TYPE_REGULAR:
           case EO_CLASS_TYPE_REGULAR_NO_INSTANT:
              if ((klass->parent->desc->type != EO_CLASS_TYPE_REGULAR) &&
                    (klass->parent->desc->type != EO_CLASS_TYPE_REGULAR_NO_INSTANT))
                {
                   ERR("Regular classes ('%s') aren't allowed to inherit from non-regular classes ('%s').", klass->desc->name, klass->parent->desc->name);
                   goto cleanup;
                }
              break;
           case EO_CLASS_TYPE_INTERFACE:
           case EO_CLASS_TYPE_MIXIN:
              if ((klass->parent->desc->type != EO_CLASS_TYPE_INTERFACE) &&
                    (klass->parent->desc->type != EO_CLASS_TYPE_MIXIN))
                {
                   ERR("Non-regular classes ('%s') aren't allowed to inherit from regular classes ('%s').", klass->desc->name, klass->parent->desc->name);
                   goto cleanup;
                }
              break;
          }


        /* Update the current offset. */
        /* FIXME: Make sure this alignment is enough. */
        klass->data_offset = klass->parent->data_offset +
           EO_ALIGN_SIZE(klass->parent->desc->data_size);
     }

   if (!_eo_class_check_op_descs(klass))
     {
        goto cleanup;
     }

   if (!_eo_class_mro_init(klass))
     {
        goto cleanup;
     }

   /* create MIXIN offset table. */
     {
        const Eo_Class **mro_itr = klass->mro;
        Eo_Extension_Data_Offset *extn_data_itr;
        size_t extn_num = 0;
        size_t extn_data_off = klass->data_offset +
           EO_ALIGN_SIZE(klass->desc->data_size);

        /* FIXME: Make faster... */
        while (*mro_itr)
          {
             if (((*mro_itr)->desc->type == EO_CLASS_TYPE_MIXIN) &&
                   ((*mro_itr)->desc->data_size > 0))
               {
                  extn_num++;
               }
             mro_itr++;
          }

        klass->extn_data_off = calloc(extn_num + 1,
              sizeof(*klass->extn_data_off));

        extn_data_itr = klass->extn_data_off;
        mro_itr = klass->mro;
        while (*mro_itr)
          {
             if (((*mro_itr)->desc->type == EO_CLASS_TYPE_MIXIN) &&
                   ((*mro_itr)->desc->data_size > 0))
               {
                  extn_data_itr->klass = *mro_itr;
                  extn_data_itr->offset = extn_data_off;

                  extn_data_off += EO_ALIGN_SIZE(extn_data_itr->klass->desc->data_size);
                  extn_data_itr++;
               }
             mro_itr++;
          }

        klass->extn_data_size = extn_data_off;
     }

   eina_lock_take(&_eo_class_creation_lock);

   klass->class_id = ++_eo_classes_last_id;

     {
        /* FIXME: Handle errors. */
        size_t arrsize = _eo_classes_last_id * sizeof(*_eo_classes);
        Eo_Class **tmp;
        tmp = realloc(_eo_classes, arrsize);

        /* If it's the first allocation, memset. */
        if (!_eo_classes)
           memset(tmp, 0, arrsize);

        _eo_classes = tmp;
        _eo_classes[klass->class_id - 1] = klass;
     }
   eina_lock_release(&_eo_class_creation_lock);

   EINA_MAGIC_SET(klass, EO_CLASS_EINA_MAGIC);

   _eo_class_base_op_init(klass);
   /* Flatten the function array */
     {
        const Eo_Class **mro_itr = klass->mro;
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
        const Eo_Class **extn_itr;

        for (extn_itr = klass->extensions ; *extn_itr ; extn_itr++)
          {
             const Eo_Class *extn = *extn_itr;
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

   _eo_class_constructor(klass);

   va_end(p_list);

   return klass;

cleanup:
   eo_class_free(klass);
   return NULL;
}

EAPI Eina_Bool
eo_isa(const Eo *obj, const Eo_Class *klass)
{
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, EINA_FALSE);
   EO_MAGIC_RETURN_VAL(klass, EO_CLASS_EINA_MAGIC, EINA_FALSE);
   const op_type_funcs *func = _dich_func_get(obj->klass,
         klass->base_id + klass->desc->ops.count);

   /* Currently implemented by reusing the LAST op id. Just marking it with
    * _eo_class_isa_func. */
   return (func && (func->func == _eo_class_isa_func));
}

EAPI Eina_Bool
eo_parent_set(Eo *obj, const Eo *parent)
{
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, EINA_FALSE);
   if (parent)
      EO_MAGIC_RETURN_VAL(parent, EO_EINA_MAGIC, EINA_FALSE);

   if (obj->parent == parent)
      return EINA_TRUE;

   _eo_ref(obj);

   if (eo_composite_is(obj))
     {
        eo_composite_detach(obj, obj->parent);
     }

   if (obj->parent)
     {
        obj->parent->children =
           eina_inlist_remove(obj->parent->children, EINA_INLIST_GET(obj));
        eo_xunref(obj, obj->parent);
     }

   obj->parent = (Eo *) parent;
   if (obj->parent)
     {
        obj->parent->children =
           eina_inlist_append(obj->parent->children, EINA_INLIST_GET(obj));
        eo_xref(obj, obj->parent);
     }

   _eo_unref(obj);

   return EINA_TRUE;
}

EAPI Eo *
eo_add(const Eo_Class *klass, Eo *parent)
{
   Eina_Bool do_err;
   EO_MAGIC_RETURN_VAL(klass, EO_CLASS_EINA_MAGIC, NULL);

   if (parent) EO_MAGIC_RETURN_VAL(parent, EO_EINA_MAGIC, NULL);

   if (EINA_UNLIKELY(klass->desc->type != EO_CLASS_TYPE_REGULAR))
     {
        ERR("Class '%s' is not instantiate-able. Aborting.", klass->desc->name);
        return NULL;
     }

   Eo *obj = calloc(1, EO_ALIGN_SIZE(sizeof(*obj)) +
         (klass->data_offset + EO_ALIGN_SIZE(klass->desc->data_size)) +
         klass->extn_data_size);
   EINA_MAGIC_SET(obj, EO_EINA_MAGIC);
   obj->refcount++;
   obj->klass = klass;

   eo_parent_set(obj, parent);

   _eo_condtor_reset(obj);

   _eo_ref(obj);
   do_err = !eo_do(obj, eo_constructor());

   if (EINA_UNLIKELY(do_err))
     {
        ERR("Object of class '%s' - One of the object constructors have failed.", klass->desc->name);
        goto fail;
     }

   if (!obj->condtor_done)
     {
        ERR("Object of class '%s' - Not all of the object constructors have been executed.", klass->desc->name);
        goto fail;
     }
   _eo_unref(obj);

   return obj;

fail:
   /* Unref twice, once for the ref above, and once for the basic object ref. */
   _eo_unref(obj);
   _eo_unref(obj);
   return NULL;
}

typedef struct
{
   EINA_INLIST;
   const Eo *ref_obj;
   const char *file;
   int line;
} Eo_Xref_Node;

EAPI Eo *
eo_xref_internal(Eo *obj, const Eo *ref_obj, const char *file, int line)
{
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, obj);

   _eo_ref(obj);

#ifndef NDEBUG
   Eo_Xref_Node *xref = calloc(1, sizeof(*xref));
   xref->ref_obj = ref_obj;
   xref->file = file;
   xref->line = line;

   obj->xrefs = eina_inlist_prepend(obj->xrefs, EINA_INLIST_GET(xref));
#else
   (void) ref_obj;
   (void) file;
   (void) line;
#endif

   return obj;
}

EAPI void
eo_xunref(Eo *obj, const Eo *ref_obj)
{
   EO_MAGIC_RETURN(obj, EO_EINA_MAGIC);
#ifndef NDEBUG
   Eo_Xref_Node *xref = NULL;
   EINA_INLIST_FOREACH(obj->xrefs, xref)
     {
        if (xref->ref_obj == ref_obj)
           break;
     }

   if (xref)
     {
        obj->xrefs = eina_inlist_remove(obj->xrefs, EINA_INLIST_GET(xref));
        free(xref);
     }
   else
     {
        ERR("ref_obj (%p) does not reference obj (%p). Aborting unref.", ref_obj, obj);
        return;
     }
#else
   (void) ref_obj;
#endif
   _eo_unref(obj);
}

static inline Eo *
_eo_ref(Eo *obj)
{
   obj->refcount++;
   return obj;
}

EAPI Eo *
eo_ref(const Eo *_obj)
{
   Eo *obj = (Eo *) _obj;
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, obj);

   return _eo_ref(obj);
}

static inline void
_eo_del_internal(Eo *obj)
{
   Eina_Bool do_err;
   /* We need that for the event callbacks that may ref/unref. */
   obj->refcount++;

   eo_do(obj, eo_event_callback_call(EO_EV_DEL, NULL, NULL));

   const Eo_Class *klass = eo_class_get(obj);

   _eo_condtor_reset(obj);

   do_err = eo_do(obj, eo_destructor());
   if (EINA_UNLIKELY(!do_err))
     {
        ERR("Object of class '%s' - One of the object destructors have failed.", klass->desc->name);
     }

   if (!obj->condtor_done)
     {
        ERR("Object of class '%s' - Not all of the object destructors have been executed.", klass->desc->name);
     }
   /*FIXME: add eo_class_unref(klass) ? - just to clear the caches. */

     {
        Eina_List *itr, *itr_n;
        Eo *emb_obj;
        EINA_LIST_FOREACH_SAFE(obj->composite_objects, itr, itr_n, emb_obj)
          {
             eo_composite_detach(emb_obj, obj);
          }
     }

   while (obj->children)
     {
        eo_parent_set(EINA_INLIST_CONTAINER_GET(obj->children, Eo), NULL);
     }

   obj->del = EINA_TRUE;
   obj->refcount--;
}

static inline void
_eo_free(Eo *obj)
{
   EINA_MAGIC_SET(obj, EO_FREED_EINA_MAGIC);
   free(obj);
}

static inline void
_eo_unref(Eo *obj)
{
   --(obj->refcount);
   if (obj->refcount == 0)
     {
        if (obj->del)
          {
             ERR("Object %p already deleted.", obj);
             return;
          }

        _eo_del_internal(obj);

#ifndef NDEBUG
        /* If for some reason it's not empty, clear it. */
        while (obj->xrefs)
          {
             ERR("obj->xrefs is not empty, possibly a bug, please report. - An error will be reported for each xref in the stack.");
             Eina_Inlist *nitr = obj->xrefs->next;
             free(EINA_INLIST_CONTAINER_GET(obj->xrefs, Eo_Xref_Node));
             obj->xrefs = nitr;
          }
#endif

        if (!obj->manual_free)
           _eo_free(obj);
        else
           _eo_ref(obj); /* If we manual free, we keep a phantom ref. */
     }
   else if (obj->refcount < 0)
     {
        ERR("Obj:%p. Refcount (%d) < 0. Too many unrefs.", obj, obj->refcount);
        return;
     }
}

EAPI void
eo_unref(const Eo *_obj)
{
   Eo *obj = (Eo *) _obj;
   EO_MAGIC_RETURN(obj, EO_EINA_MAGIC);

   _eo_unref(obj);
}

EAPI void
eo_del(const Eo *obj)
{
   eo_parent_set((Eo *) obj, NULL);
   eo_unref(obj);
}

EAPI int
eo_ref_get(const Eo *obj)
{
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, 0);

   return obj->refcount;
}

EAPI Eo *
eo_parent_get(const Eo *obj)
{
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, NULL);

   return obj->parent;
}

EAPI void
eo_error_set_internal(const Eo *obj, const char *file, int line)
{
   EO_MAGIC_RETURN(obj, EO_EINA_MAGIC);

   ERR("Error with obj '%p' at %s:%d", obj, file, line);

   ((Eo *) obj)->do_error = EINA_TRUE;
}

void
_eo_condtor_done(Eo *obj)
{
   if (obj->condtor_done)
     {
        ERR("Object %p is already constructed at this point.", obj);
        return;
     }

   obj->condtor_done = EINA_TRUE;
}

static void
_eo_condtor_reset(Eo *obj)
{
   obj->condtor_done = EINA_FALSE;
}

static inline void *
_eo_data_get(const Eo *obj, const Eo_Class *klass)
{
   if (EINA_LIKELY(klass->desc->data_size > 0))
     {
        if (EINA_UNLIKELY(klass->desc->type == EO_CLASS_TYPE_MIXIN))
          {
             Eo_Extension_Data_Offset *doff_itr =
                eo_class_get(obj)->extn_data_off;

             if (!doff_itr)
                return NULL;

             while (doff_itr->klass)
               {
                  if (doff_itr->klass == klass)
                     return ((char *) obj) + EO_ALIGN_SIZE(sizeof(*obj)) +
                        doff_itr->offset;
                  doff_itr++;
               }
          }
        else
          {
             return ((char *) obj) + EO_ALIGN_SIZE(sizeof(*obj)) +
                klass->data_offset;
          }
     }

   return NULL;
}

EAPI void *
eo_data_get(const Eo *obj, const Eo_Class *klass)
{
   void *ret;
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, NULL);
   EO_MAGIC_RETURN_VAL(klass, EO_CLASS_EINA_MAGIC, NULL);

#ifndef NDEBUG
   if (!_eo_class_mro_has(obj->klass, klass))
     {
        ERR("Tried getting data of class '%s' from object of class '%s', but the former is not a direct inheritance of the latter.", klass->desc->name, obj->klass->desc->name);
        return NULL;
     }
#endif

   ret = _eo_data_get(obj, klass);

#ifndef NDEBUG
   if (!ret && (klass->desc->data_size == 0))
     {
        ERR("Tried getting data of class '%s', but it has none..", klass->desc->name);
     }
#endif

   return ret;
}

EAPI Eina_Bool
eo_init(void)
{
   const char *log_dom = "eo";
   if (_eo_init_count++ > 0)
      return EINA_TRUE;

   eina_init();

   _eo_classes = NULL;
   _eo_classes_last_id = EO_CLASS_IDS_FIRST - 1;
   _eo_ops_last_id = EO_OP_IDS_FIRST;
   _eo_log_dom = eina_log_domain_register(log_dom, EINA_COLOR_LIGHTBLUE);
   if (_eo_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", log_dom);
        return EINA_FALSE;
     }

   if (!eina_lock_new(&_eo_class_creation_lock))
     {
        EINA_LOG_ERR("Could not init lock.");
        return EINA_FALSE;
     }

   eina_magic_string_static_set(EO_EINA_MAGIC, EO_EINA_MAGIC_STR);
   eina_magic_string_static_set(EO_FREED_EINA_MAGIC,
         EO_FREED_EINA_MAGIC_STR);
   eina_magic_string_static_set(EO_CLASS_EINA_MAGIC,
         EO_CLASS_EINA_MAGIC_STR);

#ifndef NDEBUG
   /* Call it just for coverage purposes. Ugly I know, but I like it better than
    * casting everywhere else. */
   _eo_class_isa_func(NULL, NULL, NULL);
#endif

   return EINA_TRUE;
}

EAPI Eina_Bool
eo_shutdown(void)
{
   size_t i;
   Eo_Class **cls_itr = _eo_classes;

   if (--_eo_init_count > 0)
      return EINA_TRUE;

   for (i = 0 ; i < _eo_classes_last_id ; i++, cls_itr++)
     {
        if (*cls_itr)
           eo_class_free(*cls_itr);
     }

   if (_eo_classes)
      free(_eo_classes);

   eina_lock_free(&_eo_class_creation_lock);

   eina_log_domain_unregister(_eo_log_dom);
   _eo_log_dom = -1;

   eina_shutdown();
   return EINA_TRUE;
}

EAPI void
eo_composite_attach(Eo *comp_obj, Eo *parent)
{
   EO_MAGIC_RETURN(comp_obj, EO_EINA_MAGIC);
   EO_MAGIC_RETURN(parent, EO_EINA_MAGIC);

   comp_obj->composite = EINA_TRUE;
   eo_parent_set(comp_obj, parent);
   parent->composite_objects = eina_list_prepend(parent->composite_objects, comp_obj);
}

EAPI void
eo_composite_detach(Eo *comp_obj, Eo *parent)
{
   EO_MAGIC_RETURN(comp_obj, EO_EINA_MAGIC);
   EO_MAGIC_RETURN(parent, EO_EINA_MAGIC);

   comp_obj->composite = EINA_FALSE;
   parent->composite_objects = eina_list_remove(parent->composite_objects, comp_obj);
   eo_parent_set(comp_obj, NULL);
}

EAPI Eina_Bool
eo_composite_is(const Eo *comp_obj)
{
   if (!EINA_MAGIC_CHECK(comp_obj, EO_EINA_MAGIC))
     {
        EINA_MAGIC_FAIL(comp_obj, EO_EINA_MAGIC);
        return EINA_FALSE;
     }

   return comp_obj->composite;
}

EAPI void
eo_manual_free_set(Eo *obj, Eina_Bool manual_free)
{
   EO_MAGIC_RETURN(obj, EO_EINA_MAGIC);
   obj->manual_free = manual_free;
}

EAPI void
eo_manual_free(Eo *obj)
{
   EO_MAGIC_RETURN(obj, EO_EINA_MAGIC);

   if (EINA_FALSE == obj->manual_free)
     {
        ERR("Tried to manually free the object %p while the option has not been set; see eo_manual_free_set for more information.", obj);
        return;
     }

   if (!obj->del)
     {
        ERR("Tried deleting the object %p while still referenced(%d).", obj, eo_ref_get(obj));
        return;
     }

   _eo_free(obj);
}

