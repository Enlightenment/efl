#include <Elementary.h>

#include "Eobj.h"
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
_constructor(Eobj *obj, void *class_data EINA_UNUSED)
{
   eobj_constructor_super(obj);

   Eobj *bt = eobj_add(ELW_BUTTON_CLASS, obj);
   eobj_composite_object_attach(obj, bt);
   eobj_event_callback_forwarder_add(bt, SIG_CLICKED, obj);
   eobj_do(bt, EVAS_OBJ_VISIBILITY_SET(EINA_TRUE));

   eobj_do(obj, ELW_BOX_PACK_END(bt));
   eobj_unref(bt);
}

static const Eobj_Class_Description class_desc = {
     "Elw BoxedButton",
     EOBJ_CLASS_TYPE_REGULAR,
     EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     sizeof(Widget_Data),
     _constructor,
     NULL,
     NULL,
     NULL
};

EOBJ_DEFINE_CLASS(elw_boxedbutton_class_get, &class_desc, ELW_BOX_CLASS, ELW_BUTTON_CLASS, NULL)

