#ifndef ELM_WIDGET_FILESELECTOR_H
#define ELM_WIDGET_FILESELECTOR_H

#include "elm_widget_layout.h"

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
 * @def ELM_FILESELECTOR_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Fileselector_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_FILESELECTOR_CLASS(x) ((Elm_Fileselector_Smart_Class *)x)

/**
 * @def ELM_FILESELECTOR_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Fileselector_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_FILESELECTOR_DATA(x)  ((Elm_Fileselector_Smart_Data *)x)

/**
 * @def ELM_FILESELECTOR_SMART_CLASS_VERSION
 *
 * Current version for Elementary fileselector @b base smart class, a value
 * which goes to _Elm_Fileselector_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_FILESELECTOR_SMART_CLASS_VERSION 1

/**
 * @def ELM_FILESELECTOR_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Fileselector_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_FILESELECTOR_SMART_CLASS_INIT_NULL
 * @see ELM_FILESELECTOR_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_FILESELECTOR_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_FILESELECTOR_SMART_CLASS_VERSION}

/**
 * @def ELM_FILESELECTOR_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Fileselector_Smart_Class structure.
 *
 * @see ELM_FILESELECTOR_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_FILESELECTOR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_FILESELECTOR_SMART_CLASS_INIT_NULL \
  ELM_FILESELECTOR_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_FILESELECTOR_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Fileselector_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_FILESELECTOR_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Fileselector_Smart_Class (base field)
 * to the latest #ELM_FILESELECTOR_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_FILESELECTOR_SMART_CLASS_INIT_NULL
 * @see ELM_FILESELECTOR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_FILESELECTOR_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_FILESELECTOR_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary fileselector base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a fileselector.
 *
 * All of the functions listed on @ref Fileselector namespace will work for
 * objects deriving from #Elm_Fileselector_Smart_Class.
 */
typedef struct _Elm_Fileselector_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Fileselector_Smart_Class;

/**
 * Base layout smart data extended with fileselector instance data.
 */
typedef struct _Elm_Fileselector_Smart_Data Elm_Fileselector_Smart_Data;
struct _Elm_Fileselector_Smart_Data
{
   Elm_Layout_Smart_Data base;

   EINA_REFCOUNT;

   Evas_Object          *filename_entry;
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

EAPI extern const char ELM_FILESELECTOR_SMART_NAME[];
EAPI const Elm_Fileselector_Smart_Class *elm_fileselector_smart_class_get(void);

#define ELM_FILESELECTOR_DATA_GET(o, sd) \
  Elm_Fileselector_Smart_Data * sd = evas_object_smart_data_get(o)

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
  if (!obj || !elm_widget_type_check                    \
        ((obj), ELM_FILESELECTOR_SMART_NAME, __func__)) \
    return

#endif
