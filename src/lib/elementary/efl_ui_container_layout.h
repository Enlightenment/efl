#ifndef _EFL_UI_CONTAINER_HELPER_H_
#define _EFL_UI_CONTAINER_HELPER_H_

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_Container_Item_Hints Efl_Ui_Container_Item_Hints;
typedef struct _Efl_Ui_Container_Layout_Calc Efl_Ui_Container_Layout_Calc;

struct _Efl_Ui_Container_Item_Hints
{
   int max;
   int min;
   int aspect;
   int margin[2]; // start, end
   Efl_Gfx_Hint_Aspect aspect_type;
   double weight;
   double align;
   double space;
   Eina_Bool fill;
};

struct _Efl_Ui_Container_Layout_Calc
{
   int pos;
   int size;
   int margin[2];
   double align;
   double scale;
   double pad;
   Eina_Bool fill : 1;
};

void _efl_ui_container_layout_min_max_calc(Efl_Ui_Container_Item_Hints *item, int *cw, int *ch, Eina_Bool aspect_check);
void _efl_ui_container_layout_item_init(Eo* o, Efl_Ui_Container_Item_Hints *item);
void _efl_ui_container_layout_init(Eo* obj, Efl_Ui_Container_Layout_Calc *calc);

#endif
