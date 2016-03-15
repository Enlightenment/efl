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
static void
_eio_stat_done_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_Stat *stat);
static void
_eio_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error);

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
   _Eio_Property_Promise* promise;
   Eina_List *l;
   EINA_LIST_FOREACH(priv->property_promises, l, promise)
     {
        Eina_Value* v = ecore_promise_buffer_get(promise->promise);
        switch(promise->property)
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
            eina_value_setup(v, EINA_VALUE_TYPE_TIMEVAL);
            eina_value_set(v, eio_file_mtime(stat));
            break;
          case EIO_MODEL_PROP_SIZE:
            eina_value_setup(v, EINA_VALUE_TYPE_INT64);
            eina_value_set(v, eio_file_size(stat));
            break;
          default:
            break;
          };

        ecore_promise_value_set(promise->promise, NULL);
        free(promise);
     }
   eina_list_free(priv->property_promises);

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
   Eina_Array *properties  = eina_array_new(20);

   EINA_SAFETY_ON_FALSE_RETURN(eo_ref_get(priv->obj));

   memset(&evt, 0, sizeof(Efl_Model_Property_Event));
   eina_array_push(properties, _eio_model_prop_names[EIO_MODEL_PROP_PATH]);
   eina_array_push(properties, _eio_model_prop_names[EIO_MODEL_PROP_FILENAME]);
   evt.changed_properties = properties;

   eo_event_callback_call(priv->obj, EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt);
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

   fprintf(stderr, __FILE__ ":%d %s added ecore_cb %s\n", __LINE__, __func__, evt->filename); fflush(stderr);
   cevt.child = eo_add_ref(EIO_MODEL_CLASS, priv->obj, eio_model_path_set(eoid, evt->filename));
   priv->children_list = eina_list_append(priv->children_list, cevt.child);
   cevt.index = eina_list_count(priv->children_list);

   eina_value_setup(&path, EINA_VALUE_TYPE_STRING);
   eina_value_set(&path, evt->filename);
   eio_model_children_filter_set(cevt.child, priv->filter_cb, priv->filter_userdata);
   eina_value_flush(&path);

   eo_event_callback_call(priv->obj, EFL_MODEL_BASE_EVENT_CHILD_ADDED, &cevt);

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

             eo_event_callback_call(priv->obj, EFL_MODEL_BASE_EVENT_CHILD_REMOVED, &cevt);

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
static Eina_Array const *
_eio_model_efl_model_base_properties_get(Eo *obj EINA_UNUSED, Eio_Model_Data *_pd)
{
   Eio_Model_Data *priv = _pd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->obj, EINA_FALSE);

   return priv->properties_name;
}

/**
 * Property Get
 */
static void
_eio_model_efl_model_base_property_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char *property, Ecore_Promise **promise)
{
   _Eio_Property_Name property_name;
   const char* value;

   EINA_SAFETY_ON_NULL_RETURN(property);
   EINA_SAFETY_ON_NULL_RETURN(priv);

   if(!strcmp("filename", property))
     {
        fprintf(stderr, __FILE__ ":%d %s Getting filename property %s\n", __LINE__, __func__, priv->path); fflush(stderr);
        value = basename(priv->path);
        property_name = EIO_MODEL_PROP_FILENAME;
     }
   else if(!strcmp("path", property))
     {
        value = priv->path;
        property_name = EIO_MODEL_PROP_PATH;
     }
   else if(!strcmp("mtime", property))
     property_name = EIO_MODEL_PROP_MTIME;
   else if(!strcmp("is_dir", property))
     property_name = EIO_MODEL_PROP_IS_DIR;
   else if(!strcmp("is_lnk", property))
     property_name = EIO_MODEL_PROP_IS_LNK;
   else if(!strcmp("size", property))
     property_name = EIO_MODEL_PROP_SIZE;

   switch(property_name)
     {
     case EIO_MODEL_PROP_FILENAME:
     case EIO_MODEL_PROP_PATH:
       {
          Eina_Value* v = ecore_promise_buffer_get(*promise);
          eina_value_setup(v, EINA_VALUE_TYPE_STRING);
          eina_value_set(v, value);
          ecore_promise_value_set(*promise, NULL);
       }
       break;
     default:
       {
          _Eio_Property_Promise* p = malloc(sizeof(_Eio_Property_Promise));
          p->promise = *promise;
          p->property = property_name;;
          priv->property_promises = eina_list_prepend(priv->property_promises, p);

          if(!priv->stat_file)
            _eio_stat_do(priv);
       }
       break;
     }
}

/**
 * Property Set
 */
