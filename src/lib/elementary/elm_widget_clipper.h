#ifndef ELM_WIDGET_CLIPPER_H
#define ELM_WIDGET_CLIPPER_H

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
 * @section elm-clipper-class The Elementary Clipper Class
 *
 * Elementary, besides having the @ref Clipper widget, exposes its
 * foundation -- the Elementary Clipper Class -- in order to create other
 * widgets which are a clipper object with some more logic on top.
 */


/**
 * Base widget smart data extended with clipper instance data.
 */
typedef struct _Elm_Clipper_Data Elm_Clipper_Data;
struct _Elm_Clipper_Data
{
   Evas_Object *content, *clipper;
   float region_x1;
   float region_y1;
   float region_x2;
   float region_y2;
};

/**
 * @}
 */

#define ELM_CLIPPER_DATA_GET(o, sd) \
  Elm_Clipper_Data * sd = efl_data_scope_get(o, ELM_CLIPPER_CLASS)

#define ELM_CLIPPER_DATA_GET_OR_RETURN(o, ptr)        \
  ELM_CLIPPER_DATA_GET(o, ptr);                       \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_CLIPPER_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_CLIPPER_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                             \
    {                                                  \
       ERR("No widget data for object %p (%s)",        \
           o, evas_object_type_get(o));                \
       return val;                                     \
    }

#define ELM_CLIPPER_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_CLIPPER_CLASS))) \
    return

#endif
