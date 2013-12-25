#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "Eo.h"
#include "eo_ptr_indirection.h"
#include "eo_private.h"

EAPI Eo_Op EO_BASE_BASE_ID = 0;

static int event_freeze_count = 0;

typedef struct _Eo_Callback_Description Eo_Callback_Description;

typedef struct
{
   Eina_List *children;
   Eo *parent;

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
_data_set(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;

   EO_PARAMETER_GET(const char *, key, list);
   EO_PARAMETER_GET(const void *, data, list);
   EO_PARAMETER_GET(eo_base_data_free_func, free_func, list);

   Eo_Generic_Data_Node *node;

   if (!key) return;

   eo_do(obj, eo_base_data_del(key));

   node = malloc(sizeof(Eo_Generic_Data_Node));
   if (!node) return;
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
   Eo_Generic_Data_Node *node;
   Private_Data *pd = (Private_Data *) class_data;

   EO_PARAMETER_GET(const char *, key, list);
   EO_PARAMETER_GET(void **, data, list);

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
_parent_set(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = (Private_Data *) class_data;

   EO_PARAMETER_GET(Eo *, parent_id, list);

   if (pd->parent == parent_id)
     return ;

   if (eo_composite_is(obj) && pd->parent)
     {
        eo_composite_detach(obj, pd->parent);
     }

   if (pd->parent)
     {
        Private_Data *old_parent_pd;

        old_parent_pd = eo_data_scope_get(pd->parent, EO_BASE_CLASS);
        if (old_parent_pd)
          {
             old_parent_pd->children = eina_list_remove(old_parent_pd->children,
                   obj);
          }
        else
          {
             ERR("CONTACT DEVS!!! SHOULD NEVER HAPPEN!!! Old parent %p for object %p is not a valid Eo object.",
                 pd->parent, obj);
          }

        eo_xunref(obj, pd->parent);
     }

   /* Set new parent */
   if (parent_id)
     {
        Private_Data *parent_pd = NULL;
        parent_pd = eo_data_scope_get(parent_id, EO_BASE_CLASS);

        if (EINA_LIKELY(parent_pd != NULL))
          {
             pd->parent = parent_id;
             parent_pd->children = eina_list_append(parent_pd->children,
                   obj);
             eo_xref(obj, pd->parent);
          }
        else
          {
             pd->parent = NULL;
             ERR("New parent %p for object %p is not a valid Eo object.",
                 parent_id, obj);
          }
     }
   else
     {
        pd->parent = NULL;
     }
}

static void
_parent_get(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Private_Data *pd = (Private_Data *) class_data;

   EO_PARAMETER_GET(Eo **, parent_id, list);

   if (!parent_id) return ;
   *parent_id = pd->parent;
}


/* Children accessor */
typedef struct _Eo_Children_Iterator Eo_Children_Iterator;
struct _Eo_Children_Iterator
{
   Eina_Iterator iterator;
   Eina_List *current;
   _Eo_Object *obj;
   Eo *obj_id;
   
};

static Eina_Bool
_eo_children_iterator_next(Eo_Children_Iterator *it, void **data)
{
   if (!it->current) return EINA_FALSE;

   if (data) *data = eina_list_data_get(it->current);
   it->current = eina_list_next(it->current);

   return EINA_TRUE;
}

static Eo *
_eo_children_iterator_container(Eo_Children_Iterator *it)
{
   return it->obj_id;
}

static void
_eo_children_iterator_free(Eo_Children_Iterator *it)
{
   _Eo_Class *klass;
   _Eo_Object *obj;

   klass = (_Eo_Class*) it->obj->klass;
   obj = it->obj;

   eina_spinlock_take(&klass->iterators.trash_lock);
   if (klass->iterators.trash_count < 8)
     {
        klass->iterators.trash_count++;
        eina_trash_push(&klass->iterators.trash, it);
     }
   else
     {
        free(it);
     }
   eina_spinlock_release(&klass->iterators.trash_lock);
   
   _eo_unref(obj);
}

static void
_children_iterator_new(Eo *obj_id, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   _Eo_Class *klass;

   EO_PARAMETER_GET(Eo_Children_Iterator **, it, list);
   EO_OBJ_POINTER_RETURN(obj_id, obj);

   if (!it) return ;
   *it = NULL;

   if (!pd->children) return ;

   klass = (_Eo_Class *) obj->klass;

   eina_spinlock_take(&klass->iterators.trash_lock);
   *it = eina_trash_pop(&klass->iterators.trash);
   if (*it)
     {
        klass->iterators.trash_count--;
        memset(*it, 0, sizeof (Eo_Children_Iterator));
     }
   else
     {
        *it = calloc(1, sizeof (Eo_Children_Iterator));
     }
   eina_spinlock_release(&klass->iterators.trash_lock);
   if (!*it) return ;

   EINA_MAGIC_SET(&(*it)->iterator, EINA_MAGIC_ITERATOR);
   (*it)->current = obj->children;
   (*it)->obj = _eo_ref(obj);
   (*it)->obj_id = obj_id;

   (*it)->iterator.next = FUNC_ITERATOR_NEXT(_eo_children_iterator_next);
   (*it)->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eo_children_iterator_container);
   (*it)->iterator.free = FUNC_ITERATOR_FREE(_eo_children_iterator_free);
}

static void
_dbg_info_get(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED,
      va_list *data EINA_UNUSED)
{  /* No info required in the meantime */
   return;
}

static void
_data_del(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Eo_Generic_Data_Node *node;
   Private_Data *pd = class_data;

   EO_PARAMETER_GET(const char *, key, list);

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
   Eo ***tmp;

   EO_PARAMETER_GET(Eo **, wref, list);

   count = _wref_count(pd);
   count += 1; /* New wref. */

   tmp = realloc(pd->wrefs, sizeof(*pd->wrefs) * (count + 1));
   if (!tmp) return ;
   pd->wrefs = tmp;

   pd->wrefs[count - 1] = wref;
   pd->wrefs[count] = NULL;
   *wref = obj;
}

static void
_wref_del(Eo *obj, void *class_data, va_list *list)
{
   Private_Data *pd = (Private_Data *) class_data;
   size_t count;

   EO_PARAMETER_GET(Eo **, wref, list);

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
        Eo ***tmp;
        // No count--; because of the NULL that is not included in the count. */
        tmp = realloc(pd->wrefs, sizeof(*pd->wrefs) * count);
        if (!tmp) return ;
        pd->wrefs = tmp;
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
   Eo_Callback_Description *itr, *pitr = NULL;

   itr = pd->callbacks;

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
             else
               {
                  pd->callbacks = titr->next;
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

static inline void
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
_ev_cb_priority_add(Eo *obj, void *class_data, va_list *list)
{
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   EO_PARAMETER_GET(const Eo_Event_Description *, desc, list);
   EO_PARAMETER_ENUM_GET(Eo_Callback_Priority, priority, list);
   EO_PARAMETER_GET(Eo_Event_Cb, func, list);
   EO_PARAMETER_GET(const void *, data, list);

   cb = calloc(1, sizeof(*cb));
   if (!cb) return ;
   cb->items.item.desc = desc;
   cb->items.item.func = func;
   cb->func_data = (void *) data;
   cb->priority = priority;
   _eo_callbacks_sorted_insert(pd, cb);

     {
        const Eo_Callback_Array_Item arr[] = { {desc, func}, {NULL, NULL}};
        eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_ADD, arr, NULL));
     }
}

static void
_ev_cb_del(Eo *obj, void *class_data, va_list *list)
{
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   EO_PARAMETER_GET(const Eo_Event_Description *, desc, list);
   EO_PARAMETER_GET(Eo_Event_Cb, func, list);
   EO_PARAMETER_GET(void *, user_data, list);

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

static void
_ev_cb_array_priority_add(Eo *obj, void *class_data, va_list *list)
{
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   EO_PARAMETER_GET(const Eo_Callback_Array_Item *, array, list);
   EO_PARAMETER_ENUM_GET(Eo_Callback_Priority, priority, list);
   EO_PARAMETER_GET(const void *, data, list);

   cb = calloc(1, sizeof(*cb));
   if (!cb) return ;
   cb->func_data = (void *) data;
   cb->priority = priority;
   cb->items.item_array = array;
   cb->func_array = EINA_TRUE;
   _eo_callbacks_sorted_insert(pd, cb);

     {
        eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_ADD, array, NULL));
     }
}

