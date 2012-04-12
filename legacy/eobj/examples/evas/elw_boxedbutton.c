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

static Eobj_Class *_my_class = NULL;

static void
_constructor(Eobj *obj, void *class_data __UNUSED__)
{
   eobj_constructor_super(obj);

   Eobj *bt = eobj_add(ELW_BUTTON_CLASS, obj);
   eobj_composite_object_attach(obj, bt);
   eobj_event_callback_forwarder_add(bt, SIG_CLICKED, obj);
   eobj_do(bt, EVAS_OBJ_VISIBILITY_SET(EINA_TRUE));

   eobj_do(obj, ELW_BOX_PACK_END(bt));
   eobj_unref(bt);
}

const Eobj_Class *
elw_boxedbutton_class_get(void)
{
   if (_my_class) return _my_class;

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

   return _my_class = eobj_class_new(&class_desc, ELW_BOX_CLASS,
         ELW_BUTTON_CLASS, NULL);
}

