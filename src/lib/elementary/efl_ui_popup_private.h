#ifndef EFL_UI_WIDGET_POPUP_H
#define EFL_UI_WIDGET_POPUP_H

typedef struct _Efl_Ui_Popup_Data Efl_Ui_Popup_Data;
struct _Efl_Ui_Popup_Data
{
   Eo                *win_parent;
   Eo                *event_bg;
   Efl_Ui_Popup_Align align;
   Ecore_Timer       *timer;
   double             timeout;
   Eina_Bool          bg_repeat_events : 1;
};

#endif
