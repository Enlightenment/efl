#ifndef EFL_UI_MI_RULE_PRIVATE_H
#define EFL_UI_MI_RULE_PRIVATE_H

#include "Elementary.h"
#include "efl_ui_mi_rule.eo.h"

typedef struct _Efl_Ui_Mi_Rule_Data Efl_Ui_Mi_Rule_Data;

struct _Efl_Ui_Mi_Rule_Data
{
   Evas_Object *event_rect;
   Eina_Stringshare *keypath;
};

#define EFL_UI_MI_RULE_DATA_GET(o, sd) \
  Efl_Ui_Mi_Rule_Data * sd = efl_data_scope_safe_get(o, EFL_UI_MI_RULE_CLASS)

#define EFL_UI_MI_RULE_DATA_GET_OR_RETURN(o, ptr)   \
  EFL_UI_MI_RULE_DATA_GET(o, ptr);                  \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return;                                          \
    }

#endif
