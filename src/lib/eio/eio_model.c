#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Emodel.h>
#include <Eina.h>
#include <eio_model.h>
#include <Eio.h>
#include <Ecore.h>
#include <Eo.h>

#include "eio_model_private.h"

#define MY_CLASS EIO_MODEL_CLASS
#define MY_CLASS_NAME "Eio_Model"

static Eina_Value_Struct_Desc *EIO_MODEL_PROPERTIES_DESC = NULL;
static void _eio_prop_set_error_cb(void *, Eio_File *, int);
static void _eio_model_emodel_properties_load(Eo *, Eio_Model_Data *);
static void _eio_model_emodel_children_load(Eo *, Eio_Model_Data *);

static void
_load_set(Eio_Model_Data *priv, Emodel_Load load)
{
   if ((priv->load.status & (EMODEL_LOAD_STATUS_LOADED | EMODEL_LOAD_STATUS_LOADING)) &&
                        (load.status & (EMODEL_LOAD_STATUS_LOADED | EMODEL_LOAD_STATUS_LOADING)))
     {
        load.status = priv->load.status | load.status;
        switch (load.status)
          {
           case EMODEL_LOAD_STATUS_LOADED_PROPERTIES:
             load.status &= ~EMODEL_LOAD_STATUS_LOADING_PROPERTIES;
             break;
           case EMODEL_LOAD_STATUS_LOADING_PROPERTIES:
             load.status &= ~EMODEL_LOAD_STATUS_LOADED_PROPERTIES;
             break;
           case EMODEL_LOAD_STATUS_LOADED_CHILDREN:
             load.status &= ~EMODEL_LOAD_STATUS_LOADING_CHILDREN;
             break;
           case EMODEL_LOAD_STATUS_LOADING_CHILDREN:
             load.status &= ~EMODEL_LOAD_STATUS_LOADED_CHILDREN;
             break;
           default: break;
          }
     }

   if (priv->load.status != load.status)
     {
         priv->load.status = load.status;
         eo_do(priv->obj, eo_event_callback_call(EMODEL_EVENT_LOAD_STATUS, &load));
     }
}

static void
_stat_pro_set(Eio_Model_Data *priv, int prop_id, const Eina_Value *value, Emodel_Property_Event *evt)
{
   Eina_Value old_value;
   int changed;
   Emodel_Property_Pair *pair = NULL;
   const char *prop = EIO_MODEL_PROPERTIES_DESC->members[prop_id].name;

   eina_value_struct_value_get(priv->properties, prop, &old_value);
   changed = eina_value_compare(&old_value, value);

   if(changed) eina_value_struct_value_set(priv->properties, prop, value);
   eina_value_flush(&old_value);
   if(!changed) return;

   pair = calloc(1, sizeof(Emodel_Property_Pair));
   EINA_SAFETY_ON_NULL_RETURN(pair);
   if(!eina_value_copy((Eina_Value*)value, &pair->value))
     {
        free(pair);
        return;
     }

   pair->property = eina_stringshare_add(prop);
   evt->changed_properties = eina_list_append(evt->changed_properties, pair);
}

/**
 *  Callbacks
 *  Property
 */
