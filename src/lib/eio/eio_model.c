#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdint.h>

#include <Efl.h>
#include <Eina.h>
#include <Ecore.h>
#include <Eo.h>

#include "eio_private.h"
#include "eio_model.h"
#include "eio_model_private.h"

#define MY_CLASS EIO_MODEL_CLASS
#define MY_CLASS_NAME "Eio_Model"

static void _eio_prop_set_error_cb(void *, Eio_File *, int);
static void _eio_model_efl_model_base_properties_load(Eo *, Eio_Model_Data *);
static void _eio_model_efl_model_base_children_load(Eo *, Eio_Model_Data *);

static void
_load_set(Eio_Model_Data *priv, Efl_Model_Load_Status status)
{
   Efl_Model_Load load;

   load.status = status;
   if ((priv->load.status & (EFL_MODEL_LOAD_STATUS_LOADED | EFL_MODEL_LOAD_STATUS_LOADING)) &&
       (load.status & (EFL_MODEL_LOAD_STATUS_LOADED | EFL_MODEL_LOAD_STATUS_LOADING)))
     {
        load.status = priv->load.status | status;
        switch (status)
          {
           case EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES:
              load.status &= ~EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES;
              break;
           case EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES:
              load.status &= ~EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES;
              break;
           case EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN:
              load.status &= ~EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN;
              break;
           case EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN:
              load.status &= ~EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN;
              break;
           default: break;
          }
     }

   if (priv->load.status != load.status)
     {
        priv->load.status = load.status;
        eo_do(priv->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_LOAD_STATUS, &load));
     }
}

/**
 *  Callbacks
 *  Property
 */
static void
_eio_stat_done_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_Stat *stat)
{
   Efl_Model_Property_Event evt;
   Eio_Model_Data *priv = data;
   EINA_SAFETY_ON_FALSE_RETURN(eo_ref_get(priv->obj));

   priv->is_dir = eio_file_is_dir(stat);
   memset(&evt, 0, sizeof(Efl_Model_Property_Event));

   eina_value_set(priv->properties_value[EIO_MODEL_PROP_IS_DIR], eio_file_is_dir(stat));
   eina_value_set(priv->properties_value[EIO_MODEL_PROP_IS_LNK], eio_file_is_lnk(stat));
   eina_value_set(priv->properties_value[EIO_MODEL_PROP_MTIME], eio_file_mtime(stat));
   eina_value_set(priv->properties_value[EIO_MODEL_PROP_SIZE], eio_file_size(stat));

   evt.changed_properties = priv->properties_name;
   eo_do(priv->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));

   _load_set(priv, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);

   if (priv->load_pending & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     _eio_model_efl_model_base_children_load(priv->obj, priv);
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

   EINA_SAFETY_ON_FALSE_RETURN(eo_ref_get(priv->obj));

   memset(&evt, 0, sizeof(Efl_Model_Property_Event));

   /**
    * When mv is executed we update our values and
    * notify both path and filename properties listeners.
    */
   eina_value_set(priv->properties_value[EIO_MODEL_PROP_PATH], priv->path);
   eina_value_set(priv->properties_value[EIO_MODEL_PROP_FILENAME], basename(priv->path));

   properties = eina_array_new(2);
   eina_array_push(properties, _eio_model_prop_names[EIO_MODEL_PROP_PATH]);
   eina_array_push(properties, _eio_model_prop_names[EIO_MODEL_PROP_FILENAME]);
   evt.changed_properties = properties;

   eo_do(priv->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));
   eina_array_free(properties);
}

static void
_eio_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   if (error != 0)
     {
        WRN("%d: %s.", error, strerror(error));
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
_efl_model_evt_added_ecore_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eio_Monitor_Event *evt = (Eio_Monitor_Event*)event;
   Eio_Model_Data *priv = data;
   Efl_Model_Children_Event cevt;
   Eina_Value path;

   cevt.child = eo_add_ref(EIO_MODEL_CLASS, priv->obj, eio_model_path_set(evt->filename));
   priv->children_list = eina_list_append(priv->children_list, cevt.child);
   cevt.index = eina_list_count(priv->children_list);

   eina_value_setup(&path, EINA_VALUE_TYPE_STRING);
   eina_value_set(&path, evt->filename);
   eo_do(cevt.child, eio_model_children_filter_set(priv->filter_cb, priv->filter_userdata));
   eina_value_flush(&path);

   eo_do(priv->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILD_ADDED, &cevt));

   return EINA_TRUE;
}

