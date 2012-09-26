#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "Eo.h"
#include "eo_private.h"

EAPI Eo_Op EO_BASE_BASE_ID = 0;

static int event_freeze_count = 0;

typedef struct
{
   Eina_Inlist *generic_data;
   Eo ***wrefs;

   Eina_Inlist *callbacks;
   int walking_list;
   int event_freeze_count;
   Eina_Bool deletions_waiting : 1;
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
_data_get(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
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

static inline size_t
_wref_count(Private_Data *pd)
{
   size_t count = 0;
   if (!pd->wrefs)
      return 0;

   Eo ***itr;
   for (itr = pd->wrefs ; *itr ; itr++)
      count++;

   return count;
}

static void
_wref_add(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = (Private_Data *) class_data;
   size_t count;
   Eo **wref = va_arg(*list, Eo **);

   count = _wref_count(pd);
   count += 1; /* New wref. */

   pd->wrefs= realloc(pd->wrefs, sizeof(*pd->wrefs) * (count + 1));

   pd->wrefs[count - 1] = wref;
   pd->wrefs[count] = NULL;
   *wref = obj;
}

static void
_wref_del(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = (Private_Data *) class_data;
   size_t count;
   Eo **wref = va_arg(*list, Eo **);
   if (*wref != obj)
     {
        ERR("Wref is a weak ref to %p, while this function was called on %p.",
              *wref, obj);
        return;
     }

   if (!pd->wrefs)
     {
        ERR("There are no weak refs for object %p", obj);
        *wref = NULL;
        return;
     }

   /* Move the last item in the array instead of the current wref. */
   count = _wref_count(pd);

     {
        Eo ***itr;
        for (itr = pd->wrefs ; *itr ; itr++)
          {
             if (*itr == wref)
               {
                  *itr = pd->wrefs[count - 1];
                  break;
               }
          }

        if (!*itr)
          {
             ERR("Wref %p is not associated with object %p", wref, obj);
             *wref = NULL;
             return;
          }
     }

   if (count > 1)
     {
        // No count--; because of the NULL that is not included in the count. */
        pd->wrefs = realloc(pd->wrefs, sizeof(*pd->wrefs) * count);
        pd->wrefs[count - 1] = NULL;
     }
   else
     {
        free(pd->wrefs);
        pd->wrefs = NULL;
     }

   *wref = NULL;
}

static inline void
_wref_destruct(Private_Data *pd)
{
   Eo ***itr;
   if (!pd->wrefs)
      return;

   for (itr = pd->wrefs ; *itr ; itr++)
     {
        **itr = NULL;
     }

   free(pd->wrefs);
}

/* EOF Weak reference. */

/* Event callbacks */

/* Callbacks */

typedef struct
{
   EINA_INLIST;
   const Eo_Event_Description *event;
   Eo_Event_Cb func;
   void *func_data;
   Eo_Callback_Priority priority;
   Eina_Bool delete_me : 1;
} Eo_Callback_Description;

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eo_callback_remove(Private_Data *pd, Eo_Callback_Description *cb)
{
   pd->callbacks = eina_inlist_remove(pd->callbacks,
         EINA_INLIST_GET(cb));
   free(cb);
}

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eo_callback_remove_all(Private_Data *pd)
{
   Eina_Inlist *initr;
   Eo_Callback_Description *cb = NULL;
   EINA_INLIST_FOREACH_SAFE(pd->callbacks, initr, cb)
     {
        _eo_callback_remove(pd, cb);
     }
}

static void
_eo_callbacks_clear(Private_Data *pd)
{
   Eina_Inlist *itn;
   Eo_Callback_Description *cb = NULL;

   /* Abort if we are currently walking the list. */
   if (pd->walking_list > 0)
      return;

   /* If there are no deletions waiting. */
   if (!pd->deletions_waiting)
      return;

   pd->deletions_waiting = EINA_FALSE;

   EINA_INLIST_FOREACH_SAFE(pd->callbacks, itn, cb)
     {
        if (cb->delete_me)
          {
             _eo_callback_remove(pd, cb);
          }
     }
}

static int
_callback_priority_cmp(const void *_a, const void *_b)
{
   const Eo_Callback_Description *a, *b;
   a = (const Eo_Callback_Description *) _a;
   b = (const Eo_Callback_Description *) _b;
   if (a->priority < b->priority)
      return -1;
   else
      return 1;
}

static void
_ev_cb_priority_add(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = (Private_Data *) class_data;
   const Eo_Event_Description *desc = va_arg(*list, const Eo_Event_Description *);
   Eo_Callback_Priority priority = va_arg(*list, int);
   Eo_Event_Cb func = va_arg(*list, Eo_Event_Cb);
   const void *data = va_arg(*list, const void *);

   Eo_Callback_Description *cb = calloc(1, sizeof(*cb));
   cb->event = desc;
   cb->func = func;
   cb->func_data = (void *) data;
   cb->priority = priority;
   pd->callbacks = eina_inlist_sorted_insert(pd->callbacks,
         EINA_INLIST_GET(cb), _callback_priority_cmp);

   eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_ADD, desc, NULL));
}

