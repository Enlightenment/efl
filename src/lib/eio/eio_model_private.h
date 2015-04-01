#ifndef _EIO_MODEL_PRIVATE_H
#define _EIO_MODEL_PRIVATE_H

#define PROP_LIST_SIZE 7

typedef struct _Eio_Model_Data                 Eio_Model_Data;
typedef struct _Eio_Model_Monitor_Data         Eio_Model_Monitor_Data;

struct _Eio_Model_Monitor_Data
{
   Ecore_Event_Handler *ecore_child_add_handler[3];
   Ecore_Event_Handler *ecore_child_del_handler[3];
   int mon_event_child_add[3]; /**< plus EIO_MONITOR_ERROR */
   int mon_event_child_del[3]; /**< plus EIO_MONITOR_ERROR */
};

enum {
   EIO_MODEL_PROP_FILENAME = 0,
   EIO_MODEL_PROP_PATH,
   EIO_MODEL_PROP_MTIME,
   EIO_MODEL_PROP_IS_DIR,
   EIO_MODEL_PROP_IS_LNK,
   EIO_MODEL_PROP_SIZE,
   EIO_MODEL_PROP_LAST
};

static const char* _eio_model_prop_names[] =
{
   [EIO_MODEL_PROP_FILENAME]  = "filename",
   [EIO_MODEL_PROP_PATH]      = "path",
   [EIO_MODEL_PROP_MTIME]     = "mtime",
   [EIO_MODEL_PROP_IS_DIR]    = "is_dir",
   [EIO_MODEL_PROP_IS_LNK]    = "is_lnk",
   [EIO_MODEL_PROP_SIZE]      = "size"
};

struct _Eio_Model_Data
{
   Eo *obj;
   char *path;
   Eina_Array *properties_name;
   Eina_Value *properties_value[EIO_MODEL_PROP_LAST];
   Efl_Model_Load load;
   int load_pending;
   Eina_List *children_list;
   /**< EIO data */
   Eio_File *file;
   Eina_Bool is_dir;
   Eio_Monitor *monitor;
   Eio_Model_Monitor_Data mon;
   int cb_count_child_add; /**< monitor reference counter for child add event */
   int cb_count_child_del; /**< monitor reference counter for child del event*/
   Eio_Filter_Direct_Cb filter_cb;
   void *filter_userdata;
};

#endif