static Eina_Bool
_efl_model_evt_deleted_ecore_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eio_Monitor_Event *evt = (Eio_Monitor_Event*)event;
   Eio_Model_Data *priv = data;

   if (priv->children_list)
     {
        Eina_List* cur = priv->children_list;
        int i;

        for (i = 0; cur; ++i, cur = cur->next)
          {
             Eio_Model_Data *cur_priv = eo_data_scope_get(cur->data, MY_CLASS);
             if(strcmp(cur_priv->path, evt->filename) == 0)
               break;
          }

        if (cur)
          {
             Efl_Model_Children_Event cevt;
             cevt.index = i;
             cevt.child = cur->data;

             eo_do(priv->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILD_REMOVED, &cevt));

             priv->children_list = eina_list_remove_list(priv->children_list, cur);
             eo_unref(cevt.child);
          }
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

   eo_unref(priv->obj);
}

static void
_eio_error_unlink_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   Eio_Model_Data *priv = data;

   ERR("%d: %s.", error, strerror(error));

   eo_unref(priv->obj);
}


/**
 * Interfaces impl.
 */
static Efl_Model_Load_Status
_eio_model_efl_model_base_properties_get(Eo *obj EINA_UNUSED,
                                      Eio_Model_Data *_pd, Eina_Array * const* properties)
{
   Eio_Model_Data *priv = _pd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->obj, EINA_FALSE);

   *(Eina_Array **)properties = priv->properties_name;

   return priv->load.status;
}

/**
 * Property Get
 */
static Efl_Model_Load_Status
_eio_model_efl_model_base_property_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char *property, const Eina_Value **value)
{
   unsigned int i;
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, EFL_MODEL_LOAD_STATUS_ERROR);

   *value = NULL;
   if (priv->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     {
        for (i = 0; i < EIO_MODEL_PROP_LAST; ++i)
          {
             if (!strcmp(property, _eio_model_prop_names[i]))
               break;
          }

        if ( i < EIO_MODEL_PROP_LAST)
          {
             *value = priv->properties_value[i];
          }
     }

   return priv->load.status;
}

/**
 * Property Set
 */
static Efl_Model_Load_Status
_eio_model_efl_model_base_property_set(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char * property, const Eina_Value *value)
{
   char *dest;

   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EINA_FALSE);

   if (strcmp(property, "path") != 0)
     return EINA_FALSE;

   dest = eina_value_to_string(value);
   if (priv->path == NULL)
     {
        priv->path = dest;

        INF("path '%s' with filename '%s'.", priv->path, basename(priv->path));

        eina_value_set(priv->properties_value[EIO_MODEL_PROP_PATH], priv->path);
        eina_value_set(priv->properties_value[EIO_MODEL_PROP_FILENAME], basename(priv->path));

        _eio_monitors_list_load(priv);

        _eio_move_done_cb(priv, NULL);

        if (priv->load_pending & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
          _eio_model_efl_model_base_properties_load(obj, priv);
        else if (priv->load_pending & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
          _eio_model_efl_model_base_children_load(obj, priv);

        return priv->load.status;
     }

   priv->file = eio_file_move(priv->path, dest, _eio_progress_cb, _eio_move_done_cb, _eio_prop_set_error_cb, priv);
   free(priv->path);
   priv->path = dest;

   return priv->load.status;
}
/**
 * Children Count Get
 */
static Efl_Model_Load_Status
_eio_model_efl_model_base_children_count_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, unsigned int *children_count)
{
   /**< eina_list_count returns 'unsigned int' */
   *children_count = eina_list_count(priv->children_list);
   return priv->load.status;
}

/**
 * Properties Load
 */
static void
_eio_model_efl_model_base_properties_load(Eo *obj EINA_UNUSED, Eio_Model_Data *priv)
{
   if (priv->path == NULL)
     {
        priv->load_pending |= EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES;
        return;
     }
   priv->load_pending &= ~EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES;

   if (!(priv->load.status & (EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES | EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES)))
     {
        _load_set(priv, EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES);
        priv->file = eio_file_direct_stat(priv->path, _eio_stat_done_cb, _eio_error_cb, priv);
     }
}