static void
_ev_cb_del(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = (Private_Data *) class_data;
   const Eo_Event_Description *desc = va_arg(*list, const Eo_Event_Description *);
   Eo_Event_Cb func = va_arg(*list, Eo_Event_Cb);
   void *user_data = va_arg(*list, void *);

   Eo_Callback_Description *cb;
   EINA_INLIST_FOREACH(pd->callbacks, cb)
     {
        if ((cb->event == desc) && (cb->func == func) &&
              (cb->func_data == user_data))
          {
             cb->delete_me = EINA_TRUE;
             pd->deletions_waiting = EINA_TRUE;
             _eo_callbacks_clear(pd);
             eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_DEL, desc, NULL));
             return;
          }
     }

   ERR("Callback of object %p with function %p and data %p not found.", obj, func, user_data);
}

static void
_ev_cb_call(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = (Private_Data *) class_data;
   const Eo_Event_Description *desc = va_arg(*list, const Eo_Event_Description *);
   void *event_info = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   Eo_Callback_Description *cb;

   if (ret) *ret = EINA_TRUE;

   if (event_freeze_count || pd->event_freeze_count)
      return;

   /* FIXME: Change eo_ref to _eo_ref and unref. */
   eo_ref(obj);
   pd->walking_list++;

   EINA_INLIST_FOREACH(pd->callbacks, cb)
     {
        if (!cb->delete_me && (cb->event == desc))
          {
             /* Abort callback calling if the func says so. */
             if (!cb->func((void *) cb->func_data, obj, desc,
                      (void *) event_info))
               {
                  if (ret) *ret = EINA_FALSE;
                  break;
               }
          }
     }
   pd->walking_list--;
   _eo_callbacks_clear(pd);
   eo_unref(obj);
}

static Eina_Bool
_eo_event_forwarder_callback(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   (void) obj;
   Eo *new_obj = (Eo *) data;
   Eina_Bool ret;
   eo_do(new_obj, eo_event_callback_call(desc, event_info, &ret));
   return ret;
}

/* FIXME: Change default priority? Maybe call later? */
static void
_ev_cb_forwarder_add(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   const Eo_Event_Description *desc = va_arg(*list, const Eo_Event_Description *);
   Eo *new_obj = va_arg(*list, Eo *);
   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   eo_do(obj, eo_event_callback_add(desc, _eo_event_forwarder_callback, new_obj));
}

static void
_ev_cb_forwarder_del(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   const Eo_Event_Description *desc = va_arg(*list, const Eo_Event_Description *);
   Eo *new_obj = va_arg(*list, Eo *);
   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   eo_do(obj, eo_event_callback_del(desc, _eo_event_forwarder_callback, new_obj));
}

static void
_ev_freeze(Eo *obj EINA_UNUSED, void *class_data, va_list *list EINA_UNUSED)
{
   Private_Data *pd = (Private_Data *) class_data;
   pd->event_freeze_count++;
}

static void
_ev_thaw(Eo *obj, void *class_data, va_list *list EINA_UNUSED)
{
   Private_Data *pd = (Private_Data *) class_data;
   if (pd->event_freeze_count > 0)
     {
        pd->event_freeze_count--;
     }
   else
     {
        ERR("Events for object %p have already been thawed.", obj);
     }
}

