#ifndef EFL_UI_MI_STATE_PRIVATE_H
#define EFL_UI_MI_STATE_PRIVATE_H

#include "Elementary.h"
#include "efl_ui_mi_state.eo.h"

typedef struct _Efl_Ui_Mi_State_Data Efl_Ui_Mi_State_Data;

struct _Efl_Ui_Mi_State_Data
{
   const char *start;
   const char *end;
};

#define EFL_UI_MI_STATE_DATA_GET(o, sd) \
  Efl_Ui_Mi_State_Data * sd = efl_data_scope_safe_get(o, EFL_UI_MI_STATE_CLASS)

#define EFL_UI_MI_STATE_DATA_GET_OR_RETURN(o, ptr)   \
  EFL_UI_MI_STATE_DATA_GET(o, ptr);                  \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return;                                          \
    }

#endif