static void
_eio_model_efl_model_base_monitor_add(Eio_Model_Data *priv)
{
   if (!priv->monitor)
     {
        priv->monitor = eio_monitor_add(priv->path);
        int i = 0;

        for (i = 0; priv->mon.mon_event_child_add[i] != EIO_MONITOR_ERROR ; ++i)
          priv->mon.ecore_child_add_handler[i] =
            ecore_event_handler_add(priv->mon.mon_event_child_add[i], _efl_model_evt_added_ecore_cb, priv);

        for (i = 0; priv->mon.mon_event_child_del[i] != EIO_MONITOR_ERROR ; ++i)
          priv->mon.ecore_child_add_handler[i] =
            ecore_event_handler_add(priv->mon.mon_event_child_del[i], _efl_model_evt_deleted_ecore_cb, priv);
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

   child = eo_add(MY_CLASS, NULL, eio_model_path_set(info->path));
   eina_spinlock_take(&priv->filter_lock);
   if (priv->filter_cb)
     eo_do(child, eio_model_children_filter_set(priv->filter_cb, priv->filter_userdata));
   eina_spinlock_release(&priv->filter_lock);

   priv->children_list = eina_list_append(priv->children_list, child);
}

static void
_eio_done_children_load_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   unsigned long count;
   Eio_Model_Data *priv = data;
   EINA_SAFETY_ON_NULL_RETURN(priv);

   count = eina_list_count(priv->children_list);
   _load_set(priv, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   eo_do(priv->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, &count));
}

static void
_eio_error_children_load_cb(void *data, Eio_File *handler EINA_UNUSED, int error)
{
   Eio_Model_Data *priv = data;
   Eo *child;

   WRN("%d: %s.", error, strerror(error));

   EINA_LIST_FREE(priv->children_list, child)
     eo_unref(child);

   _load_set(priv, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);
}

/**
 * Children Load
 */
static void
_eio_model_efl_model_base_children_load(Eo *obj EINA_UNUSED, Eio_Model_Data *priv)
{
   if (priv->path == NULL)
     {
        priv->load_pending |= EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN;
        return;
     }

   priv->load_pending &= ~EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN;

   if (priv->children_list == NULL && priv->is_dir &&
       !(priv->load.status & (EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN | EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN)))
     {
        _eio_model_efl_model_base_monitor_add(priv);

        _load_set(priv, EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN);
        eio_file_direct_ls(priv->path, _eio_filter_children_load_cb,
                           _eio_main_children_load_cb, _eio_done_children_load_cb,
                           _eio_error_children_load_cb, priv);
     }
}

/**
 * Load
 */
static void
_eio_model_efl_model_base_load(Eo *obj, Eio_Model_Data *priv)
{
   priv->load_pending |= EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN;
   _eio_model_efl_model_base_properties_load(obj, priv);
}

/**
 * Load status get
 */
static Efl_Model_Load_Status
_eio_model_efl_model_base_load_status_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv)
{
   return priv->load.status;
}

/**
 * Unload
 */
