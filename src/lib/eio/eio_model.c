#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdint.h>

#include <Efl.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Eo.h>
#include <Efreet_Mime.h>

#include "Eio.h"

#include "eio_private.h"
#include "eio_model_private.h"

#define MY_CLASS EIO_MODEL_CLASS
#define MY_CLASS_NAME "Eio_Model"

static void _eio_prop_set_error_cb(void *, Eio_File *, int);
static void _eio_stat_done_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_Stat *stat);
static void _eio_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error);

static void
_eio_stat_do(Eio_Model_Data *priv)
{
   priv->stat_file = eio_file_direct_stat(priv->path, _eio_stat_done_cb, _eio_error_cb, priv);
}

/**
 *  Callbacks
 *  Property
 */
static void
_eio_stat_done_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_Stat *stat)
{
   _Eio_Model_Data *priv = data;
   _Eio_Property_Promise* p;
   Eina_List *l;
   EINA_LIST_FOREACH(priv->property_promises, l, p)
     {
        Eina_Value* v = eina_value_new(EINA_VALUE_TYPE_CHAR);
        switch(p->property)
          {
          case EIO_MODEL_PROP_IS_DIR:
            eina_value_setup(v, EINA_VALUE_TYPE_CHAR);
            eina_value_set(v, eio_file_is_dir(stat) ? EINA_TRUE : EINA_FALSE);
            break;
          case EIO_MODEL_PROP_IS_LNK:
            eina_value_setup(v, EINA_VALUE_TYPE_CHAR);
            eina_value_set(v, eio_file_is_lnk(stat) ? EINA_TRUE : EINA_FALSE);
            break;
          case EIO_MODEL_PROP_MTIME:
            eina_value_setup(v, EINA_VALUE_TYPE_DOUBLE);
            eina_value_set(v, eio_file_mtime(stat));
            break;
          case EIO_MODEL_PROP_SIZE:
            eina_value_setup(v, EINA_VALUE_TYPE_INT64);
            eina_value_set(v, eio_file_size(stat));
            break;
          default:
            break;
          };

        eina_promise_owner_value_set(p->promise, v, (Eina_Promise_Free_Cb)&eina_value_free);
        free(p);
     }
   eina_list_free(priv->property_promises);
   priv->property_promises = NULL;

   eio_file_cancel(priv->stat_file);
   priv->stat_file = NULL;
}

static void
_eio_progress_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const Eio_Progress *info EINA_UNUSED)
{
   //TODO: implement
}

static void
_eio_move_done_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   Efl_Model_Property_Event evt;
   Eio_Model_Data *priv = data;
   Eina_Array *properties;

   EINA_SAFETY_ON_FALSE_RETURN(efl_ref_get(priv->obj));
   properties = eina_array_new(20);

   memset(&evt, 0, sizeof(Efl_Model_Property_Event));
   eina_array_push(properties, _eio_model_prop_names[EIO_MODEL_PROP_PATH]);
   eina_array_push(properties, _eio_model_prop_names[EIO_MODEL_PROP_FILENAME]);
   evt.changed_properties = properties;

   efl_event_callback_call(priv->obj, EFL_MODEL_EVENT_PROPERTIES_CHANGED, &evt);
   eina_array_free(properties);
}

static void
_eio_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   if (error != 0)
     {
        _Eio_Model_Data *priv = data;
        _Eio_Property_Promise* p;
        Eina_List *l;
        WRN("%d: %s.", error, strerror(error));

        EINA_LIST_FOREACH(priv->property_promises, l, p)
          {
              eina_promise_owner_error_set(p->promise, EFL_MODEL_ERROR_UNKNOWN);
          }
        eina_list_free(priv->property_promises);
        priv->property_promises = NULL;

        eio_file_cancel(priv->stat_file);
        priv->stat_file = NULL;
     }
}

static void
_eio_prop_set_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   if (error != 0)
     {
        WRN("%d: %s.", error, strerror(error));
     }
}

/**
 *  Callbacks
 *  Ecore Events
 */
