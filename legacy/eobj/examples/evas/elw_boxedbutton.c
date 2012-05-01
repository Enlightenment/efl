#include <Elementary.h>

#include "Eo.h"
#include "evas_obj.h"
#include "elw_box.h"
#include "elw_button.h"
#include "elw_boxedbutton.h"

#include "config.h"

typedef struct
{
//   Evas_Object *bx;
} Widget_Data;

#define MY_CLASS ELW_BOXEDBUTTON_CLASS

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_constructor_super(obj);

   Eo *bt = eo_add(ELW_BUTTON_CLASS, obj);
   eo_composite_object_attach(obj, bt);
   eo_event_callback_forwarder_add(bt, EV_CLICKED, obj);
   eo_do(bt, evas_obj_visibility_set(EINA_TRUE));

   eo_do(obj, elw_box_pack_end(bt));
   eo_unref(bt);
}

static const Eo_Class_Description class_desc = {
     "Elw BoxedButton",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     sizeof(Widget_Data),
     _constructor,
     NULL,
     NULL,
     NULL
};

EO_DEFINE_CLASS(elw_boxedbutton_class_get, &class_desc, ELW_BOX_CLASS, ELW_BUTTON_CLASS, NULL)

