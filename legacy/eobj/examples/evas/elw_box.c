#include <Elementary.h>

#include "Eobj.h"
#include "evas_obj.h"
#include "elw_box.h"

#include "config.h"

EAPI Eobj_Op ELW_BOX_BASE_ID = 0;

typedef struct
{
   Evas_Object *bx;
} Widget_Data;

static Eobj_Class *_my_class = NULL;

static void
_pack_end(Eobj *obj __UNUSED__, void *class_data, va_list *list)
{
   Widget_Data *wd = class_data;
   Eobj *child_obj;
   child_obj = va_arg(*list, Eobj *);
   /* FIXME: Ref and the later uref child_obj here... */
   elm_box_pack_end(wd->bx, eobj_evas_object_get(child_obj));
}

static void
_constructor(Eobj *obj, void *class_data)
{
   eobj_constructor_super(obj);

   Widget_Data *wd = class_data;

   /* FIXME: An hack, because our tree is not yet only Eobj */
   wd->bx = elm_box_add(eobj_evas_object_get(eobj_parent_get(obj)));
   evas_object_size_hint_align_set(wd->bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(wd->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   eobj_evas_object_set(obj, wd->bx);
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC(ELW_BOX_ID(ELW_BOX_SUB_ID_PACK_END), _pack_end),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
elw_box_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(ELW_BOX_SUB_ID_PACK_END, "o", "Pack obj at the end of box."),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Class_Description class_desc = {
        "Elw Box",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(&ELW_BOX_BASE_ID, op_desc, ELW_BOX_SUB_ID_LAST),
        NULL,
        sizeof(Widget_Data),
        _constructor,
        NULL,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, EVAS_OBJ_CLASS, NULL);
}