static void
_eio_model_efl_model_base_unload(Eo *obj  EINA_UNUSED, Eio_Model_Data *priv)
{
   if (!(priv->load.status & EFL_MODEL_LOAD_STATUS_UNLOADED))
     {
        Eo *child;
        EINA_LIST_FREE(priv->children_list, child)
          {
             eo_unref(child);
          }

        _load_set(priv, EFL_MODEL_LOAD_STATUS_UNLOADED);
     }
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
_eio_model_efl_model_base_child_add(Eo *obj EINA_UNUSED, Eio_Model_Data *priv EINA_UNUSED)
{
   return eo_add(EIO_MODEL_CLASS, obj);
}

static void
_eio_model_efl_model_base_child_del_stat(void* data, Eio_File* handler EINA_UNUSED, const Eina_Stat* stat)
{
   Eo* child = data;
   Eio_Model_Data *child_priv = eo_data_scope_get(child, MY_CLASS);

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
static Efl_Model_Load_Status
_eio_model_efl_model_base_child_del(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, Eo *child)
{
   Eio_Model_Data *child_priv;
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EFL_MODEL_LOAD_STATUS_ERROR);

   child_priv = eo_data_scope_get(child, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_priv, EFL_MODEL_LOAD_STATUS_ERROR);

   eio_file_direct_stat(child_priv->path,
                        &_eio_model_efl_model_base_child_del_stat,
                        &_eio_error_unlink_cb,
                        child);
   eo_ref(child);
   return priv->load.status;
}

/**
 * Children Slice Get
 */
static Efl_Model_Load_Status
_eio_model_efl_model_base_children_slice_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv,
                                     unsigned start, unsigned count, Eina_Accessor **children_accessor)
{
   Eo *child;
   Eina_List *l, *ln, *lr = NULL;

   /**
    * children must be already loaded otherwise we do nothing
    * and parameter is set to NULL.
    */

   if (!(priv->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN))
     {
        /**
         * Status should be in either unloaded state or unitialized
         * so we simply return without much alarm.
         */
        *children_accessor = NULL;
        return priv->load.status;
     }

   if ((start == 0) && (count == 0)) /* this is full data */
     {
        /*
         * children_accessor will be set to NULL by
         * eina_list_accessor_new if the later fails.
         */
        *children_accessor = eina_list_accessor_new(priv->children_list);
     }
   else /* this is only slice */
     {
        ln = eina_list_nth_list(priv->children_list, (start-1));
        if (!ln)
          {
             *children_accessor = NULL;
             ERR("children not found !");
             return  priv->load.status;
          }

        EINA_LIST_FOREACH(ln, l, child)
          {
             eo_ref(child);
             lr = eina_list_append(lr, child);
             if (eina_list_count(lr) == count)
               break;
          }
        // This may leak the children Eina_List.
        *children_accessor = eina_list_accessor_new(lr);
     }

   return priv->load.status;
}

/**
 * Class definitions
 */
static Eo *
_eio_model_eo_base_constructor(Eo *obj, Eio_Model_Data *priv)
{
   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
   unsigned int i;
   priv->obj = obj;

   priv->properties_name = eina_array_new(EIO_MODEL_PROP_LAST);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->properties_name, NULL);
   for (i = 0; i < EIO_MODEL_PROP_LAST; ++i)
     eina_array_push(priv->properties_name, _eio_model_prop_names[i]);

   priv->properties_value[EIO_MODEL_PROP_FILENAME] = eina_value_new(EINA_VALUE_TYPE_STRING);
   priv->properties_value[EIO_MODEL_PROP_PATH] = eina_value_new(EINA_VALUE_TYPE_STRING);
   priv->properties_value[EIO_MODEL_PROP_MTIME] = eina_value_new(EINA_VALUE_TYPE_TIMEVAL);
   priv->properties_value[EIO_MODEL_PROP_IS_DIR] = eina_value_new(EINA_VALUE_TYPE_INT);
   priv->properties_value[EIO_MODEL_PROP_IS_LNK] = eina_value_new(EINA_VALUE_TYPE_INT);
   priv->properties_value[EIO_MODEL_PROP_SIZE] = eina_value_new(EINA_VALUE_TYPE_INT64);

   priv->load.status = EFL_MODEL_LOAD_STATUS_UNLOADED;
   priv->monitor = NULL;
   eina_spinlock_new(&priv->filter_lock);

   return obj;
}

static void
_eio_model_path_set(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char *path)
{
   priv->path = strdup(path);

   eina_value_set(priv->properties_value[EIO_MODEL_PROP_PATH], priv->path);
   eina_value_set(priv->properties_value[EIO_MODEL_PROP_FILENAME], basename(priv->path));

   priv->monitor = NULL;
   _eio_monitors_list_load(priv);
}

static void
_eio_model_eo_base_destructor(Eo *obj , Eio_Model_Data *priv)
{
   Eo *child;
   unsigned int i;

   if (priv->monitor)
     eio_monitor_del(priv->monitor);

   eina_spinlock_free(&priv->filter_lock);

   if (priv->properties_name)
     eina_array_free(priv->properties_name);

   for (i = 0; i < EIO_MODEL_PROP_LAST; ++i)
     {
       eina_value_free(priv->properties_value[i]);
     }

   EINA_LIST_FREE(priv->children_list, child)
     eo_unref(child);

   free(priv->path);
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "eio_model.eo.c"
