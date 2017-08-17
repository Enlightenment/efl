#ifndef EFL_UI_WIDGET_POPUP_H
#define EFL_UI_WIDGET_POPUP_H

typedef struct _Efl_Ui_Popup_Data Efl_Ui_Popup_Data;
struct _Efl_Ui_Popup_Data
{
   Evas_Object *self;
   Evas_Object *win_parent;
   Evas_Object *event_bg;
   Efl_Ui_Popup_Align align;
   Eina_Bool    bg_repeat_events : 1;
};

#endif
