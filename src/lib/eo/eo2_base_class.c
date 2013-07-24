#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "Eo.h"
#include "eo_ptr_indirection.h"
#include "eo_private.h"

static int event_freeze_count = 0;

typedef struct _Eo_Callback_Description Eo_Callback_Description;

typedef struct
{
   Eina_Inlist *generic_data;
   Eo ***wrefs;

   Eo_Callback_Description *callbacks;
   unsigned short walking_list;
   unsigned short event_freeze_count;
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
_data_set(Eo *obj, void *class_data,
          const char *key, const void *data, eo_base_data_free_func free_func)
{
   Private_Data *pd = class_data;

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
EAPI EO2_VOID_FUNC_BODYV(eo2_base_data_set, EO2_FUNC_CALL(key, data, free_func),
                        const char *key, const void *data, eo_base_data_free_func free_func);

static void *
_data_get(Eo *obj EINA_UNUSED, void *class_data, const char *key)
{
   /* We don't really change it... */
   Eo_Generic_Data_Node *node;
   Private_Data *pd = (Private_Data *) class_data;

   if (!key) return NULL;

   EINA_INLIST_FOREACH(pd->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             pd->generic_data =
                eina_inlist_promote(pd->generic_data, EINA_INLIST_GET(node));
             return node->data;
          }
     }

   return NULL;
}
EAPI EO2_VOID_FUNC_BODYV(eo2_base_data_get, EO2_FUNC_CALL(key), const char *key);

static void
_dbg_info_get(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED)
{  /* No info required in the meantime */
   return;
}
EAPI EO2_VOID_FUNC_BODY(eo2_dbg_info_get);

static void
_data_del(Eo *obj EINA_UNUSED, void *class_data, const char *key)
{
   Eo_Generic_Data_Node *node;
   Private_Data *pd = class_data;

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
EAPI EO2_VOID_FUNC_BODYV(eo2_base_data_del, EO2_FUNC_CALL(key), const char *key);

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
_wref_add(Eo *obj, void *class_data, Eo **wref)
{
   Private_Data *pd = (Private_Data *) class_data;
   size_t count;

   count = _wref_count(pd);
   count += 1; /* New wref. */

   pd->wrefs= realloc(pd->wrefs, sizeof(*pd->wrefs) * (count + 1));

   pd->wrefs[count - 1] = wref;
   pd->wrefs[count] = NULL;
   *wref = obj;
}
EAPI EO2_VOID_FUNC_BODYV(eo2_wref_add, EO2_FUNC_CALL(wref), Eo **wref);

static void
_wref_del(Eo *obj, void *class_data, Eo **wref)
{
   Private_Data *pd = (Private_Data *) class_data;
   size_t count;

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
EAPI EO2_VOID_FUNC_BODYV(eo2_wref_del, EO2_FUNC_CALL(wref), Eo **wref);

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

struct _Eo_Callback_Description
{
   Eo_Callback_Description *next;

   union
     {
        Eo_Callback_Array_Item item;
        const Eo_Callback_Array_Item *item_array;
     } items;

   void *func_data;
   Eo_Callback_Priority priority;

   Eina_Bool delete_me : 1;
   Eina_Bool func_array : 1;
};

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eo_callback_remove(Private_Data *pd, Eo_Callback_Description *cb)
{
   Eo_Callback_Description *itr, *pitr;

   itr = pitr = pd->callbacks;
   if (pd->callbacks == cb)
      pd->callbacks = cb->next;

   for ( ; itr ; )
     {
        Eo_Callback_Description *titr = itr;
        itr = itr->next;

        if (titr == cb)
          {
             if (pitr)
               {
                  pitr->next = titr->next;
               }
             free(titr);
          }
        else
          {
             pitr = titr;
          }
     }
}

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eo_callback_remove_all(Private_Data *pd)
{
   while (pd->callbacks)
     {
        Eo_Callback_Description *next = pd->callbacks->next;
        free(pd->callbacks);
        pd->callbacks = next;
     }
}

static void
_eo_callbacks_clear(Private_Data *pd)
{
   Eo_Callback_Description *cb = NULL;

   /* If there are no deletions waiting. */
   if (!pd->deletions_waiting)
      return;

   /* Abort if we are currently walking the list. */
   if (pd->walking_list > 0)
      return;

   pd->deletions_waiting = EINA_FALSE;

   for (cb = pd->callbacks ; cb ; )
     {
        Eo_Callback_Description *titr = cb;
        cb = cb->next;

        if (titr->delete_me)
          {
             _eo_callback_remove(pd, titr);
          }
     }
}

static void
_eo_callbacks_sorted_insert(Private_Data *pd, Eo_Callback_Description *cb)
{
   Eo_Callback_Description *itr, *itrp = NULL;
   for (itr = pd->callbacks ; itr && (itr->priority < cb->priority) ;
         itr = itr->next)
     {
        itrp = itr;
     }

   if (itrp)
     {
        cb->next = itrp->next;
        itrp->next = cb;
     }
   else
     {
        cb->next = pd->callbacks;
        pd->callbacks = cb;
     }
}

static void
_ev_cb_priority_add(Eo *obj, void *class_data,
                    const Eo_Event_Description *desc,
                    Eo_Callback_Priority priority,
                    Eo_Event_Cb func,
                    const void *user_data)
{
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   cb = calloc(1, sizeof(*cb));
   cb->items.item.desc = desc;
   cb->items.item.func = func;
   cb->func_data = (void *) user_data;
   cb->priority = priority;
   _eo_callbacks_sorted_insert(pd, cb);

     {
        const Eo_Callback_Array_Item arr[] = { {desc, func}, {NULL, NULL}};
        eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_ADD, arr, NULL));
     }
}
EAPI EO2_VOID_FUNC_BODYV(eo2_event_callback_priority_add,
                        EO2_FUNC_CALL(desc, priority, func, user_data),
                        const Eo_Event_Description *desc,
                        Eo_Callback_Priority priority,
                        Eo_Event_Cb func,
                        const void *user_data);

static void
_ev_cb_del(Eo *obj, void *class_data,
                    const Eo_Event_Description *desc,
                    Eo_Event_Cb func,
                    void *user_data)
{
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   for (cb = pd->callbacks ; cb ; cb = cb->next)
     {
        if ((cb->items.item.desc == desc) && (cb->items.item.func == func) &&
              (cb->func_data == user_data))
          {
             const Eo_Callback_Array_Item arr[] = { {desc, func}, {NULL, NULL}};

             cb->delete_me = EINA_TRUE;
             pd->deletions_waiting = EINA_TRUE;
             _eo_callbacks_clear(pd);
             eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_DEL, arr, NULL));
             return;
          }
     }

   DBG("Callback of object %p with function %p and data %p not found.", obj, func, user_data);
}
EAPI EO2_VOID_FUNC_BODYV(eo2_event_callback_del,
                        EO2_FUNC_CALL(desc, func, user_data),
                        const Eo_Event_Description *desc,
                        Eo_Event_Cb func,
                        const void *user_data);

