#include <Elementary.h>

#if 1
#include <pthread.h>
#define LK(x)  pthread_mutex_t x
#define LKI(x) pthread_mutex_init(&(x), NULL);
#define LKD(x) pthread_mutex_destroy(&(x));
#define LKL(x) pthread_mutex_lock(&(x));
#define LKT(x) pthread_mutex_trylock(&(x));
#define LKU(x) pthread_mutex_unlock(&(x));
#else
#define LK(x)
#define LKI(x)
#define LKD(x)
#define LKL(x)
#define LKT(x)
#define LKU(x)
#endif

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

// public api
typedef struct _Elm_Store                      Elm_Store;
typedef struct _Elm_Store_Item                 Elm_Store_Item;
typedef struct _Elm_Store_Item_Filesystem      Elm_Store_Item_Filesystem;
typedef struct _Elm_Store_Item_Info            Elm_Store_Item_Info;
typedef struct _Elm_Store_Item_Info_Filesystem Elm_Store_Item_Info_Filesystem;

typedef Eina_Bool (*Elm_Store_Item_List_Cb) (void *data, Elm_Store_Item_Info *info);
typedef void      (*Elm_Store_Item_Fetch_Cb) (void *data, Elm_Store_Item *sti);
typedef void      (*Elm_Store_Item_Unfetch_Cb) (void *data, Elm_Store_Item *sti);

EAPI Elm_Store              *elm_store_new(void);
EAPI void                    elm_store_free(Elm_Store *st);
EAPI void                    elm_store_target_genlist_set(Elm_Store *st, Evas_Object *obj);
EAPI void                    elm_store_source_filesystem_set(Elm_Store *st, const char *dir);
EAPI const char             *elm_store_source_filesystem_get(const Elm_Store *st);
EAPI void                    elm_store_cache_set(Elm_Store *st, int max);
EAPI int                     elm_store_cache_get(const Elm_Store *st);
EAPI void                    elm_store_list_func_set(Elm_Store *st, Elm_Store_Item_List_Cb func, const void *data);
EAPI void                    elm_store_fetch_func_set(Elm_Store *st, Elm_Store_Item_Fetch_Cb func, const void *data);
EAPI void                    elm_store_unfetch_func_set(Elm_Store *st, Elm_Store_Item_Unfetch_Cb func, const void *data);
EAPI void                    elm_store_sorted_set(Elm_Store *st, Eina_Bool sorted);
EAPI Eina_Bool               elm_store_sorted_get(const Elm_Store *st);
EAPI void                    elm_store_item_data_set(Elm_Store_Item *sti, void *data);
EAPI void                   *elm_store_item_data_get(Elm_Store_Item *sti);
EAPI const Elm_Store        *elm_store_item_store_get(const Elm_Store_Item *sti);
EAPI const Elm_Genlist_Item *elm_store_item_genlist_item_get(const Elm_Store_Item *sti);
EAPI const char             *elm_store_item_filesystem_path_get(const Elm_Store_Item *sti);

// private
#if 0
#define DBG(f, args...) printf(f, ##args)
#else
#define DBG(f, args...)
#endif

#define ELM_STORE_MAGIC 0x3f89ea56
#define ELM_STORE_ITEM_MAGIC 0x5afe8c1d

struct _Elm_Store
{
  EINA_MAGIC;
  Evas_Object   *genlist;
  Ecore_Thread  *list_th;
  struct {
    const char  *dir;
  } filesystem;
  Eina_Inlist   *items;
  Eina_List     *realized;
  int            realized_count;
  int            cache_max;
  struct {
    struct {
      Elm_Store_Item_List_Cb     func;
      void                      *data;
    } list;
    struct {
      Elm_Store_Item_Fetch_Cb    func;
      void                      *data;
    } fetch;
    struct {
      Elm_Store_Item_Unfetch_Cb  func;
      void                      *data;
    } unfetch;
  } cb;
  Eina_Bool sorted : 1;
};

struct _Elm_Store_Item_Filesystem
{
  const char *path;
};

