#include <Eina.h>

#include "Eobj.h"

static int _eobj_log_dom = -1;

static Eobj_Class **_eobj_classes;
static Eobj_Class_Id _eobj_classes_last_id;
static Eina_Bool _eobj_init_count = 0;

#define CONSTRUCT_ERROR_KEY "__construct_error"

static void _eobj_callback_remove_all(Eobj *obj);
static void _eobj_generic_data_del_all(Eobj *obj);
static void eobj_class_constructor(Eobj *obj, const Eobj_Class *klass);
static void eobj_class_destructor(Eobj *obj, const Eobj_Class *klass);

#ifdef CRITICAL
#undef CRITICAL
#endif
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_eobj_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eobj_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eobj_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eobj_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eobj_log_dom, __VA_ARGS__)

typedef struct _Eobj_Callback_Description Eobj_Callback_Description;

struct _Eobj {
     Eobj *parent;
     const Eobj_Class *klass;
     void *data_blob;
     void **datas;
     int refcount;
     Eina_List *composite_objects;

     Eina_Inlist *callbacks;
     int walking_list;

     Eina_Inlist *generic_data;

     const Eobj_Class *kls_itr;

     Eina_Bool delete:1;
     EINA_MAGIC
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
     Eina_Bool exists : 1; /* < True if already exists in class (incl parents). */
} Eobj_Extension_Node;

