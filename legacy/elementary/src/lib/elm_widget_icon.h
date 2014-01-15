#ifndef ELM_WIDGET_ICON_H
#define ELM_WIDGET_ICON_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-icon-class The Elementary Icon Class
 *
 * This class defines a common interface for @b icon objects having
 * an icon as their basic graphics.
 */

typedef struct _Elm_Icon_Smart_Data Elm_Icon_Smart_Data;
struct _Elm_Icon_Smart_Data
{
   Evas_Object          *obj; // the object itself
   const char           *stdicon;
   Elm_Icon_Lookup_Order lookup_order;

   struct
   {
      struct
      {
         const char *path;
         const char *key;
      } file, thumb;

      Ecore_Event_Handler *eeh;

      Ethumb_Thumb_Format  format;

      Ethumb_Client_Async *request;

      Eina_Bool            retry : 1;
   } thumb;

   struct
   {
      int       requested_size;
      Eina_Bool use : 1;
   } freedesktop;

   int        in_eval;

   /* WARNING: to be deprecated */
   Eina_List *edje_signals;

   Eina_Bool  is_video : 1;
};

/**
 * @}
 */

#define ELM_ICON_DATA_GET(o, sd) \
  Elm_Icon_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_ICON_CLASS)

#define ELM_ICON_DATA_GET_OR_RETURN(o, ptr)          \
  ELM_ICON_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_ICON_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_ICON_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return val;                                   \
    }

#define ELM_ICON_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_ICON_CLASS))) \
    return

#endif
