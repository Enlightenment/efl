#include <Eina.h>

#include "Eobj.h"
#include "eobj_private.h"

#include "config.h"

typedef int Eobj_Class_Id;

/* Used inside the class_get functions of classes, see #EOBJ_DEFINE_CLASS */
EAPI Eina_Lock _eobj_class_creation_lock;
int _eobj_log_dom = -1;

static Eobj_Class **_eobj_classes;
static Eobj_Class_Id _eobj_classes_last_id;
static Eina_Bool _eobj_init_count = 0;

static void _eobj_callback_remove_all(Eobj *obj);
static void _eobj_constructor(Eobj *obj, const Eobj_Class *klass);
static void _eobj_destructor(Eobj *obj, const Eobj_Class *klass);
static void eobj_constructor_error_unset(Eobj *obj);

typedef struct _Eobj_Callback_Description Eobj_Callback_Description;

#define EOBJ_EINA_MAGIC 0xa186bc32 /* Nothing magical about this number. */
#define EOBJ_EINA_MAGIC_STR "Eobj"
#define EOBJ_CLASS_EINA_MAGIC 0xa186bb32 /* Nothing magical about this number. */
#define EOBJ_CLASS_EINA_MAGIC_STR "Eobj Class"

#define EOBJ_MAGIC_RETURN_VAL(d, magic, ret) \
   do { \
        if (!EINA_MAGIC_CHECK(d, magic)) \
          { \
             EINA_MAGIC_FAIL(d, magic); \
             return ret; \
          } \
   } while (0)

#define EOBJ_MAGIC_RETURN(d, magic) \
   do { \
        if (!EINA_MAGIC_CHECK(d, magic)) \
          { \
             EINA_MAGIC_FAIL(d, magic); \
             return; \
          } \
   } while (0)

struct _Eobj {
     EINA_MAGIC
     Eobj *parent;
     const Eobj_Class *klass;
     int refcount;
#ifndef NDEBUG
     Eina_Inlist *xrefs;
#endif

     Eina_List *composite_objects;

     Eina_Inlist *callbacks;
     int walking_list;

     Eina_Inlist *kls_itr;

     Eina_Bool delete:1;
     Eina_Bool construct_error:1;
};

/* Start of Dich */
/* Dich search, split to 0xff 0xff 0xffff */

#define DICH_CHAIN1_MASK (0xff)
#define DICH_CHAIN2_MASK (0xff)
#define DICH_CHAIN_LAST_MASK (0xffff)
#define DICH_CHAIN1_SIZE (DICH_CHAIN1_MASK + 1)
#define DICH_CHAIN2_SIZE (DICH_CHAIN2_MASK + 1)
#define DICH_CHAIN_LAST_SIZE (DICH_CHAIN_LAST_MASK + 1)
#define DICH_CHAIN1(x) (((x) >> 24) & DICH_CHAIN1_MASK)
#define DICH_CHAIN2(x) (((x) >> 16) & DICH_CHAIN2_MASK)
#define DICH_CHAIN_LAST(x) ((x) & DICH_CHAIN_LAST_MASK)

#define OP_CLASS_OFFSET 16
#define OP_CLASS_OFFSET_GET(x) (((x) >> OP_CLASS_OFFSET) & 0xffff)
#define OP_CLASS_GET(op) ({ \
      Eobj_Class_Id tmp = OP_CLASS_OFFSET_GET(op); \
      (Eobj_Class *) ((tmp <= _eobj_classes_last_id) && (tmp > 0)) ? \
      (_eobj_classes[tmp - 1]) : NULL; \
      })
#define OP_SUB_ID_GET(op) ((op) & 0xffff)

#define EOBJ_ALIGN_SIZE(size) \
        ((size) + (sizeof(void *) - ((size) % sizeof(void *))))

/* Structure of Eobj_Op is:
 * 16bit: class
 * 16bit: op.
 */

typedef struct _Dich_Chain1 Dich_Chain1;

typedef struct
{
   eobj_op_func_type func;
} op_type_funcs;

typedef struct
{
   op_type_funcs *funcs;
} Dich_Chain2;

struct _Dich_Chain1
{
   Dich_Chain2 *chain;
};

typedef struct
{
     EINA_INLIST;
     const Eobj_Class *klass;
} Eobj_Extension_Node;

typedef struct
{
     const Eobj_Class *klass;
     size_t offset;
} Eobj_Extension_Data_Offset;

struct _Eobj_Class
{
   EINA_MAGIC
   Eobj_Class_Id class_id;
   const Eobj_Class *parent;
   const Eobj_Class_Description *desc;
   Dich_Chain1 chain[DICH_CHAIN1_SIZE];
   Eina_Inlist *extensions;

   Eobj_Extension_Data_Offset *extn_data_off;
   size_t extn_data_size;

   const Eobj_Class **mro;

   size_t data_offset; /* < Offset of the data within object data. */

   Eina_Bool constructed : 1;
};