struct _Elm_Store_Item
{
  EINA_INLIST;
  EINA_MAGIC;
  Elm_Store        *store;
  Elm_Genlist_Item *item;
  Ecore_Thread     *fetch_th;
  Ecore_Job        *eval_job;
  void *data;
  union {
    Elm_Store_Item_Filesystem filesystem;
    // add more types supported here
  } type;
  LK(lock);
  Eina_Bool         live : 1;
  Eina_Bool         was_live : 1;
  Eina_Bool         realized : 1;
  Eina_Bool         fetched : 1;
};

struct _Elm_Store_Item_Info_Filesystem
{
  char *path;
};

struct _Elm_Store_Item_Info
{
  Elm_Genlist_Item_Class *item_class;
  void                   *data;
  char                   *sort_id;
  union {
    Elm_Store_Item_Info_Filesystem filesystem;
    // add more types supported here
  } type;
};

static Elm_Genlist_Item_Class _store_item_class;

static void
_store_cache_trim(Elm_Store *st)
{
  while ((st->realized ) &&
         (((int)eina_list_count(st->realized) - st->realized_count)
           > st->cache_max))
    {
      Elm_Store_Item *sti = st->realized->data;
      if (sti->realized)
        {
          st->realized = eina_list_remove_list(st->realized, st->realized);
          sti->realized = EINA_FALSE;
        }
      LKL(sti->lock);
      if (!sti->fetched)
        {
          DBG(".. do cancel [%i] [%i] %p for: %s\n", 
              sti->live,
              sti->fetched,
              sti->fetch_th,
              elm_store_item_filesystem_path_get(sti));
          LKU(sti->lock);
          ecore_thread_cancel(sti->fetch_th);
          sti->fetch_th = NULL;
          LKL(sti->lock);
        }
      sti->fetched = EINA_FALSE;
      LKU(sti->lock);
      if (st->cb.unfetch.func)
        st->cb.unfetch.func(st->cb.unfetch.data, sti);
      LKL(sti->lock);
      sti->data = NULL;
      LKU(sti->lock);
    }
}

static void 
_store_genlist_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
  Elm_Store *st = data;
  st->genlist = NULL;
  if (st->list_th)
    {
      ecore_thread_cancel(st->list_th);
      st->list_th = NULL;
    }
  eina_list_free(st->realized);
  while (st->items)
    {
      Elm_Store_Item *sti = (Elm_Store_Item *)st->items;
      if (sti->eval_job) ecore_job_del(sti->eval_job);
      if (sti->fetch_th)
        {
          DBG(".. do cancel3 [%i] [%i] %p for: %s\n", 
              sti->live,
              sti->fetched,
              sti->fetch_th,
              elm_store_item_filesystem_path_get(sti));
          ecore_thread_cancel(sti->fetch_th);
          sti->fetch_th = NULL;
        }
      if (sti->type.filesystem.path)
        {
          eina_stringshare_del(sti->type.filesystem.path);
          sti->type.filesystem.path = NULL;
        }
      if (sti->data)
        {
          if (st->cb.unfetch.func)
            st->cb.unfetch.func(st->cb.unfetch.data, sti);
          sti->data = NULL;
        }
      LKD(sti->lock);
      free(sti);
    }
  // FIXME: kill threads and more
}

////// **** WARNING ***********************************************************
////   * This function runs inside a thread outside efl mainloop. Be careful! *
//     ************************************************************************
static void
_store_filesystem_fetch_do(void *data, Ecore_Thread *th __UNUSED__)
{
  Elm_Store_Item *sti = data;
  LKL(sti->lock);
  if (sti->data)
    {
      LKU(sti->lock);
      return;
    }
  if (!sti->fetched)
    {
      LKU(sti->lock);
      if (sti->store->cb.fetch.func)
        sti->store->cb.fetch.func(sti->store->cb.fetch.data, sti);
      LKL(sti->lock);
      sti->fetched = EINA_TRUE;
    }
  LKU(sti->lock);
}
//     ************************************************************************
////   * End of separate thread function.                                     *
////// ************************************************************************

static void
_store_filesystem_fetch_end(void *data, Ecore_Thread *th)
{
  Elm_Store_Item *sti = data;
  LKL(sti->lock);
  if (sti->data)
    {
      DBG("------UPDATE: %p %s\n", th, elm_store_item_filesystem_path_get(sti));
      elm_genlist_item_update(sti->item);
    }
  LKU(sti->lock);
  if (th == sti->fetch_th) sti->fetch_th = NULL;
}