static void
_ev_cb_array_priority_add(Eo *obj, void *class_data,
                          const Eo_Callback_Array_Item *array,
                          Eo_Callback_Priority priority,
                          const void *user_data)
{
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   cb = calloc(1, sizeof(*cb));
   cb->func_data = (void *) user_data;
   cb->priority = priority;
   cb->items.item_array = array;
   cb->func_array = EINA_TRUE;
   _eo_callbacks_sorted_insert(pd, cb);

     {
        eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_ADD, array, NULL));
     }
}
EAPI EO2_VOID_FUNC_BODYV(eo2_event_callback_array_priority_add,
                        EO2_FUNC_CALL(array, priority, user_data),
                        const Eo_Callback_Array_Item *array,
                        Eo_Callback_Priority priority,
                        const void *user_data);

static void
_ev_cb_array_del(Eo *obj, void *class_data,
                 const Eo_Callback_Array_Item *array,
                 void *user_data)
{
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   for (cb = pd->callbacks ; cb ; cb = cb->next)
     {
        if ((cb->items.item_array == array) && (cb->func_data == user_data))
          {
             cb->delete_me = EINA_TRUE;
             pd->deletions_waiting = EINA_TRUE;
             _eo_callbacks_clear(pd);

             eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_DEL, array, NULL));
             return;
          }
     }

   DBG("Callback of object %p with function array %p and data %p not found.", obj, array, user_data);
}
EAPI EO2_VOID_FUNC_BODYV(eo2_event_callback_array_del,
                        EO2_FUNC_CALL(array, user_data),
                        const Eo_Callback_Array_Item *array,
                        const void *user_data);

