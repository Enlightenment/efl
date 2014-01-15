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

#define ELM_LIST_SWIPE_MOVES 12

/**
 * Base widget smart data extended with list instance data.
 */
typedef struct _Elm_List_Smart_Data Elm_List_Smart_Data;
struct _Elm_List_Smart_Data
{
   Evas_Object                          *box, *hit_rect;

   Eina_List                            *items, *selected, *to_delete;
   Elm_Object_Item                      *last_selected_item;
   Evas_Coord                            minw[2], minh[2];
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

#define ELM_LIST_DATA_GET(o, sd) \
  Elm_List_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_LIST_CLASS)

#define ELM_LIST_DATA_GET_OR_RETURN(o, ptr)          \
  ELM_LIST_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_LIST_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_LIST_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return val;                                   \
    }

#define ELM_LIST_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_LIST_CLASS))) \
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
