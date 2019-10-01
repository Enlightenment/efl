#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdint.h>
#include <libgen.h>

#include <Efl.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Eo.h>
#include <Efreet_Mime.h>

#include "Eio.h"

#include "eio_private.h"
#include "efl_io_model_private.h"

#define MY_CLASS EFL_IO_MODEL_CLASS
#define MY_CLASS_NAME "Efl_Io_Model"

static void _efl_io_model_info_free(Efl_Io_Model_Info *info, Eina_Bool model);
static void _efl_io_model_efl_model_monitor_add(Efl_Io_Model_Data *priv);

EINA_VALUE_STRUCT_DESC_DEFINE(_eina_file_direct_info_desc,
                              NULL,
                              sizeof (Efl_Io_Model_Info),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Efl_Io_Model_Info, path_length),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Efl_Io_Model_Info, name_length),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Efl_Io_Model_Info, name_start),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_UINT, Efl_Io_Model_Info, type),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_STRINGSHARE, Efl_Io_Model_Info, path));
/**
 *  Callbacks
 *  Property
 */
static void
_eio_move_done_cb(void *data, Eio_File *handler)
{
   Efl_Io_Model_Data *pd = ecore_thread_local_data_find(handler->thread, ".pd");
   Eina_Promise *p = data;

   // FIXME: generate events

   eina_promise_resolve(p, eina_value_string_init(pd->path));
   pd->request.move = NULL;
}

static void
_eio_file_error_cb(void *data, Eio_File *handler, int error)
{
   Efl_Io_Model_Data *pd = ecore_thread_local_data_find(handler->thread, ".pd");
   Eina_Promise *p = data;

   eina_promise_reject(p, error);
   pd->request.move = NULL;
}


static Eina_Bool
_already_added(Efl_Io_Model_Data *pd, const char *path)
{
   Efl_Io_Model_Info *mi;
   Eina_List *node;

   EINA_LIST_FOREACH(pd->files, node, mi)
     {
        if (eina_streq(mi->path, path))
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

/**
 *  Callbacks
 *  Ecore Events
 */
static Eina_Bool
_efl_model_evt_added_ecore_cb(void *data, int type, void *event)
{
   Eio_Monitor_Event *ev = event;
   Efl_Io_Model *obj;
   Efl_Io_Model_Data *pd = data;
   Efl_Model_Children_Event cevt;
   Efl_Io_Model_Info *mi;
   Eina_List *l;
   Eina_Stringshare *spath = NULL;
   char *path = NULL;

   if (type != EIO_MONITOR_DIRECTORY_CREATED && type != EIO_MONITOR_FILE_CREATED)
     return EINA_TRUE;

   if (ev->monitor != pd->monitor) return EINA_TRUE;

   if (_already_added(pd, ev->filename))
     return EINA_TRUE;

   obj = pd->self;

   path = ecore_file_dir_get(ev->filename);
   if (!eina_streq(pd->path, path))
     goto end;

   spath = eina_stringshare_add(ev->filename);

   EINA_LIST_FOREACH(pd->files, l, mi)
     {
        if (mi->path == spath)
          goto end;
     }

   mi = calloc(1, sizeof (Efl_Io_Model_Info));
   if (!mi) goto end;

   mi->path_length = eina_stringshare_strlen(spath);
   mi->path = eina_stringshare_ref(spath);
   mi->name_start = eina_stringshare_strlen(pd->path) + 1;
   mi->name_length = mi->path_length - mi->name_start;
   mi->type = EINA_FILE_UNKNOWN;
   mi->parent_ref = EINA_FALSE;
   mi->child_ref = EINA_TRUE;

   // Honor filter on new added file too
   if (pd->filter.cb)
     {
        Eina_File_Direct_Info info = { 0 };

        info.path_length = mi->path_length;
        info.name_start = mi->name_start;
        info.name_length = mi->name_length;
        info.type = EINA_FILE_UNKNOWN;
        strncpy(info.path, mi->path, (EINA_PATH_MAX - 1));

        if (!pd->filter.cb(pd->filter.data, obj, &info))
          {
             eina_stringshare_replace(&mi->path, NULL);
             free(mi);
             goto end;
          }
     }

   cevt.index = eina_list_count(pd->files);
   pd->files = eina_list_append(pd->files, mi);

   // Notify of the new child being added
   efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
   efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);

 end:
   eina_stringshare_del(spath);
   free(path);

   return EINA_TRUE;
}

static void
_model_child_remove(Efl_Io_Model_Data *pd, Eina_Stringshare *path)
{
   Efl_Io_Model_Info *mi;
   Eina_List *l;
   Efl_Io_Model *obj = pd->self;
   Efl_Model_Children_Event cevt = { 0 };
   unsigned int i = 0;

   // FIXME: Linear search is pretty slow
   EINA_LIST_FOREACH(pd->files, l, mi)
     {
        if (mi->path == path)
          break ;
        ++i;
     }

   if (i >= eina_list_count(pd->files))
     return;

   cevt.index = i;
   cevt.child = mi->object;

   efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_REMOVED, &cevt);
   efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);

   // Remove the entry from the files list
   pd->files = eina_list_remove_list(pd->files, l);

   // This will only trigger the data destruction if no object is referencing them.
   _efl_io_model_info_free(mi, EINA_FALSE);
}

