#include <Eina.h>

#include "Eo.h"
#include "eo_private.h"

#include "config.h"

/* The last id that should be reserved for statically allocated classes. */
#define EO_STATIC_IDS_LAST 10

/* Used inside the class_get functions of classes, see #EO_DEFINE_CLASS */
EAPI Eina_Lock _eo_class_creation_lock;
int _eo_log_dom = -1;

static Eo_Class **_eo_classes;
static Eo_Class_Id _eo_classes_last_id;
static Eina_Bool _eo_init_count = 0;

static void _eo_constructor(Eo *obj, const Eo_Class *klass);
static void _eo_destructor(Eo *obj, const Eo_Class *klass);
static inline Eina_Bool _eo_error_get(const Eo *obj);
static inline void _eo_error_unset(Eo *obj);
static inline void *_eo_data_get(const Eo *obj, const Eo_Class *klass);
static inline Eo *_eo_ref(Eo *obj);
static inline void _eo_unref(Eo *obj);

typedef struct
{
   Eo_Op op;
   const Eo_Class **kls_itr;
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

     Eina_Bool composite:1;
     Eina_Bool del:1;
     Eina_Bool construct_error:1;
     Eina_Bool manual_free:1;
};

/* Start of Dich */
/* Dich search, split to 0xff 0xff 0xffff */

#define DICH_CHAIN1_MASK (0xffff)
#define DICH_CHAIN_LAST_MASK (0xffff)
#define DICH_CHAIN1(x) (((x) >> 16) & DICH_CHAIN1_MASK)
#define DICH_CHAIN_LAST(x) ((x) & DICH_CHAIN_LAST_MASK)

#define OP_CLASS_OFFSET_GET(x) (((x) >> EO_OP_CLASS_OFFSET) & 0xffff)
#define OP_CLASS_GET(op) ({ \
      Eo_Class_Id tmp = OP_CLASS_OFFSET_GET(op); \
      ID_CLASS_GET(tmp); \
      })
#define OP_SUB_ID_GET(op) ((op) & 0xffff)

#define ID_CLASS_GET(id) ({ \
      (Eo_Class *) ((id <= _eo_classes_last_id) && (id > 0)) ? \
      (_eo_classes[id - 1]) : NULL; \
      })

#define EO_ALIGN_SIZE(size) \
        ((size) + (sizeof(void *) - ((size) % sizeof(void *))))

/* Structure of Eo_Op is:
 * 16bit: class
 * 16bit: op.
 */

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
     EINA_INLIST;
     const Eo_Class *klass;
} Eo_Extension_Node;

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
   Dich_Chain1 *chain; /**< The size is class_id */
   Eina_Inlist *extensions;

   Eo_Extension_Data_Offset *extn_data_off;
   size_t extn_data_size;

   const Eo_Class **mro;
   Eo_Kls_Itr mro_itr;

   size_t data_offset; /* < Offset of the data within object data. */

   Eina_Bool constructed : 1;
};

static inline void
_dich_chain_alloc(Dich_Chain1 *chain1, size_t num_ops)
{
   if (!chain1->funcs)
     {
        chain1->funcs = calloc(num_ops, sizeof(*(chain1->funcs)));
     }
}

