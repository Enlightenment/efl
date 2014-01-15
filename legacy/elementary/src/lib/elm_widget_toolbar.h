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

typedef struct _Elm_Toolbar_Item Elm_Toolbar_Item;

/**
 * Base widget smart data extended with toolbar instance data.
 */
typedef struct _Elm_Toolbar_Smart_Data Elm_Toolbar_Smart_Data;
struct _Elm_Toolbar_Smart_Data
{
   Evas_Object                          *hit_rect;

   Evas_Object                          *bx, *more, *bx_more, *bx_more2;
   Evas_Object                          *menu_parent;
   Eina_Inlist                          *items;
   Elm_Toolbar_Item                     *more_item;
   Elm_Toolbar_Item                     *selected_item; /**< a selected item by mouse click, return key, api, and etc. */
   Elm_Toolbar_Item                     *highlighted_item; /**< a highlighted item by keypard arrow */
   Elm_Toolbar_Item                     *reorder_empty, *reorder_item;
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
   Eina_Bool                             delete_me : 1;
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
   Evas_Object  *proxy;
   Evas_Smart_Cb func;
   Elm_Transit  *trans;
   Elm_Toolbar_Item *reorder_to;
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
   Eina_Bool     on_move : 1;
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

#define ELM_TOOLBAR_DATA_GET(o, sd) \
  Elm_Toolbar_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_TOOLBAR_CLASS)

#define ELM_TOOLBAR_DATA_GET_OR_RETURN(o, ptr)       \
  ELM_TOOLBAR_DATA_GET(o, ptr);                      \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_TOOLBAR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_TOOLBAR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       CRI("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return val;                                      \
    }

#define ELM_TOOLBAR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_TOOLBAR_CLASS))) \
    return

#define ELM_TOOLBAR_ITEM_CHECK(it)                          \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_TOOLBAR_CHECK(it->base.widget);

#define ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, ...)                      \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_TOOLBAR_CHECK(it->base.widget) __VA_ARGS__;

#define ELM_TOOLBAR_ITEM_CHECK_OR_GOTO(it, label)              \
  ELM_WIDGET_ITEM_CHECK_OR_GOTO((Elm_Widget_Item *)it, label); \
  if (!it->base.widget || !eo_isa ((it->base.widget), ELM_OBJ_TOOLBAR_CLASS)) goto label;

#endif
