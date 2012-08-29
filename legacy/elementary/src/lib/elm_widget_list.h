#ifndef ELM_WIDGET_LIST_H
#define ELM_WIDGET_LIST_H

#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-list-class The Elementary List Class
 *
 * Elementary, besides having the @ref List widget, exposes its
 * foundation -- the Elementary List Class -- in order to create
 * other widgets which are a list with some more logic on top.
 */

/**
 * @def ELM_LIST_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_List_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_LIST_CLASS(x) ((Elm_List_Smart_Class *)x)

/**
 * @def ELM_LIST_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_List_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_LIST_DATA(x)  ((Elm_List_Smart_Data *)x)

/**
 * @def ELM_LIST_SMART_CLASS_VERSION
 *
 * Current version for Elementary list @b base smart class, a value
 * which goes to _Elm_List_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_LIST_SMART_CLASS_VERSION 1

/**
 * @def ELM_LIST_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_List_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_LIST_SMART_CLASS_INIT_NULL
 * @see ELM_LIST_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_LIST_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_LIST_SMART_CLASS_VERSION}

/**
 * @def ELM_LIST_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_List_Smart_Class structure.
 *
 * @see ELM_LIST_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_LIST_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_LIST_SMART_CLASS_INIT_NULL \
  ELM_LIST_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_LIST_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_List_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_LIST_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_List_Smart_Class (base field)
 * to the latest #ELM_LIST_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_LIST_SMART_CLASS_INIT_NULL
 * @see ELM_LIST_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_LIST_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_LIST_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary list base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a list.
 *
 * All of the functions listed on @ref List namespace will work for
 * objects deriving from #Elm_List_Smart_Class.
 */
typedef struct _Elm_List_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_List_Smart_Class;

#define ELM_LIST_SWIPE_MOVES 12

/**
 * Base widget smart data extended with list instance data.
 */
typedef struct _Elm_List_Smart_Data Elm_List_Smart_Data;
struct _Elm_List_Smart_Data
{
   Elm_Layout_Smart_Data                 base; /* base widget smart data as
                                                * first member obligatory, as
                                                * we're inheriting from it */

   Evas_Object                          *box, *hit_rect;
   const Elm_Scrollable_Smart_Interface *s_iface;

   Eina_List                            *items, *selected, *to_delete;
   Elm_Object_Item                      *last_selected_item;
   Evas_Coord                            minw[2], minh[2];
   Elm_Object_Select_Mode                select_mode;
   int                                   movements;
   int                                   walking;
   Elm_List_Mode                         h_mode;
   Elm_List_Mode                         mode;

   struct
   {
      Evas_Coord x, y;
   } history[ELM_LIST_SWIPE_MOVES];

   Eina_Bool                             was_selected : 1;
   Eina_Bool                             fix_pending : 1;
   Eina_Bool                             longpressed : 1;
   Eina_Bool                             scr_minw : 1;
   Eina_Bool                             scr_minh : 1;
   Eina_Bool                             on_hold : 1;
   Eina_Bool                             multi : 1;
   Eina_Bool                             swipe : 1;
   Eina_Bool                             delete_me : 1;
};

typedef struct _Elm_List_Item Elm_List_Item;
struct _Elm_List_Item
{
   ELM_WIDGET_ITEM;

   Elm_List_Smart_Data *sd;

   Ecore_Timer         *swipe_timer;
   Ecore_Timer         *long_timer;
   Evas_Object         *icon, *end;
   Evas_Smart_Cb        func;

   const char          *label;
   Eina_List           *node;

   Eina_Bool            is_separator : 1;
   Eina_Bool            highlighted : 1;
   Eina_Bool            dummy_icon : 1;
   Eina_Bool            dummy_end : 1;
   Eina_Bool            selected : 1;
   Eina_Bool            deleted : 1;
   Eina_Bool            is_even : 1;
   Eina_Bool            fixed : 1;
   Eina_Bool            even : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_LIST_SMART_NAME[];
EAPI const Elm_List_Smart_Class *elm_list_smart_class_get(void);

#define ELM_LIST_DATA_GET(o, sd) \
  Elm_List_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_LIST_DATA_GET_OR_RETURN(o, ptr)          \
  ELM_LIST_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_LIST_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_LIST_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_LIST_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_LIST_SMART_NAME, __func__)) \
    return

#define ELM_LIST_ITEM_CHECK(it)                             \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_LIST_CHECK(it->base.widget);                          \
  if (((Elm_List_Item *)it)->deleted)                       \
    {                                                       \
       ERR("ERROR: " #it " has been DELETED.\n");           \
       return;                                              \
    }

#define ELM_LIST_ITEM_CHECK_OR_RETURN(it, ...)                         \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_LIST_CHECK(it->base.widget) __VA_ARGS__;                         \
  if (((Elm_List_Item *)it)->deleted)                                  \
    {                                                                  \
       ERR("ERROR: " #it " has been DELETED.\n");                      \
       return __VA_ARGS__;                                             \
    }

#endif
