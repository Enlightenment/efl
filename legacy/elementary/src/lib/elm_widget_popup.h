#ifndef ELM_WIDGET_POPUP_H
#define ELM_WIDGET_POPUP_H

#include "Elementary.h"
#include "elm_widget_layout.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-popup-class The Elementary Popup Class
 *
 * Elementary, besides having the @ref Popup widget, exposes its
 * foundation -- the Elementary Popup Class -- in order to create other
 * widgets which are a popup with some more logic on top.
 */

#define ELM_POPUP_ACTION_BUTTON_MAX 3

typedef struct _Action_Area_Data Action_Area_Data;

/**
 * Base layout smart data extended with popup instance data.
 */
typedef struct _Elm_Popup_Smart_Data Elm_Popup_Smart_Data;
struct _Elm_Popup_Smart_Data
{
   Evas_Object          *notify;
   Evas_Object          *title_icon;
   Evas_Object          *content_area;
   Evas_Object          *text_content_obj;
   Evas_Object          *action_area;
   Evas_Object          *box;
   Evas_Object          *tbl;
   Evas_Object          *spacer;
   Evas_Object          *scr;
   Evas_Object          *content;
   Eina_List            *items;
   const char           *title_text;
   Action_Area_Data     *buttons[ELM_POPUP_ACTION_BUTTON_MAX];
   Elm_Wrap_Type         content_text_wrap_type;
   unsigned int          last_button_number;
   Evas_Coord            max_sc_w;
   Evas_Coord            max_sc_h;

   Eina_Bool             visible : 1;
   Eina_Bool             scr_size_recalc : 1;
};

typedef struct _Elm_Popup_Item Elm_Popup_Item;
struct _Elm_Popup_Item
{
   Elm_Widget_Item base;

   const char     *label;
   Evas_Object    *icon;
   Evas_Smart_Cb   func;
   Eina_Bool       disabled : 1;
};

struct _Action_Area_Data
{
   Evas_Object *obj;
   Evas_Object *btn;
   Eina_Bool    delete_me;
};

/**
 * @}
 */

#define ELM_POPUP_DATA_GET(o, sd) \
  Elm_Popup_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_POPUP_CLASS)

#define ELM_POPUP_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_POPUP_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_POPUP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_POPUP_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_POPUP_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_POPUP_CLASS))) \
    return

#define ELM_POPUP_ITEM_CHECK(it)                            \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_POPUP_CHECK(it->base.widget);

#define ELM_POPUP_ITEM_CHECK_OR_RETURN(it, ...)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_POPUP_CHECK(it->base.widget) __VA_ARGS__;

#endif
