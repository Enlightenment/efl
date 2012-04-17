#include <Eina.h>

#include "Eobj.h"
#include "eobj_private.h"

#include "config.h"

EAPI Eobj_Op EOBJ_BASE_BASE_ID = EOBJ_NOOP;

typedef struct
{
   Eina_Inlist *generic_data;
} Private_Data;

typedef struct
{
   EINA_INLIST;
   Eina_Stringshare *key;
   void *data;
   eobj_base_data_free_func free_func;
} Eobj_Generic_Data_Node;

static void
_eobj_generic_data_node_free(Eobj_Generic_Data_Node *node)
{
   eina_stringshare_del(node->key);
   if (node->free_func)
      node->free_func(node->data);
   free(node);
}

static void
_eobj_generic_data_del_all(Private_Data *pd)
{
   Eina_Inlist *nnode;
   Eobj_Generic_Data_Node *node;

   EINA_INLIST_FOREACH_SAFE(pd->generic_data, nnode, node)
     {
        pd->generic_data = eina_inlist_remove(pd->generic_data,
              EINA_INLIST_GET(node));

        _eobj_generic_data_node_free(node);
     }
}

static void
_data_set(Eobj *obj, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   const char *key = va_arg(*list, const char *);
   const void *data = va_arg(*list, const void *);
   eobj_base_data_free_func free_func = va_arg(*list, eobj_base_data_free_func);

   Eobj_Generic_Data_Node *node;

   if (!key) return;

   eobj_do(obj, EOBJ_BASE_DATA_DEL(key));

   node = malloc(sizeof(Eobj_Generic_Data_Node));
   node->key = eina_stringshare_add(key);
   node->data = (void *) data;
   node->free_func = free_func;
   pd->generic_data = eina_inlist_prepend(pd->generic_data,
         EINA_INLIST_GET(node));
}

static void
_data_get(Eobj *obj __UNUSED__, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   const char *key = va_arg(*list, const char *);
   void **data = va_arg(*list, void **);
   Eobj_Generic_Data_Node *node;

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
_data_del(Eobj *obj __UNUSED__, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   const char *key = va_arg(*list, const char *);

   Eobj_Generic_Data_Node *node;

   if (!key) return;

   EINA_INLIST_FOREACH(pd->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             pd->generic_data = eina_inlist_remove(pd->generic_data,
                   EINA_INLIST_GET(node));
             _eobj_generic_data_node_free(node);
             return;
          }
     }
}

/* EOBJ_BASE_CLASS stuff */
static const Eobj_Class *_my_class = NULL;

/* FIXME: Set proper type descriptions. */
EAPI const Eobj_Event_Description _EOBJ_EV_CALLBACK_ADD =
   EOBJ_EVENT_DESCRIPTION("callback,add", "?", "A callback was added.");
EAPI const Eobj_Event_Description _EOBJ_EV_CALLBACK_DEL =
   EOBJ_EVENT_DESCRIPTION("callback,del", "?", "A callback was deleted.");
EAPI const Eobj_Event_Description _EOBJ_EV_FREE =
   EOBJ_EVENT_DESCRIPTION("free", "", "Obj is being freed.");
EAPI const Eobj_Event_Description _EOBJ_EV_DEL =
   EOBJ_EVENT_DESCRIPTION("del", "", "Obj is being deleted.");

static void
_constructor(Eobj *obj, void *class_data __UNUSED__)
{
   DBG("%p - %s.", obj, eobj_class_name_get(_my_class));
}

static void
_destructor(Eobj *obj, void *class_data)
{
   DBG("%p - %s.", obj, eobj_class_name_get(_my_class));

   _eobj_generic_data_del_all(class_data);
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC(EOBJ_BASE_ID(EOBJ_BASE_SUB_ID_DATA_SET), _data_set),
        EOBJ_OP_FUNC(EOBJ_BASE_ID(EOBJ_BASE_SUB_ID_DATA_GET), _data_get),
        EOBJ_OP_FUNC(EOBJ_BASE_ID(EOBJ_BASE_SUB_ID_DATA_DEL), _data_del),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

EAPI const Eobj_Class *
eobj_base_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(EOBJ_BASE_SUB_ID_DATA_SET, "?", "Set data for key."),
        EOBJ_OP_DESCRIPTION(EOBJ_BASE_SUB_ID_DATA_GET, "?", "Get data for key."),
        EOBJ_OP_DESCRIPTION(EOBJ_BASE_SUB_ID_DATA_DEL, "?", "Del key."),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Event_Description *event_desc[] = {
        EOBJ_EV_CALLBACK_ADD,
        EOBJ_EV_CALLBACK_DEL,
        EOBJ_EV_FREE,
        EOBJ_EV_DEL,
        NULL
   };

   static const Eobj_Class_Description class_desc = {
        "Eobj Base",
        EOBJ_CLASS_TYPE_REGULAR_NO_INSTANT,
        EOBJ_CLASS_DESCRIPTION_OPS(&EOBJ_BASE_BASE_ID, op_desc, EOBJ_BASE_SUB_ID_LAST),
        event_desc,
        sizeof(Private_Data),
        _constructor,
        _destructor,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, NULL, NULL);
}