static void
_eio_stat_done_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_Stat *stat)
{
   Emodel_Property_Event evt;
   Eina_Value value;
   Eio_Model_Data *priv = data;
   Emodel_Load load;
   EINA_SAFETY_ON_FALSE_RETURN(eo_ref_get(priv->obj));

   priv->stat = stat;
   memset(&evt, 0, sizeof(Emodel_Property_Event));

   // Setup for Eina_Bool
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, eio_file_is_dir(stat));
   _stat_pro_set(priv, EIO_MODEL_PROP_IS_DIR, (const Eina_Value*)&value, &evt);
   eina_value_flush(&value);

   // Setup for Eina_Bool
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, eio_file_is_lnk(stat));
   _stat_pro_set(priv, EIO_MODEL_PROP_IS_LNK, (const Eina_Value*)&value, &evt);
   eina_value_flush(&value);

   // Setup for double
   eina_value_setup(&value, EINA_VALUE_TYPE_TIMEVAL);
   eina_value_set(&value, eio_file_mtime(stat));
   _stat_pro_set(priv, EIO_MODEL_PROP_MTIME, (const Eina_Value*)&value, &evt);
   eina_value_flush(&value);

   // Setup for long long
   eina_value_setup(&value, EINA_VALUE_TYPE_INT64);
   eina_value_set(&value, eio_file_size(stat));
   _stat_pro_set(priv, EIO_MODEL_PROP_SIZE, (const Eina_Value*)&value, &evt);
   eina_value_flush(&value);

   if (evt.changed_properties != NULL)
     {
        Emodel_Property_Pair *pair;
        eo_do(priv->obj, eo_event_callback_call(EMODEL_EVENT_PROPERTIES_CHANGED, &evt));
        EINA_LIST_FREE(evt.changed_properties, pair)
          {
             eina_stringshare_del(pair->property);
             eina_value_flush(&pair->value);
             free(pair);
          }
     }

   load.status = EMODEL_LOAD_STATUS_LOADED_PROPERTIES;
   _load_set(priv, load);

   if (priv->load_pending & EMODEL_LOAD_STATUS_LOADED_CHILDREN)
     _eio_model_emodel_children_load(priv->obj, priv);
}

static void
_eio_progress_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const Eio_Progress *info EINA_UNUSED)
{
   //TODO: implement
}

static void
_eio_move_done_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   Emodel_Property_Event evt;
   Emodel_Property_Pair pair_path, pair_filename;
   Eio_Model_Data *priv = data;
   Eina_Value_Struct_Desc *desc = EIO_MODEL_PROPERTIES_DESC;

   EINA_SAFETY_ON_FALSE_RETURN(eo_ref_get(priv->obj));

   memset(&evt, 0, sizeof(Emodel_Property_Event));

   /**
    * When mv is executed we update our values and
    * notify both path and filename properties listeners.
    */
   pair_path.property = eina_stringshare_add(desc->members[EIO_MODEL_PROP_PATH].name);
   eina_value_struct_set(priv->properties, pair_path.property, priv->path);
   eina_value_struct_value_get(priv->properties, pair_path.property, &pair_path.value);
   evt.changed_properties = eina_list_append(evt.changed_properties, &pair_path);

   pair_filename.property = eina_stringshare_add(desc->members[EIO_MODEL_PROP_FILENAME].name);
   eina_value_struct_set(priv->properties, pair_filename.property, basename(priv->path));
   eina_value_struct_value_get(priv->properties, pair_filename.property, &pair_filename.value);
   evt.changed_properties = eina_list_append(evt.changed_properties, &pair_filename);

   eo_do(priv->obj, eo_event_callback_call(EMODEL_EVENT_PROPERTIES_CHANGED, &evt));

   eina_stringshare_del(pair_path.property);
   eina_stringshare_del(pair_filename.property);
   eina_value_flush(&pair_filename.value);
   eina_value_flush(&pair_path.value);

   eina_list_free(evt.changed_properties);
}

static void
_eio_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   if(error != 0)
     {
        fprintf(stderr, "Error: %s : %d: %s\n", __FUNCTION__, error, strerror(errno));
        EINA_SAFETY_ON_FALSE_RETURN(EINA_FALSE); /**< force check error only to be more verbose */
     }
   fprintf(stdout, "%s : %d\n", __FUNCTION__, error);
}

static void
_eio_prop_set_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error EINA_UNUSED)
{
   fprintf(stdout, "%s : %d\n", __FUNCTION__, error);
}


/**
 *  Callbacks
 *  Ecore Events
 */
