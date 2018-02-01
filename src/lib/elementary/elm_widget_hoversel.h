#ifndef ELM_WIDGET_HOVERSEL_H
#define ELM_WIDGET_HOVERSEL_H

#include "Elementary.h"


#include <Eio.h>

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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
typedef struct _Elm_Hoversel_Data Elm_Hoversel_Data;
struct _Elm_Hoversel_Data
{
   /* aggregates a hover */
   Evas_Object          *hover;
   Evas_Object          *hover_parent;
   Evas_Object          *spacer;
   Evas_Object          *tbl;
   Evas_Object          *scr;
   Evas_Object          *bx;
   const char           *last_location;

   Eina_List            *items;
   Ecore_Job            *resize_job;

   Eina_Bool             horizontal    : 1;
   Eina_Bool             expanded      : 1;
   Eina_Bool             scrollable    : 1;
   Eina_Bool             auto_update   : 1;
};

typedef struct _Elm_Hoversel_Item_Data Elm_Hoversel_Item_Data;
struct _Elm_Hoversel_Item_Data
{
   Elm_Widget_Item_Data *base;

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
  Elm_Hoversel_Data * sd = efl_data_scope_get(o, ELM_HOVERSEL_CLASS)

#define ELM_HOVERSEL_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_HOVERSEL_DATA_GET(o, ptr);                     \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_HOVERSEL_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_HOVERSEL_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                               \
    {                                                    \
       ERR("No widget data for object %p (%s)",          \
           o, evas_object_type_get(o));                  \
       return val;                                       \
    }

#define ELM_HOVERSEL_CHECK(obj)                          \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_HOVERSEL_CLASS))) \
    return

#define ELM_HOVERSEL_ITEM_DATA_GET(o, sd) \
  Elm_Hoversel_Item_Data *sd = efl_data_scope_get(o, ELM_HOVERSEL_ITEM_CLASS)

#define ELM_HOVERSEL_ITEM_CHECK(it)                           \
  if (EINA_UNLIKELY(!efl_isa(it->base->eo_obj, ELM_HOVERSEL_ITEM_CLASS))) \
    return

#define ELM_HOVERSEL_ITEM_CHECK_OR_RETURN(it, ...)         \
  if (EINA_UNLIKELY(!efl_isa(it->base->eo_obj, ELM_HOVERSEL_ITEM_CLASS))) \
    return __VA_ARGS__;

#endif
