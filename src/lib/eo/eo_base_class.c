#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "Eo.h"
#include "eo_ptr_indirection.h"
#include "eo_private.h"

static int event_freeze_count = 0;

typedef struct _Eo_Callback_Description Eo_Callback_Description;

#define CALLSIZE  62

typedef struct
{
   Eina_List *children;
   Eo *parent;
   Eina_List *parent_list;

   Eina_Inlist *generic_data;
   Eo ***wrefs;

  // Eo_Callback_Description *callbacks;
   Eina_Inarray *callbacks;
  int callbacks_hash[ CALLSIZE +1][2];
  Eina_Bool callbacks_hash_dirty ;

   unsigned short walking_list;
   unsigned short event_freeze_count;
   Eina_Bool deletions_waiting : 1;
unsigned int called_counter;
unsigned int callbacks_counter;
unsigned int called_loop_counter;
unsigned int called_inner_loop_counter;
unsigned int arrays_counter;
clock_t  called_sum_clocks ;

unsigned int events_counter;
unsigned long long int have_events;

} Eo_Base_Data;
static unsigned int called_counter=0;
static unsigned int callbacks_counter=0;
 static unsigned int called_loop_counter=0;
 static unsigned int called_inner_loop_counter=0;

 static unsigned int arrays_counter=0;
 static unsigned int objects_counter=0;

 static clock_t  called_sum_clocks =0;
 static clock_t start_clock =0;
 static clock_t  key_data_get_sum_clocks =0;


static unsigned int events_counter=0;