static Eina_Bool
 _emodel_evt_added_ecore_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eio_Monitor_Event *evt = (Eio_Monitor_Event*)event;
   Eio_Model_Data *priv = data;
   Emodel_Children_Event cevt;
   Eina_Value path;

   if(priv->children_list)
     {
       cevt.child = eo_add_ref(EIO_MODEL_CLASS, priv->obj, eio_model_path_set(evt->filename));
       priv->children_list = eina_list_append(priv->children_list, cevt.child);
       cevt.index = eina_list_count(priv->children_list);

       eina_value_setup(&path, EINA_VALUE_TYPE_STRING);
       eina_value_set(&path, evt->filename);
       eo_do(cevt.child, eio_model_children_filter_set(priv->filter_cb, priv->filter_userdata));
       eina_value_flush(&path);

       eo_do(priv->obj, eo_event_callback_call(EMODEL_EVENT_CHILD_ADDED, &cevt));
     }

   return EINA_TRUE;
}

static Eina_Bool
_emodel_evt_deleted_ecore_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eio_Monitor_Event *evt = (Eio_Monitor_Event*)event;
   Eio_Model_Data *priv = data;

   if(priv->children_list)
     {
       Eina_List* cur = priv->children_list;
       int i;
       for(i = 0; cur; ++i, cur = cur->next)
         {
           Eio_Model_Data *cur_priv = eo_data_scope_get(cur->data, MY_CLASS);
           if(strcmp(cur_priv->path, evt->filename) == 0)
             break;
         }
       if(cur)
         {
           Emodel_Children_Event cevt;


           cevt.index = i;
           cevt.child = cur->data;

           eo_do(priv->obj, eo_event_callback_call(EMODEL_EVENT_CHILD_REMOVED, &cevt));

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

   fprintf(stdout, "%s : %d\n", __FUNCTION__, error);

   eo_unref(priv->obj);
}


/**
 * Interfaces impl.
 */
static Emodel_Load_Status
_eio_model_emodel_properties_list_get(Eo *obj EINA_UNUSED,
                                       Eio_Model_Data *_pd, Eina_List * const* properties_list)
{
   Eio_Model_Data *priv = _pd;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->obj, EINA_FALSE);

   if(priv->properties_list == NULL)
     {
        Eina_Value_Struct_Desc *desc = EIO_MODEL_PROPERTIES_DESC;
        for(i = 0; i < desc->member_count; ++i)
          priv->properties_list = eina_list_append(priv->properties_list, desc->members[i].name);
     }

   *(Eina_List **)properties_list = priv->properties_list;

   return priv->load.status;
}

/**
 * Property Fetch //carlos
 */
static Emodel_Load_Status
_eio_model_emodel_property_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char *property, Eina_Value *value)
{
   Eina_Value _v;
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EMODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, EMODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->obj, EMODEL_LOAD_STATUS_ERROR);

   if (eina_value_struct_value_get(priv->properties, property, &_v) == EINA_TRUE)
     {
        eina_value_copy(&_v, value);
     }

   return priv->load.status;
}

/**
 * Property Set
 */
static Emodel_Load_Status
_eio_model_emodel_property_set(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char * property, Eina_Value value)
{
   char *dest;
   Eina_Value v = value;

   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EINA_FALSE);

   if (strcmp(property, "path") != 0)
     return EINA_FALSE;

   dest = eina_value_to_string(&v);
   if (priv->path == NULL)
     {
         priv->path = dest;

         fprintf(stdout, " path %s filename %s\n", priv->path, basename(priv->path));

         eina_value_struct_set(priv->properties, "path", priv->path);
         eina_value_struct_set(priv->properties, "filename", eina_stringshare_add(basename(priv->path)));

         _eio_monitors_list_load(priv);

         _eio_move_done_cb(priv, NULL);

         if (priv->load_pending & EMODEL_LOAD_STATUS_LOADED_PROPERTIES)
            _eio_model_emodel_properties_load(obj, priv);

         if (priv->load_pending & EMODEL_LOAD_STATUS_LOADED_CHILDREN)
            _eio_model_emodel_children_load(obj, priv);

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
static Emodel_Load_Status
_eio_model_emodel_children_count_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, unsigned int *children_count)
{
   /**< eina_list_count returns 'unsigned int' */
   *children_count = eina_list_count(priv->children_list);
   return priv->load.status;
}

