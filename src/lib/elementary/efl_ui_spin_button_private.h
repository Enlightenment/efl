#ifndef EFL_UI_SPIN_BUTTON_PRIVATE_H
#define EFL_UI_SPIN_BUTTON_PRIVATE_H

typedef struct _Efl_Ui_Spin_Button_Data    Efl_Ui_Spin_Button_Data;
struct _Efl_Ui_Spin_Button_Data
{
   double                interval, first_interval;
   Evas_Object          *ent, *inc_button, *dec_button, *text_button;
   Ecore_Timer          *delay_change_timer; /**< a timer for a delay,changed smart callback */
   Ecore_Timer          *spin_timer; /**< a timer for a repeated spinner value change on mouse down */
   Ecore_Timer          *longpress_timer; /**< a timer to detect long press. After lonpress timeout,
                                          start continuous change of values until mouse up */
   Efl_Ui_Dir            dir;

   Eina_Bool             entry_visible : 1;
   Eina_Bool             entry_reactivate : 1;
   Eina_Bool             editable : 1;
   Eina_Bool             inc_val : 1;
   Eina_Bool             circulate : 1;
};

#endif
