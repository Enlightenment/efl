#include "config.h"
#include "Efl.h"

typedef struct
{
   Eina_Hash *observers;
   int walking;
   Eina_List *pending_deletion;
} Efl_Observable_Data;

typedef struct
{
   EINA_REFCOUNT;
   Efl_Observer *o;
} Efl_Observer_Refcount;

typedef struct
{
   Eina_List *list;
   Eina_Stringshare *key;
} Efl_Observer_List;

static int
_search_cb(const void *data1, const void *data2)
{
   const Efl_Observer_Refcount *or = data1;
   const Efl_Observer *obs = data2;

   if (or->o > obs) return 1;
   else if (or->o < obs) return -1;
   else return 0;
}

static int
_insert_cb(const void *data1, const void *data2)
{
   const Efl_Observer_Refcount *or1 = data1;
   const Efl_Observer_Refcount *or2 = data2;

   if (or1->o > or2->o) return 1;
   else if (or1->o < or2->o) return -1;
   else return 0;
}

static void
_free_cb(void *data)
{
   Efl_Observer_List *observers = data;
   eina_list_free(observers->list);
   eina_stringshare_del(observers->key);
   free(observers);
}

static void
_observer_del(Eo *obj EINA_UNUSED, Efl_Observable_Data *pd, Efl_Observer_List *observers, Eina_List *target)
{
   if (!target) return;

   Efl_Observer_Refcount *or;

   or = eina_list_data_get(target);
   EINA_REFCOUNT_UNREF(or)
     {
        observers->list = eina_list_remove_list(observers->list, target);

        if (!observers->list)
          {
             if (!pd->walking)
               eina_hash_del(pd->observers, observers->key, observers);
             else
               pd->pending_deletion = eina_list_append(pd->pending_deletion, observers);
          }
     }
}

static void
_process_deletion(Eo *obj EINA_UNUSED, Efl_Observable_Data *pd)
{
   if (!pd || pd->walking) return;

   Efl_Observer_List *observers;

   EINA_LIST_FREE(pd->pending_deletion, observers)
     eina_hash_del(pd->observers, observers->key, observers);
}

EOLIAN static void
_efl_observable_efl_object_destructor(Eo *obj, Efl_Observable_Data *pd)
{
   if (pd->observers)
     eina_hash_free(pd->observers);

   efl_destructor(efl_super(obj, EFL_OBSERVABLE_CLASS));
}

EOLIAN static void
_efl_observable_observer_add(Eo *obj EINA_UNUSED, Efl_Observable_Data *pd, const char *key, Efl_Observer *obs)
{
   Efl_Observer_List *observers = NULL;
   Efl_Observer_Refcount *or;

   if (!key) return;

   if (pd->observers)
     observers = eina_hash_find(pd->observers, key);
   else
     pd->observers = eina_hash_string_superfast_new(_free_cb);

   if (!observers)
     {
        observers = calloc(1, sizeof(Efl_Observer_List));
        observers->key = eina_stringshare_add(key);
        eina_hash_direct_add(pd->observers, observers->key, observers);
     }
   else if (!observers->list)
     pd->pending_deletion = eina_list_remove(pd->pending_deletion, observers);

   or = eina_list_search_sorted(observers->list, _search_cb, obs);
   if (!or)
     {
        or = calloc(1, sizeof(Efl_Observer_Refcount));
        or->o = obs;
        EINA_REFCOUNT_INIT(or);

        observers->list = eina_list_sorted_insert(observers->list, _insert_cb, or);
     }
   else
     EINA_REFCOUNT_REF(or);
}

EOLIAN static void
_efl_observable_observer_del(Eo *obj EINA_UNUSED, Efl_Observable_Data *pd, const char *key, Efl_Observer *obs)
{
   Efl_Observer_List *observers;

   if (!key || !pd->observers) return;

   observers = eina_hash_find(pd->observers, key);
   if (!observers) return;

   _observer_del(obj, pd, observers, eina_list_search_sorted_list(observers->list, _search_cb, obs));
}

EOLIAN static void
_efl_observable_observer_clean(Eo *obj EINA_UNUSED, Efl_Observable_Data *pd, Efl_Observer *obs)
{
   Eina_Iterator *it;
   Efl_Observer_List *observers;

   if (!pd->observers) return;

   it = eina_hash_iterator_data_new(pd->observers);
   EINA_ITERATOR_FOREACH(it, observers)
     _observer_del(obj, pd, observers, eina_list_search_sorted_list(observers->list, _search_cb, obs));
   eina_iterator_free(it);
}