static Eina_Bool
_efl_model_evt_added_ecore_cb(void *data, int type, void *event)
{
   Eio_Monitor_Event *evt = event;
   Eio_Model_Data *priv = data;
   Efl_Model_Children_Event cevt;
   Eina_Value path;

   if (type != EIO_MONITOR_DIRECTORY_CREATED && type != EIO_MONITOR_FILE_CREATED)
     return EINA_TRUE;

   char *dir = ecore_file_dir_get(evt->filename);
   if (strcmp(priv->path, dir) != 0)
     {
        free(dir);
        return EINA_TRUE;
     }
   free(dir);

   if (priv->children_list)
     {
        Eina_List* cur = priv->children_list;
        Eina_Stringshare *spath = eina_stringshare_add(evt->filename);
        int i;

        for (i = 0; cur; ++i, cur = cur->next)
          {
             Eio_Model_Data *cur_priv = efl_data_scope_get(cur->data, MY_CLASS);
             if(cur_priv->path == spath)
               {
                   eina_stringshare_del(spath);
                   return EINA_TRUE;
               }
          }
        eina_stringshare_del(spath);
     }

   cevt.child = efl_add_ref(EIO_MODEL_CLASS, priv->obj, eio_model_path_set(efl_added, evt->filename));
   priv->children_list = eina_list_append(priv->children_list, cevt.child);
   cevt.index = eina_list_count(priv->children_list);

   eina_value_setup(&path, EINA_VALUE_TYPE_STRING);
   eina_value_set(&path, evt->filename);
   eio_model_children_filter_set(cevt.child, priv->filter_cb, priv->filter_userdata);
   eina_value_flush(&path);

   efl_event_callback_call(priv->obj, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);

   return EINA_TRUE;
}

static Eina_Bool
_efl_model_evt_deleted_ecore_cb(void *data, int type, void *event)
{
   Eio_Monitor_Event *evt = event;
   Eio_Model_Data *priv = data;

   if (type != EIO_MONITOR_DIRECTORY_DELETED && type != EIO_MONITOR_FILE_DELETED)
     return EINA_TRUE;

   if (priv->children_list)
     {
        Eina_List* cur = priv->children_list;
        Eina_Stringshare *spath = eina_stringshare_add(evt->filename);
        int i;

        for (i = 0; cur; ++i, cur = cur->next)
          {
             Eio_Model_Data *cur_priv = efl_data_scope_get(cur->data, MY_CLASS);
             if(cur_priv->path == spath)
               break;
          }

        if (cur)
          {
             Efl_Model_Children_Event cevt;
             cevt.index = i;
             cevt.child = cur->data;

             efl_event_callback_call(priv->obj, EFL_MODEL_EVENT_CHILD_REMOVED, &cevt);

             priv->children_list = eina_list_remove_list(priv->children_list, cur);
             efl_unref(cevt.child);
          }

        eina_stringshare_del(spath);
     }

   return EINA_TRUE;
}

static void
_eio_monitors_list_load(Eio_Model_Data *priv)
{
   priv->mon.mon_event_child_add[0] = EIO_MONITOR_DIRECTORY_CREATED;
   priv->mon.mon_event_child_add[1] = EIO_MONITOR_FILE_CREATED;
   priv->mon.mon_event_child_add[2] = EIO_MONITOR_ERROR;
   priv->mon.mon_event_child_del[0] = EIO_MONITOR_DIRECTORY_DELETED;
   priv->mon.mon_event_child_del[1] = EIO_MONITOR_FILE_DELETED;
   priv->mon.mon_event_child_del[2] = EIO_MONITOR_ERROR;
}

/**
 *  Callbacks
 *  Child Del
 */
static Eina_Bool
_eio_filter_child_del_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const Eina_File_Direct_Info *info EINA_UNUSED)
{
   return EINA_TRUE;
}

static void
_eio_progress_child_del_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const Eio_Progress *info EINA_UNUSED)
{}

static void
_eio_done_unlink_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   Eio_Model_Data *priv = data;

   EINA_SAFETY_ON_NULL_RETURN(priv);
   EINA_SAFETY_ON_NULL_RETURN(priv->obj);

   efl_unref(priv->obj);
}

static void
_eio_error_unlink_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   Eio_Model_Data *priv = data;

   ERR("%d: %s.", error, strerror(error));

   efl_unref(priv->obj);
}


/**
 * Interfaces impl.
 */
static Eina_Array const *
_eio_model_efl_model_properties_get(Eo *obj EINA_UNUSED, Eio_Model_Data *_pd)
{
   Eio_Model_Data *priv = _pd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->obj, NULL);

   return priv->properties_name;
}

