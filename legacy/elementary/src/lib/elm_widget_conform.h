#ifndef ELM_WIDGET_CONFORMANT_H
#define ELM_WIDGET_CONFORMANT_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-conformant-class The Elementary Conformant Class
 *
 * Elementary, besides having the @ref Conformant widget, exposes its
 * foundation -- the Elementary Conformant Class -- in order to create other
 * widgets which are a conformant with some more logic on top.
 */

/**
 * @def ELM_CONFORMANT_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Conformant_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_CONFORMANT_CLASS(x) ((Elm_Conformant_Smart_Class *)x)

/**
 * @def ELM_CONFORMANT_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Conformant_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_CONFORMANT_DATA(x)  ((Elm_Conformant_Smart_Data *)x)

/**
 * @def ELM_CONFORMANT_SMART_CLASS_VERSION
 *
 * Current version for Elementary conformant @b base smart class, a value
 * which goes to _Elm_Conformant_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_CONFORMANT_SMART_CLASS_VERSION 1

/**
 * @def ELM_CONFORMANT_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Conformant_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_CONFORMANT_SMART_CLASS_INIT_NULL
 * @see ELM_CONFORMANT_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_CONFORMANT_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_CONFORMANT_SMART_CLASS_VERSION}

/**
 * @def ELM_CONFORMANT_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Conformant_Smart_Class structure.
 *
 * @see ELM_CONFORMANT_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_CONFORMANT_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_CONFORMANT_SMART_CLASS_INIT_NULL \
  ELM_CONFORMANT_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_CONFORMANT_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Conformant_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_CONFORMANT_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Conformant_Smart_Class (base field)
 * to the latest #ELM_CONFORMANT_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_CONFORMANT_SMART_CLASS_INIT_NULL
 * @see ELM_CONFORMANT_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_CONFORMANT_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_CONFORMANT_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary conformant base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a conformant.
 *
 * All of the functions listed on @ref Conformant namespace will work for
 * objects deriving from #Elm_Conformant_Smart_Class.
 */
typedef struct _Elm_Conformant_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Conformant_Smart_Class;

/**
 * Base layout smart data extended with conformant instance data.
 */
typedef struct _Elm_Conformant_Smart_Data Elm_Conformant_Smart_Data;
struct _Elm_Conformant_Smart_Data
{
   Elm_Layout_Smart_Data          base;

   Evas_Object                   *indicator;
   Evas_Object                   *softkey;
   Evas_Object                   *virtualkeypad;
   Evas_Object                   *clipboard;
   Evas_Object                   *scroller;
#ifdef HAVE_ELEMENTARY_X
   Ecore_Event_Handler           *prop_hdl;
   Ecore_X_Virtual_Keyboard_State vkb_state;
#endif
   struct
   {
      Ecore_Animator *animator; // animaton timer
      double          start; // time started
      Evas_Coord      auto_x, auto_y; // desired delta
      Evas_Coord      x, y; // current delta
   } delta;
   Ecore_Job                     *show_region_job;
};

/* Enum to identify conformant swallow parts */
typedef enum _Conformant_Part_Type Conformant_Part_Type;
enum _Conformant_Part_Type
{
   ELM_CONFORMANT_INDICATOR_PART      = 1,
   ELM_CONFORMANT_SOFTKEY_PART        = 2,
   ELM_CONFORMANT_VIRTUAL_KEYPAD_PART = 4,
   ELM_CONFORMANT_CLIPBOARD_PART    = 8
};

/**
 * @}
 */

EAPI extern const char ELM_CONFORMANT_SMART_NAME[];
EAPI const Elm_Conformant_Smart_Class *elm_conformant_smart_class_get(void);

#define ELM_CONFORMANT_DATA_GET(o, sd) \
  Elm_Conformant_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_CONFORMANT_DATA_GET_OR_RETURN(o, ptr)    \
  ELM_CONFORMANT_DATA_GET(o, ptr);                   \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_CONFORMANT_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_CONFORMANT_DATA_GET(o, ptr);                         \
  if (!ptr)                                                \
    {                                                      \
       CRITICAL("No widget data for object %p (%s)",       \
                o, evas_object_type_get(o));               \
       return val;                                         \
    }

#define ELM_CONFORMANT_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                  \
        ((obj), ELM_CONFORMANT_SMART_NAME, __func__)) \
    return

#endif
