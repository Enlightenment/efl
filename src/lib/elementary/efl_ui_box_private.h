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
   Efl_Ui_Dir dir;
   Eo *clipper;
   Eina_List *children;

   struct {
      double h, v;
      Eina_Bool scalable: 1;
   } pad;

   struct {
      double h, v;
   } align;

   Eina_Bool homogeneous : 1;
};

#endif
