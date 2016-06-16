#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <fnmatch.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif
#include <Eina.h>
#include <Efl.h>

#include "Ecore.h"
#include "ecore_private.h"

typedef struct _Efl_Uri_Manager_Data Efl_Uri_Manager_Data;
typedef struct _Efl_Uri_Fetcher_Tuple Efl_Uri_Fetcher_Tuple;
typedef struct _Efl_Uri_Fetcher_Promise Efl_Uri_Fetcher_Promise;
typedef struct _Efl_Uri_Mapper_Promise Efl_Uri_Mapper_Promise;

struct _Efl_Uri_Manager_Data
{
   Eina_Hash *mappers;
   Eina_List *fetchers;

   Efl_Uri_Mapper *def;
};

struct _Efl_Uri_Fetcher_Tuple
{
   Efl_Uri_Fetcher *manager;
   const char regexp[1];
};

struct _Efl_Uri_Fetcher_Promise
{
   Eina_Promise_Owner *promise;

   Efl_Loop *loop;
   Efl_Uri_Manager *o;
   Efl_Uri_Manager_Data *pd;

   Eina_Promise *progress;

   unsigned int count;

   char *current;
};

struct _Efl_Uri_Mapper_Promise
{
   Eina_Promise_Owner *promise;

   Efl_Uri_Manager *o;
   Efl_Uri_Manager_Data *pd;

   Eina_Promise *request;
};

static Eina_Error EFL_URI_MANAGER_INFINITE_LOOP = -1;
static void _efl_uri_idle_run(void *data, const Eo_Event *ev);
static void _efl_uri_main_loop_del(void *data, const Eo_Event *ev);

static void
_efl_uri_fetcher_promise_clean(Efl_Uri_Fetcher_Promise *p)
{
   eo_event_callback_del(p->loop, EFL_LOOP_EVENT_IDLE, _efl_uri_idle_run, p);
   eo_event_callback_del(p->loop, EO_EVENT_DEL, _efl_uri_main_loop_del, p);

   free(p->current);
   free(p);
}

static void
_efl_uri_main_loop_del(void *data, const Eo_Event *ev EINA_UNUSED)
{
   Efl_Uri_Fetcher_Promise *p = data;

   eina_promise_owner_error_set(p->promise, EINA_ERROR_PROMISE_CANCEL);
   _efl_uri_fetcher_promise_clean(p);
}

static void
_fetcher_done(void *data, void *value)
{
   Efl_Uri_Fetcher_Promise *p = data;
   char *s = value;

   free(p->current);
   p->current = s;

   // Start processing the fetchers again
   eo_event_callback_add(p->loop, EFL_LOOP_EVENT_IDLE, _efl_uri_idle_run, p);
}

static void
_fetcher_error(void *data, Eina_Error error)
{
   Efl_Uri_Fetcher_Promise *p = data;

   eina_promise_owner_error_set(p->promise, error);
   _efl_uri_fetcher_promise_clean(p);
}

static void
_efl_uri_idle_run(void *data, const Eo_Event *ev EINA_UNUSED)
{
   Efl_Uri_Fetcher_Promise *p = data;
   Efl_Uri_Fetcher_Tuple *tuple;
   Eina_List *l;

   p->count++;

   if (p->count > eina_list_count(p->pd->fetchers))
     {
        eina_promise_owner_error_set(p->promise, EFL_URI_MANAGER_INFINITE_LOOP);
        _efl_uri_fetcher_promise_clean(p);
        return ;
     }

   /* Walking the list and running fnmatch shouldn't take us
      that long. If it does, we might just do that in batch and
      slice the list in chunk. */
   EINA_LIST_FOREACH(p->pd->fetchers, l, tuple)
     if (!fnmatch(tuple->regexp, p->current, 0))
       {
          p->progress = efl_uri_fetcher_fetch(tuple->manager, p->current);
          eo_event_callback_del(p->loop, EFL_LOOP_EVENT_IDLE, _efl_uri_idle_run, p);
          eina_promise_then(p->progress, &_fetcher_done, &_fetcher_error, p);
          return ;
       }

   // No more match, we are done !
   eina_promise_owner_value_set(p->promise, eina_stringshare_add(p->current), (void*)eina_stringshare_del);

   _efl_uri_fetcher_promise_clean(p);
   return ;
}