static void
_store_filesystem_fetch_cancel(void *data, Ecore_Thread *th)
{
  Elm_Store_Item *sti = data;
  DBG("------CANCEL: %p %s\n", th, elm_store_item_filesystem_path_get(sti));
  LKL(sti->lock);
  if (th == sti->fetch_th) sti->fetch_th = NULL;
  if (sti->data)
    {
      DBG("------UPDATE: %p %s\n", th, elm_store_item_filesystem_path_get(sti));
      elm_genlist_item_update(sti->item);
    }
  LKU(sti->lock);
}

static void
_store_item_eval(void *data)
{
  Elm_Store_Item *sti = data;
  sti->eval_job = NULL;
  if (sti->live == sti->was_live) return;
  sti->was_live = sti->live;
  if (sti->live)
    {
      _store_cache_trim(sti->store);
      if (sti->realized)
        sti->store->realized = eina_list_remove(sti->store->realized, sti);
      sti->store->realized = eina_list_append(sti->store->realized, sti);
      sti->realized = EINA_TRUE;
      if (!sti->fetch_th)
        {
          sti->fetch_th = ecore_thread_run(_store_filesystem_fetch_do,
                                           _store_filesystem_fetch_end,
                                           _store_filesystem_fetch_cancel,
                                           sti);
          DBG(".. do fetch [%i] [%i] = %p for: %s\n", 
              sti->live,
              sti->fetched,
              sti->fetch_th,
              elm_store_item_filesystem_path_get(sti));
        }
    }
  else
    {
      if (sti->fetch_th)
        {
          DBG(".. do cancel2 [%i] [%i] %p for: %s\n", 
              sti->live,
              sti->fetched,
              sti->fetch_th,
              elm_store_item_filesystem_path_get(sti));
          ecore_thread_cancel(sti->fetch_th);
          sti->fetch_th = NULL;
        }
      _store_cache_trim(sti->store);
    }
}

static void
_store_genlist_item_realized(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
  Elm_Store *st = data;
  Elm_Genlist_Item *gli = event_info;
  Elm_Store_Item *sti = elm_genlist_item_data_get(gli);
  if (!sti) return;
  st->realized_count++;
  sti->live = EINA_TRUE;
  if (sti->eval_job) ecore_job_del(sti->eval_job);
  sti->eval_job = ecore_job_add(_store_item_eval, sti);
}

static void
_store_genlist_item_unrealized(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
  Elm_Store *st = data;
  Elm_Genlist_Item *gli = event_info;
  Elm_Store_Item *sti = elm_genlist_item_data_get(gli);
  if (!sti) return;
  st->realized_count--;
  sti->live = EINA_FALSE;
  if (sti->eval_job) ecore_job_del(sti->eval_job);
  sti->eval_job = ecore_job_add(_store_item_eval, sti);
}

static char *
_store_item_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
  Elm_Store_Item *sti = data;
  const char *s = "";
  // FIXME: use item mapping for partname -> data pointer offset for string
  LKL(sti->lock);
  if (sti->data) s = sti->type.filesystem.path;
  LKU(sti->lock);
  return strdup(s);
}

static Evas_Object *
_store_item_icon_get(void *data, Evas_Object *obj, const char *part __UNUSED__)
{
  Elm_Store_Item *sti = data;
  char buf[PATH_MAX];
  LKL(sti->lock);
  if (sti->data)
    {
      Evas_Object *ic = elm_icon_add(obj);
      LKU(sti->lock);
      snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
      elm_icon_file_set(ic, buf, NULL);
      evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
      return ic;
    }
  LKU(sti->lock);
  return NULL;
}

static void
_store_item_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

////// **** WARNING ***********************************************************
////   * This function runs inside a thread outside efl mainloop. Be careful! *
//     ************************************************************************
static int
_store_filesystem_sort_cb(void *d1, void *d2)
{
  Elm_Store_Item_Info *info1 = d1, *info2 = d2;
  if ((!info1->sort_id) || (!info2->sort_id)) return 0;
  return strcoll(info1->sort_id, info2->sort_id);
}