static inline eobj_op_func_type
dich_func_get(const Eobj_Class *klass, Eobj_Op op)
{
   const Dich_Chain1 *chain1 = &klass->chain[DICH_CHAIN1(op)];
   if (!chain1) return NULL;
   if (!chain1->chain) return NULL;
   Dich_Chain2 *chain2 = &chain1->chain[DICH_CHAIN2(op)];
   if (!chain2) return NULL;
   if (!chain2->funcs) return NULL;

   /* num_ops is calculated from the class. */
   const Eobj_Class *op_klass = OP_CLASS_GET(op);
   if (!op_klass || (DICH_CHAIN_LAST(op) >= op_klass->desc->ops.count))
      return NULL;

   return chain2->funcs[DICH_CHAIN_LAST(op)].func;
}

static inline void
dich_func_set(Eobj_Class *klass, Eobj_Op op, eobj_op_func_type func)
{
   const Eobj_Class *op_klass = OP_CLASS_GET(op);
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

   Dich_Chain1 *chain1 = &klass->chain[DICH_CHAIN1(op)];
   if (!chain1->chain)
     {
        klass->chain[DICH_CHAIN1(op)].chain =
           chain1->chain =
           calloc(DICH_CHAIN2_SIZE, sizeof(*(chain1->chain)));
     }

   Dich_Chain2 *chain2 = &chain1->chain[DICH_CHAIN2(op)];
   if (!chain2->funcs)
     {
        chain2->funcs = chain1->chain[DICH_CHAIN2(op)].funcs =
           calloc(num_ops, sizeof(*(chain2->funcs)));
     }

   chain2->funcs[DICH_CHAIN_LAST(op)].func = func;
}

static inline void
dich_func_clean_all(Eobj_Class *klass)
{
   int i;
   Dich_Chain1 *chain1 = klass->chain;

   for (i = 0 ; i < DICH_CHAIN1_SIZE ; i++, chain1++)
     {
        int j;
        Dich_Chain2 *chain2 = chain1->chain;

        if (!chain2)
           continue;

        for (j = 0 ; j < DICH_CHAIN2_SIZE ; j++, chain2++)
          {
             free(chain2->funcs);
          }
        free(chain1->chain);
        chain1->chain = NULL;
     }
}

/* END OF DICH */

static const Eobj_Op_Description noop_desc =
        EOBJ_OP_DESCRIPTION(EOBJ_NOOP, "", "No operation.");

static const Eobj_Op_Description *
_eobj_op_id_desc_get(Eobj_Op op)
{
   const Eobj_Class *klass = OP_CLASS_GET(op);
   Eobj_Op sub_id = OP_SUB_ID_GET(op);

   if (op == EOBJ_NOOP)
      return &noop_desc;

   if (klass && (sub_id < klass->desc->ops.count))
      return klass->desc->ops.descs + sub_id;

   return NULL;
}

static const char *
_eobj_op_id_name_get(Eobj_Op op)
{
   const Eobj_Op_Description *desc = _eobj_op_id_desc_get(op);
   return (desc) ? desc->name : NULL;
}

typedef struct
{
   EINA_INLIST;
   Eobj_Op op;
   const Eobj_Class **kls_itr;
} Eobj_Kls_Itr_Node;

static inline Eina_Bool
_eobj_kls_itr_init(Eobj *obj, Eobj_Op op)
{
   if (obj->kls_itr)
     {
        Eobj_Kls_Itr_Node *node =
           EINA_INLIST_CONTAINER_GET(obj->kls_itr, Eobj_Kls_Itr_Node);
        if (node->op == op)
          {
             return EINA_FALSE;
          }
     }


     {
        Eobj_Kls_Itr_Node *node = calloc(1, sizeof(*node));
        node->op = op;
        node->kls_itr = obj->klass->mro;
        obj->kls_itr = eina_inlist_prepend(obj->kls_itr,
              EINA_INLIST_GET(node));

        return EINA_TRUE;
     }
}

static inline void
_eobj_kls_itr_end(Eobj *obj, Eobj_Op op)
{
   Eobj_Kls_Itr_Node *node =
      EINA_INLIST_CONTAINER_GET(obj->kls_itr, Eobj_Kls_Itr_Node);

   if (node->op != op)
      return;

   obj->kls_itr = eina_inlist_remove(obj->kls_itr, obj->kls_itr);
   free(node);
}

static inline const Eobj_Class *
_eobj_kls_itr_get(Eobj *obj)
{
   Eobj_Kls_Itr_Node *node =
      EINA_INLIST_CONTAINER_GET(obj->kls_itr, Eobj_Kls_Itr_Node);

   return (node) ? *(node->kls_itr) : NULL;
}

static inline const Eobj_Class *
_eobj_kls_itr_next(Eobj *obj, Eobj_Op op)
{
   Eobj_Kls_Itr_Node *node =
      EINA_INLIST_CONTAINER_GET(obj->kls_itr, Eobj_Kls_Itr_Node);

   if (!node || (node->op != op))
     {
        Eobj_Op node_op = (node) ? node->op : EOBJ_NOOP;
        ERR("Called with op %d ('%s') while expecting: %d ('%s'). This probaly means you called eobj_*_super functions from a wrong place.",
              op, _eobj_op_id_name_get(op),
              node_op, _eobj_op_id_name_get(node_op));
        return NULL;
     }

   const Eobj_Class **kls_itr = node->kls_itr;
   if (*kls_itr)
     {
        kls_itr++;
        node->kls_itr = kls_itr;
        return *kls_itr;
     }
   else
     {
        return NULL;
     }
}

