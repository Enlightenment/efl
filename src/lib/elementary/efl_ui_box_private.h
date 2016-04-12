#ifndef EFL_UI_BOX_PRIVATE_H
#define EFL_UI_BOX_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

// FIXME: stop using Evas.Box
#include <../evas/canvas/evas_box.eo.h>

#define MY_CLASS EFL_UI_BOX_CLASS
#define MY_CLASS_NAME "Efl.Ui.Box"

// FIXME: stop using evas box
void _efl_ui_box_custom_layout(Efl_Ui_Box *box, Evas_Object_Box_Data *priv);

typedef struct _Efl_Ui_Box_Data Efl_Ui_Box_Data;
typedef struct _Box_Item_Iterator Box_Item_Iterator;

struct _Efl_Ui_Box_Data
{
   const Eo_Class *layout_engine;
   const void *layout_data;

   Efl_Orient orient;
   Eina_Bool homogeneous : 1;
   Eina_Bool delete_me : 1;
   Eina_Bool recalc : 1;

   struct {
      double h, v;
      Eina_Bool scalable: 1;
   } pad;
};

struct _Box_Item_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Efl_Ui_Box    *object;
};

static inline Eina_Bool
_horiz(Efl_Orient dir)
{
   return dir % 180 == EFL_ORIENT_RIGHT;
}

#endif
