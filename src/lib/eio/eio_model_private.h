#ifndef _EIO_MODEL_PRIVATE_H
#define _EIO_MODEL_PRIVATE_H

#define PROP_LIST_SIZE 8

typedef struct _Eio_Model_Data                 Eio_Model_Data;
typedef struct _Eio_Model_Info                 Eio_Model_Info;

typedef struct _Eio_Model_Monitor_Data         Eio_Model_Monitor_Data;
struct _Eio_Model_Monitor_Data
{
   Ecore_Event_Handler *ecore_child_add_handler[3];
   Ecore_Event_Handler *ecore_child_del_handler[3];
   int mon_event_child_add[3]; /**< plus EIO_MONITOR_ERROR */
   int mon_event_child_del[3]; /**< plus EIO_MONITOR_ERROR */
};

// FIXME: Would be more efficient to introduce an Eina_Path that assemble
// an array of stringshare instead of using one mega stringshare directly.

struct _Eio_Model_Info
{
   Eina_Stringshare *path;
   Eo *object;

   size_t path_length;
   size_t name_length;
   size_t name_start;

   Eina_File_Type type;

   Eina_Bool parent_ref : 1;
   Eina_Bool child_ref : 1;
};

struct _Eio_Model_Data
{
   Efl_Loop *loop;
   Eio_Model *self;

   Eina_Stringshare *path;

   Eio_Model_Info *info;
   Eina_Stat *st;
   const char *mime_type;

   struct {
      Eio_File *stat; // Move to use efl.io.manager.stat
      Eina_Future *listing; // Move to use efl.io.manager.direct_ls
      Eio_File *move;
      Eio_File *del;
      Eina_Future *mime;
   } request;

   struct {
      EflIoFilter cb;
      Eina_Free_Cb free;
      void *data;
   } filter;

   Eio_Model_Monitor_Data mon;

   Eio_Monitor *monitor; // Notification stuff
   // FIXME: would be interesting to figure a more efficient structure for holding files
   Eina_List *parent;
   Eina_List *files; // Eio_Model_Info

   Eina_Error error;

   Eina_Bool listed : 1;
};

#endif