static void
_fetch_cancel(void *data, Eina_Promise_Owner *promise)
{
   eina_promise_owner_error_set(promise, EINA_ERROR_PROMISE_CANCEL);
   _efl_uri_fetcher_promise_clean(data);
}

static Eina_Promise *
_efl_uri_manager_fetch(Eo *obj, Efl_Uri_Manager_Data *pd, const char *file)
{
   Efl_Uri_Fetcher_Promise *p;

   p = calloc (1, sizeof (Efl_Uri_Fetcher_Promise));
   if (!p) return NULL;

   p->promise = eina_promise_add();
   if (!p->promise) goto on_error;

   eina_promise_owner_default_cancel_cb_add(p->promise, &_fetch_cancel, p, NULL);

   p->loop = eo_provider_find(obj, EFL_LOOP_CLASS);
   eo_event_callback_add(p->loop, EFL_LOOP_EVENT_IDLE, _efl_uri_idle_run, p);
   eo_event_callback_add(p->loop, EO_EVENT_DEL, _efl_uri_main_loop_del, p);

   p->current = strdup(file);
   p->o = obj;
   p->pd = pd;

   return eina_promise_owner_promise_get(p->promise);

 on_error:
   free(p);
   return NULL;
}

static void
_efl_uri_mapper_promise_clean(Efl_Uri_Mapper_Promise *p)
{
   free(p);
}

static void
_request_error(void *data, Eina_Error error)
{
   Efl_Uri_Mapper_Promise *p = data;

   eina_promise_owner_error_set(p->promise, error);
   _efl_uri_mapper_promise_clean(p);
}

static void
_mapper_then(void *data, void *value)
{
   Efl_Uri_Mapper_Promise *p = data;

   eina_promise_owner_value_set(p->promise, eina_file_dup(value), (void*)eina_file_close);
   _efl_uri_mapper_promise_clean(p);
}

static void
_mmap_cancel(void* data, Eina_Promise_Owner* promise EINA_UNUSED)
{
   Efl_Uri_Mapper_Promise *p = data;

   eina_promise_cancel(p->request);
}

static void
_fetch_then(void *data, void *value)
{
   Efl_Uri_Mapper_Promise *p = data;
   const Eina_Stringshare *file = value;
   Efl_Uri_Mapper *mapper;
   char *search;

   search = strstr(file, "://");
   if (!search)
     {
        mapper = p->pd->def;
     }
   else
     {
        char *prefix = alloca(search - file + 1);
        if (!prefix) goto on_error;
        strncpy(prefix, file, search - file);
        mapper = eina_hash_find(p->pd->mappers, prefix);
        mapper = mapper ? mapper : p->pd->def;
     }

   p->request = efl_uri_mapper_fetch(mapper, file);
   eina_promise_then(p->request, &_mapper_then, &_request_error, p);

   return ;

 on_error:
   eina_promise_owner_error_set(p->promise, EINA_ERROR_OUT_OF_MEMORY);
   _efl_uri_mapper_promise_clean(p);
}

static Eina_Promise *
_efl_uri_manager_mmap(Eo *obj, Efl_Uri_Manager_Data *pd, const char *file)
{
   Efl_Uri_Mapper_Promise *p;

   p = calloc(1, sizeof (Efl_Uri_Mapper_Promise));
   if (!p) return NULL;

   p->promise = eina_promise_add();
   if (!p->promise) goto on_error;

   eina_promise_owner_default_cancel_cb_add(p->promise, &_mmap_cancel, p, NULL);

   p->o = obj;
   p->pd = pd;

   p->request = efl_uri_manager_fetch(obj, file);
   eina_promise_then(p->request, &_fetch_then, &_request_error, p);

   return eina_promise_owner_promise_get(p->promise);

 on_error:
   free(p);
   return NULL;
}

