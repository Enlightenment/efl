#ifndef ELM_WIDGET_MULTIBUTTONENTRY_H
#define ELM_WIDGET_MULTIBUTTONENTRY_H

#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-multibuttonentry-class The Elementary Multi Button Entry Class
 *
 * Elementary, besides having the @ref Multibuttonentry widget,
 * exposes its foundation -- the Elementary Multi Button Entry Class --
 * in order to create other widgets which are a multi button entry with
 * some more logic on top.
 */

/**
 * Base widget smart data extended with multibuttonentry instance data.
 */

typedef enum _Multibuttonentry_Pos
{
   MULTIBUTTONENTRY_POS_START,
   MULTIBUTTONENTRY_POS_END,
   MULTIBUTTONENTRY_POS_BEFORE,
   MULTIBUTTONENTRY_POS_AFTER,
} Multibuttonentry_Pos;

typedef enum _Multibuttonentry_Button_State
{
   MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT,
   MULTIBUTTONENTRY_BUTTON_STATE_SELECTED,
} Multibuttonentry_Button_State;

typedef enum _MultiButtonEntry_Closed_Button_Type
{
   MULTIBUTTONENTRY_CLOSED_IMAGE,
   MULTIBUTTONENTRY_CLOSED_LABEL
} MultiButtonEntry_Closed_Button_Type;

typedef enum _Multibuttonentry_View_State
{
   MULTIBUTTONENTRY_VIEW_NONE,
   MULTIBUTTONENTRY_VIEW_GUIDETEXT,
   MULTIBUTTONENTRY_VIEW_ENTRY,
   MULTIBUTTONENTRY_VIEW_SHRINK
} Multibuttonentry_View_State;

typedef struct _Multibuttonentry_Item Elm_Multibuttonentry_Item;

struct _Multibuttonentry_Item
{
   ELM_WIDGET_ITEM;

   Evas_Object  *button;
   Evas_Coord    vw, rw; // vw: visual width, real width
   Eina_Bool     visible : 1;
   Evas_Smart_Cb func;
};

typedef struct _Elm_Multibuttonentry_Item_Filter
{
   Elm_Multibuttonentry_Item_Filter_Cb callback_func;
   void                               *data;
} Elm_Multibuttonentry_Item_Filter;

typedef struct _Elm_Multibuttonentry_Smart_Data
  Elm_Multibuttonentry_Smart_Data;
struct _Elm_Multibuttonentry_Smart_Data
{
   Evas_Object                        *parent;
   Evas_Object                        *box;
   Evas_Object                        *entry;
   Evas_Object                        *label;
   Evas_Object                        *guide_text;
   Evas_Object                        *end; /* used to represent the
                                             * total number of
                                             * invisible buttons */

   Eina_List                          *items;
   Eina_List                          *filter_list;
   Elm_Object_Item                    *selected_it; /* selected item */

   Elm_Multibuttonentry_Format_Cb      format_func;
   const void                         *format_func_data;

   const char                         *label_str, *guide_text_str;

   int                                 n_str;
   Multibuttonentry_View_State         view_state;

   Evas_Coord                          w_box, h_box;
   int                                 shrink;

   Elm_Multibuttonentry_Item_Filter_Cb add_callback;
   void                               *add_callback_data;

   Eina_Bool                           last_btn_select : 1;
   Eina_Bool                           editable : 1;
   Eina_Bool                           focused : 1;
};

/**
 * @}
 */

#define ELM_MULTIBUTTONENTRY_DATA_GET_OR_RETURN(o, ptr) \
  Elm_Multibuttonentry_Smart_Data *ptr = eo_data_scope_get(o, ELM_OBJ_MULTIBUTTONENTRY_CLASS); \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       CRI("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return;                                          \
    }

#define ELM_MULTIBUTTONENTRY_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  Elm_Multibuttonentry_Smart_Data * ptr = eo_data_scope_get(o, ELM_OBJ_MULTIBUTTONENTRY_CLASS); \
  if (EINA_UNLIKELY(!ptr))                                       \
    {                                                            \
       CRI("No widget data for object %p (%s)",                  \
           o, evas_object_type_get(o));                          \
       return val;                                               \
    }

#define ELM_MULTIBUTTONENTRY_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_MULTIBUTTONENTRY_CLASS))) \
    return

#define ELM_MULTIBUTTONENTRY_ITEM_CHECK(it)                 \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_MULTIBUTTONENTRY_CHECK(it->base.widget);

#define ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it, ...)             \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_MULTIBUTTONENTRY_CHECK(it->base.widget) __VA_ARGS__;

#endif
