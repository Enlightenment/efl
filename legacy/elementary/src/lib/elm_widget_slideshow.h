#ifndef ELM_WIDGET_SLIDESHOW_H
#define ELM_WIDGET_SLIDESHOW_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-slideshow-class The Elementary Slideshow Class
 *
 * Elementary, besides having the @ref Slideshow widget, exposes its
 * foundation -- the Elementary Slideshow Class -- in order to create other
 * widgets which are a slideshow with some more logic on top.
 */

/**
 * Base layout smart data extended with slideshow instance data.
 */
typedef struct _Elm_Slideshow_Smart_Data Elm_Slideshow_Smart_Data;
typedef struct _Elm_Slideshow_Item       Elm_Slideshow_Item;

struct _Elm_Slideshow_Item
{
   ELM_WIDGET_ITEM;

   Eina_List                      *l, *l_built;

   const Elm_Slideshow_Item_Class *itc;
};

struct _Elm_Slideshow_Smart_Data
{
   // list of Elm_Slideshow_Item*
   Eina_List            *items;
   Eina_List            *items_built;

   Elm_Slideshow_Item   *current;
   Elm_Slideshow_Item   *previous;

   Eina_List            *transitions;
   const char           *transition;

   int                   count_item_pre_before;
   int                   count_item_pre_after;
   Ecore_Timer          *timer;
   double                timeout;
   Eina_Bool             loop : 1;

   struct
   {
      const char *current;
      Eina_List  *list;  //list of const char *
   } layout;
};

/**
 * @}
 */

#define ELM_SLIDESHOW_DATA_GET(o, sd) \
  Elm_Slideshow_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_SLIDESHOW_CLASS)

#define ELM_SLIDESHOW_DATA_GET_OR_RETURN(o, ptr)     \
  ELM_SLIDESHOW_DATA_GET(o, ptr);                    \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_SLIDESHOW_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SLIDESHOW_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                \
    {                                                     \
       CRI("No widget data for object %p (%s)",           \
           o, evas_object_type_get(o));                   \
       return val;                                        \
    }

#define ELM_SLIDESHOW_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_SLIDESHOW_CLASS))) \
    return

#define ELM_SLIDESHOW_ITEM_CHECK(it)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, ); \
  ELM_SLIDESHOW_CHECK(it->base.widget);

#define ELM_SLIDESHOW_ITEM_CHECK_OR_RETURN(it, ...)                    \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_SLIDESHOW_CHECK(it->base.widget) __VA_ARGS__;

#endif