/**
 * Property Get
 */
static Eina_Promise*
_eio_model_efl_model_property_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char *property)
{
   _Eio_Property_Name property_name;
   const char* value = NULL;
   Eina_Promise_Owner *promise = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise);
   
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, rpromise);

   if (property == NULL)
     {
        eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
        return rpromise;
     }

   if(strcmp(_eio_model_prop_names[EIO_MODEL_PROP_FILENAME], property) == 0)
     {
        char* tmp = strdup(priv->path);
        char* basename_ = basename(tmp);
        value = strdup(basename_);
        property_name = EIO_MODEL_PROP_FILENAME;
        free(tmp);
     }
   else if(strcmp(_eio_model_prop_names[EIO_MODEL_PROP_PATH], property) == 0)
     {
        value = priv->path;
        property_name = EIO_MODEL_PROP_PATH;
     }
   else if(strcmp(_eio_model_prop_names[EIO_MODEL_PROP_MIME_TYPE], property) == 0)
     {
        value = efreet_mime_type_get(priv->path);
        if (value == NULL)
          {
             eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
             return rpromise;
          }
        property_name = EIO_MODEL_PROP_MIME_TYPE;
     }
   else if(strcmp(_eio_model_prop_names[EIO_MODEL_PROP_MTIME], property) == 0)
     property_name = EIO_MODEL_PROP_MTIME;
   else if(strcmp(_eio_model_prop_names[EIO_MODEL_PROP_IS_DIR], property) == 0)
     property_name = EIO_MODEL_PROP_IS_DIR;
   else if(strcmp(_eio_model_prop_names[EIO_MODEL_PROP_IS_LNK], property) == 0)
     property_name = EIO_MODEL_PROP_IS_LNK;
   else if(strcmp(_eio_model_prop_names[EIO_MODEL_PROP_SIZE], property) == 0)
     property_name = EIO_MODEL_PROP_SIZE;
   else
     {
        eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
        return rpromise;
     }

   switch(property_name)
     {
     case EIO_MODEL_PROP_FILENAME:
     case EIO_MODEL_PROP_PATH:
     case EIO_MODEL_PROP_MIME_TYPE:
       {
          Eina_Value* v = eina_value_new(EINA_VALUE_TYPE_STRING);
          eina_value_set(v, value);
          eina_promise_owner_value_set(promise, v, (Eina_Promise_Free_Cb)&eina_value_free);
       }
       break;
     default:
       {
          _Eio_Property_Promise* p = calloc(1, sizeof(_Eio_Property_Promise));
          p->promise = promise;
          p->property = property_name;;
          priv->property_promises = eina_list_prepend(priv->property_promises, p);

          if(!priv->stat_file)
            _eio_stat_do(priv);
       }
       break;
     }
   return rpromise;
}

/**
 * Property Set
 */
static void
_eio_model_efl_model_property_set(Eo *obj EINA_UNUSED,
                                                Eio_Model_Data *priv,
                                                const char * property,
                                                const Eina_Value *value,
                                                Eina_Promise_Owner *promise)
{
   char *dest;

   EINA_SAFETY_ON_NULL_RETURN(property);

   if (strcmp(property, "path") != 0)
     {
        eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_SUPPORTED);
        return;
     }

   eina_value_get(value, &dest);
   if (priv->path == NULL || !ecore_file_exists(priv->path))
     {
        eina_stringshare_replace(&priv->path, dest);

        if (!ecore_file_exists(priv->path))
          {
             eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
             return;
          }

        char* tmp = strdup(priv->path);
        char* basename_ = basename(tmp);
        INF("path '%s' with filename '%s'.", priv->path, basename_);
        free(tmp);
        (void)basename_;

        _eio_monitors_list_load(priv);

        _eio_move_done_cb(priv, NULL);
     }
   else
     {
        priv->move_file = eio_file_move(priv->path, dest, _eio_progress_cb, _eio_move_done_cb, _eio_prop_set_error_cb, priv);
        eina_stringshare_replace(&priv->path, dest);
     }

     eina_promise_owner_value_set(promise, &value, NULL);
}

/**
 * Children Count Get
 */
