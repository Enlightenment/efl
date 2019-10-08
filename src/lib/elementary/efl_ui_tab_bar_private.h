#ifndef EFL_UI_TAB_BAR_PRIVATE_H
#define EFL_UI_TAB_BAR_PRIVATE_H

typedef struct _Efl_Ui_Tab_Bar_Data                   Efl_Ui_Tab_Bar_Data;

struct _Efl_Ui_Tab_Bar_Data
{
   Efl_Ui_Box                           *bx;
   Efl_Ui_Item                          *selected, *fallback_selection;
   Eina_Bool                             in_value_change;
   Eina_Bool                             allow_manual_deselection : 1;
};

#define EFL_UI_TAB_BAR_DATA_GET(o, sd) \
  Efl_Ui_Tab_Bar_Data *sd = efl_data_scope_get(o, EFL_UI_TAB_BAR_CLASS)

#define EFL_UI_TAB_BAR_DATA_GET_OR_RETURN(o, sd, ...) \
  Efl_Ui_Tab_Bar_Data *sd = efl_data_scope_safe_get(o, EFL_UI_TAB_BAR_CLASS); \
  if (EINA_UNLIKELY(!sd))                            \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }


#endif
