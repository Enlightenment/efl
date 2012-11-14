#ifndef ELM_WIDGET_TOOLBAR_H
#define ELM_WIDGET_TOOLBAR_H

#include "elm_interface_scrollable.h"
#include "els_box.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-toolbar-class The Elementary Toolbar Class
 *
 * Elementary, besides having the @ref Toolbar widget, exposes its
 * foundation -- the Elementary Toolbar Class -- in order to create other
 * widgets which are a toolbar with some more logic on top.
 */

/**
 * @def ELM_TOOLBAR_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Toolbar_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_TOOLBAR_CLASS(x) ((Elm_Toolbar_Smart_Class *)x)

/**
 * @def ELM_TOOLBAR_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Toolbar_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_TOOLBAR_DATA(x)  ((Elm_Toolbar_Smart_Data *)x)

/**
 * @def ELM_TOOLBAR_SMART_CLASS_VERSION
 *
 * Current version for Elementary toolbar @b base smart class, a value
 * which goes to _Elm_Toolbar_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_TOOLBAR_SMART_CLASS_VERSION 1

/**
 * @def ELM_TOOLBAR_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Toolbar_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_TOOLBAR_SMART_CLASS_INIT_NULL
 * @see ELM_TOOLBAR_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_TOOLBAR_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_TOOLBAR_SMART_CLASS_VERSION}

/**
 * @def ELM_TOOLBAR_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Toolbar_Smart_Class structure.
 *
 * @see ELM_TOOLBAR_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_TOOLBAR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_TOOLBAR_SMART_CLASS_INIT_NULL \
  ELM_TOOLBAR_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_TOOLBAR_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Toolbar_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_TOOLBAR_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Toolbar_Smart_Class (base field)
 * to the latest #ELM_TOOLBAR_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_TOOLBAR_SMART_CLASS_INIT_NULL
 * @see ELM_TOOLBAR_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_TOOLBAR_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_TOOLBAR_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary toolbar base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a toolbar.
 *
 * All of the functions listed on @ref Toolbar namespace will work for
 * objects deriving from #Elm_Toolbar_Smart_Class.
 */
typedef struct _Elm_Toolbar_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Toolbar_Smart_Class;

typedef struct _Elm_Toolbar_Item Elm_Toolbar_Item;

/**
 * Base widget smart data extended with toolbar instance data.
 */
typedef struct _Elm_Toolbar_Smart_Data Elm_Toolbar_Smart_Data;
struct _Elm_Toolbar_Smart_Data
{
   Elm_Widget_Smart_Data                 base; /* base widget smart data as
                                                * first member obligatory, as
                                                * we're inheriting from it */

   Evas_Object                          *hit_rect;
   const Elm_Scrollable_Smart_Interface *s_iface;

   Evas_Object                          *bx, *more, *bx_more, *bx_more2;
   Evas_Object                          *menu_parent;
   Eina_Inlist                          *items;
   Elm_Toolbar_Item                     *more_item, *selected_item;
   Elm_Toolbar_Item                     *reorder_from, *reorder_to;
   Elm_Toolbar_Shrink_Mode               shrink_mode;
   Elm_Icon_Lookup_Order                 lookup_order;
   int                                   theme_icon_size, priv_icon_size,
                                         icon_size;
   int                                   standard_priority;
   unsigned int                          item_count;
   unsigned int                          separator_count;
   double                                align;
   Elm_Object_Select_Mode                select_mode;
   Ecore_Timer                          *long_timer;
   Ecore_Job                            *resize_job;

   Eina_Bool                             vertical : 1;
   Eina_Bool                             long_press : 1;
   Eina_Bool                             homogeneous : 1;
   Eina_Bool                             on_deletion : 1;
   Eina_Bool                             reorder_mode : 1;
   Eina_Bool                             transverse_expanded : 1;
};

struct _Elm_Toolbar_Item
{
   ELM_WIDGET_ITEM;
   EINA_INLIST;

   const char   *label;
   const char   *icon_str;
   Evas_Object  *icon;
   Evas_Object  *object;
   Evas_Object  *o_menu;
   Evas_Object  *in_box;
   Evas_Smart_Cb func;
   struct
   {
      int       priority;
      Eina_Bool visible : 1;
   } prio;

   Eina_List    *states;
   Eina_List    *current_state;

   Eina_Bool     separator : 1;
   Eina_Bool     selected : 1;
   Eina_Bool     menu : 1;
};

struct _Elm_Toolbar_Item_State
{
   const char   *label;
   const char   *icon_str;
   Evas_Object  *icon;
   Evas_Smart_Cb func;
   const void   *data;
};

/**
 * @}
 */

EAPI extern const char ELM_TOOLBAR_SMART_NAME[];
EAPI const Elm_Toolbar_Smart_Class
*elm_toolbar_smart_class_get(void);

#define ELM_TOOLBAR_DATA_GET(o, sd) \
  Elm_Toolbar_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_TOOLBAR_DATA_GET_OR_RETURN(o, ptr)       \
  ELM_TOOLBAR_DATA_GET(o, ptr);                      \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_TOOLBAR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_TOOLBAR_DATA_GET(o, ptr);                         \
  if (!ptr)                                             \
    {                                                   \
       CRITICAL("No widget data for object %p (%s)",    \
                o, evas_object_type_get(o));            \
       return val;                                      \
    }

#define ELM_TOOLBAR_CHECK(obj)                                          \
  if (!obj || !elm_widget_type_check((obj),                             \
                                     ELM_TOOLBAR_SMART_NAME, __func__)) \
    return

#define ELM_TOOLBAR_ITEM_CHECK(it)                          \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_TOOLBAR_CHECK(it->base.widget);

#define ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, ...)                      \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_TOOLBAR_CHECK(it->base.widget) __VA_ARGS__;

#define ELM_TOOLBAR_ITEM_CHECK_OR_GOTO(it, label)              \
  ELM_WIDGET_ITEM_CHECK_OR_GOTO((Elm_Widget_Item *)it, label); \
  if (!it->base.widget || !elm_widget_type_check               \
        ((it->base.widget), ELM_TOOLBAR_SMART_NAME, __func__)) goto label;

#endif
