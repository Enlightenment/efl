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

/**
 * !! Warning: Do not change enum's order
 * before checking _eio_model_constructor.
 * @see Eina_Value_Struct_Member.
 */
enum {
   EIO_MODEL_PROP_FILENAME = 0,
   EIO_MODEL_PROP_PATH,
   EIO_MODEL_PROP_MTIME,
   EIO_MODEL_PROP_IS_DIR,
   EIO_MODEL_PROP_IS_LNK,
   EIO_MODEL_PROP_SIZE
};

struct _Eio_Model_Data
{
   Eo *obj;
   char *path;
   Eina_Array *properties_array;
   Eina_Value *properties;
   Emodel_Load load;
   int load_pending;
   Eina_List *children_list;
   /**< EIO data */
   Eio_File *file;
   const Eina_Stat *stat;
   Eio_Monitor *monitor;
   Eio_Model_Monitor_Data mon;
   int cb_count_child_add; /**< monitor reference counter for child add event */
   int cb_count_child_del; /**< monitor reference counter for child del event*/
   Eio_Filter_Direct_Cb filter_cb;
   void *filter_userdata;
};

#endif
