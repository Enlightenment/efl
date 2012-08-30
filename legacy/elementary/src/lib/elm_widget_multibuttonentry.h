#ifndef ELM_WIDGET_MULTIBUTTONENTRY_H
#define ELM_WIDGET_MULTIBUTTONENTRY_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-multibuttonentry-class The Elementary Multi Button Entry Class
 *
 * Elementary, besides having the @ref Multibuttonentry widget,
 * exposes its foundation -- the Elementary Multi Button Entry Class --
 * in order to create other widgets which are a multi button entry with
 * some more logic on top.
 */

/**
 * @def ELM_MULTIBUTTONENTRY_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Multibuttonentry_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_MULTIBUTTONENTRY_CLASS(x) ((Elm_Multibuttonentry_Smart_Class *)x)

/**
 * @def ELM_MULTIBUTTONENTRY_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Multibuttonentry_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_MULTIBUTTONENTRY_DATA(x)  ((Elm_Multibuttonentry_Smart_Data *)x)

/**
 * @def ELM_MULTIBUTTONENTRY_SMART_CLASS_VERSION
 *
 * Current version for Elementary multibuttonentry @b base smart class, a value
 * which goes to _Elm_Multibuttonentry_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_MULTIBUTTONENTRY_SMART_CLASS_VERSION 1

/**
 * @def ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Multibuttonentry_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NULL
 * @see ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_MULTIBUTTONENTRY_SMART_CLASS_VERSION}

/**
 * @def ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Multibuttonentry_Smart_Class structure.
 *
 * @see ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NULL \
  ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Multibuttonentry_Smart_Class
 * structure and set its name and version.
 *
 * This is similar to #ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NULL, but
 * it will also set the version field of
 * #Elm_Multibuttonentry_Smart_Class (base field) to the latest
 * #ELM_MULTIBUTTONENTRY_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NULL
 * @see ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary multibuttonentry base smart class. This inherits
 * directly from #Elm_Layout_Smart_Class and is meant to build widgets
 * extending the behavior of a multibuttonentry.
 *
 * All of the functions listed on @ref Multibuttonentry namespace will
 * work for objects deriving from #Elm_Multibuttonentry_Smart_Class.
 */
typedef struct _Elm_Multibuttonentry_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Multibuttonentry_Smart_Class;

/**
 * Base widget smart data extended with multibuttonentry instance data.
 */

typedef enum _Multibuttonentry_Pos
{
   MULTIBUTTONENTRY_POS_START,
   MULTIBUTTONENTRY_POS_END,
   MULTIBUTTONENTRY_POS_BEFORE,
   MULTIBUTTONENTRY_POS_AFTER,
} Multibuttonentry_Pos;

typedef enum _Multibuttonentry_Button_State
{
   MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT,
   MULTIBUTTONENTRY_BUTTON_STATE_SELECTED,
} Multibuttonentry_Button_State;

typedef enum _MultiButtonEntry_Closed_Button_Type
{
   MULTIBUTTONENTRY_CLOSED_IMAGE,
   MULTIBUTTONENTRY_CLOSED_LABEL
} MultiButtonEntry_Closed_Button_Type;

typedef enum _Multibuttonentry_View_State
{
   MULTIBUTTONENTRY_VIEW_NONE,
   MULTIBUTTONENTRY_VIEW_GUIDETEXT,
   MULTIBUTTONENTRY_VIEW_ENTRY,
   MULTIBUTTONENTRY_VIEW_SHRINK
} Multibuttonentry_View_State;

typedef struct _Multibuttonentry_Item Elm_Multibuttonentry_Item;

struct _Multibuttonentry_Item
{
   ELM_WIDGET_ITEM;

   Evas_Object  *button;
   Evas_Coord    vw, rw; // vw: visual width, real width
   Eina_Bool     visible : 1;
   Evas_Smart_Cb func;
};

typedef struct _Elm_Multibuttonentry_Item_Filter
{
   Elm_Multibuttonentry_Item_Filter_Cb callback_func;
   void                               *data;
} Elm_Multibuttonentry_Item_Filter;

typedef struct _Elm_Multibuttonentry_Smart_Data
  Elm_Multibuttonentry_Smart_Data;
struct _Elm_Multibuttonentry_Smart_Data
{
   Elm_Layout_Smart_Data               base;

   Evas_Object                        *box;
   Evas_Object                        *entry;
   Evas_Object                        *label;
   Evas_Object                        *guide_text;
   Evas_Object                        *end; /* used to represent the
                                             * total number of
                                             * invisible buttons */

   Evas_Object                        *rect_for_end;

   Eina_List                          *items;
   Eina_List                          *filter_list;
   Elm_Object_Item                    *selected_it; /* selected item */

   const char                         *label_str, *guide_text_str;

   MultiButtonEntry_Closed_Button_Type end_type;

   int                                 n_str;
   Multibuttonentry_View_State         view_state;

   Evas_Coord                          w_box, h_box;
   int                                 shrink;

   Elm_Multibuttonentry_Item_Filter_Cb add_callback;
   void                               *add_callback_data;

   Eina_Bool                           last_btn_select : 1;
   Eina_Bool                           editable : 1;
   Eina_Bool                           focused : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_MULTIBUTTONENTRY_SMART_NAME[];
EAPI const Elm_Multibuttonentry_Smart_Class
*elm_multibuttonentry_smart_class_get(void);

#define ELM_MULTIBUTTONENTRY_DATA_GET(o, sd) \
  Elm_Multibuttonentry_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_MULTIBUTTONENTRY_DATA_GET_OR_RETURN(o, ptr) \
  ELM_MULTIBUTTONENTRY_DATA_GET(o, ptr);                \
  if (!ptr)                                             \
    {                                                   \
       CRITICAL("No widget data for object %p (%s)",    \
                o, evas_object_type_get(o));            \
       return;                                          \
    }

#define ELM_MULTIBUTTONENTRY_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_MULTIBUTTONENTRY_DATA_GET(o, ptr);                         \
  if (!ptr)                                                      \
    {                                                            \
       CRITICAL("No widget data for object %p (%s)",             \
                o, evas_object_type_get(o));                     \
       return val;                                               \
    }

#define ELM_MULTIBUTTONENTRY_CHECK(obj)             \
  if (!obj || !elm_widget_type_check                \
        ((obj),                                     \
        ELM_MULTIBUTTONENTRY_SMART_NAME, __func__)) \
    return

#define ELM_MULTIBUTTONENTRY_ITEM_CHECK(it)                 \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_MULTIBUTTONENTRY_CHECK(it->base.widget);

#define ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it, ...)             \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_MULTIBUTTONENTRY_CHECK(it->base.widget) __VA_ARGS__;

#endif