static Eina_Bool
_ev_cb_call(Eo *obj_id, void *class_data,
            const Eo_Event_Description *desc,
            void *event_info)
{
   Eina_Bool ret;
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

   ret = EINA_TRUE;

   _eo_ref(obj);
   pd->walking_list++;

   for (cb = pd->callbacks ; cb ; cb = cb->next)
     {
        if (!cb->delete_me)
          {
             if (cb->func_array)
               {
                  const Eo_Callback_Array_Item *it;

                  for (it = cb->items.item_array ; it->func ; it++)
                    {
                       if (it->desc != desc)
                          continue;
                       if (!it->desc->unfreezable &&
                           (event_freeze_count || pd->event_freeze_count))
                          continue;

                       /* Abort callback calling if the func says so. */
                       if (!it->func((void *) cb->func_data, obj_id, desc,
                                (void *) event_info))
                         {
                            ret = EINA_FALSE;
                            goto end;
                         }
                    }
               }
             else
               {
                  if (cb->items.item.desc != desc)
                    continue;
                  if ((!cb->items.item.desc
                       || !cb->items.item.desc->unfreezable) &&
                      (event_freeze_count || pd->event_freeze_count))
                    continue;

                  /* Abort callback calling if the func says so. */
                  if (!cb->items.item.func((void *) cb->func_data, obj_id, desc,
                                           (void *) event_info))
                    {
                       ret = EINA_FALSE;
                       goto end;
                    }
               }
          }
     }

end:
   pd->walking_list--;
   _eo_callbacks_clear(pd);
   _eo_unref(obj);

   return ret;
}
EAPI EO2_FUNC_BODYV(eo2_event_callback_call, Eina_Bool,
                   EO2_FUNC_CALL(desc, event_info),
                   EINA_FALSE,
                   const Eo_Event_Description *desc,
                   void *event_info);

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
_ev_cb_forwarder_add(Eo *obj, void *class_data EINA_UNUSED,
                     const Eo_Event_Description *desc,
                     Eo *new_obj)
{

   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   eo_do(obj, eo_event_callback_add(desc, _eo_event_forwarder_callback, new_obj));
}
EAPI EO2_VOID_FUNC_BODYV(eo2_event_callback_forwarder_add,
                        EO2_FUNC_CALL(desc, new_obj),
                        const Eo_Event_Description *desc,
                        Eo *new_obj);

static void
_ev_cb_forwarder_del(Eo *obj, void *class_data EINA_UNUSED,
                     const Eo_Event_Description *desc,
                     Eo *new_obj)
{

   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   eo_do(obj, eo_event_callback_del(desc, _eo_event_forwarder_callback, new_obj));
}
EAPI EO2_VOID_FUNC_BODYV(eo2_event_callback_forwarder_del,
                        EO2_FUNC_CALL(desc, new_obj),
                        const Eo_Event_Description *desc,
                        Eo *new_obj);

static void
_ev_freeze(Eo *obj EINA_UNUSED, void *class_data)
{
   Private_Data *pd = (Private_Data *) class_data;
   pd->event_freeze_count++;
}
EAPI EO2_VOID_FUNC_BODY(eo2_event_freeze);

static void
_ev_thaw(Eo *obj, void *class_data)
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
EAPI EO2_VOID_FUNC_BODY(eo2_event_thaw);