static void
_ev_cb_array_del(Eo *obj, void *class_data, va_list *list)
{
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   EO_PARAMETER_GET(const Eo_Callback_Array_Item *, array, list);
   EO_PARAMETER_GET(void *, user_data, list);

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

static void
_ev_cb_call(Eo *obj_id, void *class_data, va_list *list)
{
   Eo_Callback_Description *cb;
   Private_Data *pd = (Private_Data *) class_data;

   EO_PARAMETER_GET(const Eo_Event_Description *, desc, list);
   EO_PARAMETER_GET(void *, event_info, list);
   EO_PARAMETER_GET(Eina_Bool *, ret, list);

   EO_OBJ_POINTER_RETURN(obj_id, obj);

   if (ret) *ret = EINA_TRUE;

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
                            if (ret) *ret = EINA_FALSE;
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
                       if (ret) *ret = EINA_FALSE;
                       goto end;
                    }
               }
          }
     }

end:
   pd->walking_list--;
   _eo_callbacks_clear(pd);
   _eo_unref(obj);
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
   EO_PARAMETER_GET(const Eo_Event_Description *, desc, list);
   EO_PARAMETER_GET(Eo *, new_obj, list);

   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   eo_do(obj, eo_event_callback_add(desc, _eo_event_forwarder_callback, new_obj));
}

static void
_ev_cb_forwarder_del(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   EO_PARAMETER_GET(const Eo_Event_Description *, desc, list);
   EO_PARAMETER_GET(Eo *, new_obj, list);

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
   EO_PARAMETER_GET(int *, ret, list);

   *ret = pd->event_freeze_count;
}