static Eina_Bool
_efl_model_evt_deleted_ecore_cb(void *data, int type, void *event)
{
   Eio_Monitor_Event *ev = event;
   Efl_Io_Model_Data *pd = data;
   Eina_Stringshare *spath = NULL;

   if (type != EIO_MONITOR_DIRECTORY_DELETED && type != EIO_MONITOR_FILE_DELETED)
     return EINA_TRUE;

   if (ev->monitor != pd->monitor) return EINA_TRUE;

   spath = eina_stringshare_add(ev->filename);
   _model_child_remove(pd, spath);
   eina_stringshare_del(spath);

   return EINA_TRUE;
}

/**
 *  Callbacks
 *  Child Del
 */
static void
_eio_del_cleanup(Efl_Io_Model *obj)
{
   Efl_Io_Model_Data *pd = efl_data_scope_get(obj, EFL_IO_MODEL_CLASS);

   pd->request.del = NULL;
   efl_unref(obj);
}

static void
_eio_done_unlink_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   Efl_Io_Model *child = data;
   Efl_Io_Model_Data *child_pd, *pd;

   child_pd = efl_data_scope_get(child, MY_CLASS);
   pd = efl_data_scope_get(efl_parent_get(child), MY_CLASS);

   _model_child_remove(pd, child_pd->path);
   _eio_del_cleanup(child);
}

static void
_eio_error_unlink_cb(void *data, Eio_File *handler EINA_UNUSED, int error)
{
   Efl_Io_Model *child = data;

   ERR("%d: %s.", error, strerror(error));

   _eio_del_cleanup(child);
}

static void
_efl_io_model_info_free(Efl_Io_Model_Info *info, Eina_Bool model)
{
   if (!info) return ;

   if (!model)
     {
        info->child_ref = EINA_FALSE;
     }
   else
     {
        info->parent_ref = EINA_FALSE;
     }

   if (info->child_ref ||
       info->parent_ref)
     return ;

   eina_stringshare_replace(&info->path, NULL);
   free(info);
}

static Eina_File_Type
_efl_io_model_info_type_get(const Eina_File_Direct_Info *info, const Eina_Stat *st)
{
   if (info && info->type != EINA_FILE_UNKNOWN)
     return info->type;
   if (st)
     {
        if (S_ISREG(st->mode))
          return EINA_FILE_REG;
        else if (S_ISDIR(st->mode))
          return EINA_FILE_DIR;
        else if (S_ISCHR(st->mode))
          return EINA_FILE_CHR;
        else if (S_ISBLK(st->mode))
          return EINA_FILE_BLK;
        else if (S_ISFIFO(st->mode))
          return EINA_FILE_FIFO;
#ifndef _WIN32
        else if (S_ISLNK(st->mode))
          return EINA_FILE_LNK;
#endif
#ifdef S_ISSOCK
        else if (S_ISSOCK(st->mode))
          return EINA_FILE_SOCK;
#endif
     }
   return EINA_FILE_UNKNOWN;
}

