#ifndef ELM_WIDGET_BG_H
#define ELM_WIDGET_BG_H

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
 * @section elm-bg-class The Elementary Bg Class
 *
 * Elementary, besides having the @ref Bg widget, exposes its
 * foundation -- the Elementary Bg Class -- in order to create other
 * widgets which are a bg with some more logic on top.
 */

/**
 * Base layout smart data extended with bg instance data.
 */
typedef struct _Elm_Bg_Data Elm_Bg_Data;
struct _Elm_Bg_Data
{
   Evas_Object          *rect; /*<< Used for elm_bg_color_set(): elm.swallow.rectangle */
   Evas_Object          *img; /*<< Used for elm_bg_file_set(): elm.swallow.content */
   const char           *file, *group;  /*<< path to file and group name to give life to "img": elm.swallow.background */
   Elm_Bg_Option         option;
   struct
     {
        Evas_Coord w, h;
     } load_opts;
};

/**
 * @}
 */

#define ELM_BG_DATA_GET(o, sd) \
  Elm_Bg_Data * sd = efl_data_scope_get(o, ELM_BG_CLASS)

#define ELM_BG_DATA_GET_OR_RETURN(o, ptr)            \
  ELM_BG_DATA_GET(o, ptr);                           \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_BG_DATA_GET_OR_RETURN_VAL(o, ptr, val)   \
  ELM_BG_DATA_GET(o, ptr);                           \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return val;                                   \
    }

#define ELM_BG_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_BG_CLASS))) \
    return

#endif
