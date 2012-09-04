#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>

#include "Eo.h"
#include "evas_obj.h"
#include "elw_box.h"

EAPI Eo_Op ELW_BOX_BASE_ID = 0;

typedef struct
{
   Evas_Object *bx;
} Widget_Data;

#define MY_CLASS ELW_BOX_CLASS

static void
_pack_end(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Widget_Data *wd = class_data;
   Eo *child_obj;
   child_obj = va_arg(*list, Eo *);
   /* FIXME: Ref and the later uref child_obj here... */
   elm_box_pack_end(wd->bx, eo_evas_object_get(child_obj));
}

static void
_constructor(Eo *obj, void *class_data, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());

   Widget_Data *wd = class_data;

   /* FIXME: An hack, because our tree is not yet only Eo */
   wd->bx = elm_box_add(eo_evas_object_get(eo_parent_get(obj)));
   evas_object_size_hint_align_set(wd->bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(wd->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   eo_evas_object_set(obj, wd->bx);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(ELW_BOX_ID(ELW_BOX_SUB_ID_PACK_END), _pack_end),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELW_BOX_SUB_ID_PACK_END, "Pack obj at the end of box."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Elw Box",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELW_BOX_BASE_ID, op_desc, ELW_BOX_SUB_ID_LAST),
     NULL,
     sizeof(Widget_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elw_box_class_get, &class_desc, EXEVAS_OBJ_CLASS, NULL)