static void
_ev_global_freeze(Eo_Class *klass EINA_UNUSED, void *data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   event_freeze_count++;
}

static void
_ev_global_thaw(Eo_Class *klass EINA_UNUSED, void *data EINA_UNUSED, va_list *list EINA_UNUSED)
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
_ev_global_freeze_get(Eo_Class *klass EINA_UNUSED, void *data EINA_UNUSED, va_list *list)
{
   EO_PARAMETER_GET(int *, ret, list);

   *ret = event_freeze_count;
}

/* Eo_Dbg */
EAPI void
eo_dbg_info_free(Eo_Dbg_Info *info)
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
   if (!*dst) return EINA_FALSE;
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

static const Eina_Value_Type _EO_DBG_INFO_TYPE = {
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

EAPI const Eina_Value_Type *EO_DBG_INFO_TYPE = &_EO_DBG_INFO_TYPE;


/* EOF event callbacks */


/* EO_BASE_CLASS stuff */
#define MY_CLASS EO_BASE_CLASS

/* FIXME: Set proper type descriptions. */
EAPI const Eo_Event_Description _EO_EV_CALLBACK_ADD =
   EO_EVENT_DESCRIPTION("callback,add", "A callback was added.");
EAPI const Eo_Event_Description _EO_EV_CALLBACK_DEL =
   EO_EVENT_DESCRIPTION("callback,del", "A callback was deleted.");
EAPI const Eo_Event_Description _EO_EV_DEL =
   EO_HOT_EVENT_DESCRIPTION("del", "Obj is being deleted.");

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));

   _eo_condtor_done(obj);
}

static void
_destructor(Eo *obj, void *class_data, va_list *list EINA_UNUSED)
{
   Private_Data *pd = class_data;
   Eo *child;

   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));

   EINA_LIST_FREE(pd->children, child)
      eo_do(child, eo_parent_set(NULL));

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
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_PARENT_SET), _parent_set),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_PARENT_GET), _parent_get),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CHILDREN_ITERATOR_NEW), _children_iterator_new),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_SET), _data_set),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_GET), _data_get),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_DEL), _data_del),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_WREF_ADD), _wref_add),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_WREF_DEL), _wref_del),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_PRIORITY_ADD), _ev_cb_priority_add),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_DEL), _ev_cb_del),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_ARRAY_PRIORITY_ADD), _ev_cb_array_priority_add),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_ARRAY_DEL), _ev_cb_array_del),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_CALL), _ev_cb_call),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_ADD), _ev_cb_forwarder_add),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_DEL), _ev_cb_forwarder_del),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_FREEZE), _ev_freeze),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_THAW), _ev_thaw),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_FREEZE_GET), _ev_freeze_get),
        EO_OP_FUNC_CLASS(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE), _ev_global_freeze),
        EO_OP_FUNC_CLASS(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_GLOBAL_THAW), _ev_global_thaw),
        EO_OP_FUNC_CLASS(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE_GET), _ev_global_freeze_get),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DBG_INFO_GET), _dbg_info_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_CONSTRUCTOR, "Constructor"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DESTRUCTOR, "Destructor"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_PARENT_SET, "Set parent"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_PARENT_GET, "Get parent"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_CHILDREN_ITERATOR_NEW, "Children Iterator"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DATA_SET, "Set data for key."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DATA_GET, "Get data for key."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DATA_DEL, "Del key."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_WREF_ADD, "Add a weak ref to the object."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_WREF_DEL, "Delete the weak ref."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_PRIORITY_ADD, "Add an event callback with a priority."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_DEL, "Delete an event callback"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_ARRAY_PRIORITY_ADD, "Add an event callback array with a priority."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_ARRAY_DEL, "Delete an event callback array"),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_CALL, "Call the event callbacks for an event."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_ADD, "Add an event forwarder."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_DEL, "Delete an event forwarder."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_FREEZE, "Freezes events."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_THAW, "Thaws events."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_EVENT_FREEZE_GET, "Get event freeze counter."),
     EO_OP_DESCRIPTION_CLASS(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE, "Freezes events globally."),
     EO_OP_DESCRIPTION_CLASS(EO_BASE_SUB_ID_EVENT_GLOBAL_THAW, "Thaws events globally."),
     EO_OP_DESCRIPTION_CLASS(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE_GET, "Get global event freeze counter."),
     EO_OP_DESCRIPTION(EO_BASE_SUB_ID_DBG_INFO_GET, "Get debug info list for obj."),
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
     "Eo_Base",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO_CLASS_DESCRIPTION_OPS(&EO_BASE_BASE_ID, op_desc, EO_BASE_SUB_ID_LAST),
     event_desc,
     sizeof(Private_Data),
     _class_constructor,
     NULL,
     NULL
};

EO_DEFINE_CLASS(eo_base_class_get, &class_desc, NULL, NULL)