static void
_efl_io_model_info_build(const Efl_Io_Model *model, Efl_Io_Model_Data *pd)
{
   char *path;

   if (pd->info) goto end;

   pd->info = calloc(1, sizeof (Efl_Io_Model_Info));
   if (!pd->info) return ;

   pd->info->path_length = eina_stringshare_strlen(pd->path);
   pd->info->path = eina_stringshare_ref(pd->path);

   path = strdup(pd->path);
   pd->info->name_start = basename(path) - path;
   pd->info->name_length = pd->info->path_length - pd->info->name_start;
   free(path);

   pd->info->type = _efl_io_model_info_type_get(NULL, pd->st);

   efl_model_properties_changed(model, "direct_info");

 end:
   pd->info->parent_ref = EINA_TRUE;
}

static void
_eio_build_st_done(void *data, Eio_File *handler EINA_UNUSED, const Eina_Stat *stat)
{
   Efl_Io_Model *model = data;
   Efl_Io_Model_Data *pd = efl_data_scope_get(model, EFL_IO_MODEL_CLASS);

   if (!pd) return ;
   pd->request.stat = NULL;

   pd->st = malloc(sizeof (Eina_Stat));
   if (!pd->st) return ;

   memcpy(pd->st, stat, sizeof (Eina_Stat));

   if (!pd->info) _efl_io_model_info_build(model, pd);
   if (pd->info->type == EINA_FILE_UNKNOWN)
     pd->info->type = _efl_io_model_info_type_get(NULL, stat);

   efl_model_properties_changed(model, "mtime", "atime", "ctime", "is_dir", "is_lnk", "size", "stat");

   if (eio_file_is_dir(pd->st))
     {
        // Now that we know we are a directory, we should whatch it
        _efl_io_model_efl_model_monitor_add(pd);

        // And start listing its child
        efl_model_children_count_get(model);
     }

   efl_unref(model);
}

static void
_eio_build_st_done_clobber(void *data, Eio_File *handler, const Eina_Stat *stat)
{
   Efl_Io_Model *model = data;
   Efl_Io_Model *parent;

   efl_ref(model); // For st_done
   _eio_build_st_done(data, handler, stat);
   parent = efl_parent_get(model);
   efl_model_child_del(parent, model);
   efl_unref(model); // From the async thread early ref
}

static void
_eio_build_st_error(void *data, Eio_File *handler EINA_UNUSED, int error)
{
   Efl_Io_Model *model = data;
   Efl_Io_Model_Data *pd = efl_data_scope_get(model, EFL_IO_MODEL_CLASS);

   pd->request.stat = NULL;
   pd->error = error;

   efl_model_properties_changed(model, "direct_info", "mtime", "atime", "ctime", "is_dir", "is_lnk", "size", "stat");

   efl_unref(model);
}

static void
_eio_build_st_error_clobber(void *data, Eio_File *handler, int error)
{
   Efl_Io_Model *model = data;

   efl_ref(model); // For st_error unref
   _eio_build_st_error(data, handler, error);
   efl_unref(model); // From the async thread early ref
}

static void
_eio_build_st(const Efl_Io_Model *model, Efl_Io_Model_Data *pd)
{
   if (pd->st) return ;
   if (pd->request.stat) return ;
   if (pd->error) return ;

   pd->request.stat = eio_file_direct_stat(pd->path,
                                           _eio_build_st_done,
                                           _eio_build_st_error,
                                           efl_ref(model));
}

static void
_eio_build_st_then_clobber(const Efl_Io_Model *model, Efl_Io_Model_Data *pd)
{
   if (pd->st) return ;
   if (pd->request.stat) return ;
   if (pd->error) return ;

   pd->request.stat = eio_file_direct_stat(pd->path,
                                           _eio_build_st_done_clobber,
                                           _eio_build_st_error_clobber,
                                           efl_ref(model));
}

static Eina_List *delayed_queue = NULL;
static Eina_Bool delayed_one = EINA_FALSE;

static void
_delayed_flush(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Promise *p;
   Eina_List *tmp = delayed_queue;

   efl_event_callback_del(ev->object, EFL_LOOP_EVENT_IDLE, _delayed_flush, NULL);

   delayed_one = EINA_FALSE;
   delayed_queue = NULL;
   EINA_LIST_FREE(tmp, p)
     eina_promise_resolve(p, EINA_VALUE_EMPTY);
}

static Eina_Value
_cancel_request(Eo *model EINA_UNUSED, void *data, Eina_Error error)
{
   delayed_queue = eina_list_remove_list(delayed_queue, data);

   return eina_value_error_init(error);
}