static void
_store_filesystem_list_do(void *data, Ecore_Thread *th __UNUSED__)
{
  Elm_Store *st = data;
  Eina_Iterator *it;
  const Eina_File_Direct_Info *finf;
  Eina_List *sorted = NULL;
  Elm_Store_Item_Info *info;

  // FIXME: need a way to abstract the open, list, feed items from list
  // and maybe get initial sortable key vals etc.
  it = eina_file_stat_ls(st->filesystem.dir);
  if (!it) return;
  EINA_ITERATOR_FOREACH(it, finf)
    {
      Eina_Bool ok;

      info = calloc(1, sizeof(Elm_Store_Item_Info) + finf->path_length + 1);
      if (!info) continue;
      info->type.filesystem.path = ((char *)info) + sizeof(Elm_Store_Item_Info);
      memcpy(info->type.filesystem.path, finf->path, finf->path_length + 1);
      ok = EINA_TRUE;
      if (st->cb.list.func)
        ok = st->cb.list.func(st->cb.list.data, info);
      if (ok)
        {
          if (!st->sorted) ecore_thread_feedback(th, info);
          else sorted = eina_list_append(sorted, info);
        }
      else
        {
          if (info->sort_id) free(info->sort_id);
          free(info);
        }
      if (ecore_thread_check(th)) break;
    }
  eina_iterator_free(it);
  if (sorted)
    {
      sorted = eina_list_sort(sorted, 0,
                              EINA_COMPARE_CB(_store_filesystem_sort_cb));
      EINA_LIST_FREE(sorted, info)
        {
          if (!ecore_thread_check(th)) ecore_thread_feedback(th, info);
        }
    }
}
//     ************************************************************************
////   * End of separate thread function.                                     *
////// ************************************************************************

static void
_store_filesystem_list_end(void *data, Ecore_Thread *th)
{
  Elm_Store *st = data;
  if (th == st->list_th) st->list_th = NULL;
}

static void
_store_filesystem_list_cancel(void *data, Ecore_Thread *th)
{
  Elm_Store *st = data;
  if (th == st->list_th) st->list_th = NULL;
}

static void
_store_filesystem_list_update(void *data, Ecore_Thread *th __UNUSED__, void *msg)
{
  Elm_Store *st = data;
  Elm_Store_Item *sti;
  Elm_Genlist_Item_Class *itc;
  Elm_Store_Item_Info *info = msg;
  
  sti = calloc(1, sizeof(Elm_Store_Item));
  if (!sti) goto done;
  LKI(sti->lock);
  EINA_MAGIC_SET(sti, ELM_STORE_ITEM_MAGIC);
  sti->store = st;
  sti->type.filesystem.path = eina_stringshare_add(info->type.filesystem.path);
  sti->data = info->data;
  
  itc = info->item_class;
  if (!itc) itc = &_store_item_class;
  else
    {
      itc->func.label_get = _store_item_label_get;
      itc->func.icon_get  = _store_item_icon_get;
      itc->func.state_get = NULL; // FIXME: support state gets later
      itc->func.del       = _store_item_del;
    }

  // FIXME: handle being a parent (tree)
  sti->item = elm_genlist_item_append(st->genlist, itc,
                                      sti/* item data */,
                                      NULL/* parent */,
                                      ELM_GENLIST_ITEM_NONE,
                                      NULL/* func */,
                                      NULL/* func data */);
  st->items = eina_inlist_append(st->items, (Eina_Inlist *)sti);
done:
  if (info->sort_id) free(info->sort_id);
  free(info);
}

// public api calls
EAPI Elm_Store *
elm_store_new(void)
{
  Elm_Store *st;
  
  st = calloc(1, sizeof(Elm_Store));

  // TODO: BEGIN - move to elm_store_init()
  eina_magic_string_set(ELM_STORE_MAGIC, "Elm_Store");
  eina_magic_string_set(ELM_STORE_ITEM_MAGIC, "Elm_Store_Item");
  // setup default item class (always the same) if list cb doesnt provide one
  _store_item_class.item_style = "default";
  _store_item_class.func.label_get = _store_item_label_get;
  _store_item_class.func.icon_get  = _store_item_icon_get;
  _store_item_class.func.state_get = NULL; // FIXME: support state gets later
  _store_item_class.func.del       = _store_item_del;
  // TODO: END - move to elm_store_init()

  EINA_MAGIC_SET(st, ELM_STORE_MAGIC);
  st->cache_max = 128;
  return st;
}

