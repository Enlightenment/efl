#ifndef ELM_WIDGET_TOOLBAR_PRIVATE_H
#define ELM_WIDGET_TOOLBAR_PRIVATE_H

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

typedef struct _Efl_Ui_Toolbar_Item_Data Efl_Ui_Toolbar_Item_Data;

/**
 * Base widget smart data extended with toolbar instance data.
 */
typedef struct _Efl_Ui_Toolbar_Data Efl_Ui_Toolbar_Data;
struct _Efl_Ui_Toolbar_Data
{
   Evas_Object                          *bx;
   Eina_Inlist                          *items;
   Elm_Object_Item                      *selected_item; /**< a selected item by mouse click, return key, api, and etc. */
   Elm_Object_Item                      *focused_item; /**< a focused item by keypad arrow or mouse. This is set to NULL if widget looses focus. */
   Elm_Object_Item                      *last_focused_item; /**< This records the last focused item when widget looses focus. This is required to set the focus on last focused item when widgets gets focus. */
   unsigned int                          item_count;
   Efl_Ui_Dir                            dir;

   Eina_Bool                             delete_me : 1;
};

struct _Efl_Ui_Toolbar_Item_Data
{
   Elm_Widget_Item_Data     *base;
   EINA_INLIST;

   const char   *label;
   const char   *icon_str;
   Evas_Object  *icon;
   Evas_Object  *object;
   Evas_Smart_Cb func;

   Eina_Bool     selected : 1;
};


/**
 * @}
 */

#define EFL_UI_TOOLBAR_DATA_GET(o, sd) \
  Efl_Ui_Toolbar_Data * sd = efl_data_scope_get(o, EFL_UI_TOOLBAR_CLASS)

#define EFL_UI_TOOLBAR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_TOOLBAR_CLASS))) \
    return

#define EFL_UI_TOOLBAR_ITEM_CHECK(it)                          \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN(it->base, ); \
  EFL_UI_TOOLBAR_CHECK(it->base->widget);

#define EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(it, ...)                      \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN(it->base, __VA_ARGS__); \
  EFL_UI_TOOLBAR_CHECK(it->base->widget) __VA_ARGS__;

#define EFL_UI_TOOLBAR_ITEM_DATA_GET(o, sd) \
  Efl_Ui_Toolbar_Item_Data *sd = efl_data_scope_get(o, EFL_UI_TOOLBAR_ITEM_CLASS)

#endif