static inline Eina_Bool
_eobj_kls_itr_reached_end(const Eobj *obj)
{
   Eobj_Kls_Itr_Node *node =
      EINA_INLIST_CONTAINER_GET(obj->kls_itr, Eobj_Kls_Itr_Node);
   const Eobj_Class **kls_itr = node->kls_itr;
   return !(*kls_itr && *(kls_itr + 1));
}

static Eina_Bool
_eobj_op_internal(Eobj *obj, Eobj_Op op, va_list *p_list)
{
   const Eobj_Class *klass;
   Eina_Bool ret = EINA_FALSE;
   Eina_Bool _itr_init;

   _itr_init = _eobj_kls_itr_init(obj, op);
   klass = _eobj_kls_itr_get(obj);
   while (klass)
     {
        eobj_op_func_type func = dich_func_get(klass, op);

        if (func)
          {
             func(obj, eobj_data_get(obj, klass), p_list);
             ret = EINA_TRUE;
             goto end;
          }

        klass = _eobj_kls_itr_next(obj, op);
     }

   /* Try composite objects */
     {
        Eina_List *itr;
        Eobj *emb_obj;
        EINA_LIST_FOREACH(obj->composite_objects, itr, emb_obj)
          {
             if (_eobj_op_internal(emb_obj, op, p_list))
               {
                  ret = EINA_TRUE;
                  goto end;
               }
          }
     }

end:

   if (_itr_init) _eobj_kls_itr_end(obj, op);
   return ret;
}

EAPI Eina_Bool
eobj_do_internal(Eobj *obj, ...)
{
   Eina_Bool ret = EINA_TRUE;
   Eobj_Op op = EOBJ_NOOP;
   va_list p_list;

   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, EINA_FALSE);

   eobj_ref(obj);

   va_start(p_list, obj);

   op = va_arg(p_list, Eobj_Op);
   while (op)
     {
        if (!_eobj_op_internal(obj, op, &p_list))
          {
             const Eobj_Class *op_klass = OP_CLASS_GET(op);
             const char *_dom_name = (op_klass) ? op_klass->desc->name : NULL;
             ERR("Can't find func for op %x ('%s' of domain '%s') for class '%s'. Aborting.",
                   op, _eobj_op_id_name_get(op), _dom_name,
                   obj->klass->desc->name);
             ret = EINA_FALSE;
             break;
          }
        op = va_arg(p_list, Eobj_Op);
     }

   va_end(p_list);

   eobj_unref(obj);
   return ret;
}

EAPI Eina_Bool
eobj_do_super(Eobj *obj, Eobj_Op op, ...)
{
   const Eobj_Class *obj_klass;
   Eina_Bool ret = EINA_TRUE;
   va_list p_list;
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, EINA_FALSE);

   /* Advance the kls itr. */
   obj_klass = _eobj_kls_itr_next(obj, op);

   if (!obj_klass)
     {
        return EINA_FALSE;
     }

   va_start(p_list, op);
   if (!_eobj_op_internal(obj, op, &p_list))
     {
        const Eobj_Class *op_klass = OP_CLASS_GET(op);
        const char *_dom_name = (op_klass) ? op_klass->desc->name : NULL;
        ERR("Can't find func for op %x ('%s' of domain '%s') for class '%s'. Aborting.",
              op, _eobj_op_id_name_get(op), _dom_name,
              (obj_klass) ? obj_klass->desc->name : NULL);
        ret = EINA_FALSE;
     }
   va_end(p_list);

   return ret;
}

EAPI const Eobj_Class *
eobj_class_get(const Eobj *obj)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, EINA_FALSE);

   return obj->klass;
}

EAPI const char *
eobj_class_name_get(const Eobj_Class *klass)
{
   EOBJ_MAGIC_RETURN_VAL(klass, EOBJ_CLASS_EINA_MAGIC, NULL);

   return klass->desc->name;
}

static void
_eobj_class_base_op_init(Eobj_Class *klass)
{
   const Eobj_Class_Description *desc = klass->desc;
   if (!desc || !desc->ops.base_op_id)
      return;

   /* FIXME: Depends on values defined above! */
   *(desc->ops.base_op_id) = klass->class_id << OP_CLASS_OFFSET;
}

