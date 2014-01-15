#ifndef ELM_WIDGET_BG_H
#define ELM_WIDGET_BG_H

#include "Elementary.h"

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
typedef struct _Elm_Bg_Smart_Data Elm_Bg_Smart_Data;
struct _Elm_Bg_Smart_Data
{
/* the basic background's edje object has three swallow spots, namely:
 *  - "elm.swallow.rectangle" (elm_bg_color_set),
 *  - "elm.swallow.background" (elm_bg_file_set) and
 *  - "elm.swallow.content" (elm_bg_overlay_set).
 * the following three variables hold possible content to fit in each
 * of them, respectively. */

   Evas_Object          *rect, *img;
   const char           *file, *group;  /* path to file and group name
                                         * to give life to "img" */
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
  Elm_Bg_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_BG_CLASS)

#define ELM_BG_DATA_GET_OR_RETURN(o, ptr)            \
  ELM_BG_DATA_GET(o, ptr);                           \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_BG_DATA_GET_OR_RETURN_VAL(o, ptr, val)   \
  ELM_BG_DATA_GET(o, ptr);                           \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return val;                                   \
    }

#define ELM_BG_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_BG_CLASS))) \
    return

#endif
