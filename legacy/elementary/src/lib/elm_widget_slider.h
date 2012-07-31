#ifndef ELM_WIDGET_SLIDER_H
#define ELM_WIDGET_SLIDER_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-slider-class The Elementary Slider Class
 *
 * Elementary, besides having the @ref Slider widget, exposes its
 * foundation -- the Elementary Slider Class -- in order to create other
 * widgets which are a slider with some more logic on top.
 */

/**
 * @def ELM_SLIDER_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Slider_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_SLIDER_CLASS(x) ((Elm_Slider_Smart_Class *)x)

/**
 * @def ELM_SLIDER_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Slider_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_SLIDER_DATA(x)  ((Elm_Slider_Smart_Data *)x)

/**
 * @def ELM_SLIDER_SMART_CLASS_VERSION
 *
 * Current version for Elementary slider @b base smart class, a value
 * which goes to _Elm_Slider_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_SLIDER_SMART_CLASS_VERSION 1

/**
 * @def ELM_SLIDER_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Slider_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_SLIDER_SMART_CLASS_INIT_NULL
 * @see ELM_SLIDER_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_SLIDER_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_SLIDER_SMART_CLASS_VERSION}

/**
 * @def ELM_SLIDER_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Slider_Smart_Class structure.
 *
 * @see ELM_SLIDER_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_SLIDER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_SLIDER_SMART_CLASS_INIT_NULL \
  ELM_SLIDER_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_SLIDER_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Slider_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_SLIDER_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Slider_Smart_Class (base field)
 * to the latest #ELM_SLIDER_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_SLIDER_SMART_CLASS_INIT_NULL
 * @see ELM_SLIDER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_SLIDER_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_SLIDER_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary slider base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a slider.
 *
 * All of the functions listed on @ref Slider namespace will work for
 * objects deriving from #Elm_Slider_Smart_Class.
 */
typedef struct _Elm_Slider_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Slider_Smart_Class;

/**
 * Base layout smart data extended with slider instance data.
 */
typedef struct _Elm_Slider_Smart_Data Elm_Slider_Smart_Data;
struct _Elm_Slider_Smart_Data
{
   Elm_Layout_Smart_Data base;

   Evas_Object          *spacer, *popup, *track;
   Ecore_Timer          *delay;

   const char           *units;
   const char           *indicator;

   char                 *(*indicator_format_func)(double val);
   void                  (*indicator_format_free)(char *str);

   char                 *(*units_format_func)(double val);
   void                  (*units_format_free)(char *str);

   double                val, val_min, val_max, val2;
   Evas_Coord            size;
   Evas_Coord            downx, downy;

   Eina_Bool             horizontal : 1;
   Eina_Bool             inverted : 1;
   Eina_Bool             indicator_show : 1;
   Eina_Bool             spacer_down : 1;
   Eina_Bool             frozen : 1;
   Eina_Bool             popup_hiding : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_SLIDER_SMART_NAME[];
EAPI const Elm_Slider_Smart_Class *elm_slider_smart_class_get(void);

#define ELM_SLIDER_DATA_GET(o, sd) \
  Elm_Slider_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_SLIDER_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_SLIDER_DATA_GET(o, ptr);                       \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_SLIDER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SLIDER_DATA_GET(o, ptr);                         \
  if (!ptr)                                            \
    {                                                  \
       CRITICAL("No widget data for object %p (%s)",   \
                o, evas_object_type_get(o));           \
       return val;                                     \
    }

#define ELM_SLIDER_CHECK(obj)                                      \
  if (!obj || !elm_widget_type_check((obj), ELM_SLIDER_SMART_NAME, \
                                     __func__))                    \
    return

#endif
