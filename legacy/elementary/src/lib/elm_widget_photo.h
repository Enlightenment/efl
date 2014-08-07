#ifndef ELM_WIDGET_PHOTO_H
#define ELM_WIDGET_PHOTO_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-photo-class The Elementary Photo Class
 *
 * Elementary, besides having the @ref Photo widget, exposes its
 * foundation -- the Elementary Photo Class -- in order to create
 * other widgets which are a photo with some more logic on top.
 */

/**
 * Base widget smart data extended with photo instance data.
 */
typedef struct _Elm_Photo_Data Elm_Photo_Data;

struct _Elm_Photo_Data
{
   Evas_Object          *icon;
   int                   size;
   Ecore_Timer          *long_press_timer;

   struct
   {
      struct
      {
         const char *path;
         const char *key;
      } file, thumb;
   } thumb;

   Eina_Bool             fill_inside: 1;
   Eina_Bool             drag_started: 1; /**< set true when drag started */
};

/**
 * @}
 */

#define ELM_PHOTO_DATA_GET(o, sd) \
  Elm_Photo_Data * sd = eo_data_scope_get(o, ELM_PHOTO_CLASS)

#define ELM_PHOTO_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_PHOTO_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_PHOTO_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_PHOTO_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_PHOTO_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_PHOTO_CLASS))) \
    return

#endif