static Eina_Future *
_build_delay(Efl_Io_Model *model)
{
   Eina_Promise *p;

   p = efl_loop_promise_new(model);

   if (!delayed_queue)
     {
        // Remove callback, just in case it is still there.
        efl_event_callback_del(efl_loop_get(model), EFL_LOOP_EVENT_IDLE, _delayed_flush, NULL);
        efl_event_callback_add(efl_loop_get(model), EFL_LOOP_EVENT_IDLE, _delayed_flush, NULL);
        // FIXME: It would be nice to be able to build a future directly to be triggered on one event
     }

   delayed_queue = eina_list_append(delayed_queue, p);

   return efl_future_then(model, eina_future_new(p),
                          .error = _cancel_request,
                          .data = eina_list_last(delayed_queue));
}

static Eina_Value
_eio_build_mime_now(Eo *model, void *data, const Eina_Value v)
{
   Efl_Io_Model_Data *pd = data;

   // Make sure that we are not over consuming time in the main loop
   if (!delayed_one &&
       (delayed_queue || ecore_time_get() - ecore_loop_time_get() > 0.004))
     {
        Eina_Future *f = efl_future_then(model, _build_delay(model),
                                         .success = _eio_build_mime_now,
                                         .data = pd);
        return eina_future_as_value(efl_future_then(model, f));
     }

   pd->mime_type = efreet_mime_type_get(pd->path);

   efl_model_properties_changed(model, "mime_type");
   delayed_one = EINA_TRUE;

   return v;
}

static void
_eio_build_mime(const Efl_Object *model, Efl_Io_Model_Data *pd)
{
   if (pd->mime_type) return ;
   if (pd->request.mime) return ;

   pd->request.mime = efl_future_then(model, efl_loop_job(efl_loop_get(model)),
                                      .success = _eio_build_mime_now,
                                      .data = pd);
}

static Eina_Value *
_property_filename_cb(const Eo *obj, Efl_Io_Model_Data *pd)
{
   _efl_io_model_info_build(obj, pd);
   if (pd->info)
     return eina_value_string_new(pd->info->path + pd->info->name_start);
   return eina_value_error_new(EAGAIN);
}

static Eina_Value *
_property_direct_info_cb(const Eo *obj, Efl_Io_Model_Data *pd)
{
   _efl_io_model_info_build(obj, pd);

   if (pd->info)
     {
        Eina_Value *r;

        r = eina_value_struct_new(_eina_file_direct_info_desc());
        if (!r) return NULL;
        if (!eina_value_pset(r, pd->info))
          {
             eina_value_free(r);
             return NULL;
          }

        return r;
     }
   if (pd->error)
     return eina_value_error_new(pd->error);
   return eina_value_error_new(EAGAIN);
}

#define TIMECB(Prop)                                                    \
  static Eina_Value *                                                   \
  _property_##Prop##_cb(const Eo *obj, Efl_Io_Model_Data *pd)              \
  {                                                                     \
     if (pd->st)                                                        \
       return eina_value_time_new(pd->st->Prop);                        \
     if (pd->error)                                                     \
       return eina_value_error_new(pd->error);                          \
                                                                        \
     _eio_build_st(obj, pd);                                            \
     return eina_value_error_new(EAGAIN);                               \
  }

TIMECB(mtime);
TIMECB(atime);
TIMECB(ctime);

static Eina_Value *
_property_is_dir_cb(const Eo *obj, Efl_Io_Model_Data *pd)
{
   if (pd->st)
     return eina_value_bool_new(eio_file_is_dir(pd->st));
   if (pd->error)
     return eina_value_error_new(pd->error);

   _eio_build_st(obj, pd);
   return eina_value_error_new(EAGAIN);
}

static Eina_Value *
_property_is_lnk_cb(const Eo *obj, Efl_Io_Model_Data *pd)
{
   if (pd->st)
     return eina_value_bool_new(eio_file_is_lnk(pd->st));
   if (pd->error)
     return eina_value_error_new(pd->error);

   _eio_build_st(obj, pd);
   return eina_value_error_new(EAGAIN);
}

static Eina_Value *
_property_size_cb(const Eo *obj, Efl_Io_Model_Data *pd)
{
   if (pd->st)
     return eina_value_ulong_new(pd->st->size);
   if (pd->error)
     return eina_value_error_new(pd->error);

   _eio_build_st(obj, pd);
   return eina_value_error_new(EAGAIN);
}

