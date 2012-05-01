#include <Eina.h>

#include "Eo.h"
#include "eo_private.h"

#include "config.h"

EAPI Eo_Op EO_BASE_BASE_ID = EO_NOOP;

typedef struct
{
   Eina_Inlist *generic_data;
} Private_Data;

typedef struct
{
   EINA_INLIST;
   Eina_Stringshare *key;
   void *data;
   eo_base_data_free_func free_func;
} Eo_Generic_Data_Node;

static void
_eo_generic_data_node_free(Eo_Generic_Data_Node *node)
{
   eina_stringshare_del(node->key);
   if (node->free_func)
      node->free_func(node->data);
   free(node);
}

static void
_eo_generic_data_del_all(Private_Data *pd)
{
   Eina_Inlist *nnode;
   Eo_Generic_Data_Node *node = NULL;

   EINA_INLIST_FOREACH_SAFE(pd->generic_data, nnode, node)
     {
        pd->generic_data = eina_inlist_remove(pd->generic_data,
              EINA_INLIST_GET(node));

        _eo_generic_data_node_free(node);
     }
}

static void
_data_set(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   const char *key = va_arg(*list, const char *);
   const void *data = va_arg(*list, const void *);
   eo_base_data_free_func free_func = va_arg(*list, eo_base_data_free_func);

   Eo_Generic_Data_Node *node;

   if (!key) return;

   eo_do(obj, eo_base_data_del(key));

   node = malloc(sizeof(Eo_Generic_Data_Node));
   node->key = eina_stringshare_add(key);
   node->data = (void *) data;
   node->free_func = free_func;
   pd->generic_data = eina_inlist_prepend(pd->generic_data,
         EINA_INLIST_GET(node));
}

static void
_data_get(const Eo *obj EINA_UNUSED, const void *class_data, va_list *list)
{
   /* We don't really change it... */
   Private_Data *pd = (Private_Data *) class_data;
   const char *key = va_arg(*list, const char *);
   void **data = va_arg(*list, void **);
   Eo_Generic_Data_Node *node;

   if (!data) return;
   *data = NULL;

   if (!key) return;

   EINA_INLIST_FOREACH(pd->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             pd->generic_data =
                eina_inlist_promote(pd->generic_data, EINA_INLIST_GET(node));
             *data = node->data;
             return;
          }
     }
}

static void
_data_del(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   const char *key = va_arg(*list, const char *);

   Eo_Generic_Data_Node *node;

   if (!key) return;

   EINA_INLIST_FOREACH(pd->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             pd->generic_data = eina_inlist_remove(pd->generic_data,
                   EINA_INLIST_GET(node));
             _eo_generic_data_node_free(node);
             return;
          }
     }
}

/* Weak reference. */
static Eina_Bool
_eo_weak_ref_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo **wref = data;
   *wref = NULL;

   return EO_CALLBACK_CONTINUE;
}

static void
_wref_add(const Eo *obj, const void *class_data EINA_UNUSED, va_list *list)
{
   Eo **wref = va_arg(*list, Eo **);

   *wref = (Eo *) obj;
   /* FIXME: The cast and the one in the next func are both bad and should be
    * fixed once the event callback functions are fixed. */
   eo_event_callback_add((Eo *) obj, EO_EV_DEL, _eo_weak_ref_cb, wref);
}

static void
_wref_del(const Eo *obj, const void *class_data EINA_UNUSED, va_list *list)
{
   Eo **wref = va_arg(*list, Eo **);
   if (*wref != obj)
     {
        ERR("Wref is a weak ref to %p, while this function was called on %p.",
              *wref, obj);
        return;
     }
   eo_event_callback_del((Eo *) obj, EO_EV_DEL, _eo_weak_ref_cb, wref);
   *wref = NULL;
}

/* EOF Weak reference. */


/* EO_BASE_CLASS stuff */
#define MY_CLASS EO_BASE_CLASS

/* FIXME: Set proper type descriptions. */
EAPI const Eo_Event_Description _EO_EV_CALLBACK_ADD =
   EO_EVENT_DESCRIPTION("callback,add", "?", "A callback was added.");
EAPI const Eo_Event_Description _EO_EV_CALLBACK_DEL =
   EO_EVENT_DESCRIPTION("callback,del", "?", "A callback was deleted.");
EAPI const Eo_Event_Description _EO_EV_FREE =
   EO_EVENT_DESCRIPTION("free", "", "Obj is being freed.");
EAPI const Eo_Event_Description _EO_EV_DEL =
   EO_EVENT_DESCRIPTION("del", "", "Obj is being deleted.");

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));
}

static void
_destructor(Eo *obj, void *class_data)
{
   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));

   _eo_generic_data_del_all(class_data);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_SET), _data_set),
        EO_OP_FUNC_CONST(EO_BASE_ID(EO_BASE_SUB_ID_DATA_GET), _data_get),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_DEL), _data_del),
        EO_OP_FUNC_CONST(EO_BASE_ID(EO_BASE_SUB_ID_WREF_ADD), _wref_add),
        EO_OP_FUNC_CONST(EO_BASE_ID(EO_BASE_SUB_ID_WREF_DEL), _wref_del),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DATA_SET, "?", "Set data for key."),
     EO_OP_DESCRIPTION_CONST(EO_BASE_SUB_ID_DATA_GET, "?", "Get data for key."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DATA_DEL, "?", "Del key."),
     EO_OP_DESCRIPTION_CONST(EO_BASE_SUB_ID_WREF_ADD, "?", "Add a weak ref to the object."),
     EO_OP_DESCRIPTION_CONST(EO_BASE_SUB_ID_WREF_DEL, "?", "Delete the weak ref."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     EO_EV_CALLBACK_ADD,
     EO_EV_CALLBACK_DEL,
     EO_EV_FREE,
     EO_EV_DEL,
     NULL
};

static const Eo_Class_Description class_desc = {
     "Eo Base",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO_CLASS_DESCRIPTION_OPS(&EO_BASE_BASE_ID, op_desc, EO_BASE_SUB_ID_LAST),
     event_desc,
     sizeof(Private_Data),
     _constructor,
     _destructor,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(eo_base_class_get, &class_desc, NULL, NULL)