struct _Eobj_Class
{
   Eobj_Class_Id class_id;
   const Eobj_Class *parent;
   const Eobj_Class_Description *desc;
   Dich_Chain1 chain[DICH_CHAIN1_SIZE];
   Eina_Inlist *extensions;
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

/* FIXME: Decide if it should be fast, and if so, add a mapping.
 * Otherwise, this is very slow. But since it's only for debugging... */
static const Eobj_Op_Description *
_eobj_op_id_desc_get(Eobj_Op op)
{
   int i;
   Eobj_Class **cls_itr = _eobj_classes;

   for (i = 0 ; i < _eobj_classes_last_id ; i++, cls_itr++)
     {
        if (*cls_itr)
          {
             const Eobj_Op_Description *desc = (*cls_itr)->desc->ops.descs;
             if (!desc)
                continue;

             Eobj_Op base_op_id = *(*cls_itr)->desc->ops.base_op_id;
             while (desc->sub_op)
               {
                  if ((base_op_id + desc->sub_op) == op)
                     return desc;
                  desc++;
               }
          }
     }

   return NULL;
}

static Eina_Bool
_eobj_op_internal(Eobj *obj, const Eobj_Class *obj_klass, Eobj_Op op, va_list *p_list)
{
   const Eobj_Class *klass = obj_klass;
   eobj_op_func_type func;

   if (!obj_klass)
      return EINA_FALSE;

   while (klass)
     {
        func = dich_func_get(klass, op);

        if (func)
          {
             func(obj, op, p_list);
             return EINA_TRUE;
          }
        else
          {
             klass = klass->parent;
          }
     }

   if (!klass)
     {
        klass = obj_klass;
        /* FIXME: Should probably flatten everything. to be faster. */
          {
             /* Try MIXINS */
             Eobj_Extension_Node *itr;
             EINA_INLIST_FOREACH(klass->extensions, itr)
               {
                  if (_eobj_op_internal(obj, itr->klass, op, p_list))
                    {
                       return EINA_TRUE;
                    }
               }
          }

        /* Try composite objects */
          {
             Eina_List *itr;
             Eobj *emb_obj;
             EINA_LIST_FOREACH(obj->composite_objects, itr, emb_obj)
               {
                  if (_eobj_op_internal(emb_obj, eobj_class_get(emb_obj), op, p_list))
                    {
                       return EINA_TRUE;
                    }
               }
          }

        /* If haven't found anything, return FALSE */
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static inline Eina_Bool
_eobj_ops_internal(Eobj *obj, const Eobj_Class *obj_klass, va_list *p_list)
{
   Eina_Bool ret = EINA_TRUE;
   Eobj_Op op = 0;

   op = va_arg(*p_list, Eobj_Op);
   while (op)
     {
        if (!_eobj_op_internal(obj, obj_klass, op, p_list))
          {
             const Eobj_Op_Description *desc = _eobj_op_id_desc_get(op);
             const char *_id_name = (desc) ? desc->name : NULL;
             const Eobj_Class *op_klass = OP_CLASS_GET(op);
             const char *_dom_name = (op_klass) ? op_klass->desc->name : NULL;
             ERR("Can't find func for op %x ('%s' of domain '%s') for class '%s'. Aborting.",
                   op, _id_name, _dom_name,
                   obj_klass->desc->name);
             ret = EINA_FALSE;
             break;
          }
        op = va_arg(*p_list, Eobj_Op);
     }

   return ret;
}

EAPI Eina_Bool
eobj_do_internal(Eobj *obj, ...)
{
   Eina_Bool ret;
   va_list p_list;
   va_start(p_list, obj);
   ret = _eobj_ops_internal(obj, eobj_class_get(obj), &p_list);
   va_end(p_list);
   return ret;
}

EAPI Eina_Bool
eobj_class_do_internal(Eobj *obj, const Eobj_Class *klass, ...)
{
   Eina_Bool ret;
   va_list p_list;
   va_start(p_list, klass);
   ret = _eobj_ops_internal(obj, klass, &p_list);
   va_end(p_list);
   return ret;
}

EAPI const Eobj_Class *
eobj_class_get(Eobj *obj)
{
   return obj->klass;
}

EAPI const Eobj_Class *
eobj_class_parent_get(const Eobj_Class *klass)
{
   return klass->parent;
}

EAPI const char *
eobj_class_name_get(const Eobj_Class *klass)
{
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

static void
_eobj_class_constructor(Eobj_Class *klass)
{
   if (klass->constructed)
      return;

   klass->constructed = EINA_TRUE;

   if (!klass->desc->class_constructor)
      return;

   klass->desc->class_constructor(klass);
}

EAPI void
eobj_class_funcs_set(Eobj_Class *klass, const Eobj_Op_Func_Description *func_descs)
{
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

static Eina_Bool
_eobj_class_extn_exists(const Eobj_Class *klass, const Eobj_Class *extn_cls)
{
   while (klass)
     {
        Eobj_Extension_Node *extn;
        EINA_INLIST_FOREACH(klass->extensions, extn)
          {
             if (extn->klass == extn_cls)
                return EINA_TRUE;
          }

        klass = klass->parent;
     }

   return EINA_FALSE;
}

EAPI Eobj_Class *
eobj_class_new(const Eobj_Class_Description *desc, const Eobj_Class *parent, ...)
{
   Eobj_Class *klass;
   va_list p_list;

   va_start(p_list, parent);

#define _CLS_NEW_CHECK(x) \
   do \
     { \
        if (!x) \
          { \
             ERR("%s can't be NULL! Aborting.", #x); \
             return NULL; \
          } \
     } \
   while(0)

   _CLS_NEW_CHECK(desc);
   _CLS_NEW_CHECK(desc->name);

   klass = calloc(1, sizeof(Eobj_Class));
   klass->parent = parent;
   klass->class_id = ++_eobj_classes_last_id;
     {
        /* FIXME: Handle errors. */
        Eobj_Class **tmp;
        tmp = realloc(_eobj_classes, _eobj_classes_last_id * sizeof(*_eobj_classes));
        _eobj_classes = tmp;
        _eobj_classes[klass->class_id - 1] = klass;
     }

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
                        node->exists = _eobj_class_extn_exists(klass, extn);
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
   _eobj_class_base_op_init(klass);

   /* FIXME: Shouldn't be called here - should be called from eobj_add. */
   _eobj_class_constructor(klass);

   va_end(p_list);

   return klass;
}
#undef _CLS_NEW_CHECK

EAPI void
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

   free(klass);
}

static int
_eobj_class_count_parents(const Eobj_Class *klass)
{
   int count = 0;

   for (count = 0 ; klass->parent ; klass = klass->parent)
      count++;

   return count;
}

EAPI Eobj *
eobj_add(const Eobj_Class *klass, Eobj *parent)
{
   if (klass->desc->type != EOBJ_CLASS_TYPE_REGULAR)
     {
        ERR("Class '%s' is not instantiate-able. Aborting.", klass->desc->name);
        return NULL;
     }

   Eobj *obj = calloc(1, sizeof(*obj));
   obj->klass = klass;
   obj->parent = parent;

   obj->refcount++;
     {
        size_t datas_count = 0;
        intptr_t offset = 0;
        size_t i;
        const Eobj_Class *kls_itr;
        void **pvt_itr;
        datas_count = _eobj_class_count_parents(klass) + 1;

        obj->datas = calloc(datas_count, sizeof(*(obj->datas)));

        /* Calculate all the offsets and set in the datas array. */
        pvt_itr = obj->datas + datas_count - 1;
        for (kls_itr = klass ; kls_itr->parent ; kls_itr = kls_itr->parent)
           {
              *pvt_itr = (void *) offset;

              /* FIXME: Make sure this alignment is enough. */
              offset += kls_itr->desc->private_size +
                 (sizeof(void *) -
                  (kls_itr->desc->private_size % sizeof(void *)));
              pvt_itr--;
           }

        /* Allocate the datas blob and update the offsets. */
        obj->data_blob = calloc(1, offset);

        pvt_itr = obj->datas;
        for (i = 0 ; i < datas_count ; i++)
          {
             *pvt_itr = ((char *) obj->data_blob) + (intptr_t) *pvt_itr;

              pvt_itr++;
          }
     }

   eobj_class_constructor(obj, klass);
   if (obj->kls_itr && obj->kls_itr->parent)
     {
        ERR("Type '%s' - Not all of the object constructors have been executed.", klass->desc->name);
        goto fail;
     }

   if (eobj_generic_data_get(obj, CONSTRUCT_ERROR_KEY))
     {
        ERR("Type '%s' - One of the object constructors have failed.", klass->desc->name);
        goto fail;
     }

   return obj;

fail:
   eobj_unref(obj);
   return NULL;
}

EAPI Eobj *
eobj_ref(Eobj *obj)
{
   obj->refcount++;
   return obj;
}

EAPI void
eobj_unref(Eobj *obj)
{
   if (--(obj->refcount) == 0)
     {
        const Eobj_Class *klass = eobj_class_get(obj);
        eobj_class_destructor(obj, klass);
        /*FIXME: add eobj_class_unref(klass) ? - just to clear the caches. */

        Eina_List *itr, *itr_n;
        Eobj *emb_obj;
        EINA_LIST_FOREACH_SAFE(obj->composite_objects, itr, itr_n, emb_obj)
          {
             eobj_del(emb_obj);
             obj->composite_objects =
                eina_list_remove_list(obj->composite_objects, itr);
          }

        _eobj_callback_remove_all(obj);

        if (obj->data_blob)
           free(obj->data_blob);
        free(obj->datas);

        _eobj_generic_data_del_all(obj);

        free(obj);
     }
}

EAPI void
eobj_del(Eobj *obj)
{
   obj->delete = EINA_TRUE;
   eobj_unref(obj);
}

EAPI Eobj *
eobj_parent_get(Eobj *obj)
{
   return obj->parent;
}

EAPI void
eobj_constructor_error_set(Eobj *obj)
{
   eobj_generic_data_set(obj, CONSTRUCT_ERROR_KEY, (void *) EINA_TRUE);
}

EAPI Eina_Bool
eobj_constructor_error_get(const Eobj *obj)
{
   return (intptr_t) eobj_generic_data_get(obj, CONSTRUCT_ERROR_KEY);
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
eobj_class_constructor(Eobj *obj, const Eobj_Class *klass)
{
   const Eobj_Extension_Node *extn;

   obj->kls_itr = klass;

   if (!klass)
      return;

   EINA_INLIST_FOREACH(klass->extensions, extn)
     {
        /* Only call if it's the first one in the class. */
        if (!extn->exists && extn->klass->desc->constructor)
           extn->klass->desc->constructor(obj);
     }

   if (klass->desc->constructor)
      klass->desc->constructor(obj);
   else
      _eobj_constructor_default(obj);
}

static void
eobj_class_destructor(Eobj *obj, const Eobj_Class *klass)
{
   const Eobj_Extension_Node *extn;

   obj->kls_itr = klass;

   if (!klass)
      return;

   if (klass->desc->destructor)
      klass->desc->destructor(obj);
   else
      _eobj_destructor_default(obj);

   EINA_INLIST_REVERSE_FOREACH(klass->extensions, extn)
     {
        /* Only call if it's the first one in the class. */
        if (!extn->exists && extn->klass->desc->destructor)
           extn->klass->desc->destructor(obj);
     }

}

EAPI void
eobj_constructor_super(Eobj *obj)
{
   if (obj->kls_itr->parent)
      eobj_class_constructor(obj, obj->kls_itr->parent);
}

EAPI void
eobj_destructor_super(Eobj *obj)
{
   if (obj->kls_itr->parent)
      eobj_class_destructor(obj, obj->kls_itr->parent);
}

EAPI void *
eobj_data_get(Eobj *obj, const Eobj_Class *klass)
{
   /* FIXME: Add a check that this is of the right klass and we don't seg.
    * Probably just return NULL. */
   return obj->datas[_eobj_class_count_parents(klass)];
}

typedef struct
{
   EINA_INLIST;
   Eina_Stringshare *key;
   void *data;
} Eobj_Generic_Data_Node;

static void
_eobj_generic_data_node_free(Eobj_Generic_Data_Node *node)
{
   eina_stringshare_del(node->key);
   free(node);
}

static void
_eobj_generic_data_del_all(Eobj *obj)
{
   Eina_Inlist *nnode;
   Eobj_Generic_Data_Node *node;

   EINA_INLIST_FOREACH_SAFE(obj->generic_data, nnode, node)
     {
        obj->generic_data = eina_inlist_remove(obj->generic_data,
              EINA_INLIST_GET(node));

        _eobj_generic_data_node_free(node);
     }
}

EAPI void *
eobj_generic_data_set(Eobj *obj, const char *key, const void *data)
{
   void *prev_data;
   Eobj_Generic_Data_Node *node;

   if (!key) return NULL;
   if (!data) return NULL;

   prev_data = eobj_generic_data_del(obj, key);

   node = malloc(sizeof(Eobj_Generic_Data_Node));
   node->key = eina_stringshare_add(key);
   node->data = (void *) data;
   obj->generic_data = eina_inlist_prepend(obj->generic_data,
         EINA_INLIST_GET(node));

   return prev_data;
}

EAPI void *
eobj_generic_data_get(const Eobj *obj, const char *key)
{
   Eobj_Generic_Data_Node *node;

   if (!key) return NULL;

   EINA_INLIST_FOREACH(obj->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             ((Eobj *) obj)->generic_data =
                eina_inlist_promote(obj->generic_data, EINA_INLIST_GET(node));
             return node->data;
          }
     }
   return NULL;
}

EAPI void *
eobj_generic_data_del(Eobj *obj, const char *key)
{
   Eobj_Generic_Data_Node *node;

   if (!key) return NULL;

   EINA_INLIST_FOREACH(obj->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             void *data;

             data = node->data;
             obj->generic_data = eina_inlist_remove(obj->generic_data,
                   EINA_INLIST_GET(node));
             _eobj_generic_data_node_free(node);
             return data;
          }
     }
   return NULL;
}

EAPI Eina_Bool
eobj_init(void)
{
   if (_eobj_init_count++ > 0)
      return EINA_TRUE;

   eina_init();

   _eobj_classes = NULL;
   _eobj_classes_last_id = 0;
   _eobj_log_dom = eina_log_domain_register("eobj", EINA_COLOR_LIGHTBLUE);
   if (_eobj_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eobj");
        return EINA_FALSE;
     }

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

   eina_log_domain_unregister(_eobj_log_dom);
   _eobj_log_dom = -1;

   eina_shutdown();
   return EINA_TRUE;
}

EAPI void
eobj_composite_object_attach(Eobj *obj, Eobj *emb_obj)
{
   eobj_ref(emb_obj);
   obj->composite_objects = eina_list_prepend(obj->composite_objects, emb_obj);
}

EAPI void
eobj_composite_object_detach(Eobj *obj, Eobj *emb_obj)
{
   obj->composite_objects = eina_list_remove(obj->composite_objects, emb_obj);
   eobj_unref(emb_obj);
}

EAPI Eina_Bool
eobj_composite_is(Eobj *emb_obj)
{
   Eobj *obj = eobj_parent_get(emb_obj);
   Eina_List *itr;
   Eobj *tmp;
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
eobj_event_callback_add(Eobj *obj,
      const Eobj_Event_Description *desc,
      Eobj_Event_Cb cb,
      const void *data)
{
   return eobj_event_callback_priority_add(obj, desc,
         EOBJ_CALLBACK_PRIORITY_DEFAULT, cb, data);
}

EAPI Eina_Bool
eobj_event_callback_priority_add(Eobj *obj,
      const Eobj_Event_Description *desc,
      Eobj_Callback_Priority priority,
      Eobj_Event_Cb func,
      const void *data)
{
   Eobj_Callback_Description *cb = calloc(1, sizeof(*cb));
   cb->event = desc;
   cb->func = func;
   cb->func_data = (void *) data;
   cb->priority = priority;
   obj->callbacks = eina_inlist_sorted_insert(obj->callbacks,
         EINA_INLIST_GET(cb), _callback_priority_cmp);

   eobj_event_callback_call(obj, EOBJ_SIG_CALLBACK_ADD, desc);

   return EINA_TRUE;
}

EAPI void *
eobj_event_callback_del(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Event_Cb func)
{
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
             goto end;
          }
     }

end:
   eobj_event_callback_call(obj, EOBJ_SIG_CALLBACK_DEL, desc);
   return ret;
}

EAPI void *
eobj_event_callback_del_full(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Event_Cb func, const void *user_data)
{
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
             goto end;
          }
     }

end:
   eobj_event_callback_call(obj, EOBJ_SIG_CALLBACK_DEL, desc);
   return ret;
}

EAPI Eina_Bool
eobj_event_callback_call(Eobj *obj, const Eobj_Event_Description *desc,
      const void *event_info)
{
   Eobj_Callback_Description *cb;

   obj->walking_list++;

   EINA_INLIST_FOREACH(obj->callbacks, cb)
     {
        if (!cb->delete_me  && (cb->event == desc))
          {
             /* Abort callback calling if the func says so. */
             if (!cb->func((void *) cb->func_data, obj, desc,
                      (void *) event_info))
               {
                  break;
               }
          }
        if (obj->delete)
          break;
     }
   obj->walking_list--;
   _eobj_callbacks_clear(obj);

   return EINA_TRUE;
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
   return eobj_event_callback_add(obj, desc, _eobj_event_forwarder_callback, new_obj);
}

EAPI Eina_Bool
eobj_event_callback_forwarder_del(Eobj *obj, const Eobj_Event_Description *desc, Eobj *new_obj)
{
   eobj_event_callback_del_full(obj, desc, _eobj_event_forwarder_callback, new_obj);
   return EINA_TRUE;
}

/* EOBJ_CLASS_BASE stuff */
static Eobj_Class *_my_class = NULL;

/* FIXME: Set proper type descriptions. */
EAPI const Eobj_Event_Description _EOBJ_SIG_CALLBACK_ADD =
   EOBJ_EVENT_DESCRIPTION("callback,add", "?", "Called when a callback was added.");
EAPI const Eobj_Event_Description _EOBJ_SIG_CALLBACK_DEL =
   EOBJ_EVENT_DESCRIPTION("callback,del", "?", "Called when a callback was deleted.");

static void
_constructor(Eobj *obj)
{
   DBG("%p - %s.", obj, _my_class->desc->name);
}

static void
_destructor(Eobj *obj)
{
   DBG("%p - %s.", obj, _my_class->desc->name);
}

EAPI const Eobj_Class *
eobj_base_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Class_Description class_desc = {
        "Eobj Base",
        EOBJ_CLASS_TYPE_REGULAR_NO_INSTANT,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        _constructor,
        _destructor,
        NULL,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, NULL, NULL);
}