static Eina_Promise*
_eio_model_efl_model_children_count_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv)
{
   Eina_Promise_Owner *promise = eina_promise_add();
   Eina_Promise* rpromise = eina_promise_owner_promise_get(promise);
   unsigned int *c = calloc(sizeof(unsigned int), 1);
   *c = eina_list_count(priv->children_list);
   eina_promise_owner_value_set(promise, c, free);
   return rpromise;
}

static void
_eio_model_efl_model_monitor_add(Eio_Model_Data *priv)
{
   if (!priv->monitor)
     {
        priv->monitor = eio_monitor_add(priv->path);
        int i = 0;

        for (i = 0; priv->mon.mon_event_child_add[i] != EIO_MONITOR_ERROR ; ++i)
          priv->mon.ecore_child_add_handler[i] =
            ecore_event_handler_add(priv->mon.mon_event_child_add[i], _efl_model_evt_added_ecore_cb, priv);

        for (i = 0; priv->mon.mon_event_child_del[i] != EIO_MONITOR_ERROR ; ++i)
          priv->mon.ecore_child_del_handler[i] =
            ecore_event_handler_add(priv->mon.mon_event_child_del[i], _efl_model_evt_deleted_ecore_cb, priv);
     }
}

static void
_eio_model_efl_model_monitor_del(Eio_Model_Data *priv)
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
static Eina_Bool
_eio_filter_children_load_cb(void *data, Eio_File *handler, const Eina_File_Direct_Info *info)
{
   Eio_Model_Data *priv = data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, EINA_FALSE);

   eina_spinlock_take(&priv->filter_lock);
   if (priv->filter_cb)
     {
        Eina_Bool r = priv->filter_cb(priv->filter_userdata, handler, info);
        eina_spinlock_release(&priv->filter_lock);
        return r;
     }
   else
     eina_spinlock_release(&priv->filter_lock);

   return EINA_TRUE;
}

static void
_eio_main_children_load_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_File_Direct_Info *info)
{
   Eo *child;
   Eio_Model_Data *priv = data;
   EINA_SAFETY_ON_NULL_RETURN(priv);

   child = efl_add_ref(MY_CLASS, priv->obj, eio_model_path_set(efl_added, info->path));
   eina_spinlock_take(&priv->filter_lock);
   if (priv->filter_cb)
     eio_model_children_filter_set(child, priv->filter_cb, priv->filter_userdata);
   eina_spinlock_release(&priv->filter_lock);

   priv->children_list = eina_list_append(priv->children_list, child);
}

static void
_eio_done_children_load_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   Eio_Model_Data *priv = data;
   EINA_SAFETY_ON_NULL_RETURN(priv);

   eio_file_cancel(priv->listing_file);
   priv->listing_file = NULL;
   priv->is_listed = EINA_TRUE;
   priv->is_listing = EINA_FALSE;

   _eio_model_efl_model_monitor_add(priv);

   Eina_List* i;
   _Eio_Children_Slice_Promise* p;
   EINA_LIST_FOREACH(priv->children_promises, i, p)
     {
       Eina_Accessor* accessor = efl_model_list_slice(priv->children_list, p->start, p->count);
       eina_promise_owner_value_set(p->promise, accessor, (Eina_Promise_Free_Cb)&eina_accessor_free);
       free(p);
     }

   eina_list_free(priv->children_promises);
   priv->children_promises = NULL;
}

static void
_eio_error_children_load_cb(void *data, Eio_File *handler EINA_UNUSED, int error)
{
   Eio_Model_Data *priv = data;
   Eo *child;

   WRN("%d: %s.", error, strerror(error));

   EINA_LIST_FREE(priv->children_list, child)
     efl_unref(child);
}

static void
_eio_model_children_filter_set(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, Eio_Filter_Direct_Cb filter_cb, void *data)
{
   eina_spinlock_take(&priv->filter_lock);

   priv->filter_cb = filter_cb;
   priv->filter_userdata = data;

   eina_spinlock_release(&priv->filter_lock);
}

/**
 * Child Add
 */
static Eo *
_eio_model_efl_model_child_add(Eo *obj EINA_UNUSED, Eio_Model_Data *priv EINA_UNUSED)
{
   return efl_add(EIO_MODEL_CLASS, obj);
}

