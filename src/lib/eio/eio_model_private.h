#ifndef _EIO_MODEL_PRIVATE_H
#define _EIO_MODEL_PRIVATE_H

#define PROP_LIST_SIZE 8

typedef struct _Eio_Model_Data                 Eio_Model_Data;
typedef struct _Eio_Model_Monitor_Data         Eio_Model_Monitor_Data;

struct _Eio_Model_Monitor_Data
{
   Ecore_Event_Handler *ecore_child_add_handler[3];
   Ecore_Event_Handler *ecore_child_del_handler[3];
   int mon_event_child_add[3]; /**< plus EIO_MONITOR_ERROR */
   int mon_event_child_del[3]; /**< plus EIO_MONITOR_ERROR */
};

typedef enum _Eio_Property_Name {
   EIO_MODEL_PROP_FILENAME = 0,
   EIO_MODEL_PROP_PATH,
   EIO_MODEL_PROP_MTIME,
   EIO_MODEL_PROP_IS_DIR,
   EIO_MODEL_PROP_IS_LNK,
   EIO_MODEL_PROP_SIZE,
   EIO_MODEL_PROP_MIME_TYPE,
   EIO_MODEL_PROP_LAST
} _Eio_Property_Name;

static const char* _eio_model_prop_names[] =
{
   [EIO_MODEL_PROP_FILENAME]  = "filename",
   [EIO_MODEL_PROP_PATH]      = "path",
   [EIO_MODEL_PROP_MTIME]     = "mtime",
   [EIO_MODEL_PROP_IS_DIR]    = "is_dir",
   [EIO_MODEL_PROP_IS_LNK]    = "is_lnk",
   [EIO_MODEL_PROP_SIZE]      = "size",
   [EIO_MODEL_PROP_MIME_TYPE] = "mime_type"
};

typedef struct _Eio_Property_Promise _Eio_Property_Promise;
struct _Eio_Property_Promise
{
  _Eio_Property_Name property;
  Efl_Promise* promise;
};

typedef struct _Eio_Children_Slice_Promise _Eio_Children_Slice_Promise;
struct _Eio_Children_Slice_Promise
{
  unsigned start;
  unsigned count;
  Efl_Promise* promise;
};

typedef struct _Eio_Model_Data _Eio_Model_Data;
struct _Eio_Model_Data
{
   Eo *obj;
   Eina_Stringshare *path;
   Eina_Array *properties_name;
   Eina_Bool is_listed : 1;
   Eina_Bool is_listing : 1;
   Eina_List *children_list;
   Eina_List *property_promises;
   Eina_List *children_promises;
   Eina_List *count_promises;
   /**< EIO data */
   Eio_File *stat_file;
   Eio_File *listing_file;
   Eio_File *move_file;
   Eio_File *del_file;
   Eio_Monitor *monitor;
   Eio_Model_Monitor_Data mon;
   int cb_count_child_add; /**< monitor reference counter for child add event */
   int cb_count_child_del; /**< monitor reference counter for child del event*/
   unsigned int count;
   Eio_Filter_Direct_Cb filter_cb;
   void *filter_userdata;
   Eina_Spinlock filter_lock; /**< filter callback is called from another thread */

   Eina_List *fetching_mime;
};

#endif
