#ifndef ELM_WIDGET_POPUP_H
#define ELM_WIDGET_POPUP_H

#include "Elementary.h"
#include "elm_widget_layout.h"
#include "elm_notify_eo.h"
#include "elm_popup_item_eo.h"
#include "elm_popup_eo.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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
typedef struct _Elm_Popup_Data Elm_Popup_Data;
struct _Elm_Popup_Data
{
   Evas_Object          *notify;
   Evas_Object          *main_layout;
   Evas_Object          *title_icon;
   Evas_Object          *content_area;
   Evas_Object          *text_content_obj;
   Evas_Object          *action_area;
   Evas_Object          *box;
   Evas_Object          *tbl;
   Evas_Object          *spacer;
   Evas_Object          *scr;
   Evas_Object          *content;
   Evas_Object          *parent; /**< Pointer to remove _parent_resize_cb when popup is deleted. */
   Eina_List            *items;
   const char           *title_text;
   Action_Area_Data     *buttons[ELM_POPUP_ACTION_BUTTON_MAX];
   Elm_Wrap_Type         content_text_wrap_type;
   unsigned int          last_button_number;
   Evas_Coord            max_sc_h;

   Eina_Bool             visible : 1;
   Eina_Bool             scr_size_recalc : 1;
   Eina_Bool             scroll : 1;
   Eina_Bool             theme_scroll : 1;
};

typedef struct _Elm_Popup_Item_Data Elm_Popup_Item_Data;
struct _Elm_Popup_Item_Data
{
   Elm_Widget_Item_Data *base;

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
  Elm_Popup_Data * sd = efl_data_scope_get(o, ELM_POPUP_CLASS)

#define ELM_POPUP_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_POPUP_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_POPUP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_POPUP_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_POPUP_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_POPUP_CLASS))) \
    return

#define ELM_POPUP_ITEM_CHECK(it)                            \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN(it->base, ); \
  ELM_POPUP_CHECK(it->base->widget);

#define ELM_POPUP_ITEM_CHECK_OR_RETURN(it, ...)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN(it->base, __VA_ARGS__); \
  ELM_POPUP_CHECK(it->base->widget) __VA_ARGS__;

#define ELM_POPUP_ITEM_DATA_GET(o, sd) \
  Elm_Popup_Item_Data* sd = efl_data_scope_get(o, ELM_POPUP_ITEM_CLASS)

#endif
