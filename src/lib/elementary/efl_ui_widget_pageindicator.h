#ifndef EFL_UI_WIDGET_PAGEINDICATOR_H
#define EFL_UI_WIDGET_PAGEINDICATOR_H


#include <Elementary.h>


typedef struct _Efl_Ui_Pageindicator_Data
{
   Efl_Ui_Box              *box;

   struct {
      Efl_Ui_Pagecontrol   *obj;
      int                   cnt;
   } target;


   Eina_List               *items;
   Evas_VG                 *on, *off;

} Efl_Ui_Pageindicator_Data;

typedef struct _Pageindicator
{
   Evas_Object             *object;
   Efl_VG                  *vector;

} Pageindicator;

#define EFL_UI_PAGEINDICATOR_DATA_GET(o, sd) \
   Efl_Ui_Pageindicator_Data *sd = efl_data_scope_get(o, EFL_UI_PAGEINDICATOR_CLASS)

#endif
