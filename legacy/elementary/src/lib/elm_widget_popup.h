#ifndef ELM_WIDGET_POPUP_H
#define ELM_WIDGET_POPUP_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-popup-class The Elementary Popup Class
 *
 * Elementary, besides having the @ref Popup widget, exposes its
 * foundation -- the Elementary Popup Class -- in order to create other
 * widgets which are a popup with some more logic on top.
 */

/**
 * @def ELM_POPUP_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Popup_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_POPUP_CLASS(x) ((Elm_Popup_Smart_Class *)x)

/**
 * @def ELM_POPUP_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Popup_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_POPUP_DATA(x)  ((Elm_Popup_Smart_Data *)x)

/**
 * @def ELM_POPUP_SMART_CLASS_VERSION
 *
 * Current version for Elementary popup @b base smart class, a value
 * which goes to _Elm_Popup_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_POPUP_SMART_CLASS_VERSION 1

/**
 * @def ELM_POPUP_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Popup_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_POPUP_SMART_CLASS_INIT_NULL
 * @see ELM_POPUP_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_POPUP_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_POPUP_SMART_CLASS_VERSION}

/**
 * @def ELM_POPUP_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Popup_Smart_Class structure.
 *
 * @see ELM_POPUP_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_POPUP_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_POPUP_SMART_CLASS_INIT_NULL \
  ELM_POPUP_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_POPUP_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Popup_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_POPUP_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Popup_Smart_Class (base field)
 * to the latest #ELM_POPUP_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_POPUP_SMART_CLASS_INIT_NULL
 * @see ELM_POPUP_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_POPUP_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_POPUP_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary popup base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a popup.
 *
 * All of the functions listed on @ref Popup namespace will work for
 * objects deriving from #Elm_Popup_Smart_Class.
 */
typedef struct _Elm_Popup_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Popup_Smart_Class;

#define ELM_POPUP_ACTION_BUTTON_MAX 3

typedef struct _Action_Area_Data Action_Area_Data;

/**
 * Base layout smart data extended with popup instance data.
 */
typedef struct _Elm_Popup_Smart_Data Elm_Popup_Smart_Data;
struct _Elm_Popup_Smart_Data
{
   Elm_Layout_Smart_Data base;

   Evas_Object          *notify;
   Evas_Object          *title_icon;
   Evas_Object          *title_access_obj;
   Evas_Object          *content_area;
   Evas_Object          *text_content_obj;
   Evas_Object          *action_area;
   Evas_Object          *box;
   Evas_Object          *tbl;
   Evas_Object          *spacer;
   Evas_Object          *scr;
   Evas_Object          *content;
   Eina_List            *items;
   const char           *title_text;
   Action_Area_Data     *buttons[ELM_POPUP_ACTION_BUTTON_MAX];
   Elm_Wrap_Type         content_text_wrap_type;
   unsigned int          button_count;
   Evas_Coord            max_sc_w;
   Evas_Coord            max_sc_h;

   Eina_Bool             visible : 1;
   Eina_Bool             no_shift : 1;
   Eina_Bool             scr_size_recalc : 1;
};

typedef struct _Elm_Popup_Item Elm_Popup_Item;
struct _Elm_Popup_Item
{
   Elm_Widget_Item base;

   const char     *label;
   Evas_Object    *icon;
   Evas_Smart_Cb   func;
   Eina_Bool       disabled : 1;
};

struct _Action_Area_Data
{
   Evas_Object *obj;
   Evas_Object *btn;
   Eina_Bool    delete_me;
};

/**
 * @}
 */

EAPI extern const char ELM_POPUP_SMART_NAME[];
EAPI const Elm_Popup_Smart_Class *elm_popup_smart_class_get(void);

#define ELM_POPUP_DATA_GET(o, sd) \
  Elm_Popup_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_POPUP_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_POPUP_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_POPUP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_POPUP_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_POPUP_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check             \
        ((obj), ELM_POPUP_SMART_NAME, __func__)) \
    return

#define ELM_POPUP_ITEM_CHECK(it)                            \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_POPUP_CHECK(it->base.widget);

#define ELM_POPUP_ITEM_CHECK_OR_RETURN(it, ...)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_POPUP_CHECK(it->base.widget) __VA_ARGS__;

#endif
