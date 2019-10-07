#ifndef EFL_UI_BOX_PRIVATE_H
#define EFL_UI_BOX_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_Box_Data Efl_Ui_Box_Data;

struct _Efl_Ui_Box_Data
{
   Efl_Ui_Layout_Orientation dir;
   Eo *clipper;
   Eina_List *children;

   struct {
      unsigned int h, v;
   } pad;

   struct {
      double h, v;
   } align;

   Eina_Position2D last_pos;

   Eina_Bool homogeneous : 1;
   Eina_Bool full_recalc : 1; //whether to force full recalc
};

#endif