/**
 * Properties Load
 */
 static void
_eio_model_emodel_properties_load(Eo *obj EINA_UNUSED, Eio_Model_Data *priv)
{
   Emodel_Load load;
   if (priv->path == NULL)
     {
         priv->load_pending |= EMODEL_LOAD_STATUS_LOADED_PROPERTIES;
         return;
     }
   priv->load_pending &= ~EMODEL_LOAD_STATUS_LOADED_PROPERTIES;

   if (!(priv->load.status & (EMODEL_LOAD_STATUS_LOADED_PROPERTIES | EMODEL_LOAD_STATUS_LOADING_PROPERTIES)))
     {
        load.status = EMODEL_LOAD_STATUS_LOADING_PROPERTIES;
         _load_set(priv, load);
         priv->file = eio_file_direct_stat(priv->path, _eio_stat_done_cb, _eio_error_cb, priv);
     }
}

static void
_eio_model_emodel_monitor_add(Eio_Model_Data *priv)
{
   if(!priv->monitor)
     {
       priv->monitor = eio_monitor_add(priv->path);
       int i = 0;
       for(i = 0; priv->mon.mon_event_child_add[i] != EIO_MONITOR_ERROR ; ++i)
         priv->mon.ecore_child_add_handler[i] =
           ecore_event_handler_add(priv->mon.mon_event_child_add[i], _emodel_evt_added_ecore_cb, priv);
       for(i = 0; priv->mon.mon_event_child_del[i] != EIO_MONITOR_ERROR ; ++i)
         priv->mon.ecore_child_add_handler[i] =
           ecore_event_handler_add(priv->mon.mon_event_child_del[i], _emodel_evt_deleted_ecore_cb, priv);
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

   if (priv->filter_cb)
     {
         return priv->filter_cb(priv->filter_userdata, handler, info);
     }

   return EINA_TRUE;
}

static void
_eio_main_children_load_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_File_Direct_Info *info)
{
   Eo *child;
   Eio_Model_Data *priv = data;
   EINA_SAFETY_ON_NULL_RETURN(priv);

   child = eo_add_ref(MY_CLASS, NULL, eio_model_path_set(info->path));
   if (priv->filter_cb)
     {
        eo_do(child, eio_model_children_filter_set(priv->filter_cb, priv->filter_userdata));
     }
   priv->children_list = eina_list_append(priv->children_list, child);
}

static void
_eio_done_children_load_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   unsigned long count;
   Eio_Model_Data *priv = data;
   Emodel_Load load;
   EINA_SAFETY_ON_NULL_RETURN(priv);

   count = eina_list_count(priv->children_list);
   load.status = EMODEL_LOAD_STATUS_LOADED_CHILDREN;

   _load_set(priv, load);
   eo_do(priv->obj, eo_event_callback_call(EMODEL_EVENT_CHILDREN_COUNT_CHANGED, &count));
}

static void
_eio_error_children_load_cb(void *data, Eio_File *handler EINA_UNUSED, int error)
{
   Eio_Model_Data *priv = data;
   Eo *child;
   Emodel_Load load;
   fprintf(stderr, "%s: err=%d\n", __FUNCTION__, error);
   EINA_LIST_FREE(priv->children_list, child)
       eo_unref(child);

   load.status = EMODEL_LOAD_STATUS_LOADED_CHILDREN;

   _load_set(priv, load);
}

/**
 * Children Load
 */