static void
_eio_model_efl_model_base_property_set(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char * property, const Eina_Value *value)
{
   char *dest;

   EINA_SAFETY_ON_NULL_RETURN(property);

   if (strcmp(property, "path") != 0)
     return;

   dest = eina_value_to_string(value);
   if (priv->path == NULL)
     {
        priv->path = dest;

        INF("path '%s' with filename '%s'.", priv->path, basename(priv->path));

        _eio_monitors_list_load(priv);

        _eio_move_done_cb(priv, NULL);
     }
   else
     {
       priv->move_file = eio_file_move(priv->path, dest, _eio_progress_cb, _eio_move_done_cb, _eio_prop_set_error_cb, priv);
       free(priv->path);
       priv->path = dest;
     }
}

/**
 * Children Count Get
 */
static void
_eio_model_efl_model_base_children_count_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, Ecore_Promise **children_count)
{
   /**< eina_list_count returns 'unsigned int' */
   unsigned int c = eina_list_count(priv->children_list);
   fprintf(stderr, "children_count_get %d\n", (int)c);
   ecore_promise_value_set(*children_count, &c);
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

   child = eo_add(MY_CLASS, NULL, eio_model_path_set(eoid, info->path));
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


   Eina_List* i;
   _Eio_Children_Slice_Promise* p;
   EINA_LIST_FOREACH(priv->children_promises, i, p)
     {
       if ((p->start == 0) && (p->count == 0)) /* this is full data */
         {
            /*
             * children_accessor will be set to NULL by
             * eina_list_accessor_new if the later fails.
             */
           Eina_Accessor* accessor = eina_list_accessor_new(priv->children_list);
           ecore_promise_value_set(p->promise, &accessor);
         }
       else /* this is only slice */
         {
            Eo *child;
            Eina_List *l, *ln, *lr = NULL;
            ln = eina_list_nth_list(priv->children_list, (p->start-1));
            if (!ln)
              {
                 ecore_promise_error_set(p->promise, EINA_ERROR_OUT_OF_MEMORY);
                 ERR("children not found !");
                 return;
              }

            EINA_LIST_FOREACH(ln, l, child)
              {
                 eo_ref(child);
                 lr = eina_list_append(lr, child);
                 if (eina_list_count(lr) == p->count)
                   break;
              }

            // This may leak the children Eina_List.
            ecore_promise_value_set(p->promise, eina_list_accessor_new(lr));
         }

       free(p);
     }
   eina_list_free(priv->children_promises);

}

static void
_eio_error_children_load_cb(void *data, Eio_File *handler EINA_UNUSED, int error)
{
   Eio_Model_Data *priv = data;
   Eo *child;

   WRN("%d: %s.", error, strerror(error));

   EINA_LIST_FREE(priv->children_list, child)
     eo_unref(child);
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
static void
_eio_model_efl_model_base_child_del(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, Eo *child)
{
   Eio_Model_Data *child_priv;
   EINA_SAFETY_ON_NULL_RETURN(child);

   child_priv = eo_data_scope_get(child, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(child_priv);

   fprintf(stderr, __FILE__ ":%d %s child path: %s\n", __LINE__, __func__, child_priv->path); fflush(stderr);
   priv->del_file = eio_file_direct_stat(child_priv->path,
                                         &_eio_model_efl_model_base_child_del_stat,
                                         &_eio_error_unlink_cb,
                                         child);
   eo_ref(child);
}

/**
 * Children Slice Get
 */
static void
_eio_model_efl_model_base_children_slice_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv,
                                             unsigned start, unsigned count, Ecore_Promise **promise)
{
   /**
    * children must be already loaded otherwise we do nothing
    * and parameter is set to NULL.
    */

   if (!(priv->is_listed))
     {
       _Eio_Children_Slice_Promise* data = malloc(sizeof(struct _Eio_Children_Slice_Promise));
       data->promise = *promise;
       data->start = start;
       data->count = count;

       priv->children_promises = eina_list_prepend(priv->children_promises, data);

       _eio_model_efl_model_base_monitor_add(priv);

       eio_file_direct_ls(priv->path, _eio_filter_children_load_cb,
                          _eio_main_children_load_cb, _eio_done_children_load_cb,
                          _eio_error_children_load_cb, priv);
       return;
     }
}


/**
 * Class definitions
 */
static Eo *
_eio_model_eo_base_constructor(Eo *obj, Eio_Model_Data *priv)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
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
   priv->path = strdup(path);

   priv->monitor = NULL;
   _eio_monitors_list_load(priv);
}

static void
_eio_model_eo_base_destructor(Eo *obj , Eio_Model_Data *priv)
{
   Eo *child;
   /* unsigned int i; */

   if (priv->monitor)
     eio_monitor_del(priv->monitor);

   eina_spinlock_free(&priv->filter_lock);

   if (priv->properties_name)
     eina_array_free(priv->properties_name);

   EINA_LIST_FREE(priv->children_list, child)
     eo_unref(child);

   free(priv->path);
   eo_destructor(eo_super(obj, MY_CLASS));
}

#include "eio_model.eo.c"
