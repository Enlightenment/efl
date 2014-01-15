#ifndef ELM_WIDGET_FILESELECTOR_H
#define ELM_WIDGET_FILESELECTOR_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-fileselector-class The Elementary Fileselector Class
 *
 * Elementary, besides having the @ref Fileselector widget, exposes its
 * foundation -- the Elementary Fileselector Class -- in order to create other
 * widgets which are a fileselector with some more logic on top.
 */

typedef struct _Elm_Fileselector_Filter Elm_Fileselector_Filter;

/**
 * Base layout smart data extended with fileselector instance data.
 */
typedef struct _Elm_Fileselector_Smart_Data Elm_Fileselector_Smart_Data;
struct _Elm_Fileselector_Smart_Data
{
   EINA_REFCOUNT;

   Evas_Object             *obj;
   Evas_Object             *path_entry;
   Evas_Object             *name_entry;
   Evas_Object             *files_view;
   Evas_Object             *up_button;
   Evas_Object             *home_button;
   Evas_Object             *spinner;
   Evas_Object             *filter_hoversel;
   Evas_Object             *ok_button;
   Evas_Object             *cancel_button;

   Eina_List               *filter_list;
   Elm_Fileselector_Filter *current_filter;

   /* a list of selected paths. only for multi selection */
   Eina_List               *paths;

   const char              *path;
   const char              *selection;
   Ecore_Idler             *populate_idler;

   const char              *path_separator;

   Eio_File                *current;
   Eio_Monitor             *monitor;
   Eina_List               *handlers;

   Evas_Coord_Size          thumbnail_size;

   /* a sort method to decide orders of files/directories */
   int                    (*sort_method)(const char *, const char *);

   Elm_Fileselector_Mode    mode;
   Elm_Fileselector_Sort    sort_type;

   Eina_Bool                only_folder : 1;
   Eina_Bool                expand : 1;
   Eina_Bool                double_tap_navigation : 1;
   Eina_Bool                multi : 1;

   /* this flag is only for multi selection.
    * If this flag is set to EINA_TRUE, it means directory is selected
    * so that fileselector will clear current selection when user clicked
    * another item. */
   Eina_Bool                dir_selected : 1;

   Eina_Bool                hidden_visible : 1;
};

struct sel_data
{
   Evas_Object *fs;
   const char  *path;
};

typedef struct _Listing_Request Listing_Request;
struct _Listing_Request
{
   Elm_Fileselector_Smart_Data *sd;
   Elm_Object_Item             *parent_it;

   Evas_Object                 *obj;
   const char                  *path;
   const char                  *selected;
   Eina_Bool                    first : 1;
};

typedef enum {
   ELM_DIRECTORY = 0,
   ELM_FILE_IMAGE = 1,
   ELM_FILE_UNKNOW = 2,
   ELM_FILE_LAST
} Elm_Fileselector_Type;

typedef enum {
   ELM_FILESELECTOR_MIME_FILTER = 0,
   ELM_FILESELECTOR_CUSTOM_FILTER,
   ELM_FILESELECTOR_FILTER_LAST
} Elm_Fileselector_Filter_Type;

typedef struct _Elm_Fileselector_Custom_Filter Elm_Fileselector_Custom_Filter;
struct _Elm_Fileselector_Custom_Filter
{
   Elm_Fileselector_Filter_Func  func;
   void                         *data;
};

struct _Elm_Fileselector_Filter
{
   const char                         *filter_name;
   Elm_Fileselector_Smart_Data        *sd;

   union {
      char                           **mime_types;
      Elm_Fileselector_Custom_Filter  *custom;
   } filter;

   Elm_Fileselector_Filter_Type        filter_type;
};

/**
 * @}
 */

#define ELM_FILESELECTOR_DATA_GET(o, sd) \
  Elm_Fileselector_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_FILESELECTOR_CLASS)

#define ELM_FILESELECTOR_DATA_GET_OR_RETURN(o, ptr)  \
  ELM_FILESELECTOR_DATA_GET(o, ptr);                 \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_FILESELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_FILESELECTOR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                   \
    {                                                        \
       CRI("No widget data for object %p (%s)",              \
           o, evas_object_type_get(o));                      \
       return val;                                           \
    }

#define ELM_FILESELECTOR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_FILESELECTOR_CLASS))) \
    return

#endif
