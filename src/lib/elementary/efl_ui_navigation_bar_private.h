#ifndef EFL_UI_WIDGET_NAVIGATION_BAR_H
#define EFL_UI_WIDGET_NAVIGATION_BAR_H

typedef struct _Efl_Ui_Navigation_Bar_Data Efl_Ui_Navigation_Bar_Data;
struct _Efl_Ui_Navigation_Bar_Data
{
   Eo         *back_button;
};

#define EFL_UI_NAVIGATION_BAR_DATA_GET_OR_RETURN(o, ptr, ...) \
  Efl_Ui_Navigation_Bar_Data *ptr;                            \
  ptr = efl_data_scope_get(o, EFL_UI_NAVIGATION_BAR_CLASS);   \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("no ui frame navigation data for object %p (%s)",    \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }

#endif
