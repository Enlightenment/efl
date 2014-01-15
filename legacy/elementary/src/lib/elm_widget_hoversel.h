#ifndef ELM_WIDGET_HOVERSEL_H
#define ELM_WIDGET_HOVERSEL_H

#include "Elementary.h"

#include <Eio.h>

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-hoversel-class The Elementary Hoversel Class
 *
 * Elementary, besides having the @ref Hoversel widget, exposes its
 * foundation -- the Elementary Hoversel Class -- in order to create other
 * widgets which are a hoversel with some more logic on top.
 */

/**
 * Base button smart data extended with hoversel instance data.
 */
typedef struct _Elm_Hoversel_Smart_Data Elm_Hoversel_Smart_Data;
struct _Elm_Hoversel_Smart_Data
{
   /* aggregates a hover */
   Evas_Object          *hover;
   Evas_Object          *hover_parent;

   Eina_List            *items;

   Eina_Bool             horizontal : 1;
   Eina_Bool             expanded   : 1;
};

typedef struct _Elm_Hoversel_Item Elm_Hoversel_Item;
struct _Elm_Hoversel_Item
{
   ELM_WIDGET_ITEM;

   const char   *label;
   const char   *icon_file;
   const char   *icon_group;

   Elm_Icon_Type icon_type;
   Evas_Smart_Cb func;
};

/**
 * @}
 */

#define ELM_HOVERSEL_DATA_GET(o, sd) \
  Elm_Hoversel_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_HOVERSEL_CLASS)

#define ELM_HOVERSEL_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_HOVERSEL_DATA_GET(o, ptr);                     \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_HOVERSEL_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_HOVERSEL_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                               \
    {                                                    \
       CRI("No widget data for object %p (%s)",          \
           o, evas_object_type_get(o));                  \
       return val;                                       \
    }

#define ELM_HOVERSEL_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_HOVERSEL_CLASS))) \
    return

#define ELM_HOVERSEL_ITEM_CHECK(it)                         \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_HOVERSEL_CHECK(it->base.widget);

#define ELM_HOVERSEL_ITEM_CHECK_OR_RETURN(it, ...)                     \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_HOVERSEL_CHECK(it->base.widget) __VA_ARGS__;

#endif