static Eina_Value *
_property_stat_cb(const Eo *obj, Efl_Io_Model_Data *pd)
{
   if (pd->st)
     {
        Eina_Value *r;

        r = eina_value_struct_new(_eina_stat_desc());
        if (!r) return NULL;
        if (!eina_value_pset(r, pd->info))
          {
             eina_value_free(r);
             return NULL;
          }

        return r;
     }
   if (pd->error)
     return eina_value_error_new(pd->error);

   _eio_build_st(obj, pd);
   return eina_value_error_new(EAGAIN);
}

static Eina_Value *
_property_mime_type_cb(const Eo *obj, Efl_Io_Model_Data *pd)
{
   if (pd->mime_type)
     return eina_value_string_new(pd->mime_type);
   if (pd->error) return eina_value_error_new(pd->error);

   _eio_build_mime(obj, pd);
   return eina_value_error_new(EAGAIN);
}

#define PP(Name)                     \
  { #Name, _property_##Name##_cb }

static struct {
   const char *name;
   Eina_Value *(*cb)(const Eo *obj, Efl_Io_Model_Data *pd);
} properties[] = {
  PP(filename),
  PP(direct_info),
  PP(mtime), PP(atime), PP(ctime), PP(is_dir), PP(is_lnk), PP(size),
  PP(stat),
  PP(mime_type)
};

typedef struct _Efl_Io_Model_Iterator Efl_Io_Model_Iterator;
struct _Efl_Io_Model_Iterator
{
   Eina_Iterator iterator;
   unsigned int i;
   unsigned int end;
};

static Eina_Bool
_efl_io_model_iterator_next(Efl_Io_Model_Iterator *it, void **data)
{
   const char **name = (const char **)data;

   if (it->i >= it->end)
     return EINA_FALSE;

   *name = properties[it->i].name;
   it->i++;

   return EINA_TRUE;
}

static void*
_efl_io_model_iterator_get_container(Efl_Io_Model_Iterator *it EINA_UNUSED)
{
   return &properties;
}

static void
_efl_io_model_iterator_free(Efl_Io_Model_Iterator *it)
{
   free(it);
}

Eina_Iterator *
_efl_io_model_properties_iterator_new(void)
{
   Efl_Io_Model_Iterator *it;

   it = calloc(1, sizeof (Efl_Io_Model_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->i = 0;
   it->end = EINA_C_ARRAY_LENGTH(properties);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_efl_io_model_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
      _efl_io_model_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_efl_io_model_iterator_free);

   return &it->iterator;
}

/**
 * Interfaces impl.
 */
static Eina_Iterator *
_efl_io_model_efl_model_properties_get(const Eo *obj EINA_UNUSED,
                                       Efl_Io_Model_Data *pd EINA_UNUSED)
{
   return _efl_io_model_properties_iterator_new();
}

static Eina_Value *
_efl_io_model_efl_model_property_get(const Eo *obj, Efl_Io_Model_Data *pd, const char *property)
{
   unsigned int i;

   if (!property) return NULL;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(properties); ++i)
     if (property == properties[i].name ||
         eina_streq(property, properties[i].name))
       return properties[i].cb(obj, pd);

   return efl_model_property_get(efl_super(obj, EFL_IO_MODEL_CLASS), property);
}

static Eina_Future *
_efl_io_model_efl_model_property_set(Eo *obj,
                                     Efl_Io_Model_Data *pd,
                                     const char *property, Eina_Value *value)
{
   const char *path;
   Eina_Future *f;
   Eina_Value s = EINA_VALUE_EMPTY;
   Eina_Error err = EFL_MODEL_ERROR_NOT_FOUND;
   Eina_Bool finalized = !!pd->path;

   if (!property) goto on_error;

   err = EFL_MODEL_ERROR_NOT_SUPPORTED;
   if (!eina_streq(property, "path")) goto on_error;

   if (finalized && pd->request.move) goto on_error;

   err = EFL_MODEL_ERROR_UNKNOWN;
   if (!eina_value_setup(&s, EINA_VALUE_TYPE_STRING)) goto on_error;
   if (!eina_value_convert(value, &s)) goto on_error;
   if (!eina_value_string_get(&s, &path)) goto on_error;

   eina_stringshare_replace(&pd->path, path);

   if (finalized)
     {
        Eina_Promise *p = efl_loop_promise_new(obj);
        f = eina_future_new(p);

        pd->request.move = eio_file_move(pd->path, path,
                                         NULL, /* FIXME: have a move progress property */
                                         _eio_move_done_cb, _eio_file_error_cb, p);

        ecore_thread_local_data_add(pd->request.move->thread, ".pd", pd, NULL, EINA_TRUE);

        f = _efl_io_manager_future(obj, f, pd->request.move);

        // FIXME: turn on monitor in the finalize stage or after move
     }
   else
     {
        f = efl_loop_future_resolved(obj, eina_value_stringshare_init(pd->path));
     }

   return f;

 on_error:
   return efl_loop_future_rejected(obj, err);
}

static Eina_Bool
_monitor_has_context(Efl_Io_Model_Data *pd, const char *path)
{
   return eio_monitor_has_context(pd->monitor, path);
}

static void
_efl_io_model_children_list(void *data, Eina_Array *entries)
{
   Eina_File_Direct_Info *info;
   Efl_Model *obj = data;
   Efl_Io_Model_Data *pd;
   Efl_Model_Children_Event cevt = { 0 };
   Eina_Array_Iterator iterator;
   unsigned int i;

   pd = efl_data_scope_get(obj, EFL_IO_MODEL_CLASS);
   if (!pd) return ;

   EINA_ARRAY_ITER_NEXT(entries, i, info, iterator)
     {
        Efl_Io_Model_Info *mi;

        if (!_monitor_has_context(pd, info->path)) continue;
        if (_already_added(pd, info->path)) continue;

        if (pd->filter.cb)
          {
             if (!pd->filter.cb(pd->filter.data, obj, info))
               continue ;
          }

        mi = calloc(1, sizeof (Efl_Io_Model_Info));
        if (!mi) continue ;

        mi->path_length = info->path_length;
        mi->path = eina_stringshare_add(info->path);

        mi->name_start = info->name_start;
        mi->name_length = info->name_length;
        mi->type = _efl_io_model_info_type_get(info, NULL);
        mi->parent_ref = EINA_FALSE;
        mi->child_ref = EINA_TRUE;

        cevt.index = eina_list_count(pd->files);
        cevt.child = NULL;

        pd->files = eina_list_append(pd->files, mi);

        efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
     }

   efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);
}