static Eina_List *
_eobj_class_mro_add(Eina_List *mro, const Eobj_Class *klass)
{
   Eina_List *extn_pos = NULL;
   Eina_Bool check_consistency = !mro;
   if (!klass)
      return mro;

   mro = eina_list_append(mro, klass);

   /* Recursively add extenions. */
     {
        Eobj_Extension_Node *extn;
        EINA_INLIST_FOREACH(klass->extensions, extn)
          {
             mro = _eobj_class_mro_add(mro, extn->klass);
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
        Eobj_Extension_Node *extn;

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


   mro = _eobj_class_mro_add(mro, klass->parent);

   return mro;
}

static Eina_Bool
_eobj_class_mro_init(Eobj_Class *klass)
{
   Eina_List *mro = NULL;

   DBG("Started creating MRO for class '%s'", klass->desc->name);
   mro = _eobj_class_mro_add(mro, klass);

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
        const Eobj_Class *kls_itr;
        const Eobj_Class **mro_itr;
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
_eobj_class_constructor(Eobj_Class *klass)
{
   if (klass->constructed)
      return;

   klass->constructed = EINA_TRUE;

   if (klass->desc->class_constructor)
      klass->desc->class_constructor(klass);
}

EAPI void
eobj_class_funcs_set(Eobj_Class *klass, const Eobj_Op_Func_Description *func_descs)
{
   EOBJ_MAGIC_RETURN(klass, EOBJ_CLASS_EINA_MAGIC);

   const Eobj_Op_Func_Description *itr;
   itr = func_descs;
   if (itr)
     {
        for ( ; itr->op != 0 ; itr++)
          {
             dich_func_set(klass, itr->op, itr->func);
          }
     }
}

static void
eobj_class_free(Eobj_Class *klass)
{
   if (klass->constructed)
     {
        if (klass->desc->class_destructor)
           klass->desc->class_destructor(klass);

        dich_func_clean_all(klass);
     }

     {
        Eina_Inlist *itrn;
        Eobj_Extension_Node *extn;
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
_eobj_class_check_op_descs(const Eobj_Class *klass)
{
   const Eobj_Class_Description *desc = klass->desc;
   const Eobj_Op_Description *itr;
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

EAPI const Eobj_Class *
eobj_class_new(const Eobj_Class_Description *desc, const Eobj_Class *parent, ...)
{
   Eobj_Class *klass;
   va_list p_list;

   if (parent && !EINA_MAGIC_CHECK(parent, EOBJ_CLASS_EINA_MAGIC))
     {
        EINA_MAGIC_FAIL(parent, EOBJ_CLASS_EINA_MAGIC);
        return NULL;
     }

   va_start(p_list, parent);

   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc->name, NULL);

   /* Check restrictions on Interface types. */
   if (desc->type == EOBJ_CLASS_TYPE_INTERFACE)
     {
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->constructor, NULL);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->destructor, NULL);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->class_constructor, NULL);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->class_destructor, NULL);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!desc->data_size, NULL);
     }

   klass = calloc(1, sizeof(Eobj_Class));
   klass->parent = parent;

   /* Handle class extensions */
     {
        Eobj_Class *extn = NULL;

        extn = va_arg(p_list, Eobj_Class *);
        while (extn)
          {
             switch (extn->desc->type)
               {
                case EOBJ_CLASS_TYPE_REGULAR:
                case EOBJ_CLASS_TYPE_REGULAR_NO_INSTANT:
                   /* Use it like an interface. */
                case EOBJ_CLASS_TYPE_INTERFACE:
                   break;
                case EOBJ_CLASS_TYPE_MIXIN:
                     {
                        Eobj_Extension_Node *node = calloc(1, sizeof(*node));
                        node->klass = extn;
                        klass->extensions =
                           eina_inlist_append(klass->extensions,
                                 EINA_INLIST_GET(node));
                     }
                   break;
               }

             extn = va_arg(p_list, Eobj_Class *);
          }
     }

   klass->desc = desc;

   /* Handle the inheritance */
   if (klass->parent)
     {
        /* Verify the inheritance is allowed. */
        switch (klass->desc->type)
          {
           case EOBJ_CLASS_TYPE_REGULAR:
           case EOBJ_CLASS_TYPE_REGULAR_NO_INSTANT:
              if ((klass->parent->desc->type != EOBJ_CLASS_TYPE_REGULAR) &&
                    (klass->parent->desc->type != EOBJ_CLASS_TYPE_REGULAR_NO_INSTANT))
                {
                   ERR("Regular classes ('%s') aren't allowed to inherit from non-regular classes ('%s').", klass->desc->name, klass->parent->desc->name);
                   goto cleanup;
                }
              break;
           case EOBJ_CLASS_TYPE_INTERFACE:
           case EOBJ_CLASS_TYPE_MIXIN:
              if ((klass->parent->desc->type != EOBJ_CLASS_TYPE_INTERFACE) &&
                    (klass->parent->desc->type != EOBJ_CLASS_TYPE_MIXIN))
                {
                   ERR("Non-regular classes ('%s') aren't allowed to inherit from regular classes ('%s').", klass->desc->name, klass->parent->desc->name);
                   goto cleanup;
                }
              break;
          }


        /* Update the current offset. */
        /* FIXME: Make sure this alignment is enough. */
        klass->data_offset = klass->parent->data_offset +
           EOBJ_ALIGN_SIZE(klass->parent->desc->data_size);
     }

   if (!_eobj_class_check_op_descs(klass))
     {
        goto cleanup;
     }

   if (!_eobj_class_mro_init(klass))
     {
        goto cleanup;
     }

   /* create MIXIN offset table. */
     {
        const Eobj_Class **mro_itr = klass->mro;
        Eobj_Extension_Data_Offset *extn_data_itr;
        size_t extn_num = 0;
        size_t extn_data_off = klass->data_offset +
           EOBJ_ALIGN_SIZE(klass->desc->data_size);

        /* FIXME: Make faster... */
        while (*mro_itr)
          {
             if (((*mro_itr)->desc->type == EOBJ_CLASS_TYPE_MIXIN) &&
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
             if (((*mro_itr)->desc->type == EOBJ_CLASS_TYPE_MIXIN) &&
                   ((*mro_itr)->desc->data_size > 0))
               {
                  extn_data_itr->klass = *mro_itr;
                  extn_data_itr->offset = extn_data_off;

                  extn_data_off += EOBJ_ALIGN_SIZE(extn_data_itr->klass->desc->data_size);
                  extn_data_itr++;
               }
             mro_itr++;
          }

        klass->extn_data_size = extn_data_off;
     }

   eina_lock_take(&_eobj_class_creation_lock);
   klass->class_id = ++_eobj_classes_last_id;
     {
        /* FIXME: Handle errors. */
        Eobj_Class **tmp;
        tmp = realloc(_eobj_classes, _eobj_classes_last_id * sizeof(*_eobj_classes));
        _eobj_classes = tmp;
        _eobj_classes[klass->class_id - 1] = klass;
     }
   eina_lock_release(&_eobj_class_creation_lock);

   EINA_MAGIC_SET(klass, EOBJ_CLASS_EINA_MAGIC);

   _eobj_class_base_op_init(klass);

   _eobj_class_constructor(klass);

   va_end(p_list);

   return klass;

cleanup:
   eobj_class_free(klass);
   return NULL;
}

EAPI Eobj *
eobj_add(const Eobj_Class *klass, Eobj *parent)
{
   EOBJ_MAGIC_RETURN_VAL(klass, EOBJ_CLASS_EINA_MAGIC, NULL);

   if (parent) EOBJ_MAGIC_RETURN_VAL(parent, EOBJ_EINA_MAGIC, NULL);

   if (EINA_UNLIKELY(klass->desc->type != EOBJ_CLASS_TYPE_REGULAR))
     {
        ERR("Class '%s' is not instantiate-able. Aborting.", klass->desc->name);
        return NULL;
     }

   Eobj *obj = calloc(1, EOBJ_ALIGN_SIZE(sizeof(*obj)) +
         (klass->data_offset + EOBJ_ALIGN_SIZE(klass->desc->data_size)) +
         klass->extn_data_size);
   obj->klass = klass;
   obj->parent = parent;

   obj->refcount++;

   _eobj_kls_itr_init(obj, EOBJ_NOOP);
   eobj_constructor_error_unset(obj);

   EINA_MAGIC_SET(obj, EOBJ_EINA_MAGIC);
   eobj_ref(obj);
   _eobj_constructor(obj, klass);

   if (EINA_UNLIKELY(eobj_constructor_error_get(obj)))
     {
        ERR("Type '%s' - One of the object constructors have failed.", klass->desc->name);
        goto fail;
     }

   if (EINA_UNLIKELY(!_eobj_kls_itr_reached_end(obj)))
     {
        ERR("Type '%s' - Not all of the object constructors have been executed.", klass->desc->name);
        goto fail;
     }
   _eobj_kls_itr_end(obj, EOBJ_NOOP);
   eobj_unref(obj);

   return obj;

fail:
   /* Unref twice, once for the ref above, and once for the basic object ref. */
   eobj_unref(obj);
   eobj_unref(obj);
   return NULL;
}

typedef struct
{
   EINA_INLIST;
   const Eobj *ref_obj;
   const char *file;
   int line;
} Eobj_Xref_Node;

EAPI Eobj *
eobj_xref_internal(Eobj *obj, const Eobj *ref_obj, const char *file, int line)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, obj);

   eobj_ref(obj);