typedef struct
{
   Eina_Iterator iterator;
   Eina_List *current;
   Eina_List *next;
} Efl_Observer_Iterator;

static Eina_Bool
_efl_observable_observers_iterator_next(Eina_Iterator *it, void **data)
{
   Efl_Observer_Iterator *et = (void *)it;
   Efl_Observer_Refcount *or = NULL;

   if (!et->current) return EINA_FALSE;

   or = eina_list_data_get(et->current);
   if (!or) return EINA_FALSE;

   et->current = et->next;
   et->next = eina_list_next(et->current);

   *data = or->o;

   return EINA_TRUE;
}

static void *
_efl_observable_observers_iterator_container(Eina_Iterator *it EINA_UNUSED)
{
   return NULL;
}

static void
_efl_observable_observers_iterator_free(Eina_Iterator *it)
{
   Efl_Observer_Iterator *et = (void *)it;

   EINA_MAGIC_SET(&et->iterator, 0);
   free(et);
}

EOLIAN static Eina_Iterator *
_efl_observable_observers_iterator_new(Eo *obj EINA_UNUSED, Efl_Observable_Data *pd, const char *key)
{
   Efl_Observer_List *observers;
   Efl_Observer_Iterator *it;

   if (!pd->observers) return NULL;

   observers = eina_hash_find(pd->observers, key);
   if (!observers) return NULL;

   it = calloc(1, sizeof(Efl_Observer_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->current = observers->list;
   it->next = eina_list_next(observers->list);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = _efl_observable_observers_iterator_next;
   it->iterator.get_container = _efl_observable_observers_iterator_container;
   it->iterator.free = _efl_observable_observers_iterator_free;

   return &it->iterator;
}

EOLIAN static void
_efl_observable_observers_update(Eo *obj, Efl_Observable_Data *pd, const char *key, void *data)
{
   Eina_Iterator *it;
   Efl_Observer *o;

   if (!pd->observers) return;

   it = efl_observable_observers_iterator_new(obj, key);
   if (!it) return;

   EINA_ITERATOR_FOREACH(it, o)
     efl_observer_update(o, obj, key, data);
}

typedef struct
{
   Eina_Iterator iterator;
   Eina_Iterator *classes;
   Efl_Observable *obs;
   Eina_List *tuples;
} Efl_Observable_Iterator;

static Eina_Bool
_efl_observable_iterator_tuple_next(Eina_Iterator *it, void **data)
{
   Efl_Observable_Iterator *et = (void *)it;
   Efl_Observable_Tuple *tuple;
   const char *key;

   if (!eina_iterator_next(et->classes, (void **)&key)) return EINA_FALSE;
   if (!key) return EINA_FALSE;

   tuple = calloc(1, sizeof(Efl_Observable_Tuple));
   if (!tuple) return EINA_FALSE;

   tuple->key = key;
   tuple->data = efl_observable_observers_iterator_new(et->obs, key);

   et->tuples = eina_list_append(et->tuples, tuple);
   *data = tuple;

   return EINA_TRUE;
}

static void *
_efl_observable_iterator_tuple_container(Eina_Iterator *it EINA_UNUSED)
{
   return NULL;
}

static void
_efl_observable_iterator_tuple_free(Eina_Iterator *it)
{
   Efl_Observable_Iterator *et = (void *)it;
   Efl_Observable_Tuple *tuple;

   eina_iterator_free(et->classes);
   EINA_LIST_FREE(et->tuples, tuple)
     {
        if (tuple->data)
          eina_iterator_free(tuple->data);
        free(tuple);
     }

   Efl_Observable_Data *pd = efl_data_scope_get(et->obs, EFL_OBSERVABLE_CLASS);
   if (pd)
     {
        pd->walking--;
        _process_deletion(et->obs, pd);
     }

   EINA_MAGIC_SET(&et->iterator, 0);
   free(et);
}

EOLIAN static Eina_Iterator *
_efl_observable_iterator_tuple_new(Eo *obj, Efl_Observable_Data *pd)
{
   Efl_Observable_Iterator *it;

   if (!pd->observers) return NULL;

   it = calloc(1, sizeof(Efl_Observable_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->classes = eina_hash_iterator_key_new(pd->observers);
   it->obs = obj;
   pd->walking++;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = _efl_observable_iterator_tuple_next;
   it->iterator.get_container = _efl_observable_iterator_tuple_container;
   it->iterator.free = _efl_observable_iterator_tuple_free;

   return &it->iterator;
}

#include "interfaces/efl_observable.eo.c"
#include "interfaces/efl_observer.eo.c"