static void
_efl_io_model_children_list_cleanup(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Io_Model_Data *pd = data;

   pd->request.listing = NULL;
   pd->listed = EINA_TRUE;
}

/**
 * Children Count Get
 */
static unsigned int
_efl_io_model_efl_model_children_count_get(const Eo *obj, Efl_Io_Model_Data *pd)
{
   // If we have no information on the object, let's build it.
   if (efl_invalidated_get(obj) ||
       efl_invalidating_get(obj))
     {
        return 0;
     }
   else if (!pd->info)
     {
        _eio_build_st(obj, pd);
     }
   else if (!pd->listed &&
            !pd->request.listing &&
            pd->info->type == EINA_FILE_DIR)
     {
        Efl_Io_Manager *iom;
        Eina_Future *f;

        iom = efl_provider_find(obj, EFL_IO_MANAGER_CLASS);
        if (!iom)
          {
             ERR("Could not find an Efl.Io.Manager on %p.", obj);
             return 0;
          }

        f = efl_io_manager_direct_ls(iom, pd->path, EINA_FALSE,
                                     (void*) obj, _efl_io_model_children_list, NULL);

        //start monitoring before listing is done
        //we will filter later on if we already published a file or not
        _efl_io_model_efl_model_monitor_add(pd);
        pd->request.listing = efl_future_then(obj, f,
                                              .free = _efl_io_model_children_list_cleanup,
                                              .data = pd);
     }

   return eina_list_count(pd->files);
}