static inline void
_dich_copy_all(Eo_Class *dst, const Eo_Class *src)
{
   if (!src->chain) return;

   if (!dst->chain)
     {
        dst->chain = calloc(dst->class_id, sizeof(*dst->chain));
     }

   Eo_Class_Id i;
   const Dich_Chain1 *sc1 = src->chain;
   Dich_Chain1 *dc1 = dst->chain;
   for (i = 0 ; i < src->class_id ; i++, sc1++, dc1++)
     {
        if (sc1->funcs)
          {
             size_t j;
             const Eo_Class *op_klass = ID_CLASS_GET(i + 1);
             /* Can be NULL because of future static classes. */
             if (!op_klass)
                continue;

             size_t num_ops = op_klass->desc->ops.count;
             _dich_chain_alloc(dc1, num_ops);

             const op_type_funcs *sf = sc1->funcs;
             op_type_funcs *df = dc1->funcs;
             for (j = 0 ; j < num_ops ; j++, df++, sf++)
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
   if (!klass->chain) return NULL;

   size_t idx1 = DICH_CHAIN1(op) - 1;
   if (idx1 >= klass->class_id) return NULL;
   const Dich_Chain1 *chain1 = &klass->chain[idx1];
   if (!chain1->funcs) return NULL;

   size_t idxl = DICH_CHAIN_LAST(op);
   /* num_ops is calculated from the class. */
   const Eo_Class *op_klass = ID_CLASS_GET(idx1 + 1);
   if (!op_klass || (idxl >= op_klass->desc->ops.count))
      return NULL;

   return &chain1->funcs[idxl];
}

static inline void
_dich_func_set(Eo_Class *klass, Eo_Op op, eo_op_func_type func)
{
   const Eo_Class *op_klass = OP_CLASS_GET(op);
   size_t num_ops;

   /* Verify op is valid. */
   if (op_klass)
     {
        /* num_ops is calculated from the class. */
        num_ops = op_klass->desc->ops.count;
        if (DICH_CHAIN_LAST(op) >= num_ops)
          {
             ERR("OP %x is too big for the domain '%s', expected value < %x.",
                   op, op_klass->desc->name, op_klass->desc->ops.count);
             return;
          }
     }
   else
     {
        ERR("OP %x is from an illegal class.", op);
        return;
     }

   if (!klass->chain)
     {
        klass->chain = calloc(klass->class_id, sizeof(*klass->chain));
     }

   size_t idx1 = DICH_CHAIN1(op) - 1;
   Dich_Chain1 *chain1 = &klass->chain[idx1];
   _dich_chain_alloc(chain1, num_ops);
   chain1->funcs[DICH_CHAIN_LAST(op)].func = func;
   chain1->funcs[DICH_CHAIN_LAST(op)].src = klass;
}

static inline void
_dich_func_clean_all(Eo_Class *klass)
{
   size_t i;
   Dich_Chain1 *chain1 = klass->chain;

   if (!chain1)
      return;

   for (i = 0 ; i < klass->class_id ; i++, chain1++)
     {
        if (chain1->funcs)
           free(chain1->funcs);
     }
   free(klass->chain);
   klass->chain = NULL;
}

/* END OF DICH */

static const Eo_Op_Description noop_desc =
        EO_OP_DESCRIPTION(EO_NOOP, "", "No operation.");

static const Eo_Op_Description *
_eo_op_id_desc_get(Eo_Op op)
{
   const Eo_Class *klass = OP_CLASS_GET(op);
   Eo_Op sub_id = OP_SUB_ID_GET(op);

   if (op == EO_NOOP)
      return &noop_desc;

   if (klass && (sub_id < klass->desc->ops.count))
      return klass->desc->ops.descs + sub_id;

   return NULL;
}

static const char *
_eo_op_id_name_get(Eo_Op op)
{
   const Eo_Op_Description *desc = _eo_op_id_desc_get(op);
   return (desc) ? desc->name : NULL;
}

static inline void
_eo_kls_itr_init(const Eo_Class *obj_klass, Eo_Kls_Itr *cur, Eo_Op op, Eo_Kls_Itr *prev_state)
{
   prev_state->op = cur->op;
   prev_state->kls_itr = cur->kls_itr;

   /* If we are in a constructor/destructor or we changed an op - init. */
   if ((op == EO_NOOP) || (cur->op != op))
     {
        cur->op = op;
        cur->kls_itr = obj_klass->mro;
     }
}

static inline void
_eo_kls_itr_end(Eo_Kls_Itr *cur, Eo_Kls_Itr *prev_state)
{
   if (cur->op != prev_state->op)
     {
        cur->op = prev_state->op;
        cur->kls_itr = prev_state->kls_itr;
     }
}

static inline const Eo_Class *
_eo_kls_itr_get(Eo_Kls_Itr *cur)
{
   return (cur->kls_itr) ? *(cur->kls_itr) : NULL;
}

static inline const Eo_Class *
_eo_kls_itr_next(Eo_Kls_Itr *cur, Eo_Op op)
{
   if (cur->op != op)
     {
        Eo_Op node_op = cur->op;
        ERR("Called with op %d ('%s') while expecting: %d ('%s'). This probaly means you called eo_*_super functions from a wrong place.",
              op, _eo_op_id_name_get(op),
              node_op, _eo_op_id_name_get(node_op));
        return NULL;
     }

   const Eo_Class **kls_itr = cur->kls_itr;
   if (*kls_itr)
     {
        if (op != EO_NOOP)
          {
             const op_type_funcs *fsrc = _dich_func_get(*kls_itr, op);

             while (*kls_itr && (*(kls_itr++) != fsrc->src))
                ;
          }
        else
          {
             kls_itr++;
          }

        cur->kls_itr = kls_itr;
        return *kls_itr;
     }
   else
     {
        return NULL;
     }
}

static inline Eina_Bool
_eo_kls_itr_reached_end(const Eo_Kls_Itr *cur)
{
   const Eo_Class **kls_itr = cur->kls_itr;
   return !(*kls_itr && *(kls_itr + 1));
}

static inline const op_type_funcs *
_eo_kls_itr_func_get(const Eo_Class *klass, Eo_Kls_Itr *mro_itr, Eo_Op op, Eo_Kls_Itr *prev_state)
{
   _eo_kls_itr_init(klass, mro_itr, op, prev_state);
   klass = _eo_kls_itr_get(mro_itr);
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

#define _EO_OP_ERR_NO_OP_PRINT(op, klass) \
   do \
      { \
         const Eo_Class *op_klass = OP_CLASS_GET(op); \
         const char *_dom_name = (op_klass) ? op_klass->desc->name : NULL; \
         ERR("Can't find func for op %x ('%s' of domain '%s') for class '%s'. Aborting.", \
               op, _eo_op_id_name_get(op), _dom_name, \
               (klass) ? klass->desc->name : NULL); \
      } \
   while (0)

static Eina_Bool
_eo_op_internal(Eo *obj, Eo_Op_Type op_type, Eo_Op op, va_list *p_list)
{
   Eina_Bool ret = EINA_FALSE;

#ifndef NDEBUG
   const Eo_Op_Description *op_desc = _eo_op_id_desc_get(op);

   if (op_desc)
     {
        if (op_desc->op_type == EO_OP_TYPE_CLASS)
          {
             ERR("Tried calling a class op '%s' (%d) from a non-class context.", (op_desc) ? op_desc->name : NULL, op);
             return EINA_FALSE;
          }
        else if ((op_type == EO_OP_TYPE_CONST) &&
              (op_desc->op_type != EO_OP_TYPE_CONST))
          {
             ERR("Tried calling non-const or non-existant op '%s' (%d) from a const (query) function.", (op_desc) ? op_desc->name : NULL, op);
             return EINA_FALSE;
          }
     }
#endif

   Eo_Kls_Itr prev_state;

     {
        const op_type_funcs *func =
           _eo_kls_itr_func_get(obj->klass, &obj->mro_itr, op, &prev_state);
        if (func)
          {
             func->func(obj, _eo_data_get(obj, func->src), p_list);
             ret = EINA_TRUE;
             goto end;
          }
     }

   /* Try composite objects */
     {
        Eina_List *itr;
        Eo *emb_obj;
        EINA_LIST_FOREACH(obj->composite_objects, itr, emb_obj)
          {
             if (_eo_op_internal(emb_obj, op_type, op, p_list))
               {
                  ret = EINA_TRUE;
                  goto end;
               }
          }
     }

end:
   _eo_kls_itr_end(&obj->mro_itr, &prev_state);
   return ret;
}

EAPI Eina_Bool
eo_do_internal(Eo *obj, Eo_Op_Type op_type, ...)
{
   Eina_Bool ret = EINA_TRUE;
   Eo_Op op = EO_NOOP;
   va_list p_list;

   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, EINA_FALSE);

   _eo_ref(obj);

   va_start(p_list, op_type);

   op = va_arg(p_list, Eo_Op);
   while (op)
     {
        if (!_eo_op_internal(obj, op_type, op, &p_list))
          {
             _EO_OP_ERR_NO_OP_PRINT(op, obj->klass);
             ret = EINA_FALSE;
             break;
          }
        op = va_arg(p_list, Eo_Op);
     }

   va_end(p_list);

   _eo_unref(obj);
   return ret;
}

EAPI Eina_Bool
eo_do_super_internal(Eo *obj, Eo_Op_Type op_type, Eo_Op op, ...)
{
   const Eo_Class *nklass;
   Eina_Bool ret = EINA_TRUE;
   va_list p_list;
   EO_MAGIC_RETURN_VAL(obj, EO_EINA_MAGIC, EINA_FALSE);

   /* Advance the kls itr. */
   nklass = _eo_kls_itr_next(&obj->mro_itr, op);

   if (obj->mro_itr.op != op)
      return EINA_FALSE;

   va_start(p_list, op);
   if (!_eo_op_internal(obj, op_type, op, &p_list))
     {
        _EO_OP_ERR_NO_OP_PRINT(op, nklass);
        ret = EINA_FALSE;
     }
   va_end(p_list);

   return ret;
}

static Eina_Bool
_eo_class_op_internal(Eo_Class *klass, Eo_Op op, va_list *p_list)
{
   Eina_Bool ret = EINA_FALSE;

#ifndef NDEBUG
   const Eo_Op_Description *op_desc = _eo_op_id_desc_get(op);

   if (op_desc)
     {
        if (op_desc->op_type != EO_OP_TYPE_CLASS)
          {
             ERR("Tried calling an instant op '%s' (%d) from a class context.", (op_desc) ? op_desc->name : NULL, op);
             return EINA_FALSE;
          }
     }
#endif

   Eo_Kls_Itr prev_state;

     {
        const op_type_funcs *func =
           _eo_kls_itr_func_get(klass, &klass->mro_itr, op, &prev_state);
        if (func)
          {
             ((eo_op_func_type_class) func->func)(klass, p_list);
             ret = EINA_TRUE;
             goto end;
          }
     }

end:
   _eo_kls_itr_end(&klass->mro_itr, &prev_state);
   return ret;
}

EAPI Eina_Bool
eo_class_do_internal(const Eo_Class *klass, ...)
{
   Eina_Bool ret = EINA_TRUE;
   Eo_Op op = EO_NOOP;
   va_list p_list;

   EO_MAGIC_RETURN_VAL(klass, EO_CLASS_EINA_MAGIC, EINA_FALSE);

   va_start(p_list, klass);

   op = va_arg(p_list, Eo_Op);
   while (op)
     {
        if (!_eo_class_op_internal((Eo_Class *) klass, op, &p_list))
          {
             _EO_OP_ERR_NO_OP_PRINT(op, klass);
             ret = EINA_FALSE;
             break;
          }
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
   EO_MAGIC_RETURN_VAL(klass, EO_CLASS_EINA_MAGIC, EINA_FALSE);

   /* Advance the kls itr. */
   nklass = _eo_kls_itr_next(&((Eo_Class *) klass)->mro_itr, op);

   if (klass->mro_itr.op != op)
      return EINA_FALSE;

   va_start(p_list, op);
   if (!_eo_class_op_internal((Eo_Class *) klass, op, &p_list))
     {
        _EO_OP_ERR_NO_OP_PRINT(op, nklass);
        ret = EINA_FALSE;
     }
   va_end(p_list);

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
   if (!desc || !desc->ops.base_op_id)
      return;

   *(desc->ops.base_op_id) = EO_CLASS_ID_TO_BASE_ID(klass->class_id);
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

   /* Recursively add extenions. */
     {
        Eo_Extension_Node *extn;
        EINA_INLIST_FOREACH(klass->extensions, extn)
          {
             mro = _eo_class_mro_add(mro, extn->klass);
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
        Eo_Extension_Node *extn;

        Eina_List *itr = extn_pos;
        EINA_INLIST_FOREACH(klass->extensions, extn)
          {
             /* Get the first one after the extension. */
             Eina_List *extn_list = eina_list_next(eina_list_data_get(itr));

             /* If we found the extension again. */
             if (eina_list_data_find_list(extn_list, extn->klass))
               {
                  eina_list_free(mro);
                  ERR("Cannot create a consistent method resolution order for class '%s' because of '%s'.", klass->desc->name, extn->klass->desc->name);
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
        for ( ; itr->op != 0 ; itr++)
          {
             const Eo_Op_Description *op_desc = _eo_op_id_desc_get(itr->op);

             if (EINA_LIKELY(!op_desc || (itr->op_type == op_desc->op_type)))
               {
                  _dich_func_set(klass, itr->op, itr->func);
               }
             else
               {
                  ERR("Set function's op type (%d) is different than the one in the op description (%d) for op '%s' in class '%s'.", itr->op_type, op_desc->op_type, op_desc->name, klass->desc->name);
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

     {
        Eina_Inlist *itrn;
        Eo_Extension_Node *extn = NULL;
        EINA_INLIST_FOREACH_SAFE(klass->extensions, itrn, extn)
          {
             free(extn);
          }
     }

   if (klass->mro)
      free(klass->mro);

   if (klass->extn_data_off)
      free(klass->extn_data_off);

   free(klass);
}

/* DEVCHECK */
static Eina_Bool
_eo_class_check_op_descs(const Eo_Class *klass, Eo_Class_Id id)
{
   const Eo_Class_Description *desc = klass->desc;
   const Eo_Op_Description *itr;
   size_t i;

   if (desc->ops.count > 0)
     {
        if (((id == 0) || (id > EO_STATIC_IDS_LAST)) && !desc->ops.base_op_id)
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
                  ERR("Wrong order in Ops description for class '%s'. Expected %d and got %d", desc->name, i, itr->sub_op);
               }
             else
               {
                  ERR("Found too few Ops description for class '%s'. Expected %d descriptions, but found %d.", desc->name, desc->ops.count, i);
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

EAPI const Eo_Class *
eo_class_new(const Eo_Class_Description *desc, Eo_Class_Id id, const Eo_Class *parent, ...)
{
   Eo_Class *klass;
   va_list p_list;

   if (parent && !EINA_MAGIC_CHECK(parent, EO_CLASS_EINA_MAGIC))
     {
        EINA_MAGIC_FAIL(parent, EO_CLASS_EINA_MAGIC);
        return NULL;
     }

   if (id > EO_STATIC_IDS_LAST)
     {
        ERR("Tried creating a class with the static id %d while the maximum static id is %d. Aborting.", id, EO_STATIC_IDS_LAST);
        return NULL;
     }

   va_start(p_list, parent);

   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->name, NULL);

   /* Check restrictions on Interface types. */
   if (desc->type == EO_CLASS_TYPE_INTERFACE)
     {
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->constructor, NULL);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->destructor, NULL);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->class_constructor, NULL);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->class_destructor, NULL);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->data_size, NULL);
     }

   klass = calloc(1, sizeof(Eo_Class));
   klass->parent = parent;

   /* Handle class extensions */
     {
        Eo_Class *extn = NULL;

        extn = va_arg(p_list, Eo_Class *);
        while (extn)
          {
             switch (extn->desc->type)
               {
                case EO_CLASS_TYPE_REGULAR:
                case EO_CLASS_TYPE_REGULAR_NO_INSTANT:
                   /* Use it like an interface. */
                case EO_CLASS_TYPE_INTERFACE:
                   break;
                case EO_CLASS_TYPE_MIXIN:
                     {
                        Eo_Extension_Node *node = calloc(1, sizeof(*node));
                        node->klass = extn;
                        klass->extensions =
                           eina_inlist_append(klass->extensions,
                                 EINA_INLIST_GET(node));
                     }
                   break;
               }

             extn = va_arg(p_list, Eo_Class *);
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

   if (!_eo_class_check_op_descs(klass, id))
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

   if (id == 0)
     {
        klass->class_id = ++_eo_classes_last_id;
     }
   else
     {
#ifndef NDEBUG
        if (_eo_classes && _eo_classes[id - 1])
          {
             ERR("A class with id %d was already defined (%s). Aborting.", id,
                   _eo_classes[id - 1]->desc->name);
             eina_lock_release(&_eo_class_creation_lock);
             goto cleanup;
          }
#endif
        klass->class_id = id;
     }


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

   _eo_class_base_op_init(klass);

   _eo_class_constructor(klass);

   va_end(p_list);

   return klass;

cleanup:
   eo_class_free(klass);
   return NULL;
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
        eo_composite_object_detach(obj->parent, obj);
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

   Eo_Kls_Itr prev_state;

   _eo_kls_itr_init(klass, &obj->mro_itr, EO_NOOP, &prev_state);
   _eo_error_unset(obj);

   _eo_ref(obj);
   _eo_constructor(obj, klass);

   if (EINA_UNLIKELY(_eo_error_get(obj)))
     {
        ERR("Type '%s' - One of the object constructors have failed.", klass->desc->name);
        goto fail;
     }

   if (EINA_UNLIKELY(!_eo_kls_itr_reached_end(&obj->mro_itr)))
     {
        ERR("Type '%s' - Not all of the object constructors have been executed.", klass->desc->name);
        goto fail;
     }
   _eo_kls_itr_end(&obj->mro_itr, &prev_state);
   _eo_unref(obj);

   return obj;

fail:
   _eo_kls_itr_end(&obj->mro_itr, &prev_state);
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
   /* We need that for the event callbacks that may ref/unref. */
   obj->refcount++;

   eo_do(obj, eo_event_callback_call(EO_EV_DEL, NULL, NULL));

   const Eo_Class *klass = eo_class_get(obj);
   Eo_Kls_Itr prev_state;

   _eo_kls_itr_init(klass, &obj->mro_itr, EO_NOOP, &prev_state);
   _eo_error_unset(obj);
   _eo_destructor(obj, klass);
   if (_eo_error_get(obj))
     {
        ERR("Type '%s' - One of the object destructors have failed.", klass->desc->name);
     }

   if (!_eo_kls_itr_reached_end(&obj->mro_itr))
     {
        ERR("Type '%s' - Not all of the object destructors have been executed.", klass->desc->name);
     }
   _eo_kls_itr_end(&obj->mro_itr, &prev_state);
   /*FIXME: add eo_class_unref(klass) ? - just to clear the caches. */

     {
        Eina_List *itr, *itr_n;
        Eo *emb_obj;
        EINA_LIST_FOREACH_SAFE(obj->composite_objects, itr, itr_n, emb_obj)
          {
             eo_composite_object_detach(obj, emb_obj);
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
   if (--(obj->refcount) == 0)
     {
        _eo_del_internal(obj);

#ifndef NDEBUG
        /* If for some reason it's not empty, clear it. */
        while (obj->xrefs)
          {
             WRN("obj->xrefs is not empty, possibly a bug, please report. - An error will be reported for each xref in the stack.");
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

   ((Eo *) obj)->construct_error = EINA_TRUE;
}

static inline void
_eo_error_unset(Eo *obj)
{
   obj->construct_error = EINA_FALSE;
}

/**
 * @internal
 * @brief Check if there was an error when constructing, destructing or calling a function of the object.
 * @param obj the object to work on.
 * @return @c EINA_TRUE if there was an error.
 */
static inline Eina_Bool
_eo_error_get(const Eo *obj)
{
   return obj->construct_error;
}

static inline void
_eo_constructor_default(Eo *obj)
{
   eo_constructor_super(obj);
}

static inline void
_eo_destructor_default(Eo *obj)
{
   eo_destructor_super(obj);
}

static void
_eo_constructor(Eo *obj, const Eo_Class *klass)
{
   if (!klass)
      return;

   if (klass->desc->constructor)
      klass->desc->constructor(obj, _eo_data_get(obj, klass));
   else
      _eo_constructor_default(obj);
}

static void
_eo_destructor(Eo *obj, const Eo_Class *klass)
{
   if (!klass)
      return;

   if (klass->desc->destructor)
      klass->desc->destructor(obj, _eo_data_get(obj, klass));
   else
      _eo_destructor_default(obj);
}

EAPI void
eo_constructor_super(Eo *obj)
{
   EO_MAGIC_RETURN(obj, EO_EINA_MAGIC);

   _eo_constructor(obj, _eo_kls_itr_next(&obj->mro_itr, EO_NOOP));
}

EAPI void
eo_destructor_super(Eo *obj)
{
   EO_MAGIC_RETURN(obj, EO_EINA_MAGIC);

   _eo_destructor(obj, _eo_kls_itr_next(&obj->mro_itr, EO_NOOP));
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
   _eo_classes_last_id = EO_STATIC_IDS_LAST;
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
eo_composite_object_attach(Eo *obj, Eo *emb_obj)
{
   EO_MAGIC_RETURN(obj, EO_EINA_MAGIC);
   EO_MAGIC_RETURN(emb_obj, EO_EINA_MAGIC);

   emb_obj->composite = EINA_TRUE;
   eo_parent_set(emb_obj, obj);
   obj->composite_objects = eina_list_prepend(obj->composite_objects, emb_obj);
}

EAPI void
eo_composite_object_detach(Eo *obj, Eo *emb_obj)
{
   EO_MAGIC_RETURN(obj, EO_EINA_MAGIC);
   EO_MAGIC_RETURN(emb_obj, EO_EINA_MAGIC);

   emb_obj->composite = EINA_FALSE;
   obj->composite_objects = eina_list_remove(obj->composite_objects, emb_obj);
   eo_parent_set(emb_obj, NULL);
}

EAPI Eina_Bool
eo_composite_is(const Eo *emb_obj)
{
   if (!EINA_MAGIC_CHECK(emb_obj, EO_EINA_MAGIC))
     {
        EINA_MAGIC_FAIL(emb_obj, EO_EINA_MAGIC);
        return EINA_FALSE;
     }

   return emb_obj->composite;
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

