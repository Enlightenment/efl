#ifndef ELM_WIDGET_NSTATE_H
#define ELM_WIDGET_NSTATE_H

#include "elm_nstate.eo.h"

typedef struct
{
   int nstate;
   int state;
} Elm_Nstate_Data;


#define ELM_NSTATE_DATA_GET(o, sd) \
  Elm_Nstate_Data * sd = eo_data_scope_get(o, ELM_NSTATE_CLASS)

#define ELM_NSTATE_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_NSTATE_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_NSTATE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_NSTATE_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_NSTATE_CHECK(obj)                       \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_NSTATE_CLASS))) \
    return

#endif
