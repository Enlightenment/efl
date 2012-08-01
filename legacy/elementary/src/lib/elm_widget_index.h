#ifndef ELM_WIDGET_INDEX_H
#define ELM_WIDGET_INDEX_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-index-class The Elementary Index Class
 *
 * Elementary, besides having the @ref Index widget, exposes its
 * foundation -- the Elementary Index Class -- in order to create other
 * widgets which are a index with some more logic on top.
 */

/**
 * @def ELM_INDEX_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Index_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_INDEX_CLASS(x) ((Elm_Index_Smart_Class *)x)

/**
 * @def ELM_INDEX_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Index_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_INDEX_DATA(x)  ((Elm_Index_Smart_Data *)x)

/**
 * @def ELM_INDEX_SMART_CLASS_VERSION
 *
 * Current version for Elementary index @b base smart class, a value
 * which goes to _Elm_Index_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_INDEX_SMART_CLASS_VERSION 1

/**
 * @def ELM_INDEX_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Index_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_INDEX_SMART_CLASS_INIT_NULL
 * @see ELM_INDEX_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_INDEX_SMART_CLASS_INIT(smart_class_init) \
  {smart_class_init, ELM_INDEX_SMART_CLASS_VERSION}

/**
 * @def ELM_INDEX_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Index_Smart_Class structure.
 *
 * @see ELM_INDEX_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_INDEX_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_INDEX_SMART_CLASS_INIT_NULL \
  ELM_INDEX_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_INDEX_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Index_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_INDEX_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Index_Smart_Class (base field)
 * to the latest #ELM_INDEX_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_INDEX_SMART_CLASS_INIT_NULL
 * @see ELM_INDEX_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_INDEX_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_INDEX_SMART_CLASS_INIT                          \
    (ELM_LAYOUT_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary index base smart class. This inherits directly from
 * #Elm_Layout_Smart_Class and is meant to build widgets extending the
 * behavior of a index.
 *
 * All of the functions listed on @ref Index namespace will work for
 * objects deriving from #Elm_Index_Smart_Class.
 */
typedef struct _Elm_Index_Smart_Class
{
   Elm_Layout_Smart_Class base;

   int                    version;    /**< Version of this smart class definition */
} Elm_Index_Smart_Class;

/**
 * Base layout smart data extended with index instance data.
 */
typedef struct _Elm_Index_Smart_Data Elm_Index_Smart_Data;
struct _Elm_Index_Smart_Data
{
   Elm_Layout_Smart_Data base;

   Evas_Object          *event[2];
   Evas_Object          *bx[2]; // 2 - for now all that's supported
   Eina_List            *items;  /* 1 list. N levels, but only 2
                                  * for now and # of items will be
                                  * small */
   int                   level;
   Evas_Coord            dx, dy;
   Ecore_Timer          *delay;
   Eina_Bool             level_active[2];

   Eina_Bool             down : 1;
   Eina_Bool             horizontal : 1;
   Eina_Bool             autohide_disabled : 1;
   Eina_Bool             indicator_disabled : 1;
};

typedef struct _Elm_Index_Item       Elm_Index_Item;
struct _Elm_Index_Item
{
   ELM_WIDGET_ITEM;

   const char   *letter;
   int           level;
   Evas_Smart_Cb func;

   Eina_Bool     selected : 1;
};

/**
 * @}
 */

EAPI extern const char ELM_INDEX_SMART_NAME[];
EAPI const Elm_Index_Smart_Class *elm_index_smart_class_get(void);

#define ELM_INDEX_DATA_GET(o, sd) \
  Elm_Index_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_INDEX_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_INDEX_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_INDEX_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_INDEX_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_INDEX_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_INDEX_SMART_NAME, __func__)) \
    return

#define ELM_INDEX_ITEM_CHECK(it)                            \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_INDEX_CHECK(it->base.widget);

#define ELM_INDEX_ITEM_CHECK_OR_RETURN(it, ...)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_INDEX_CHECK(it->base.widget) __VA_ARGS__;

#endif