#ifndef NDEBUG
   Eobj_Xref_Node *xref = calloc(1, sizeof(*xref));
   xref->ref_obj = ref_obj;
   xref->file = file;
   xref->line = line;

   /* FIXME: Make it sorted. */
   obj->xrefs = eina_inlist_prepend(obj->xrefs, EINA_INLIST_GET(xref));
#else
   (void) ref_obj;
   (void) file;
   (void) line;
#endif

   return obj;
}

EAPI void
eobj_xunref(Eobj *obj, const Eobj *ref_obj)
{
   EOBJ_MAGIC_RETURN(obj, EOBJ_EINA_MAGIC);
#ifndef NDEBUG
   Eobj_Xref_Node *xref = NULL;
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
   eobj_unref(obj);
}

EAPI Eobj *
eobj_ref(Eobj *obj)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, obj);

   obj->refcount++;
   return obj;
}

static void
_eobj_del_internal(Eobj *obj)
{
   /* We need that for the event callbacks that may ref/unref. */
   obj->refcount++;

   if (!obj->delete)
     {
        eobj_event_callback_call(obj, EOBJ_EV_DEL, NULL);
        obj->delete = EINA_TRUE;
     }

   obj->refcount--;

   const Eobj_Class *klass = eobj_class_get(obj);
   _eobj_kls_itr_init(obj, EOBJ_NOOP);
   eobj_constructor_error_unset(obj);
   _eobj_destructor(obj, klass);
   if (eobj_constructor_error_get(obj))
     {
        ERR("Type '%s' - One of the object destructors have failed.", klass->desc->name);
     }

   if (!_eobj_kls_itr_reached_end(obj))
     {
        ERR("Type '%s' - Not all of the object destructors have been executed.", klass->desc->name);
     }
   _eobj_kls_itr_end(obj, EOBJ_NOOP);
   /*FIXME: add eobj_class_unref(klass) ? - just to clear the caches. */

   /* If for some reason it's not empty, clear it. */
   while (obj->kls_itr)
     {
        WRN("Kls_Itr is not empty, possibly a bug, please report. - An error will be reported for each kls_itr in the stack.");
        Eina_Inlist *nitr = nitr->next;
        free(EINA_INLIST_CONTAINER_GET(obj->kls_itr, Eobj_Kls_Itr_Node));
        obj->kls_itr = nitr;
     }

   Eina_List *itr, *itr_n;
   Eobj *emb_obj;
   EINA_LIST_FOREACH_SAFE(obj->composite_objects, itr, itr_n, emb_obj)
     {
        eobj_composite_object_detach(obj, emb_obj);
     }
}