static int
_ev_freeze_get(Eo *obj EINA_UNUSED, void *class_data)
{
   Private_Data *pd = (Private_Data *) class_data;

   return pd->event_freeze_count;
}
EAPI EO2_FUNC_BODY(eo2_event_freeze_get, int, 0);

static void
_ev_global_freeze(const Eo_Class *klass EINA_UNUSED)
{
   event_freeze_count++;
}
EAPI EO2_VOID_FUNC_BODY(eo2_event_global_freeze);

static void
_ev_global_thaw(const Eo_Class *klass EINA_UNUSED)
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
EAPI EO2_VOID_FUNC_BODY(eo2_event_global_thaw);

static int
_ev_global_freeze_get(const Eo_Class *klass EINA_UNUSED)
{
   return event_freeze_count;
}
EAPI EO2_FUNC_BODY(eo2_event_global_freeze_get, int, 0);

/* Eo_Dbg */
EAPI void
eo2_dbg_info_free(Eo_Dbg_Info *info)
{
   eina_value_flush(&(info->value));
   free(info);
}

static Eina_Bool
_eo_dbg_info_setup(const Eina_Value_Type *type, void *mem)
{
   memset(mem, 0, type->value_size);
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_flush(const Eina_Value_Type *type EINA_UNUSED, void *_mem)
{
   Eo_Dbg_Info *mem = *(Eo_Dbg_Info **) _mem;
   eina_stringshare_del(mem->name);
   eina_value_flush(&(mem->value));
   free(mem);
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_copy(const Eina_Value_Type *type EINA_UNUSED, const void *_src, void *_dst)
{
   const Eo_Dbg_Info **src = (const Eo_Dbg_Info **) _src;
   Eo_Dbg_Info **dst = _dst;
   *dst = calloc(1, sizeof(Eo_Dbg_Info));
   (*dst)->name = eina_stringshare_ref((*src)->name);
   eina_value_copy(&((*src)->value), &((*dst)->value));
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   /* FIXME: For the meanwhile, just use the inner type for the value. */
   const Eo_Dbg_Info **src = (const Eo_Dbg_Info **) type_mem;
   if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
       convert == EINA_VALUE_TYPE_STRING)
     {
        Eina_Bool ret;
        const char *other_mem;
        char *inner_val = eina_value_to_string(&(*src)->value);
        other_mem = inner_val;
        ret = eina_value_type_pset(convert, convert_mem, &other_mem);
        free(inner_val);
        return ret;
     }

   eina_error_set(EINA_ERROR_VALUE_FAILED);
   return EINA_FALSE;
}

static Eina_Bool
_eo_dbg_info_pset(const Eina_Value_Type *type EINA_UNUSED, void *_mem, const void *_ptr)
{
   Eo_Dbg_Info **mem = _mem;
   if (*mem)
     free(*mem);
   *mem = (void *) _ptr;
   return EINA_TRUE;
}

static Eina_Bool
_eo_dbg_info_pget(const Eina_Value_Type *type EINA_UNUSED, const void *_mem, void *_ptr)
{
   Eo_Dbg_Info **ptr = _ptr;
   *ptr = (void *) _mem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EO2_DBG_INFO_TYPE = {
   EINA_VALUE_TYPE_VERSION,
   sizeof(Eo_Dbg_Info *),
   "Eo_Dbg_Info_Ptr",
   _eo_dbg_info_setup,
   _eo_dbg_info_flush,
   _eo_dbg_info_copy,
   NULL,
   _eo_dbg_info_convert_to,
   NULL,
   NULL,
   _eo_dbg_info_pset,
   _eo_dbg_info_pget
};

EAPI const Eina_Value_Type *EO2_DBG_INFO_TYPE = &_EO2_DBG_INFO_TYPE;


/* EOF event callbacks */


/* EO_BASE_CLASS stuff */
#define MY_CLASS EO_BASE_CLASS

/* FIXME: Set proper type descriptions. */
// FIXME: eo2 multiple definition
/* EAPI const Eo_Event_Description _EO_EV_CALLBACK_ADD = */
/*    EO_EVENT_DESCRIPTION("callback,add", "A callback was added."); */
/* EAPI const Eo_Event_Description _EO_EV_CALLBACK_DEL = */
/*    EO_EVENT_DESCRIPTION("callback,del", "A callback was deleted."); */
/* EAPI const Eo_Event_Description _EO_EV_DEL = */
/*    EO_HOT_EVENT_DESCRIPTION("del", "Obj is being deleted."); */

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));

   _eo_condtor_done(obj);
}
EAPI EO2_VOID_FUNC_BODY(eo2_constructor);

