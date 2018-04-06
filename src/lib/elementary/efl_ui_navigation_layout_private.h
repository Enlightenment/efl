#ifndef EFL_UI_WIDGET_NAVIGATION_LAYOUT_H
#define EFL_UI_WIDGET_NAVIGATION_LAYOUT_H

typedef struct _Efl_Ui_Navigation_Layout_Data Efl_Ui_Navigation_Layout_Data;
struct _Efl_Ui_Navigation_Layout_Data
{
   Efl_Ui_Layout_Object *bar;
};

#define EFL_UI_NAVIGATION_LAYOUT_DATA_GET_OR_RETURN(o, ptr, ...) \
  Efl_Ui_Navigation_Layout_Data *ptr;                            \
  ptr = efl_data_scope_get(o, EFL_UI_NAVIGATION_LAYOUT_CLASS);   \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("no ui navigation layout data for object %p (%s)",    \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }

#endif
