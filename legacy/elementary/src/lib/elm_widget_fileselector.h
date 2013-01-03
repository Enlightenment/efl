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

/**
 * Base layout smart data extended with fileselector instance data.
 */
typedef struct _Elm_Fileselector_Smart_Data Elm_Fileselector_Smart_Data;
struct _Elm_Fileselector_Smart_Data
{
   EINA_REFCOUNT;

   Evas_Object          *obj;
   Evas_Object          *path_entry;
   Evas_Object          *files_list;
   Evas_Object          *files_grid;
   Evas_Object          *up_button;
   Evas_Object          *home_button;
   Evas_Object          *spinner;
   Evas_Object          *ok_button;
   Evas_Object          *cancel_button;

   const char           *path;
   const char           *selection;
   Ecore_Idler          *sel_idler;

   const char           *path_separator;

#ifdef HAVE_EIO
   Eio_File             *current;
#endif

   Elm_Fileselector_Mode mode;

   Eina_Bool             only_folder : 1;
   Eina_Bool             expand : 1;
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
   Eina_Bool                    first : 1;
};

typedef enum {
   ELM_DIRECTORY = 0,
   ELM_FILE_IMAGE = 1,
   ELM_FILE_UNKNOW = 2,
   ELM_FILE_LAST
} Elm_Fileselector_Type;

/**
 * @}
 */

#define ELM_FILESELECTOR_DATA_GET(o, sd) \
  Elm_Fileselector_Smart_Data * sd = eo_data_get(o, ELM_OBJ_FILESELECTOR_CLASS)

#define ELM_FILESELECTOR_DATA_GET_OR_RETURN(o, ptr)  \
  ELM_FILESELECTOR_DATA_GET(o, ptr);                 \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_FILESELECTOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_FILESELECTOR_DATA_GET(o, ptr);                         \
  if (!ptr)                                                  \
    {                                                        \
       CRITICAL("No widget data for object %p (%s)",         \
                o, evas_object_type_get(o));                 \
       return val;                                           \
    }

#define ELM_FILESELECTOR_CHECK(obj)                     \
  if (!eo_isa((obj), ELM_OBJ_FILESELECTOR_CLASS)) \
    return

#endif