EAPI void
eobj_unref(Eobj *obj)
{
   EOBJ_MAGIC_RETURN(obj, EOBJ_EINA_MAGIC);

   if (--(obj->refcount) == 0)
     {
        _eobj_del_internal(obj);
        /* We need that for the event callbacks that may ref/unref. */
        obj->refcount++;

        eobj_event_callback_call(obj, EOBJ_EV_FREE, NULL);

        obj->refcount--;

#ifndef NDEBUG
   /* If for some reason it's not empty, clear it. */
   while (obj->xrefs)
     {
        WRN("obj->xrefs is not empty, possibly a bug, please report. - An error will be reported for each xref in the stack.");
        Eina_Inlist *nitr = nitr->next;
        free(EINA_INLIST_CONTAINER_GET(obj->xrefs, Eobj_Kls_Itr_Node));
        obj->xrefs = nitr;
     }
#endif

        _eobj_callback_remove_all(obj);

        free(obj);
     }
}

EAPI int
eobj_ref_get(const Eobj *obj)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, 0);

   return obj->refcount;
}

/* Weak reference. */
Eina_Bool
_eobj_weak_ref_cb(void *data, Eobj *obj EINA_UNUSED, const Eobj_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eobj **wref = data;
   *wref = NULL;

   return EOBJ_CALLBACK_CONTINUE;
}

EAPI void
eobj_weak_ref_add(const Eobj *_obj, Eobj **wref)
{
   Eobj *obj = (Eobj *) _obj;
   EOBJ_MAGIC_RETURN(obj, EOBJ_EINA_MAGIC);

   *wref = obj;
   eobj_event_callback_add(obj, EOBJ_EV_DEL, _eobj_weak_ref_cb, wref);
}

EAPI void
eobj_weak_ref_del(Eobj **wref)
{
   if (*wref)
     {
        eobj_event_callback_del_full(*wref, EOBJ_EV_DEL, _eobj_weak_ref_cb,
              wref);
     }
}

/* EOF Weak reference. */

EAPI void
eobj_del(Eobj *obj)
{
   EOBJ_MAGIC_RETURN(obj, EOBJ_EINA_MAGIC);

   if (!obj->delete)
     {
        _eobj_del_internal(obj);
     }
   eobj_unref(obj);
}

EAPI Eobj *
eobj_parent_get(Eobj *obj)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, NULL);

   return obj->parent;
}

EAPI void
eobj_constructor_error_set(Eobj *obj)
{
   EOBJ_MAGIC_RETURN(obj, EOBJ_EINA_MAGIC);

   obj->construct_error = EINA_TRUE;
}

static void
eobj_constructor_error_unset(Eobj *obj)
{
   obj->construct_error = EINA_FALSE;
}

EAPI Eina_Bool
eobj_constructor_error_get(const Eobj *obj)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, EINA_TRUE);

   return obj->construct_error;
}

static inline void
_eobj_constructor_default(Eobj *obj)
{
   eobj_constructor_super(obj);
}

static inline void
_eobj_destructor_default(Eobj *obj)
{
   eobj_destructor_super(obj);
}

static void
_eobj_constructor(Eobj *obj, const Eobj_Class *klass)
{
   if (!klass)
      return;

   if (klass->desc->constructor)
      klass->desc->constructor(obj, eobj_data_get(obj, klass));
   else
      _eobj_constructor_default(obj);
}

static void
_eobj_destructor(Eobj *obj, const Eobj_Class *klass)
{
   if (!klass)
      return;

   if (klass->desc->destructor)
      klass->desc->destructor(obj, eobj_data_get(obj, klass));
   else
      _eobj_destructor_default(obj);
}

EAPI void
eobj_constructor_super(Eobj *obj)
{
   EOBJ_MAGIC_RETURN(obj, EOBJ_EINA_MAGIC);

   _eobj_constructor(obj, _eobj_kls_itr_next(obj, EOBJ_NOOP));
}