EAPI void
elm_store_free(Elm_Store *st)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return;
  if (st->filesystem.dir)
    {
      eina_stringshare_del(st->filesystem.dir);
      st->filesystem.dir = NULL;
    }
  if (st->list_th)
    {
      ecore_thread_cancel(st->list_th);
      st->list_th = NULL;
    }
  eina_list_free(st->realized);
  while (st->items)
    {
      Elm_Store_Item *sti = (Elm_Store_Item *)st->items;
      if (sti->eval_job) ecore_job_del(sti->eval_job);
      if (sti->fetch_th)
        {
          DBG(".. do cancel3 [%i] [%i] %p for: %s\n", 
              sti->live,
              sti->fetched,
              sti->fetch_th,
              elm_store_item_filesystem_path_get(sti));
          ecore_thread_cancel(sti->fetch_th);
          sti->fetch_th = NULL;
        }
      if (sti->type.filesystem.path)
        {
          eina_stringshare_del(sti->type.filesystem.path);
          sti->type.filesystem.path = NULL;
        }
      if (sti->data)
        {
          if (st->cb.unfetch.func)
            st->cb.unfetch.func(st->cb.unfetch.data, sti);
          sti->data = NULL;
        }
      LKD(sti->lock);
      free(sti);
    }
  if (st->genlist)
    {
      evas_object_event_callback_del_full(st->genlist, EVAS_CALLBACK_DEL, _store_genlist_del, st);
      evas_object_smart_callback_del(st->genlist, "realized", _store_genlist_item_realized);
      evas_object_smart_callback_del(st->genlist, "unrealized", _store_genlist_item_unrealized);
      elm_genlist_clear(st->genlist);
      st->genlist = NULL;
    }
  free(st);
}

EAPI void
elm_store_target_genlist_set(Elm_Store *st, Evas_Object *obj)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return;
  if (st->genlist == obj) return;
  if (st->genlist)
    {
      evas_object_event_callback_del_full(st->genlist, EVAS_CALLBACK_DEL, _store_genlist_del, st);
      evas_object_smart_callback_del(st->genlist, "realized", _store_genlist_item_realized);
      evas_object_smart_callback_del(st->genlist, "unrealized", _store_genlist_item_unrealized);
      elm_genlist_clear(st->genlist);
    }
  st->genlist = obj;
  if (!st->genlist) return;
  evas_object_smart_callback_add(st->genlist, "realized", _store_genlist_item_realized, st);
  evas_object_smart_callback_add(st->genlist, "unrealized", _store_genlist_item_unrealized, st);
  evas_object_event_callback_add(st->genlist, EVAS_CALLBACK_DEL, _store_genlist_del, st);
  elm_genlist_clear(st->genlist);
}

EAPI void
elm_store_source_filesystem_set(Elm_Store *st, const char *dir)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return;
  if (st->list_th)
    {
      ecore_thread_cancel(st->list_th);
      st->list_th = NULL;
    }
  if (!eina_stringshare_replace(&st->filesystem.dir, dir)) return;
  st->list_th = ecore_thread_feedback_run(_store_filesystem_list_do,
                                          _store_filesystem_list_update,
                                          _store_filesystem_list_end,
                                          _store_filesystem_list_cancel,
                                          st, EINA_TRUE);
}

EAPI const char *
elm_store_source_filesystem_get(const Elm_Store *st)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return NULL;
  return st->filesystem.dir;
}

EAPI void
elm_store_cache_set(Elm_Store *st, int max)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return;
  if (max < 0) max = 0;
  st->cache_max = max;
  _store_cache_trim(st);
}

EAPI int
elm_store_cache_get(const Elm_Store *st)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return 0;
  return st->cache_max;
}

EAPI void
elm_store_list_func_set(Elm_Store *st, Elm_Store_Item_List_Cb func, const void *data)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return;
  st->cb.list.func = func;
  st->cb.list.data = (void *)data;
}

EAPI void
elm_store_fetch_func_set(Elm_Store *st, Elm_Store_Item_Fetch_Cb func, const void *data)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return;
  st->cb.fetch.func = func;
  st->cb.fetch.data = (void *)data;
}

EAPI void
elm_store_unfetch_func_set(Elm_Store *st, Elm_Store_Item_Unfetch_Cb func, const void *data)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return;
  st->cb.unfetch.func = func;
  st->cb.unfetch.data = (void *)data;
}