static void
_efl_io_model_efl_model_monitor_add(Efl_Io_Model_Data *priv)
{
   if (!priv->monitor)
     {
        int i = 0;

        priv->monitor = eio_monitor_add(priv->path);

        for (i = 0; priv->mon.mon_event_child_add[i] != EIO_MONITOR_ERROR ; ++i)
          priv->mon.ecore_child_add_handler[i] =
            ecore_event_handler_add(priv->mon.mon_event_child_add[i], _efl_model_evt_added_ecore_cb, priv);

        for (i = 0; priv->mon.mon_event_child_del[i] != EIO_MONITOR_ERROR ; ++i)
          priv->mon.ecore_child_del_handler[i] =
            ecore_event_handler_add(priv->mon.mon_event_child_del[i], _efl_model_evt_deleted_ecore_cb, priv);
     }
}

static void
_efl_io_model_efl_model_monitor_del(Efl_Io_Model_Data *priv)
{
   if (priv->monitor)
     {
        int i = 0;
        for (i = 0; priv->mon.mon_event_child_add[i] != EIO_MONITOR_ERROR ; ++i)
           ecore_event_handler_del(priv->mon.ecore_child_add_handler[i]);

        for (i = 0; priv->mon.mon_event_child_del[i] != EIO_MONITOR_ERROR ; ++i)
           ecore_event_handler_del(priv->mon.ecore_child_del_handler[i]);

        eio_monitor_del(priv->monitor);
        priv->monitor = NULL;
     }
}

/*
 *  Callbacks
 *  Children Load
 */
static void
_efl_io_model_children_filter_set(Eo *obj EINA_UNUSED, Efl_Io_Model_Data *pd,
                                  void *filter_data, EflIoFilter filter, Eina_Free_Cb filter_free_cb)
{
   pd->filter.data = filter_data;
   pd->filter.cb = filter;
   pd->filter.free = filter_free_cb;
}

/**
 * Child Add
 */
static Eo *
_efl_io_model_efl_model_child_add(Eo *obj EINA_UNUSED, Efl_Io_Model_Data *priv EINA_UNUSED)
{
   return NULL;
}

/**
 * Child Remove
 */
static void
_efl_io_model_efl_model_child_del(Eo *obj EINA_UNUSED,
                                  Efl_Io_Model_Data *priv EINA_UNUSED,
                                  Eo *child)
{
   Efl_Io_Model_Data *child_pd;
   Eina_File_Type type;

   child_pd = efl_data_scope_get(child, MY_CLASS);
   if (!child_pd->info) return;
   if (child_pd->error) return;

   type = child_pd->info->type;

   if (type == EINA_FILE_UNKNOWN)
     {
        _eio_build_st_then_clobber(child, child_pd);
        return ;
     }

   // Already in progress
   if (child_pd->request.del) return;

   efl_ref(child);
   if (type == EINA_FILE_DIR)
     {
        child_pd->request.del = eio_dir_unlink(child_pd->path,
                                               NULL,
                                               NULL,
                                               _eio_done_unlink_cb,
                                               _eio_error_unlink_cb,
                                               child);
     }
   else
     {
        child_pd->request.del = eio_file_unlink(child_pd->path,
                                                _eio_done_unlink_cb,
                                                _eio_error_unlink_cb,
                                                child);
     }
}

/**
 * Children Slice Get
 */
static Eina_Future *
_efl_io_model_efl_model_children_slice_get(Eo *obj, Efl_Io_Model_Data *pd,
                                           unsigned int start, unsigned int count)
{
   Eina_Future_Scheduler *scheduler = NULL;
   Eina_Value array = EINA_VALUE_EMPTY;
   Eina_List *ls = NULL;

   // If called on an invalidated model, we won't have a scheduler
   scheduler = efl_loop_future_scheduler_get(obj);
   if (!scheduler) return NULL;

   if (count == 0)
     {
        count = eina_list_count(pd->files);
        start = 0;
     }

   // Children must have been listed first
   if (count == 0 || (start + count > eina_list_count(pd->files)))
     return eina_future_rejected(scheduler, EFL_MODEL_ERROR_INCORRECT_VALUE);

   eina_value_array_setup(&array, EINA_VALUE_TYPE_OBJECT, count % 8);

   ls = eina_list_nth_list(pd->files, start);

   while (count > 0)
     {
        Efl_Io_Model_Info *info = eina_list_data_get(ls);
        Efl_Io_Model_Data *child_data = NULL;

        info->parent_ref = EINA_TRUE;

        if (info->object == NULL)
          // Little trick here, setting internal data before finalize
          info->object = efl_add_ref(EFL_IO_MODEL_CLASS, obj,
                                     efl_loop_model_volatile_make(efl_added),
                                     child_data = efl_data_scope_get(efl_added, EFL_IO_MODEL_CLASS),
                                     child_data->info = info,
                                     child_data->path = eina_stringshare_ref(info->path),
                                     child_data->parent = ls,
                                     // NOTE: We are assuming here that the parent model will outlive all its children
                                     child_data->filter.cb = pd->filter.cb,
                                     child_data->filter.data = pd->filter.data);
        else
          efl_ref(info->object);
        eina_value_array_append(&array, info->object);

        efl_wref_add(info->object, &info->object);
        efl_unref(info->object);

        count--;
        ls = eina_list_next(ls);
     }

   return eina_future_resolved(scheduler, array);
}