static void
_eio_model_efl_model_child_del_stat(void* data, Eio_File* handler EINA_UNUSED, const Eina_Stat* stat)
{
   Eo* child = data;
   Eio_Model_Data *child_priv = efl_data_scope_get(child, MY_CLASS);

   if(eio_file_is_dir(stat))
     eio_dir_unlink(child_priv->path,
                    _eio_filter_child_del_cb,
                    _eio_progress_child_del_cb,
                    _eio_done_unlink_cb,
                    _eio_error_unlink_cb,
                    child_priv);
   else
     eio_file_unlink(child_priv->path, _eio_done_unlink_cb, _eio_error_unlink_cb, child_priv);
}

/**
 * Child Remove
 */
static void
_eio_model_efl_model_child_del(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, Eo *child)
{
   Eio_Model_Data *child_priv;
   EINA_SAFETY_ON_NULL_RETURN(child);

   child_priv = efl_data_scope_get(child, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(child_priv);

   priv->del_file = eio_file_direct_stat(child_priv->path,
                                         &_eio_model_efl_model_child_del_stat,
                                         &_eio_error_unlink_cb,
                                         child);
   efl_ref(child);
}

/**
 * Children Slice Get
 */
static Eina_Promise*
_eio_model_efl_model_children_slice_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv,
                                             unsigned int start, unsigned int count)
{
   Eina_Promise_Owner *promise = eina_promise_add();
   Eina_Promise* rpromise = eina_promise_owner_promise_get(promise);
   /**
    * children must be already loaded otherwise we do nothing
    * and parameter is set to NULL.
    */
   if (!priv->path)
     {
        eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_INIT_FAILED);
        return rpromise;
     }

   if (!(priv->is_listed))
     {
       _Eio_Children_Slice_Promise* p = calloc(1, sizeof(struct _Eio_Children_Slice_Promise));
       p->promise = promise;
       p->start = start;
       p->count = count;

       priv->children_promises = eina_list_prepend(priv->children_promises, p);


       if (priv->is_listing == EINA_FALSE)
         {
             priv->is_listing = EINA_TRUE;
             eio_file_direct_ls(priv->path, _eio_filter_children_load_cb,
                             _eio_main_children_load_cb, _eio_done_children_load_cb,
                             _eio_error_children_load_cb, priv);
         }
       return rpromise;
     }

   Eina_Accessor* accessor = efl_model_list_slice(priv->children_list, start, count);
   eina_promise_owner_value_set(promise, accessor, (Eina_Promise_Free_Cb)&eina_accessor_free);
   return rpromise;
}


/**
 * Class definitions
 */
static Eo *
_eio_model_efl_object_constructor(Eo *obj, Eio_Model_Data *priv)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   unsigned int i;
   priv->obj = obj;
   priv->is_listed = priv->is_listing = EINA_FALSE;

   priv->properties_name = eina_array_new(EIO_MODEL_PROP_LAST);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->properties_name, NULL);
   for (i = 0; i < EIO_MODEL_PROP_LAST; ++i)
     eina_array_push(priv->properties_name, _eio_model_prop_names[i]);

   priv->monitor = NULL;
   eina_spinlock_new(&priv->filter_lock);

   return obj;
}

static void
_eio_model_path_set(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char *path)
{
   priv->path = eina_stringshare_add(path);
   _eio_monitors_list_load(priv);
}

static void
_eio_model_efl_object_destructor(Eo *obj , Eio_Model_Data *priv)
{
   Eo *child;

   _eio_model_efl_model_monitor_del(priv);

   eina_spinlock_free(&priv->filter_lock);

   if (priv->properties_name)
     eina_array_free(priv->properties_name);

   EINA_LIST_FREE(priv->children_list, child)
     efl_unref(child);

   eina_stringshare_del(priv->path);
   efl_destructor(efl_super(obj, MY_CLASS));
}


static Eo *
_eio_model_efl_object_parent_get(Eo *obj , Eio_Model_Data *priv)
{
   Eo *model = efl_parent_get(efl_super(obj, MY_CLASS));

   if (model == NULL || !efl_isa(model, EFL_MODEL_INTERFACE))
     {
        char *path = ecore_file_dir_get(priv->path);
        if (path != NULL && strcmp(priv->path, "/") != 0)
          {
             model = efl_add(MY_CLASS, NULL, eio_model_path_set(efl_added, path));
          }
        else
          model = NULL;

        free(path);
     }
   return model;
}
#include "eio_model.eo.c"
