#ifndef EFL_UI_WIDGET_POPUP_H
#define EFL_UI_WIDGET_POPUP_H

typedef struct _Efl_Ui_Popup_Data Efl_Ui_Popup_Data;
struct _Efl_Ui_Popup_Data
{
   Eo                *win_parent;
   Eo                *backwall;
   Efl_Ui_Popup_Align align;
   Ecore_Timer       *timer;
   double             timeout;
};

#define EFL_UI_POPUP_DATA_GET_OR_RETURN(o, ptr, ...) \
  Efl_Ui_Popup_Data *ptr;                            \
  ptr = efl_data_scope_get(o, EFL_UI_POPUP_CLASS);   \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("no ui popup data for object %p (%s)",    \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }

#endif
