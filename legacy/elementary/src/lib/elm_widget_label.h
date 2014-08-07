#ifndef ELM_WIDGET_LABEL_H
#define ELM_WIDGET_LABEL_H

#include "Elementary.h"

#include <Eio.h>

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * Base layout smart data extended with label instance data.
 */
typedef struct _Elm_Label_Data Elm_Label_Data;
struct _Elm_Label_Data
{
   const char           *format;
   double                slide_duration;
   double                slide_speed;
   Evas_Coord            lastw;
   Evas_Coord            wrap_w;
   Elm_Wrap_Type         linewrap;
   Elm_Label_Slide_Mode  slide_mode;

   Eina_Bool             ellipsis : 1;
   Eina_Bool             slide_ellipsis : 1;
   Eina_Bool             use_slide_speed : 1;
};

#define ELM_LABEL_DATA_GET(o, sd) \
  Elm_Label_Data * sd = eo_data_scope_get(o, ELM_LABEL_CLASS)

#define ELM_LABEL_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_LABEL_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_LABEL_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_LABEL_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_LABEL_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_LABEL_CLASS))) \
    return

#endif
