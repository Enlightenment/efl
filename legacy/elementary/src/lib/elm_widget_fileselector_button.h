#ifndef ELM_WIDGET_FILESELECTOR_BUTTON_H
#define ELM_WIDGET_FILESELECTOR_BUTTON_H

#include "Elementary.h"

#include <Eio.h>

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-fileselector-button-class The Elementary Fileselector Button Class
 *
 * Elementary, besides having the @ref Fileselector_Button widget,
 * exposes its foundation -- the Elementary Fileselector Button Class
 * -- in order to create other widgets which are a fileselector_button
 * with some more logic on top.
 */

/**
 * Base button smart data extended with fileselector_button instance data.
 */
typedef struct _Elm_Fileselector_Button_Smart_Data \
  Elm_Fileselector_Button_Smart_Data;
struct _Elm_Fileselector_Button_Smart_Data
{
   Evas_Object          *obj; // the object itself
   Evas_Object          *fs, *fsw;
   const char           *window_title;
   Evas_Coord            w, h;

   struct
   {
      const char *path;
      Eina_Bool   expandable : 1;
      Eina_Bool   folder_only : 1;
      Eina_Bool   is_save : 1;
   } fsd;

   Eina_Bool             inwin_mode : 1;
};

/**
 * @}
 */

#define ELM_FILESELECTOR_BUTTON_DATA_GET(o, sd) \
  Elm_Fileselector_Button_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_FILESELECTOR_BUTTON_CLASS)

#define ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN(o, ptr) \
  ELM_FILESELECTOR_BUTTON_DATA_GET(o, ptr);                \
  if (EINA_UNLIKELY(!ptr))                                 \
    {                                                      \
       CRI("No widget data for object %p (%s)",            \
           o, evas_object_type_get(o));                    \
       return;                                             \
    }

#define ELM_FILESELECTOR_BUTTON_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_FILESELECTOR_BUTTON_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                          \
    {                                                               \
       CRI("No widget data for object %p (%s)",                     \
           o, evas_object_type_get(o));                             \
       return val;                                                  \
    }

#define ELM_FILESELECTOR_BUTTON_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_FILESELECTOR_BUTTON_CLASS))) \
    return

#endif
