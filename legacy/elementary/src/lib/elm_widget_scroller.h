#ifndef ELM_WIDGET_SCROLLER_H
#define ELM_WIDGET_SCROLLER_H

#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-scroller-class The Elementary Scroller Class
 *
 * Elementary, besides having the @ref Scroller widget, exposes its
 * foundation -- the Elementary Scroller Class -- in order to create
 * other widgets which are a scroller with some more logic on top.
 */

/**
 * @def ELM_SCROLLER_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Scroller_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_SCROLLER_CLASS(x) ((Elm_Scroller_Smart_Class *)x)

/**
 * @def ELM_SCROLLER_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Scroller_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_SCROLLER_DATA(x)  ((Elm_Scroller_Smart_Data *)x)

/**
 * @def ELM_SCROLLER_SMART_CLASS_VERSION
 *
 * Current version for Elementary scroller @b base smart class, a value
 * which goes to _Elm_Scroller_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_SCROLLER_SMART_CLASS_VERSION 1

/**
 * @def ELM_SCROLLER_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Scroller_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_SCROLLER_SMART_CLASS_INIT_NULL
 * @see ELM_SCROLLER_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_SCROLLER_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_SCROLLER_SMART_CLASS_VERSION}

/**
 * @def ELM_SCROLLER_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Scroller_Smart_Class structure.
 *
 * @see ELM_SCROLLER_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_SCROLLER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_SCROLLER_SMART_CLASS_INIT_NULL \
  ELM_SCROLLER_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_SCROLLER_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Scroller_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_SCROLLER_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Scroller_Smart_Class (base field)
 * to the latest #ELM_SCROLLER_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_SCROLLER_SMART_CLASS_INIT_NULL
 * @see ELM_SCROLLER_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_SCROLLER_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_SCROLLER_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary scroller base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a scroller.
 *
 * All of the functions listed on @ref Scroller namespace will work for
 * objects deriving from #Elm_Scroller_Smart_Class.
 */
typedef struct _Elm_Scroller_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Scroller_Smart_Class;

/**
 * Base widget smart data extended with scroller instance data.
 */
typedef struct _Elm_Scroller_Smart_Data Elm_Scroller_Smart_Data;
struct _Elm_Scroller_Smart_Data
{
   Elm_Layout_Smart_Data                 base; /* base widget smart data as
                                                * first member obligatory, as
                                                * we're inheriting from it */

   const Elm_Scrollable_Smart_Interface *s_iface;

   Evas_Object                          *hit_rect;
   Evas_Object                          *g_layer;

   Evas_Object                          *content;

   Eina_Bool                             min_w : 1;
   Eina_Bool                             min_h : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_SCROLLER_SMART_NAME[];
EAPI const Elm_Scroller_Smart_Class *elm_scroller_smart_class_get(void);

#define ELM_SCROLLER_DATA_GET(o, sd) \
  Elm_Scroller_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_SCROLLER_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_SCROLLER_DATA_GET(o, ptr);                     \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_SCROLLER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SCROLLER_DATA_GET(o, ptr);                         \
  if (!ptr)                                              \
    {                                                    \
       CRITICAL("No widget data for object %p (%s)",     \
                o, evas_object_type_get(o));             \
       return val;                                       \
    }

#define ELM_SCROLLER_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                \
        ((obj), ELM_SCROLLER_SMART_NAME, __func__)) \
    return

#endif
