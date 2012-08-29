#ifndef ELM_WIDGET_NAVIFRAME_H
#define ELM_WIDGET_NAVIFRAME_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-naviframe-class The Elementary Naviframe Class
 *
 * Elementary, besides having the @ref Naviframe widget, exposes its
 * foundation -- the Elementary Naviframe Class -- in order to create other
 * widgets which are a naviframe with some more logic on top.
 */

/**
 * @def ELM_NAVIFRAME_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Naviframe_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_NAVIFRAME_CLASS(x) ((Elm_Naviframe_Smart_Class *)x)

/**
 * @def ELM_NAVIFRAME_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Naviframe_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_NAVIFRAME_DATA(x)  ((Elm_Naviframe_Smart_Data *)x)

/**
 * @def ELM_NAVIFRAME_SMART_CLASS_VERSION
 *
 * Current version for Elementary naviframe @b base smart class, a value
 * which goes to _Elm_Naviframe_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_NAVIFRAME_SMART_CLASS_VERSION 1

/**
 * @def ELM_NAVIFRAME_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Naviframe_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_NAVIFRAME_SMART_CLASS_INIT_NULL
 * @see ELM_NAVIFRAME_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_NAVIFRAME_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_NAVIFRAME_SMART_CLASS_VERSION}

/**
 * @def ELM_NAVIFRAME_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Naviframe_Smart_Class structure.
 *
 * @see ELM_NAVIFRAME_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_NAVIFRAME_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_NAVIFRAME_SMART_CLASS_INIT_NULL \
  ELM_NAVIFRAME_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_NAVIFRAME_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Naviframe_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_NAVIFRAME_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Naviframe_Smart_Class (base field)
 * to the latest #ELM_NAVIFRAME_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_NAVIFRAME_SMART_CLASS_INIT_NULL
 * @see ELM_NAVIFRAME_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_NAVIFRAME_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_NAVIFRAME_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary naviframe base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a naviframe.
 *
 * All of the functions listed on @ref Naviframe namespace will work for
 * objects deriving from #Elm_Naviframe_Smart_Class.
 */
typedef struct _Elm_Naviframe_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Naviframe_Smart_Class;

/**
 * Base layout smart data extended with naviframe instance data.
 */
typedef struct _Elm_Naviframe_Smart_Data Elm_Naviframe_Smart_Data;
struct _Elm_Naviframe_Smart_Data
{
   Elm_Layout_Smart_Data base;

   Eina_Inlist          *stack; /* top item is the list's LAST item */
   Evas_Object          *dummy_edje;

   Eina_Bool             preserve : 1;
   Eina_Bool             on_deletion : 1;
   Eina_Bool             auto_pushed : 1;
   Eina_Bool             freeze_events : 1;
};

typedef struct _Elm_Naviframe_Item Elm_Naviframe_Item;
struct _Elm_Naviframe_Item
{
   ELM_WIDGET_ITEM;
   EINA_INLIST;

   Eina_Inlist *content_list;
   Eina_Inlist *text_list;
   Evas_Object *content;
   Evas_Object *title_prev_btn;
   Evas_Object *title_next_btn;
   Evas_Object *title_icon;
   Evas_Object *title;
   Evas_Object *subtitle;
   const char  *style;
   const char  *title_label;
   const char  *subtitle_label;

   Evas_Coord   minw;
   Evas_Coord   minh;

   Eina_Bool    title_visible : 1;
   Eina_Bool    content_unfocusable : 1;
};

typedef struct _Elm_Naviframe_Content_Item_Pair Elm_Naviframe_Content_Item_Pair;
struct _Elm_Naviframe_Content_Item_Pair
{
   EINA_INLIST;
   const char *part;
   Elm_Naviframe_Item *it;
};

typedef struct _Elm_Naviframe_Text_Item_Pair Elm_Naviframe_Text_Item_Pair;
struct _Elm_Naviframe_Text_Item_Pair
{
   EINA_INLIST;
   const char *part;
   Evas_Object *access_object;
};

/**
 * @}
 */

EAPI extern const char ELM_NAVIFRAME_SMART_NAME[];
EAPI const Elm_Naviframe_Smart_Class *elm_naviframe_smart_class_get(void);

#define ELM_NAVIFRAME_DATA_GET(o, sd) \
  Elm_Naviframe_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_NAVIFRAME_DATA_GET_OR_RETURN(o, ptr)     \
  ELM_NAVIFRAME_DATA_GET(o, ptr);                    \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_NAVIFRAME_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_NAVIFRAME_DATA_GET(o, ptr);                         \
  if (!ptr)                                               \
    {                                                     \
       CRITICAL("No widget data for object %p (%s)",      \
                o, evas_object_type_get(o));              \
       return val;                                        \
    }

#define ELM_NAVIFRAME_CHECK(obj)                                      \
  if (!obj || !elm_widget_type_check((obj), ELM_NAVIFRAME_SMART_NAME, \
                                     __func__))                       \
    return

#define ELM_NAVIFRAME_ITEM_CHECK(it)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_NAVIFRAME_CHECK(it->base.widget);

#define ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it, ...)                    \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_NAVIFRAME_CHECK(it->base.widget) __VA_ARGS__;

#endif