EAPI void
eobj_destructor_super(Eobj *obj)
{
   EOBJ_MAGIC_RETURN(obj, EOBJ_EINA_MAGIC);

   _eobj_destructor(obj, _eobj_kls_itr_next(obj, EOBJ_NOOP));
}

EAPI void *
eobj_data_get(const Eobj *obj, const Eobj_Class *klass)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, NULL);

   /* FIXME: Add a check that this is of the right klass and we don't seg.
    * Probably just return NULL. */
   if (EINA_LIKELY(klass->desc->data_size > 0))
     {
        if (EINA_UNLIKELY(klass->desc->type == EOBJ_CLASS_TYPE_MIXIN))
          {
             Eobj_Extension_Data_Offset *doff_itr =
                eobj_class_get(obj)->extn_data_off;

             if (!doff_itr)
                return NULL;

             while (doff_itr->klass)
               {
                  if (doff_itr->klass == klass)
                     return ((char *) obj) + EOBJ_ALIGN_SIZE(sizeof(*obj)) +
                           doff_itr->offset;
                  doff_itr++;
               }
          }
        else
          {
          return ((char *) obj) + EOBJ_ALIGN_SIZE(sizeof(*obj)) +
             klass->data_offset;
          }
     }

   return NULL;
}

EAPI Eina_Bool
eobj_init(void)
{
   const char *log_dom = "eobj";
   if (_eobj_init_count++ > 0)
      return EINA_TRUE;

   eina_init();

   _eobj_classes = NULL;
   _eobj_classes_last_id = 0;
   _eobj_log_dom = eina_log_domain_register(log_dom, EINA_COLOR_LIGHTBLUE);
   if (_eobj_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", log_dom);
        return EINA_FALSE;
     }

   if (!eina_lock_new(&_eobj_class_creation_lock))
     {
        EINA_LOG_ERR("Could not init lock.");
        return EINA_FALSE;
     }

   eina_magic_string_static_set(EOBJ_EINA_MAGIC, EOBJ_EINA_MAGIC_STR);
   eina_magic_string_static_set(EOBJ_CLASS_EINA_MAGIC,
         EOBJ_CLASS_EINA_MAGIC_STR);

   return EINA_TRUE;
}

EAPI Eina_Bool
eobj_shutdown(void)
{
   int i;
   Eobj_Class **cls_itr = _eobj_classes;

   if (--_eobj_init_count > 0)
      return EINA_TRUE;

   for (i = 0 ; i < _eobj_classes_last_id ; i++, cls_itr++)
     {
        if (*cls_itr)
           eobj_class_free(*cls_itr);
     }

   if (_eobj_classes)
      free(_eobj_classes);

   eina_lock_free(&_eobj_class_creation_lock);

   eina_log_domain_unregister(_eobj_log_dom);
   _eobj_log_dom = -1;

   eina_shutdown();
   return EINA_TRUE;
}

EAPI void
eobj_composite_object_attach(Eobj *obj, Eobj *emb_obj)
{
   EOBJ_MAGIC_RETURN(obj, EOBJ_EINA_MAGIC);
   EOBJ_MAGIC_RETURN(emb_obj, EOBJ_EINA_MAGIC);

   eobj_xref(emb_obj, obj);
   obj->composite_objects = eina_list_prepend(obj->composite_objects, emb_obj);
}

EAPI void
eobj_composite_object_detach(Eobj *obj, Eobj *emb_obj)
{
   EOBJ_MAGIC_RETURN(obj, EOBJ_EINA_MAGIC);
   EOBJ_MAGIC_RETURN(emb_obj, EOBJ_EINA_MAGIC);

   obj->composite_objects = eina_list_remove(obj->composite_objects, emb_obj);
   eobj_xunref(emb_obj, obj);
}

EAPI Eina_Bool
eobj_composite_is(Eobj *emb_obj)
{
   if (!EINA_MAGIC_CHECK(emb_obj, EOBJ_EINA_MAGIC))
     {
        EINA_MAGIC_FAIL(emb_obj, EOBJ_EINA_MAGIC);
        return EINA_FALSE;
     }

   Eobj *obj = eobj_parent_get(emb_obj);
   Eina_List *itr;
   Eobj *tmp;

   if (!obj)
      return EINA_FALSE;

   EINA_LIST_FOREACH(obj->composite_objects, itr, tmp)
     {
        if (tmp == emb_obj)
           return EINA_TRUE;
     }

   return EINA_FALSE;
}

/* Callbacks */
struct _Eobj_Callback_Description
{
   EINA_INLIST;
   const Eobj_Event_Description *event;
   Eobj_Event_Cb func;
   void *func_data;
   Eobj_Callback_Priority priority;
   Eina_Bool delete_me : 1;
};

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eobj_callback_remove(Eobj *obj, Eobj_Callback_Description *cb)
{
   obj->callbacks = eina_inlist_remove(obj->callbacks,
         EINA_INLIST_GET(cb));
   free(cb);
}

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eobj_callback_remove_all(Eobj *obj)
{
   Eina_Inlist *initr;
   Eobj_Callback_Description *cb;
   EINA_INLIST_FOREACH_SAFE(obj->callbacks, initr, cb)
     {
        _eobj_callback_remove(obj, cb);
     }
}

