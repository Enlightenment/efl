#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>

#include "Eo.h"
#include "evas_obj.h"
#include "elw_box.h"
#include "elw_button.h"
#include "elw_boxedbutton.h"

typedef struct
{
//   Evas_Object *bx;
} Widget_Data;

#define MY_CLASS ELW_BOXEDBUTTON_CLASS

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());

   Eo *bt = eo_add(ELW_BUTTON_CLASS, obj);
   eo_composite_attach(bt, obj);
   eo_do(bt, eo_event_callback_forwarder_add(EV_CLICKED, obj));
   eo_do(bt, exevas_obj_visibility_set(EINA_TRUE));

   eo_do(obj, elw_box_pack_end(bt));
   eo_unref(bt);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Elw BoxedButton",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     sizeof(Widget_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elw_boxedbutton_class_get, &class_desc, ELW_BOX_CLASS, ELW_BUTTON_CLASS, NULL)