typedef struct
{
   EINA_INLIST;
   Eina_Stringshare *key;
   void *data;
   eo_key_data_free_func free_func;
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
_eo_generic_data_del_all(Eo_Base_Data *pd)
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

EOLIAN static void
_eo_base_key_data_set(Eo *obj, Eo_Base_Data *pd,
          const char *key, const void *data, eo_key_data_free_func free_func)
{
   Eo_Generic_Data_Node *node;

   if (!key) return;

   eo_do(obj, eo_key_data_del(key); );

   node = malloc(sizeof(Eo_Generic_Data_Node));
   if (!node) return;
   node->key = eina_stringshare_add(key);
   node->data = (void *) data;
   node->free_func = free_func;
   pd->generic_data = eina_inlist_prepend(pd->generic_data,
         EINA_INLIST_GET(node));
}

EOLIAN static void *
_eo_base_key_data_get(Eo *obj EINA_UNUSED, Eo_Base_Data *pd, const char *key)
{
   /* We don't really change it... */
   Eo_Generic_Data_Node *node;
   if (!key) return NULL;
clock_t start_time=clock();
   EINA_INLIST_FOREACH(pd->generic_data, node)
     {
        if (!strcmp(node->key, key))
          {
             pd->generic_data =
                eina_inlist_promote(pd->generic_data, EINA_INLIST_GET(node));
             return node->data;
          }
     }
key_data_get_sum_clocks+=clock()-start_time; 
   return NULL;
}

EOLIAN static void
_eo_base_parent_set(Eo *obj, Eo_Base_Data *pd, Eo *parent_id)
{
   Eina_Bool tmp;
   if (pd->parent == parent_id)
     return;

   if (eo_do_ret(obj, tmp, eo_composite_part_is()) && pd->parent)
     {
        eo_do(pd->parent, eo_composite_detach(obj));
     }

   if (pd->parent)
     {
        Eo_Base_Data *old_parent_pd;

        old_parent_pd = eo_data_scope_get(pd->parent, EO_BASE_CLASS);
        if (old_parent_pd)
          {
             old_parent_pd->children = eina_list_remove_list(old_parent_pd->children,
                                                             pd->parent_list);
             pd->parent_list = NULL;
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
        Eo_Base_Data *parent_pd = NULL;
        parent_pd = eo_data_scope_get(parent_id, EO_BASE_CLASS);

        if (EINA_LIKELY(parent_pd != NULL))
          {
             pd->parent = parent_id;
             parent_pd->children = eina_list_append(parent_pd->children,
                   obj);
             pd->parent_list = eina_list_last(parent_pd->children);
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

EOLIAN static Eo *
_eo_base_parent_get(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
{
   return pd->parent;
}

EOLIAN static Eina_Bool
_eo_base_finalized_get(Eo *obj_id, Eo_Base_Data *pd EINA_UNUSED)
{
   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

   return obj->finalized;
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

EOLIAN static Eina_Iterator *
_eo_base_children_iterator_new(Eo *obj_id, Eo_Base_Data *pd)
{
   _Eo_Class *klass;
   Eo_Children_Iterator *it;

   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, NULL);

   if (!pd->children) return NULL;

   klass = (_Eo_Class *) obj->klass;

   eina_spinlock_take(&klass->iterators.trash_lock);
   it = eina_trash_pop(&klass->iterators.trash);
   if (it)
     {
        klass->iterators.trash_count--;
        memset(it, 0, sizeof (Eo_Children_Iterator));
     }
   else
     {
        it = calloc(1, sizeof (Eo_Children_Iterator));
     }
   eina_spinlock_release(&klass->iterators.trash_lock);
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->current = pd->children;
   it->obj = _eo_ref(obj);
   it->obj_id = obj_id;

   it->iterator.next = FUNC_ITERATOR_NEXT(_eo_children_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eo_children_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eo_children_iterator_free);

   return (Eina_Iterator *)it;
}

EOLIAN static void
_eo_base_dbg_info_get(Eo *obj EINA_UNUSED, Eo_Base_Data *pd EINA_UNUSED, Eo_Dbg_Info *root_node EINA_UNUSED)
{  /* No info required in the meantime */
   return;
}

EOLIAN static void
_eo_base_key_data_del(Eo *obj EINA_UNUSED, Eo_Base_Data *pd, const char *key)
{
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
_wref_count(Eo_Base_Data *pd)
{
   size_t count = 0;
   if (!pd->wrefs)
      return 0;

   Eo ***itr;
   for (itr = pd->wrefs; *itr; itr++)
      count++;

   return count;
}

EOLIAN static void
_eo_base_wref_add(Eo *obj, Eo_Base_Data *pd, Eo **wref)
{
   size_t count;
   Eo ***tmp;

   count = _wref_count(pd);
   count += 1; /* New wref. */

   tmp = realloc(pd->wrefs, sizeof(*pd->wrefs) * (count + 1));
   if (!tmp) return;
   pd->wrefs = tmp;

   pd->wrefs[count - 1] = wref;
   pd->wrefs[count] = NULL;
   *wref = obj;
}

EOLIAN void
_eo_base_wref_del(Eo *obj, Eo_Base_Data *pd, Eo **wref)
{
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
        for (itr = pd->wrefs; *itr; itr++)
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
        if (!tmp) return;
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
_wref_destruct(Eo_Base_Data *pd)
{
   Eo ***itr;
   if (!pd->wrefs)
      return;

   for (itr = pd->wrefs; *itr; itr++)
     {
        **itr = NULL;
     }

   free(pd->wrefs);
}

/* EOF Weak reference. */

/* Event callbacks */

/* Callbacks */

/* XXX: Legacy support, remove when legacy is dead. */
static Eina_Hash *_legacy_events_hash = NULL;
static const char *_legacy_event_desc = "Dynamically generated legacy event";

EAPI const Eo_Event_Description *
eo_base_legacy_only_event_description_get(const char *_event_name)
{
   Eina_Stringshare *event_name = eina_stringshare_add(_event_name);
   Eo_Event_Description *event_desc = eina_hash_find(_legacy_events_hash, event_name);
   if (!event_desc)
     {
        event_desc = calloc(1, sizeof(Eo_Event_Description));
        event_desc->name = event_name;
        event_desc->doc = _legacy_event_desc;
        eina_hash_add(_legacy_events_hash, event_name, event_desc);
     }
   else
     {
        eina_stringshare_del(event_name);
     }

   return event_desc;
}

static void
_legacy_events_hash_free_cb(void *_desc)
{
   Eo_Event_Description *desc = _desc;
   eina_stringshare_del(desc->name);
   free(desc);
}

/* EOF Legacy */

struct _Eo_Callback_Description
{
   //Eo_Callback_Description *next;

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
_eo_callback_remove(Eo_Base_Data *pd, Eo_Callback_Description *cb)
{
   Eo_Callback_Description *itr, *pitr = NULL;
#if 0
   itr = pd->callbacks;

   for ( ; itr; )
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
#endif
}

/* Actually remove, doesn't care about walking list, or delete_me */
static void
_eo_callback_remove_all(Eo_Base_Data *pd)
{
#if 0
   while (pd->callbacks)
     {
        Eo_Callback_Description *next = pd->callbacks->next;
        free(pd->callbacks);
        pd->callbacks = next;
     }
#endif
}
 int  _eo_base_get_event_index_hash( const Eo_Event_Description *a){

     void * p;

   if (!a){

printf("not a \n");
      return 0;//avi todo fo change to -1
   }

   /* If either is legacy, fallback to string comparison. */
   if ((a->doc == _legacy_event_desc))
     {
        p = a->name;    
     }
   else
     {
        p=a;
     }

    int index_hash = ((unsigned long long int )p)%CALLSIZE;
return index_hash;

}
void  _eo_base_set_event_hash( const Eo_Event_Description *a , Eo_Base_Data *pd,  int i, int aIndex){

      int index_hash = _eo_base_get_event_index_hash(a );

     //  pd->callbacks_hash[index_hash] |= 1<<i;
     if(  pd->callbacks_hash[index_hash][0] ==  -1 ){
          pd->callbacks_hash[index_hash][0]  = i;
          pd->callbacks_hash[index_hash][1]  = aIndex;
 pd->callbacks_hash[index_hash][2] = 0;
     //   printf("first  one for %d is %d\n", index_hash, i);

     }
     else{
          pd->callbacks_hash[index_hash][2]++;//there is more
   //       printf("more than one for %d\n", index_hash);
     }

}
void _eo_base_update_callbacks_hash ( Eo_Base_Data *pd){

 
   Eo_Callback_Description *cb;


   for( int i=0;i<  CALLSIZE  ;i++){
        pd->callbacks_hash[i][0] = -1;
        pd->callbacks_hash[i][1]= -1;
        pd->callbacks_hash[i][2] = 0;
   }

   int i=0;
   EINA_INARRAY_FOREACH(pd->callbacks, cb){

        if (!cb->delete_me)
          {
             if (cb->func_array)
               {
                  const Eo_Callback_Array_Item *it;
                  int count = 0;
                  for (it = cb->items.item_array; it->func; it++)
                    {

                       _eo_base_set_event_hash( it->desc , pd,  i, count);
                       count++;

                    }
               }
             else{
                  _eo_base_set_event_hash( cb->items.item.desc, pd , i, -1);
             }
          }
        i++;

   }
   pd->callbacks_hash_dirty = EINA_FALSE;

}
_eo_base_callback_delete_me_check( const Eina_Inarray  *iarr,  void *a,  void *b)
{
   return ((struct _Eo_Callback_Description*)a)->delete_me;
}
static void
_eo_callbacks_clear(Eo_Base_Data *pd)
{
   Eo_Callback_Description *cb = NULL;

   /* If there are no deletions waiting. */
   if (!pd->deletions_waiting)
      return;

   /* Abort if we are currently walking the list. */
   if (pd->walking_list > 0)
      return;

   pd->deletions_waiting = EINA_FALSE;

   eina_inarray_foreach_remove( pd->callbacks, _eo_base_callback_delete_me_check, NULL);

 _eo_base_update_callbacks_hash(pd); 

}
static void
_eo_callbacks_sorted_insert(Eo_Base_Data *pd, Eo_Callback_Description *cb)
{

   Eo_Callback_Description *itr, *itrp = NULL;
   for (itr = pd->callbacks; itr && (itr->priority < cb->priority);
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
void  set_event_have( const Eo_Event_Description *a , Eo_Base_Data *pd){

     void * p;

   if (!a)
      return;

   /* If either is legacy, fallback to string comparison. */
   if ((a->doc == _legacy_event_desc))
     {
        p = a->name;    
     }
   else
     {
        p=a;
     }

   unsigned int index = ((unsigned long long int )p)%63;
   if( (pd->have_events  & (1<<index) )==0){
      pd->events_counter++;
     pd->have_events |=  1<<index;
   }

}

      
int
_eo_base_callback_priority_cmp(const void *a, const void *b)
{
   return (( Eo_Callback_Description*)a)->priority < (( Eo_Callback_Description*)b)->priority;
}

EOLIAN static void
_eo_base_event_callback_priority_add(Eo *obj, Eo_Base_Data *pd,
      const Eo_Event_Description *desc,
      Eo_Callback_Priority priority,
      Eo_Event_Cb func,
      const void *user_data)
{
   Eo_Callback_Description cb;

   //  cb = calloc(1, sizeof(*cb));
   //   if (!cb) return;
   cb.items.item.desc = desc;
   cb.items.item.func = func;
   cb.func_data = (void *) user_data;
   cb.priority = priority;
   cb.func_array = EINA_FALSE;
   cb.delete_me = EINA_FALSE;
   // _eo_callbacks_sorted_insert(pd, cb);
   eina_inarray_insert_sorted(pd->callbacks, &cb , _eo_base_callback_priority_cmp);

   //pd->callbacks_hash_dirty = EINA_TRUE;
   _eo_base_update_callbacks_hash(pd);
     {
        const Eo_Callback_Array_Item arr[] = { {desc, func}, {NULL, NULL}};
        eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_ADD, (void *)arr));
     }

   pd->callbacks_counter++;//avi debug

   set_event_have( desc, pd);

}

EOLIAN static void
_eo_base_event_callback_del(Eo *obj, Eo_Base_Data *pd,
                    const Eo_Event_Description *desc,
                    Eo_Event_Cb func,
                    const void *user_data)
{
   Eo_Callback_Description *cb;


   EINA_INARRAY_FOREACH(pd->callbacks, cb)
       if ((cb->items.item.desc == desc) && (cb->items.item.func == func) &&
              (cb->func_data == user_data))
          {
            // eina_inarray_remove(callbacks , cb );
             const Eo_Callback_Array_Item arr[] = { {desc, func}, {NULL, NULL}};

             cb->delete_me = EINA_TRUE;
             pd->deletions_waiting = EINA_TRUE;
             _eo_callbacks_clear(pd);
             eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_DEL, (void *)arr); );
             return;
          }

   DBG("Callback of object %p with function %p and data %p not found.", obj, func, user_data);
}

EOLIAN static void
_eo_base_event_callback_array_priority_add(Eo *obj, Eo_Base_Data *pd,
      const Eo_Callback_Array_Item *array,
      Eo_Callback_Priority priority,
      const void *user_data)
{
   Eo_Callback_Description cb;

   //  cb = calloc(1, sizeof(*cb));
   //  if (!cb) return;
   cb.func_data = (void *) user_data;
   cb.priority = priority;
   cb.items.item_array = array;
   cb.func_array = EINA_TRUE;
   cb.delete_me = EINA_FALSE;
   //  _eo_callbacks_sorted_insert(pd, cb);
   eina_inarray_insert_sorted(pd->callbacks, &cb , _eo_base_callback_priority_cmp);
   //pd->callbacks_hash_dirty = EINA_TRUE;
   _eo_base_update_callbacks_hash(pd);  

     {
        eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_ADD, (void *)array); );
     }
   pd->arrays_counter++;//avi debug
   const Eo_Callback_Array_Item *it;

   for (it = cb.items.item_array; it->func; it++)
     {

        set_event_have( it->desc, pd);
     }

}


EOLIAN static void
_eo_base_event_callback_array_del(Eo *obj, Eo_Base_Data *pd,
                 const Eo_Callback_Array_Item *array,
                 const void *user_data)
{
   Eo_Callback_Description *cb;


   EINA_INARRAY_FOREACH(pd->callbacks, cb)
        if ((cb->items.item_array == array) && (cb->func_data == user_data))
          {
             cb->delete_me = EINA_TRUE;
             pd->deletions_waiting = EINA_TRUE;
             _eo_callbacks_clear(pd);

             eo_do(obj, eo_event_callback_call(EO_EV_CALLBACK_DEL, (void *)array); );
             return;
          }

   DBG("Callback of object %p with function array %p and data %p not found.", obj, array, user_data);
}

static Eina_Bool
_cb_desc_match(const Eo_Event_Description *a, const Eo_Event_Description *b)
{
   if (!a)
      return EINA_FALSE;

   /* If either is legacy, fallback to string comparison. */
   if ((a->doc == _legacy_event_desc) || (b->doc == _legacy_event_desc))
     {
        /* Take stringshare shortcut if both are legacy */
        if (a->doc == b->doc)
          {
             return (a->name == b->name);
          }
        else
          {
             return !strcmp(a->name, b->name);
          }
     }
   else
     {
        return (a == b);
     }
}

EOLIAN static Eina_Bool
_eo_base_event_callback_call(Eo *obj_id, Eo_Base_Data *pd,
      const Eo_Event_Description *desc,
      void *event_info)
{
   Eina_Bool ret;
   Eo_Callback_Description *cb;

   EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, EINA_FALSE);

   ret = EINA_TRUE;

   _eo_ref(obj);
   pd->walking_list++;

   pd->called_counter++;//avi debug

   clock_t start_time = clock();


   if(pd->callbacks_hash_dirty){
        _eo_base_update_callbacks_hash(pd);
        //printf("dirty!\n");
   }
   int index_hash = _eo_base_get_event_index_hash( desc );
   if(pd->callbacks_hash[index_hash][0]==-1)
      goto end;



   if(pd->callbacks_hash[index_hash][2]==0){//the only one
        //  printf("call the only one!\n");
        cb =  eina_inarray_nth(pd->callbacks, pd->callbacks_hash[index_hash][0]);

        if (!cb->delete_me)
          {
             if(pd->callbacks_hash[index_hash][1]>-1){//array
                  //printf("array!\n");
                  const Eo_Callback_Array_Item *it= cb->items.item_array+pd->callbacks_hash[index_hash][1];


                  if (!_cb_desc_match(it->desc, desc))
                     goto end;//instead of continiue
                  if (!it->desc->unfreezable &&
                        (event_freeze_count || pd->event_freeze_count))
                     goto end;//instead of continiue
                  unsigned int before_func=clock();
                  /* Abort callback calling if the func says so. */
                  if (!it->func((void *) cb->func_data, obj_id, desc,
                           (void *) event_info))
                    {
                       start_time+=clock()-before_func;
                       ret = EINA_FALSE;
                       goto end;
                    }
                  start_time+=clock()-before_func;


             }
             else{
                  if (!_cb_desc_match(cb->items.item.desc, desc))
                     goto end;//instead of continiue
                  if ((!cb->items.item.desc
                           || !cb->items.item.desc->unfreezable) &&
                        (event_freeze_count || pd->event_freeze_count))
                     goto end;//instead of continiue
                  unsigned int before_func=clock();
                  /* Abort callback calling if the func says so. */
                  if (!cb->items.item.func((void *) cb->func_data, obj_id, desc,
                           (void *) event_info))
                    {
                       start_time+=clock()-before_func;
                       ret = EINA_FALSE;
                       goto end;
                    }
                  start_time+=clock()-before_func;

             }
          }
        goto end;
   }
   //    printf("cal: more than one for %d\n", index_hash);


   EINA_INARRAY_FOREACH(pd->callbacks, cb)
     {
        pd->called_loop_counter++;//avi debug

        if (!cb->delete_me)
          {
             if (cb->func_array)
               {
                  const Eo_Callback_Array_Item *it;
                  //int counter = 0;
                  for (it = cb->items.item_array; it->func; it++)
                    {
                       pd->called_inner_loop_counter++;//avi debug
                       //          printf("here arrary index %d \n",counter++); 
                       if (!_cb_desc_match(it->desc, desc))
                          continue;
                       if (!it->desc->unfreezable &&
                             (event_freeze_count || pd->event_freeze_count))
                          continue;
                       unsigned int before_func=clock();
                       /* Abort callback calling if the func says so. */
                       if (!it->func((void *) cb->func_data, obj_id, desc,
                                (void *) event_info))
                         {
                            start_time+=clock()-before_func;
                            ret = EINA_FALSE;
                            goto end;
                         }
                       start_time+=clock()-before_func;
                    }
               }
             else
               {
                  //         printf("here regular callback  \n"); 
                  if (!_cb_desc_match(cb->items.item.desc, desc))
                     continue;
                  if ((!cb->items.item.desc
                           || !cb->items.item.desc->unfreezable) &&
                        (event_freeze_count || pd->event_freeze_count))
                     continue;
                  unsigned int before_func=clock();
                  /* Abort callback calling if the func says so. */
                  if (!cb->items.item.func((void *) cb->func_data, obj_id, desc,
                           (void *) event_info))
                    {
                       start_time+=clock()-before_func;
                       ret = EINA_FALSE;
                       goto end;
                    }
                  start_time+=clock()-before_func;
               }
          }
     }

end:
   pd->called_sum_clocks +=clock()-start_time;//avi dbg
   pd->walking_list--;
   _eo_callbacks_clear(pd);
   _eo_unref(obj);

   return ret;
}

static Eina_Bool
_eo_event_forwarder_callback(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   (void) obj;
   Eo *new_obj = (Eo *) data;
   Eina_Bool ret = EINA_FALSE;

   eo_do(new_obj, ret = eo_event_callback_call(desc, (void *)event_info); );

   return ret;
}

/* FIXME: Change default priority? Maybe call later? */
EOLIAN static void
_eo_base_event_callback_forwarder_add(Eo *obj, Eo_Base_Data *pd EINA_UNUSED,
                     const Eo_Event_Description *desc,
                     Eo *new_obj)
{

   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   eo_do(obj, eo_event_callback_add(desc, _eo_event_forwarder_callback, new_obj); );
}

EOLIAN static void
_eo_base_event_callback_forwarder_del(Eo *obj, Eo_Base_Data *pd EINA_UNUSED,
                     const Eo_Event_Description *desc,
                     Eo *new_obj)
{

   /* FIXME: Add it EO_MAGIC_RETURN(new_obj, EO_EINA_MAGIC); */

   eo_do(obj, eo_event_callback_del(desc, _eo_event_forwarder_callback, new_obj); );
}

EOLIAN static void
_eo_base_event_freeze(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
{
   pd->event_freeze_count++;
}

EOLIAN static void
_eo_base_event_thaw(Eo *obj, Eo_Base_Data *pd)
{
   if (pd->event_freeze_count > 0)
     {
        pd->event_freeze_count--;
     }
   else
     {
        ERR("Events for object %p have already been thawed.", obj);
     }
}

EOLIAN static int
_eo_base_event_freeze_count_get(Eo *obj EINA_UNUSED, Eo_Base_Data *pd)
{
   return pd->event_freeze_count;
}

EOLIAN static void
_eo_base_event_global_freeze(Eo *klass EINA_UNUSED, void *pd EINA_UNUSED)
{
   event_freeze_count++;
}

EOLIAN static void
_eo_base_event_global_thaw(Eo *klass EINA_UNUSED, void *pd EINA_UNUSED)
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

EOLIAN static int
_eo_base_event_global_freeze_count_get(Eo *klass EINA_UNUSED, void *pd EINA_UNUSED)
{
   return event_freeze_count;
}

EOLIAN static Eina_Bool
_eo_base_composite_attach(Eo *parent_id, Eo_Base_Data *pd EINA_UNUSED, Eo *comp_obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);
   EO_OBJ_POINTER_RETURN_VAL(parent_id, parent, EINA_FALSE);

   if (!eo_isa(parent_id, _eo_class_id_get(comp_obj->klass))) return EINA_FALSE;

     {
        Eina_List *itr;
        Eo *emb_obj_id;
        EINA_LIST_FOREACH(parent->composite_objects, itr, emb_obj_id)
          {
             EO_OBJ_POINTER_RETURN_VAL(emb_obj_id, emb_obj, EINA_FALSE);
             if(emb_obj->klass == comp_obj->klass)
               return EINA_FALSE;
          }
     }

   comp_obj->composite = EINA_TRUE;
   parent->composite_objects = eina_list_prepend(parent->composite_objects, comp_obj_id);

   eo_do(comp_obj_id, eo_parent_set(parent_id));

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_eo_base_composite_detach(Eo *parent_id, Eo_Base_Data *pd EINA_UNUSED, Eo *comp_obj_id)
{
   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);
   EO_OBJ_POINTER_RETURN_VAL(parent_id, parent, EINA_FALSE);

   if (!comp_obj->composite)
      return EINA_FALSE;

   comp_obj->composite = EINA_FALSE;
   parent->composite_objects = eina_list_remove(parent->composite_objects, comp_obj_id);
   eo_do(comp_obj_id, eo_parent_set(NULL));

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_eo_base_composite_part_is(Eo *comp_obj_id, Eo_Base_Data *pd EINA_UNUSED)
{
   EO_OBJ_POINTER_RETURN_VAL(comp_obj_id, comp_obj, EINA_FALSE);

   return comp_obj->composite;
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

EOLIAN static void
_eo_base_constructor(Eo *obj, Eo_Base_Data *pd EINA_UNUSED)
{
   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));
   pd->called_counter=0;
   pd->callbacks_counter=0;
   pd-> called_loop_counter=0;

   pd-> called_loop_counter=0;
   pd->called_inner_loop_counter=0;

   pd-> arrays_counter=0;
   pd-> called_sum_clocks =0;
   pd->events_counter=0;
   pd->have_events = 0;

   pd->callbacks = eina_inarray_new(sizeof(Eo_Callback_Description), 10 );//added by avi

  for( int i=0;i<  CALLSIZE  ;i++){
     pd->callbacks_hash[i][0] =  pd->callbacks_hash[i][1]= -1;
 pd->callbacks_hash[i][2] = 0;
   }

pd->callbacks_hash_dirty = EINA_FALSE;

   _eo_condtor_done(obj);
}
static int c = 0;

static __attribute__((destructor)) void finish(void)
{
   c=1;
   printf("calbacks stats-All objects!: num objects=%u total time=%f called cal times= %u called loops=%u called inner loops=%u called clocks=%f called sec=%f callbacks count=%u clabbacks arrays=%u key_data_get_sum_clocks=%f events counter=%u \n",
         objects_counter,(double)(clock()-start_clock)/CLOCKS_PER_SEC,  called_counter,  called_loop_counter, called_inner_loop_counter,(double)called_sum_clocks,
         (double)called_sum_clocks/CLOCKS_PER_SEC, callbacks_counter, arrays_counter, (double)key_data_get_sum_clocks /CLOCKS_PER_SEC,
         events_counter);//avi debug

}
EOLIAN static void
_eo_base_destructor(Eo *obj, Eo_Base_Data *pd)
{
   Eo *child;

   DBG("%p - %s.", obj, eo_class_name_get(MY_CLASS));
   if( pd->callbacks_counter>0 && getenv("EO_DEBUG"))
      printf("calbacks stats-%s: called cal times= %u called loops=%u called inner loops=%u alled clocks=%f called sec=%f callbacks count=%u clabbacks arrays=%u events_counter = %u\n",
            eo_class_name_get(MY_CLASS ), pd->called_counter,  pd->called_loop_counter, pd->called_inner_loop_counter, (double)pd->called_sum_clocks,
            (double)pd->called_sum_clocks/CLOCKS_PER_SEC, pd->callbacks_counter, pd->arrays_counter,
            pd->events_counter);//avi debug


   called_counter+=pd->called_counter;
   callbacks_counter+=pd->callbacks_counter;
   called_loop_counter+= pd->called_loop_counter;
   arrays_counter+=pd->arrays_counter;
   called_inner_loop_counter+=pd->called_inner_loop_counter;
   called_sum_clocks+=pd-> called_sum_clocks;
   events_counter+=pd->events_counter;

   if( pd->callbacks_counter>0 )//only with atleast one calllbacks
      objects_counter++;

   EINA_LIST_FREE(pd->children, child)
      eo_do(child, eo_parent_set(NULL));

   _eo_generic_data_del_all(pd);
   _wref_destruct(pd);
 //  _eo_callback_remove_all(pd);

    eina_inarray_free(pd->callbacks);//added by avi

   _eo_condtor_done(obj);

   if(c==1)
      printf("shit\n");
}

EOLIAN static Eo *
_eo_base_finalize(Eo *obj, Eo_Base_Data *pd EINA_UNUSED)
{
   return _eo_add_internal_end(obj);
}

EOLIAN static void
_eo_base_class_constructor(Eo_Class *klass EINA_UNUSED)
{
   if(start_clock==0)
      start_clock=clock();//avi dbg

   event_freeze_count = 0;
   _legacy_events_hash = eina_hash_stringshared_new(_legacy_events_hash_free_cb);
}

EOLIAN static void
_eo_base_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   eina_hash_free(_legacy_events_hash);
}

#include "eo_base.eo.c"