static void
_eio_model_emodel_children_load(Eo *obj EINA_UNUSED, Eio_Model_Data *priv)
{
   Emodel_Load load;
   if (priv->path == NULL)
     {
         priv->load_pending |= EMODEL_LOAD_STATUS_LOADED_CHILDREN;
         return;
     }
   priv->load_pending &= ~EMODEL_LOAD_STATUS_LOADED_CHILDREN;

   if (!(priv->load.status & (EMODEL_LOAD_STATUS_LOADED_CHILDREN | EMODEL_LOAD_STATUS_LOADING_CHILDREN)))
     {
        _eio_model_emodel_monitor_add(priv);

        load.status = EMODEL_LOAD_STATUS_LOADING_CHILDREN;
         _load_set(priv, load);
         eio_file_direct_ls(priv->path, _eio_filter_children_load_cb,
                      _eio_main_children_load_cb, _eio_done_children_load_cb,
                      _eio_error_children_load_cb, priv);
     }
}

/**
 * Load
 */
 static void
_eio_model_emodel_load(Eo *obj, Eio_Model_Data *priv)
{
    priv->load_pending |= EMODEL_LOAD_STATUS_LOADED_CHILDREN;
    _eio_model_emodel_properties_load(obj, priv);
}

/**
 * Load status get
 */
static Emodel_Load_Status
_eio_model_emodel_load_status_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv)
{
   return priv->load.status;
}

/**
 * Unload
 */
static void
_eio_model_emodel_unload(Eo *obj  EINA_UNUSED, Eio_Model_Data *priv)
{
   Emodel_Load load;
   if (!(priv->load.status & EMODEL_LOAD_STATUS_UNLOADED))
     {
         Eo *child;
         EINA_LIST_FREE(priv->children_list, child)
           {
              eo_unref(child);
           }

         load.status = EMODEL_LOAD_STATUS_UNLOADED;
         _load_set(priv, load);
     }
}

static void
_eio_model_children_filter_set(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, Eio_Filter_Direct_Cb filter_cb, void *data)
{
   priv->filter_cb = filter_cb;
   priv->filter_userdata = data;
}

/**
 * Child Add
 */
static Eo *
_eio_model_emodel_child_add(Eo *obj EINA_UNUSED, Eio_Model_Data *priv EINA_UNUSED)
{
   return eo_add(EIO_MODEL_CLASS, obj);
}

static void
_eio_model_emodel_child_del_stat(void* data, Eio_File* handler EINA_UNUSED, const Eina_Stat* stat)
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
static Emodel_Load_Status
_eio_model_emodel_child_del(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, Eo *child)
{
   Eio_Model_Data *child_priv;
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EMODEL_LOAD_STATUS_ERROR);

   if (priv->children_list != NULL)
     {
         priv->children_list = eina_list_remove(priv->children_list, child);
     }

   child_priv = eo_data_scope_get(child, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_priv, EMODEL_LOAD_STATUS_ERROR);

   eio_file_direct_stat(child_priv->path, &_eio_model_emodel_child_del_stat
                        , &_eio_error_unlink_cb, child);
   eo_ref(child);
   return priv->load.status;
}

/**
 * Children Slice Get
 * TODO/XXX/FIXME: Untested code - validate this implementation
 */
static Emodel_Load_Status
_eio_model_emodel_children_slice_get(Eo *obj EINA_UNUSED, Eio_Model_Data *priv,
                                        unsigned start, unsigned count, Eina_Accessor **children_accessor)
{
   Eo *child;
   Eina_List *l, *ln, *lr = NULL;

   /**
    * children must be already loaded otherwise we do nothing
    * and parameter is set to NULL.
    */

   if(!(priv->load.status & EMODEL_LOAD_STATUS_LOADED_CHILDREN))
     {
        /**
         * Status should be in either unloaded state or unitialized
         * so we simply return without much alarm.
         */
        *children_accessor = NULL;
        return priv->load.status;
     }

   if((start == 0) && (count == 0)) /* this is full data */
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
        if(!ln)
          {
             /**
              * In error, we make it more verbose
              * by forcing warning to be displayed on terminal.
              */
             *children_accessor = NULL;
             EINA_SAFETY_ON_NULL_RETURN_VAL(NULL, priv->load.status);
          }
        EINA_LIST_FOREACH(ln, l, child)
          {
             eo_ref(child);
             lr = eina_list_append(lr, child);
             if (eina_list_count(lr) == count)
               break;
          }
        *children_accessor = eina_list_accessor_new(lr);
     }

   return priv->load.status;
}