EAPI void
elm_store_sorted_set(Elm_Store *st, Eina_Bool sorted)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return;
  st->sorted = sorted;
}

EAPI Eina_Bool
elm_store_sorted_get(const Elm_Store *st)
{
  if (!EINA_MAGIC_CHECK(st, ELM_STORE_MAGIC)) return EINA_FALSE;
  return st->sorted;
}

EAPI void
elm_store_item_data_set(Elm_Store_Item *sti, void *data)
{
  if (!EINA_MAGIC_CHECK(sti, ELM_STORE_ITEM_MAGIC)) return;
  LKL(sti->lock);
  sti->data = data;
  LKU(sti->lock);
}

EAPI void *
elm_store_item_data_get(Elm_Store_Item *sti)
{
  if (!EINA_MAGIC_CHECK(sti, ELM_STORE_ITEM_MAGIC)) return;
  void *d;
  LKL(sti->lock);
  d = sti->data;
  LKU(sti->lock);
  return d;
}

EAPI const Elm_Store *
elm_store_item_store_get(const Elm_Store_Item *sti)
{
  if (!EINA_MAGIC_CHECK(sti, ELM_STORE_ITEM_MAGIC)) return;
  // dont need lock
  return sti->store;
}

EAPI const Elm_Genlist_Item *
elm_store_item_genlist_item_get(const Elm_Store_Item *sti)
{
  if (!EINA_MAGIC_CHECK(sti, ELM_STORE_ITEM_MAGIC)) return;
  // dont need lock
  return sti->item;
}

EAPI const char *
elm_store_item_filesystem_path_get(const Elm_Store_Item *sti)
{
  if (!EINA_MAGIC_CHECK(sti, ELM_STORE_ITEM_MAGIC)) return;
  // dont need lock
  return sti->type.filesystem.path;
}
















typedef struct _My_Item My_Item;

struct _My_Item
{
  char *from, *subject, *date, *head_content;
};

// callbacks just to see user interacting with genlist
static void
_st_selected(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("selected: %p\n", event_info);
}

static void
_st_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("clicked: %p\n", event_info);
}

static void
_st_longpress(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("longpress %p\n", event_info);
}

// store callbacks to handle loading/parsing/freeing of store items from src
static Elm_Genlist_Item_Class itc1;


////// **** WARNING ***********************************************************
////   * This function runs inside a thread outside efl mainloop. Be careful! *
//     ************************************************************************
static Eina_Bool
_st_store_list(void *data __UNUSED__, Elm_Store_Item_Info *info)
{
  int id;
  char sort_id[7];

  // create a sort id based on the filename itself assuming it is a numeric
  // value like the id number in mh mail folders which is what this test
  // uses as a data source
  char *file = strrchr(info->type.filesystem.path, '/');
  if (file) file++;
  else file = info->type.filesystem.path;
  id = atoi(file);
  sort_id[0] = ((id >> 30) & 0x3f) + 32;
  sort_id[1] = ((id >> 24) & 0x3f) + 32;
  sort_id[2] = ((id >> 18) & 0x3f) + 32;
  sort_id[3] = ((id >> 12) & 0x3f) + 32;
  sort_id[4] = ((id >>  6) & 0x3f) + 32;
  sort_id[5] = ((id >>  0) & 0x3f) + 32;
  sort_id[6] = 0;
  info->sort_id = strdup(sort_id);
  // choose the item genlist item class to use (only item style should be
  // provided by the app, store will fill everything else in, so it also
  // has to be writable
  info->item_class = &itc1; // based on item info - return the item class wanted (only style field used - rest reset to internal funcs store sets up to get label/icon etc)
  info->data = NULL; // if we can already parse and load all of item here and want to - set this
  return EINA_TRUE; // return true to include this, false not to
}
//     ************************************************************************
////   * End of separate thread function.                                     *
////// ************************************************************************

