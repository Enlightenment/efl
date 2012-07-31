#ifndef ELM_WIDGET_BG_H
#define ELM_WIDGET_BG_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-bg-class The Elementary Bg Class
 *
 * Elementary, besides having the @ref Bg widget, exposes its
 * foundation -- the Elementary Bg Class -- in order to create other
 * widgets which are a bg with some more logic on top.
 */

/**
 * @def ELM_BG_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Bg_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_BG_CLASS(x) ((Elm_Bg_Smart_Class *)x)

/**
 * @def ELM_BG_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Bg_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_BG_DATA(x)  ((Elm_Bg_Smart_Data *)x)

/**
 * @def ELM_BG_SMART_CLASS_VERSION
 *
 * Current version for Elementary bg @b base smart class, a value
 * which goes to _Elm_Bg_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_BG_SMART_CLASS_VERSION 1

/**
 * @def ELM_BG_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Bg_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_BG_SMART_CLASS_INIT_NULL
 * @see ELM_BG_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_BG_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_BG_SMART_CLASS_VERSION}

/**
 * @def ELM_BG_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Bg_Smart_Class structure.
 *
 * @see ELM_BG_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_BG_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_BG_SMART_CLASS_INIT_NULL \
  ELM_BG_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_BG_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Bg_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_BG_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Bg_Smart_Class (base field)
 * to the latest #ELM_BG_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_BG_SMART_CLASS_INIT_NULL
 * @see ELM_BG_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_BG_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_BG_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary bg base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a bg.
 *
 * All of the functions listed on @ref Bg namespace will work for
 * objects deriving from #Elm_Bg_Smart_Class.
 */
typedef struct _Elm_Bg_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Bg_Smart_Class;

/**
 * Base layout smart data extended with bg instance data.
 */
typedef struct _Elm_Bg_Smart_Data Elm_Bg_Smart_Data;
struct _Elm_Bg_Smart_Data
{
   Elm_Layout_Smart_Data base;

/* the basic background's edje object has three swallow spots, namely:
 *  - "elm.swallow.rectangle" (elm_bg_color_set),
 *  - "elm.swallow.background" (elm_bg_file_set) and
 *  - "elm.swallow.content" (elm_bg_overlay_set).
 * the following three variables hold possible content to fit in each
 * of them, respectively. */

   Evas_Object          *rect, *img;
   const char           *file, *group;  /* path to file and group name
                                         * to give life to "img" */
   Elm_Bg_Option         option;
   struct
     {
        Evas_Coord w, h;
     } load_opts;
};

/**
 * @}
 */

EAPI extern const char ELM_BG_SMART_NAME[];
EAPI const Elm_Bg_Smart_Class *elm_bg_smart_class_get(void);

#define ELM_BG_DATA_GET(o, sd) \
  Elm_Bg_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_BG_DATA_GET_OR_RETURN(o, ptr)            \
  ELM_BG_DATA_GET(o, ptr);                           \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_BG_DATA_GET_OR_RETURN_VAL(o, ptr, val)   \
  ELM_BG_DATA_GET(o, ptr);                           \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_BG_CHECK(obj)                                      \
  if (!obj || !elm_widget_type_check((obj), ELM_BG_SMART_NAME, \
                                     __func__))                \
    return

#endif
