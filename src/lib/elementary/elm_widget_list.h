#ifndef ELM_WIDGET_LIST_H
#define ELM_WIDGET_LIST_H

#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"
#include "elm_list.eo.h"
#include "elm_list_item.eo.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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

#define ELM_LIST_SWIPE_MOVES 12

typedef struct _Elm_List_Item_Data Elm_List_Item_Data;
/**
 * Base widget smart data extended with list instance data.
 */
typedef struct _Elm_List_Data Elm_List_Data;
struct _Elm_List_Data
{
   Evas_Object                          *box, *hit_rect;

   Eina_List                            *items, *selected, *to_delete;
   Elm_Object_Item                      *last_selected_item;
   Elm_Object_Item                      *focused_item; /**< a focused item by keypad arrow or mouse. This is set to NULL if widget looses focus. */
   Elm_Object_Item                      *last_focused_item; /**< This records the last focused item when widget looses focus. This is required to set the focus on last focused item when widgets gets focus. */
   Evas_Coord                            minw[2], minh[2], dx, dy;
   Elm_Object_Select_Mode                select_mode;
   Elm_Object_Multi_Select_Mode          multi_select_mode; /**< select mode for multiple selection */
   int                                   movements;
   int                                   walking;
   Elm_List_Mode                         h_mode;
   Elm_List_Mode                         mode;

   struct
   {
      Evas_Coord x, y;
   } history[ELM_LIST_SWIPE_MOVES];

   Eina_Bool                             focus_on_selection_enabled : 1;
   Eina_Bool                             was_selected : 1;
   Eina_Bool                             fix_pending : 1;
   Eina_Bool                             longpressed : 1;
   Eina_Bool                             scr_minw : 1;
   Eina_Bool                             scr_minh : 1;
   Eina_Bool                             on_hold : 1;
   Eina_Bool                             multi : 1;
   Eina_Bool                             swipe : 1;
   Eina_Bool                             delete_me : 1;
   Eina_Bool                             mouse_down : 1; /**< a flag that mouse is down on the list at the moment. this flag is set to true on mouse and reset to false on mouse up */
   Eina_Bool                             item_loop_enable : 1; /**< value whether item loop feature is enabled or not. */
   Eina_Bool                             item_looping_on : 1;
};

struct _Elm_List_Item_Data
{
   Elm_Widget_Item_Data     *base;

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

#define ELM_LIST_DATA_GET(o, sd) \
  Elm_List_Data * sd = efl_data_scope_get(o, ELM_LIST_CLASS)

#define ELM_LIST_DATA_GET_FROM_ITEM(it, sd) \
  ELM_LIST_DATA_GET(WIDGET(it), sd)

#define ELM_LIST_DATA_GET_OR_RETURN(o, ptr)          \
  ELM_LIST_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_LIST_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_LIST_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return val;                                   \
    }

#define ELM_LIST_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_LIST_CLASS))) \
    return

#define ELM_LIST_ITEM_CHECK(it)                             \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN(it->base, ); \
  ELM_LIST_CHECK(it->base->widget);                          \
  if (it->deleted)                       \
    {                                                       \
       ERR("ERROR: " #it " has been DELETED.\n");           \
       return;                                              \
    }

#define ELM_LIST_ITEM_CHECK_OR_RETURN(it, ...)                         \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN(it->base, __VA_ARGS__); \
  ELM_LIST_CHECK(it->base->widget) __VA_ARGS__;                         \
  if (it->deleted)                                  \
    {                                                                  \
       ERR("ERROR: " #it " has been DELETED.\n");                      \
       return __VA_ARGS__;                                             \
    }

#define ELM_LIST_ITEM_DATA_GET(o, sd) \
  Elm_List_Item_Data* sd = efl_data_scope_get(o, ELM_LIST_ITEM_CLASS)

#endif
