#ifndef ELM_WIDGET_TOOLBAR_H
#define ELM_WIDGET_TOOLBAR_H

#include "elm_interface_scrollable.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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

typedef struct _Elm_Toolbar_Item_Data Elm_Toolbar_Item_Data;

/**
 * Base widget smart data extended with toolbar instance data.
 */
typedef struct _Elm_Toolbar_Data Elm_Toolbar_Data;
struct _Elm_Toolbar_Data
{
   Evas_Object                          *hit_rect;

   Evas_Object                          *bx, *more, *bx_more, *bx_more2;
   Evas_Object                          *menu_parent;
   Eina_Inlist                          *items;
   Elm_Toolbar_Item_Data                *more_item;
   Elm_Object_Item                      *selected_item; /**< a selected item by mouse click, return key, api, and etc. */
   Elm_Object_Item                      *focused_item; /**< a focused item by keypad arrow or mouse. This is set to NULL if widget looses focus. */
   Elm_Object_Item                      *last_focused_item; /**< This records the last focused item when widget looses focus. This is required to set the focus on last focused item when widgets gets focus. */
   Elm_Toolbar_Item_Data                *reorder_empty, *reorder_item;
   Elm_Toolbar_Shrink_Mode               shrink_mode;
   int                                   theme_icon_size, priv_icon_size,
                                         icon_size;
   int                                   standard_priority;
   int                                   minw_bx, minh_bx;
   unsigned int                          item_count;
   unsigned int                          separator_count;
   double                                align;
   Elm_Object_Select_Mode                select_mode;
   Ecore_Timer                          *long_timer;
   Ecore_Job                            *resize_job;
   Efl_Ui_Dir                            dir;

   Eina_Bool                             long_press : 1;
   Eina_Bool                             homogeneous : 1;
   Eina_Bool                             delete_me : 1;
   Eina_Bool                             reorder_mode : 1;
   Eina_Bool                             transverse_expanded : 1;
   Eina_Bool                             mouse_down : 1; /**< a flag that mouse is down on the toolbar at the moment. This flag is set to true on mouse and reset to false on mouse up. */
};

struct _Elm_Toolbar_Item_Data
{
   Elm_Widget_Item_Data     *base;
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
   Elm_Toolbar_Item_Data *reorder_to;
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


/**
 * @}
 */

#define ELM_TOOLBAR_DATA_GET(o, sd) \
  Elm_Toolbar_Data * sd = efl_data_scope_get(o, ELM_TOOLBAR_CLASS)

#define ELM_TOOLBAR_DATA_GET_OR_RETURN(o, ptr)       \
  ELM_TOOLBAR_DATA_GET(o, ptr);                      \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_TOOLBAR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_TOOLBAR_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return val;                                      \
    }

#define ELM_TOOLBAR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_TOOLBAR_CLASS))) \
    return

#define ELM_TOOLBAR_ITEM_CHECK(it)                          \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN(it->base, ); \
  ELM_TOOLBAR_CHECK(it->base->widget);

#define ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, ...)                      \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN(it->base, __VA_ARGS__); \
  ELM_TOOLBAR_CHECK(it->base->widget) __VA_ARGS__;

#define ELM_TOOLBAR_ITEM_CHECK_OR_GOTO(it, label)              \
  ELM_WIDGET_ITEM_CHECK_OR_GOTO(it->base, label); \
  if (!it->base->widget || !efl_isa ((it->base->widget), ELM_TOOLBAR_CLASS)) goto label;

#define ELM_TOOLBAR_ITEM_DATA_GET(o, sd) \
  Elm_Toolbar_Item_Data *sd = efl_data_scope_get(o, ELM_TOOLBAR_ITEM_CLASS)

#endif
