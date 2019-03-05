#ifndef ELM_WIDGET_INDEX_H
#define ELM_WIDGET_INDEX_H

#include "Elementary.h"

#include "elm_index_item_eo.h"
#include "elm_index_eo.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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
 * Base layout smart data extended with index instance data.
 */
typedef struct _Elm_Index_Data Elm_Index_Data;
struct _Elm_Index_Data
{
   Evas_Object          *event_rect[2]; /**< rectangle objects for event handling */
   Evas_Object          *bx[2]; // 2 - for now all that's supported
   Eina_List            *items;  /* 1 list. N levels, but only 2
                                  * for now and # of items will be
                                  * small */
   Eina_List            *omit;

   Efl_Ui_Dir            dir;
   int                   level;
   Evas_Coord            dx, dy;
   Ecore_Timer          *delay;
   double                delay_change_time;
   Eina_Bool             level_active[2]; /**< a flag for the activeness of a
                                            level. activeness means the box is
                                            filled with contents. */
   int                   group_num, default_num;
   int                   show_group, next_group;

   Eina_Bool             mouse_down : 1;
   Eina_Bool             autohide_disabled : 1;
   Eina_Bool             indicator_disabled : 1;
   Eina_Bool             omit_enabled : 1;
};

typedef struct _Elm_Index_Item_Data       Elm_Index_Item_Data;
struct _Elm_Index_Item_Data
{
   Elm_Widget_Item_Data *base;

   const char      *letter;
   int              level;
   Evas_Smart_Cb    func;

   Eina_List       *omitted;
   Elm_Index_Item_Data  *head;

   int              priority;
   Eina_Bool        selected : 1; /**< a flag that remembers an item is selected. this is set true when mouse down/move occur above an item and when elm_index_item_selected_set() API is called. */
};

typedef struct _Elm_Index_Omit Elm_Index_Omit;
struct _Elm_Index_Omit
{
   int offset;
   int count;
};

/**
 * @}
 */
#define ELM_INDEX_DATA_GET(o, sd) \
  Elm_Index_Data * sd = efl_data_scope_get(o, ELM_INDEX_CLASS)

#define ELM_INDEX_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_INDEX_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_INDEX_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_INDEX_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_INDEX_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_INDEX_CLASS))) \
    return

#define ELM_INDEX_ITEM_CHECK(it)                            \
  if (EINA_UNLIKELY(!efl_isa((it->base->eo_obj), ELM_INDEX_ITEM_CLASS))) \
    return

#define ELM_INDEX_ITEM_CHECK_OR_RETURN(it, ...)                        \
  if (EINA_UNLIKELY(!efl_isa((it->base->eo_obj), ELM_INDEX_ITEM_CLASS))) \
    return __VA_ARGS__;

#define ELM_INDEX_ITEM_DATA_GET(o, sd) \
  Elm_Index_Item_Data *sd = efl_data_scope_get(o, ELM_INDEX_ITEM_CLASS)

#endif