static void
_destructor(Eo *obj, void *class_data)
{
   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));

   _eo_generic_data_del_all(class_data);
   _wref_destruct(class_data);
   _eo_callback_remove_all(class_data);

   _eo_condtor_done(obj);
}
EAPI EO2_VOID_FUNC_BODY(eo2_destructor);

static void
_class_constructor(Eo_Class *klass)
{
   event_freeze_count = 0;
   eo2_class_funcs_set(klass);
}

Eo2_Op_Description op_descs [] = {
       EO2_OP_FUNC(_constructor, eo2_constructor, "Constructor."),
       EO2_OP_FUNC(_destructor, eo2_destructor, "Destructor."),
       EO2_OP_FUNC(_data_set, eo2_base_data_set, "Set data for key."),
       EO2_OP_FUNC(_data_get, eo2_base_data_get, "Get data for key."),
       EO2_OP_FUNC(_data_del, eo2_base_data_del, "Del key."),
       EO2_OP_FUNC(_wref_add, eo2_wref_add, "Add a weak ref to the object."),
       EO2_OP_FUNC(_wref_del, eo2_wref_del, "Delete the weak ref."),
       EO2_OP_FUNC(_ev_cb_priority_add, eo2_event_callback_priority_add, "Add an event callback with a priority."),
       EO2_OP_FUNC(_ev_cb_del, eo2_event_callback_del, "Delete an event callback"),
       EO2_OP_FUNC(_ev_cb_array_priority_add, eo2_event_callback_array_priority_add, "Add an event callback array with a priority."),
       EO2_OP_FUNC(_ev_cb_array_del, eo2_event_callback_array_del, "Delete an event callback array"),
       EO2_OP_FUNC(_ev_cb_call, eo2_event_callback_call, "Call the event callbacks for an event."),
       EO2_OP_FUNC(_ev_cb_forwarder_add, eo2_event_callback_forwarder_add, "Add an event forwarder."),
       EO2_OP_FUNC(_ev_cb_forwarder_del, eo2_event_callback_forwarder_del, "Delete an event forwarder."),
       EO2_OP_CLASS_FUNC(_ev_freeze, eo2_event_freeze, "Freezes events."),
       EO2_OP_CLASS_FUNC(_ev_thaw, eo2_event_thaw, "Thaws events."),
       EO2_OP_CLASS_FUNC(_ev_freeze_get, eo2_event_freeze_get, "Get event freeze counter."),
       EO2_OP_FUNC(_ev_global_freeze, eo2_event_global_freeze, "Freezes events globally."),
       EO2_OP_FUNC(_ev_global_thaw, eo2_event_global_thaw, "Thaws events globally."),
       EO2_OP_FUNC(_ev_global_freeze_get, eo2_event_global_freeze_get, "Get global event freeze counter."),
       EO2_OP_FUNC(_dbg_info_get, eo2_dbg_info_get, "Get debug info list for obj."),
       EO2_OP_SENTINEL
};

// FIXME: eo2
static const Eo_Event_Description *event_desc[] = {
     EO_EV_CALLBACK_ADD,
     EO_EV_CALLBACK_DEL,
     EO_EV_DEL,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Eo Base",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO2_CLASS_DESCRIPTION_OPS(op_descs, OP_DESC_SIZE(op_descs)),
     event_desc,
     sizeof(Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(eo2_base_class_get, &class_desc, NULL, NULL)