static void
_eobj_callbacks_clear(Eobj *obj)
{
   Eina_Inlist *itn;
   Eobj_Callback_Description *cb;

   /* Abort if we are currently walking the list. */
   if (obj->walking_list > 0)
      return;

   EINA_INLIST_FOREACH_SAFE(obj->callbacks, itn, cb)
     {
        if (cb->delete_me)
          {
             _eobj_callback_remove(obj, cb);
          }
     }
}

static int
_callback_priority_cmp(const void *_a, const void *_b)
{
   const Eobj_Callback_Description *a, *b;
   a = (const Eobj_Callback_Description *) _a;
   b = (const Eobj_Callback_Description *) _b;
   if (a->priority < b->priority)
      return -1;
   else
      return 1;
}

EAPI Eina_Bool
eobj_event_callback_priority_add(Eobj *obj,
      const Eobj_Event_Description *desc,
      Eobj_Callback_Priority priority,
      Eobj_Event_Cb func,
      const void *data)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, EINA_FALSE);

   Eobj_Callback_Description *cb = calloc(1, sizeof(*cb));
   cb->event = desc;
   cb->func = func;
   cb->func_data = (void *) data;
   cb->priority = priority;
   obj->callbacks = eina_inlist_sorted_insert(obj->callbacks,
         EINA_INLIST_GET(cb), _callback_priority_cmp);

   eobj_event_callback_call(obj, EOBJ_EV_CALLBACK_ADD, desc);

   return EINA_TRUE;
}

EAPI void *
eobj_event_callback_del(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Event_Cb func)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, NULL);

   void *ret = NULL;
   Eobj_Callback_Description *cb;
   EINA_INLIST_FOREACH(obj->callbacks, cb)
     {
        if ((cb->event == desc) && (cb->func == func))
          {
             void *data;

             data = cb->func_data;
             cb->delete_me = EINA_TRUE;
             _eobj_callbacks_clear(obj);
             ret = data;
             goto found;
          }
     }

   return NULL;

found:
   eobj_event_callback_call(obj, EOBJ_EV_CALLBACK_DEL, desc);
   return ret;
}

EAPI void *
eobj_event_callback_del_full(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Event_Cb func, const void *user_data)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, NULL);

   void *ret = NULL;
   Eobj_Callback_Description *cb;
   EINA_INLIST_FOREACH(obj->callbacks, cb)
     {
        if ((cb->event == desc) && (cb->func == func) &&
              (cb->func_data == user_data))
          {
             void *data;

             data = cb->func_data;
             cb->delete_me = EINA_TRUE;
             _eobj_callbacks_clear(obj);
             ret = data;
             goto found;
          }
     }

   return NULL;

found:
   eobj_event_callback_call(obj, EOBJ_EV_CALLBACK_DEL, desc);
   return ret;
}

EAPI Eina_Bool
eobj_event_callback_call(Eobj *obj, const Eobj_Event_Description *desc,
      const void *event_info)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, EINA_FALSE);

   Eina_Bool ret = EINA_TRUE;
   Eobj_Callback_Description *cb;

   eobj_ref(obj);
   obj->walking_list++;

   EINA_INLIST_FOREACH(obj->callbacks, cb)
     {
        if (!cb->delete_me && (cb->event == desc))
          {
             /* Abort callback calling if the func says so. */
             if (!cb->func((void *) cb->func_data, obj, desc,
                      (void *) event_info))
               {
                  ret = EINA_FALSE;
                  break;
               }
          }
        if (obj->delete)
          break;
     }
   obj->walking_list--;
   _eobj_callbacks_clear(obj);
   eobj_unref(obj);

   return ret;
}

static Eina_Bool
_eobj_event_forwarder_callback(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info)
{
   (void) obj;
   Eobj *new_obj = (Eobj *) data;
   return eobj_event_callback_call(new_obj, desc, event_info);
}

/* FIXME: Change default priority? Maybe call later? */
EAPI Eina_Bool
eobj_event_callback_forwarder_add(Eobj *obj, const Eobj_Event_Description *desc, Eobj *new_obj)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, EINA_FALSE);
   EOBJ_MAGIC_RETURN_VAL(new_obj, EOBJ_EINA_MAGIC, EINA_FALSE);

   return eobj_event_callback_add(obj, desc, _eobj_event_forwarder_callback, new_obj);
}

EAPI Eina_Bool
eobj_event_callback_forwarder_del(Eobj *obj, const Eobj_Event_Description *desc, Eobj *new_obj)
{
   EOBJ_MAGIC_RETURN_VAL(obj, EOBJ_EINA_MAGIC, EINA_FALSE);
   EOBJ_MAGIC_RETURN_VAL(new_obj, EOBJ_EINA_MAGIC, EINA_FALSE);

   eobj_event_callback_del_full(obj, desc, _eobj_event_forwarder_callback, new_obj);
   return EINA_TRUE;
}