static void
_struct_properties_init(void)
{
   typedef struct _This_Eio_Properties
     {
        const char *filename;
        const char *path;
        double mtime;
        int is_dir;
        int is_lnk;
        int size;
     } This_Eio_Properties;

   static Eina_Value_Struct_Member prop_members[] = {
     EINA_VALUE_STRUCT_MEMBER(NULL, This_Eio_Properties, filename),
     EINA_VALUE_STRUCT_MEMBER(NULL, This_Eio_Properties, path),
     EINA_VALUE_STRUCT_MEMBER(NULL, This_Eio_Properties, mtime),
     EINA_VALUE_STRUCT_MEMBER(NULL, This_Eio_Properties, is_dir),
     EINA_VALUE_STRUCT_MEMBER(NULL, This_Eio_Properties, is_lnk),
     EINA_VALUE_STRUCT_MEMBER(NULL, This_Eio_Properties, size)
   };
   //XXX: Check data types
   prop_members[EIO_MODEL_PROP_FILENAME].type = EINA_VALUE_TYPE_STRING;
   prop_members[EIO_MODEL_PROP_PATH].type = EINA_VALUE_TYPE_STRING;
   prop_members[EIO_MODEL_PROP_MTIME].type = EINA_VALUE_TYPE_TIMEVAL;
   prop_members[EIO_MODEL_PROP_IS_DIR].type = EINA_VALUE_TYPE_INT;
   prop_members[EIO_MODEL_PROP_IS_LNK].type = EINA_VALUE_TYPE_INT;
   prop_members[EIO_MODEL_PROP_SIZE].type = EINA_VALUE_TYPE_INT64;

   static Eina_Value_Struct_Desc prop_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, // no special operations
     prop_members,
     EINA_C_ARRAY_LENGTH(prop_members),
     sizeof(This_Eio_Properties)
   };
   EIO_MODEL_PROPERTIES_DESC = &prop_desc;
}

/**
 * Class definitions
 */
static void
_eio_model_eo_base_constructor(Eo *obj, Eio_Model_Data *priv)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   priv->obj = obj;
   _struct_properties_init();
   priv->properties = eina_value_struct_new(EIO_MODEL_PROPERTIES_DESC);
   EINA_SAFETY_ON_NULL_RETURN(priv->properties);

   priv->load.status = EMODEL_LOAD_STATUS_UNLOADED;
   priv->monitor = NULL;
}

static void
_eio_model_path_set(Eo *obj EINA_UNUSED, Eio_Model_Data *priv, const char *path)
{
  //_eio_model_eo_base_constructor(obj, priv);
   priv->path = strdup(path);

   eina_value_struct_set(priv->properties, "path", priv->path);
   eina_value_struct_set(priv->properties, "filename", basename(priv->path));

   priv->monitor = NULL;
   _eio_monitors_list_load(priv);
}

static void
_eio_model_eo_base_destructor(Eo *obj , Eio_Model_Data *priv)
{
   Eo *child;
   if(priv->monitor)
     eio_monitor_del(priv->monitor);

   eina_list_free(priv->properties_list);
   eina_value_free(priv->properties);

   EINA_LIST_FREE(priv->children_list, child)
     eo_unref(child);

   free(priv->path);
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "eio_model.eo.c"
