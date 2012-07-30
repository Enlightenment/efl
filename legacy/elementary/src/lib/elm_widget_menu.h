#ifndef ELM_WIDGET_MENU_H
#define ELM_WIDGET_MENU_H

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-menu-class The Elementary Menu Class
 *
 * Elementary, besides having the @ref Menu widget, exposes its
 * foundation -- the Elementary Menu Class -- in order to create
 * other widgets which are a menu with some more logic on top.
 */

/**
 * @def ELM_MENU_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Menu_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_MENU_CLASS(x) ((Elm_Menu_Smart_Class *) x)

/**
 * @def ELM_MENU_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Menu_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_MENU_DATA(x) ((Elm_Menu_Smart_Data *) x)

/**
 * @def ELM_MENU_SMART_CLASS_VERSION
 *
 * Current version for Elementary menu @b base smart class, a value
 * which goes to _Elm_Menu_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_MENU_SMART_CLASS_VERSION 1

/**
 * @def ELM_MENU_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Menu_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_MENU_SMART_CLASS_INIT_NULL
 * @see ELM_MENU_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_MENU_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_MENU_SMART_CLASS_VERSION}

/**
 * @def ELM_MENU_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Menu_Smart_Class structure.
 *
 * @see ELM_MENU_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_MENU_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_MENU_SMART_CLASS_INIT_NULL \
  ELM_MENU_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_MENU_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Menu_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_MENU_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Menu_Smart_Class (base field)
 * to the latest #ELM_MENU_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_MENU_SMART_CLASS_INIT_NULL
 * @see ELM_MENU_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_MENU_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_MENU_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary menu base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a menu.
 *
 * All of the functions listed on @ref Menu namespace will work for
 * objects deriving from #Elm_Menu_Smart_Class.
 */
typedef struct _Elm_Menu_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Menu_Smart_Class;

/**
 * Base widget smart data extended with menu instance data.
 */
typedef struct _Elm_Menu_Smart_Data Elm_Menu_Smart_Data;
struct _Elm_Menu_Smart_Data
{
   Elm_Widget_Smart_Data base;    /* base widget smart data as
                                   * first member obligatory, as
                                   * we're inheriting from it */

   Evas_Object          *hv, *bx, *location, *parent;

   Eina_List            *items;
   Evas_Coord            xloc, yloc;
};

typedef struct _Elm_Menu_Item       Elm_Menu_Item;
struct _Elm_Menu_Item
{
   ELM_WIDGET_ITEM;

   Elm_Menu_Item *parent;
   Evas_Object   *content;
   const char    *icon_str;
   const char    *label;
   Evas_Smart_Cb  func;
   unsigned int   idx;

   struct
   {
      Evas_Object *hv, *bx, *location;
      Eina_List   *items;
      Eina_Bool    open : 1;
   } submenu;

   Eina_Bool      separator : 1;
   Eina_Bool      selected : 1;
   Eina_Bool      object_item : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_MENU_SMART_NAME[];
EAPI const Elm_Menu_Smart_Class *elm_menu_smart_class_get(void);

#define ELM_MENU_DATA_GET(o, sd) \
  Elm_Menu_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_MENU_DATA_GET_OR_RETURN(o, ptr)          \
  ELM_MENU_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_MENU_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_MENU_DATA_GET(o, ptr);                         \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_MENU_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_MENU_SMART_NAME, __func__)) \
    return

#define ELM_MENU_ITEM_CHECK(it)                             \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_MENU_CHECK(it->base.widget);

#define ELM_MENU_ITEM_CHECK_OR_RETURN(it, ...)                         \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_MENU_CHECK(it->base.widget) __VA_ARGS__;

#endif