static Eina_Bool
_efl_uri_manager_register_fetcher(Eo *obj EINA_UNUSED, Efl_Uri_Manager_Data *pd, const char *regexp, const Efl_Uri_Fetcher *manager)
{
   Efl_Uri_Fetcher_Tuple *tuple;
   unsigned int length;

   if (!regexp) return EINA_FALSE;

   length = strlen(regexp);
   tuple = calloc(1, sizeof (Efl_Uri_Fetcher_Tuple) + length);
   if (!tuple) return EINA_FALSE;

   tuple->manager = eo_ref(manager);
   strcpy((char*) tuple->regexp, regexp);

   pd->fetchers = eina_list_append(pd->fetchers, tuple);

   return EINA_TRUE;
}

static Eina_Bool
_efl_uri_manager_register_mapper(Eo *obj EINA_UNUSED, Efl_Uri_Manager_Data *pd, const char *prefix, const Efl_Uri_Mapper *manager)
{
   if (eina_hash_find(pd->mappers, prefix))
     return EINA_FALSE;

   return eina_hash_add(pd->mappers, prefix, eo_ref(manager));
}

static Eina_Bool
_efl_uri_manager_unregister_fetcher(Eo *obj EINA_UNUSED, Efl_Uri_Manager_Data *pd, const char *regexp, const Efl_Uri_Fetcher *manager)
{
   Efl_Uri_Fetcher_Tuple *tuple;
   Eina_List *l;

   if (!regexp) return EINA_FALSE;

   EINA_LIST_FOREACH(pd->fetchers, l, tuple)
     if (tuple->manager == manager && !strcmp(tuple->regexp, regexp))
       {
          pd->fetchers = eina_list_remove_list(pd->fetchers, l);
          eo_unref(tuple->manager);
          free(tuple);
          return EINA_TRUE;
       }
   return EINA_FALSE;
}

static Eina_Bool
_efl_uri_manager_unregister_mapper(Eo *obj EINA_UNUSED, Efl_Uri_Manager_Data *pd, const char *prefix, const Efl_Uri_Mapper *manager)
{
   return eina_hash_del(pd->mappers, prefix, manager);
}

static Eo_Base *
_efl_uri_manager_eo_base_constructor(Eo *obj, Efl_Uri_Manager_Data *pd)
{
   eo_constructor(eo_super(obj, EFL_URI_MANAGER_CLASS));

   pd->mappers = eina_hash_string_small_new((Eina_Free_Cb) eo_unref);

   if (EFL_URI_MANAGER_INFINITE_LOOP == -1)
     EFL_URI_MANAGER_INFINITE_LOOP = eina_error_msg_static_register("Infinite loop during name fetch in Efl.Uri.Manager.");

   return obj;
}

static void
_efl_uri_manager_eo_base_destructor(Eo *obj EINA_UNUSED, Efl_Uri_Manager_Data *pd)
{
   eo_destructor(eo_super(obj, EFL_URI_MANAGER_CLASS));

   eina_hash_free(pd->mappers);
   eo_wref_del(pd->def, &pd->def);
}

EAPI void
efl_uri_manager_default_set(Efl_Uri_Manager *obj, Efl_Uri_Mapper *def)
{
   Efl_Uri_Manager_Data *pd;

   pd = obj ? eo_data_scope_get(obj, EFL_URI_MANAGER_CLASS) : NULL;
   if (!pd) return ;

   eo_wref_add(def, &pd->def);
}

#include "efl_uri_manager.eo.c"

#include "efl_uri_mapper.eo.c"

#include "efl_uri_fetcher.eo.c"