static void
_ev_freeze_get(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Private_Data *pd = (Private_Data *) class_data;
   int *ret = va_arg(*list, int *);
   *ret = pd->event_freeze_count;
}

static void
_ev_global_freeze(const Eo_Class *klass EINA_UNUSED, va_list *list EINA_UNUSED)
{
   event_freeze_count++;
}

static void
_ev_global_thaw(const Eo_Class *klass EINA_UNUSED, va_list *list EINA_UNUSED)
{
   if (event_freeze_count > 0)
     {
        event_freeze_count--;
     }
   else
     {
        ERR("Global events have already been thawed.");
     }
}

static void
_ev_global_freeze_get(const Eo_Class *klass EINA_UNUSED, va_list *list)
{
   int *ret = va_arg(*list, int *);
   *ret = event_freeze_count;
}


/* EOF event callbacks */


/* EO_BASE_CLASS stuff */
#define MY_CLASS EO_BASE_CLASS

/* FIXME: Set proper type descriptions. */
EAPI const Eo_Event_Description _EO_EV_CALLBACK_ADD =
   EO_EVENT_DESCRIPTION("callback,add", "A callback was added.");
EAPI const Eo_Event_Description _EO_EV_CALLBACK_DEL =
   EO_EVENT_DESCRIPTION("callback,del", "A callback was deleted.");
EAPI const Eo_Event_Description _EO_EV_DEL =
   EO_EVENT_DESCRIPTION("del", "Obj is being deleted.");

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));

   _eo_condtor_done(obj);
}

static void
_destructor(Eo *obj, void *class_data, va_list *list EINA_UNUSED)
{
   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));

   _eo_generic_data_del_all(class_data);
   _wref_destruct(class_data);
   _eo_callback_remove_all(class_data);

   _eo_condtor_done(obj);
}

static void
_class_constructor(Eo_Class *klass)
{
   event_freeze_count = 0;

   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_SET), _data_set),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_GET), _data_get),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_DEL), _data_del),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_WREF_ADD), _wref_add),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_WREF_DEL), _wref_del),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_PRIORITY_ADD), _ev_cb_priority_add),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_DEL), _ev_cb_del),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_CALL), _ev_cb_call),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_ADD), _ev_cb_forwarder_add),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_DEL), _ev_cb_forwarder_del),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_FREEZE), _ev_freeze),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_THAW), _ev_thaw),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_FREEZE_GET), _ev_freeze_get),
        EO_OP_FUNC_CLASS(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE), _ev_global_freeze),
        EO_OP_FUNC_CLASS(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_GLOBAL_THAW), _ev_global_thaw),
        EO_OP_FUNC_CLASS(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE_GET), _ev_global_freeze_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_CONSTRUCTOR, "Constructor"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DESTRUCTOR, "Destructor"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DATA_SET, "Set data for key."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DATA_GET, "Get data for key."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DATA_DEL, "Del key."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_WREF_ADD, "Add a weak ref to the object."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_WREF_DEL, "Delete the weak ref."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_PRIORITY_ADD, "Add an event callback with a priority."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_DEL, "Delete an event callback"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_CALL, "Call the event callbacks for an event."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_ADD, "Add an event forwarder."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_DEL, "Delete an event forwarder."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_FREEZE, "Freezes events."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_THAW, "Thaws events."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_FREEZE_GET, "Get event freeze counter."),
     EO_OP_DESCRIPTION_CLASS(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE, "Freezes events globally."),
     EO_OP_DESCRIPTION_CLASS(EO_BASE_SUB_ID_EVENT_GLOBAL_THAW, "Thaws events globally."),
     EO_OP_DESCRIPTION_CLASS(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE_GET, "Get global event freeze counter."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     EO_EV_CALLBACK_ADD,
     EO_EV_CALLBACK_DEL,
     EO_EV_DEL,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Eo Base",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO_CLASS_DESCRIPTION_OPS(&EO_BASE_BASE_ID, op_desc, EO_BASE_SUB_ID_LAST),
     event_desc,
     sizeof(Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(eo_base_class_get, &class_desc, NULL, NULL)