/**
 * Class definitions
 */
static Efl_Object *
_efl_io_model_efl_object_finalize(Eo *obj, Efl_Io_Model_Data *pd)
{
   if (!pd->path) return NULL;
   if (!efl_provider_find(obj, EFL_LOOP_CLASS))
     {
        ERR("Eio.Model require a parent that provide access to the main loop.");
        return NULL;
     }

   // If we have no info at all, let's check this path first
   if (!pd->info) _eio_build_st(obj, pd);

   // Setup monitor
   pd->mon.mon_event_child_add[0] = EIO_MONITOR_DIRECTORY_CREATED;
   pd->mon.mon_event_child_add[1] = EIO_MONITOR_FILE_CREATED;
   pd->mon.mon_event_child_add[2] = EIO_MONITOR_ERROR;
   pd->mon.mon_event_child_del[0] = EIO_MONITOR_DIRECTORY_DELETED;
   pd->mon.mon_event_child_del[1] = EIO_MONITOR_FILE_DELETED;
   pd->mon.mon_event_child_del[2] = EIO_MONITOR_ERROR;

   pd->self = obj;

   return obj;
}

static void
_efl_io_model_path_set(Eo *obj EINA_UNUSED, Efl_Io_Model_Data *priv, const char *path)
{
   char *sanitized = eina_file_path_sanitize(path);
   priv->path = eina_stringshare_add(sanitized);
   free(sanitized);
}

static const char *
_efl_io_model_path_get(const Eo *obj EINA_UNUSED, Efl_Io_Model_Data *priv)
{
   return priv->path;
}

static void
_efl_io_model_efl_object_destructor(Eo *obj , Efl_Io_Model_Data *priv)
{
   Efl_Io_Model_Info *info;

   free(priv->st);
   priv->st = NULL;

   _efl_io_model_info_free(priv->info, EINA_TRUE);
   priv->info = NULL;

   EINA_LIST_FREE(priv->files, info)
     _efl_io_model_info_free(info, EINA_FALSE);

   eina_stringshare_replace(&priv->path, NULL);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
_efl_io_model_efl_object_invalidate(Eo *obj , Efl_Io_Model_Data *priv)
{
   // This has to be done first, to make sure that the automatic
   // del is not done anymore. Or it would lead to too much
   // unref when stopping async thread (During invalidate, we
   // are already in the process of doing an implicit del).
   efl_invalidate(efl_super(obj, EFL_IO_MODEL_CLASS));

   _efl_io_model_efl_model_monitor_del(priv);

   // Unlink the object from the parent
   if (priv->info && priv->info->object)
     {
        efl_wref_del(priv->info->object, &priv->info->object);
        priv->info->object = NULL;
     }
   if (priv->request.del)
     {
        if (!ecore_thread_wait(priv->request.del->thread, 0.1))
          {
             ecore_thread_cancel(priv->request.del->thread);
             ecore_thread_wait(priv->request.del->thread, 0.1);
          }
     }
   if (priv->request.move)
     {
        if (!ecore_thread_wait(priv->request.move->thread, 0.1))
          {
             ecore_thread_cancel(priv->request.move->thread);
             ecore_thread_wait(priv->request.move->thread, 0.1);
          }
     }
   if (priv->request.stat)
     {
        if (!ecore_thread_wait(priv->request.stat->thread, 0.1))
          {
             ecore_thread_cancel(priv->request.stat->thread);
             ecore_thread_wait(priv->request.stat->thread, 0.1);
          }
     }
}

#include "efl_io_model.eo.c"
