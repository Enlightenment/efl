#ifndef EFL_UI_SPIN_BUTTON_PRIVATE_H
#define EFL_UI_SPIN_BUTTON_PRIVATE_H

typedef struct _Efl_Ui_Spin_Button_Data    Efl_Ui_Spin_Button_Data;
struct _Efl_Ui_Spin_Button_Data
{
   Evas_Object          *ent, *inc_button, *dec_button, *text_button;
   Eina_Future          *delay_change_timer; /**< a timer for a delay,changed smart callback */

   Efl_Ui_Layout_Orientation dir;
   double                step;
   Eina_Bool             entry_visible : 1;
   Eina_Bool             entry_reactivate : 1;
   Eina_Bool             editable : 1;
   Eina_Bool             wraparound : 1;
};

#endif