////// **** WARNING ***********************************************************
////   * This function runs inside a thread outside efl mainloop. Be careful! *
//     ************************************************************************
static void
_st_store_fetch(void *data __UNUSED__, Elm_Store_Item *sti)
{
  const char *path = elm_store_item_filesystem_path_get(sti);
  My_Item *myit;
  FILE *f;
  char buf[4096], *p;
  Eina_Bool have_content = EINA_FALSE;
  char *content = NULL, *content_pos = NULL, *content_end = NULL;

  // if we already have my item data - skip
  if (elm_store_item_data_get(sti)) return;
  // open the mail file and parse it
  f = fopen(path, "r");
  if (!f) return;
  
  DBG("parse %s\n", path);
  // alloc my item in memory that holds data to show in the list
  myit = calloc(1, sizeof(My_Item));
  if (!myit)
    {
      fclose(f);
      return;
    }
  while (fgets(buf, sizeof(buf), f))
    {
      if (!have_content)
        {
          if (!isblank(buf[0]))
            {
              // get key: From:, Subject: etc.
              if (!strncmp(buf, "From:", 5))
                {
                  p = buf + 5;
                  while ((*p) && (isblank(*p))) p++;
                  p = strdup(p);
                  if (p)
                    {
                      myit->from = p;
                      p = strchr(p, '\n');
                      if (p) *p = 0;
                    }
                }
              else if (!strncmp(buf, "Subject:", 8))
                {
                  p = buf + 8;
                  while ((*p) && (isblank(*p))) p++;
                  p = strdup(p);
                  if (p)
                    {
                      myit->subject = p;
                      p = strchr(p, '\n');
                      if (p) *p = 0;
                    }
                }
              else if (!strncmp(buf, "Date:", 5))
                {
                  p = buf + 5;
                  while ((*p) && (isblank(*p))) p++;
                  p = strdup(p);
                  if (p)
                    {
                      myit->date = p;
                      p = strchr(p, '\n');
                      if (p) *p = 0;
                    }
                }
              else if (buf[0] == '\n') // begin of content
                have_content = EINA_TRUE;
            }
        }
      else
        {
          // get first 320 bytes of content/body
          if (!content)
            {
              content = calloc(1, 320);
              content_pos = content;
              content_end = content + 319;
            }
          strncat(content_pos, buf, content_end - content_pos - 1);
          content_pos = content + strlen(content);
        }
    }
  fclose(f);
  myit->head_content = content;
  elm_store_item_data_set(sti, myit);
}
//     ************************************************************************
////   * End of separate thread function.                                     *
////// ************************************************************************

static void
_st_store_unfetch(void *data __UNUSED__, Elm_Store_Item *sti)
{
  My_Item *myit = elm_store_item_data_get(sti);
  if (!myit) return;
  if (myit->from) free(myit->from);
  if (myit->subject) free(myit->subject);
  if (myit->date) free(myit->date);
  if (myit->head_content) free(myit->head_content);
  free(myit);
  elm_store_item_data_set(sti, NULL);
}

void
test_store(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
  Evas_Object *win, *bg, *gl, *bx;
  
  Elm_Store *st;

  win = elm_win_add(NULL, "store", ELM_WIN_BASIC);
  elm_win_title_set(win, "Store");
  elm_win_autodel_set(win, 1);
  
  bg = elm_bg_add(win);
  elm_win_resize_object_add(win, bg);
  evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_show(bg);
  
  bx = elm_box_add(win);
  evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_win_resize_object_add(win, bx);
  evas_object_show(bx);
  
  gl = elm_genlist_add(win);
  elm_genlist_compress_mode_set(gl, EINA_TRUE);
  evas_object_smart_callback_add(gl, "selected", _st_selected, NULL);
  evas_object_smart_callback_add(gl, "clicked", _st_clicked, NULL);
  evas_object_smart_callback_add(gl, "longpressed", _st_longpress, NULL);
  evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_box_pack_end(bx, gl);
  evas_object_show(gl);

  itc1.item_style     = "default"; // set up 1 item style to use
  
  st = elm_store_new();
  elm_store_list_func_set(st, _st_store_list, NULL);
  elm_store_fetch_func_set(st, _st_store_fetch, NULL);
  elm_store_unfetch_func_set(st, _st_store_unfetch, NULL);
  elm_store_sorted_set(st, EINA_TRUE);
  elm_store_target_genlist_set(st, gl);
  elm_store_source_filesystem_set(st, "/home/raster/tst");
  
  evas_object_resize(win, 480, 800);
  evas_object_show(win);
}
#endif
