#include <Elementary.h>

#include "eobj.h"
#include "evas_obj.h"
#include "elw_button.h"

EAPI Eobj_Op ELW_BUTTON_BASE_ID = 0;

EAPI const Eobj_Event_Description _SIG_CLICKED =
        EOBJ_EVENT_DESCRIPTION("clicked", "", "Called when there was a click.");

typedef struct
{
   Evas_Object *bt;
} Widget_Data;

static Eobj_Class *_my_class = NULL;

static void
_position_set(Eobj *obj, Eobj_Op op, va_list *list)
{
   (void) op;
   (void) obj;
   Evas_Coord x, y;
   x = va_arg(*list, Evas_Coord);
   y = va_arg(*list, Evas_Coord);
   printf("But set position %d,%d\n", x, y);
   eobj_class_parent_do(obj, _my_class, EVAS_OBJ_POSITION_SET(x, y));
}

static void
_text_set(Eobj *obj, Eobj_Op op, va_list *list)
{
   Widget_Data *wd = eobj_data_get(obj, _my_class);
   (void) op;
   const char *text;
   text = va_arg(*list, const char *);
   elm_object_text_set(wd->bt, text);
}

static void
_btn_clicked(void *data, Evas_Object *evas_obj, void *event_info)
{
   (void) evas_obj;
   (void) event_info;
   Eobj *obj = data;
   eobj_event_callback_call(obj, SIG_CLICKED, NULL);
}

static void
_constructor(Eobj *obj)
{
   eobj_constructor_super(obj);

   Widget_Data *wd = eobj_data_get(obj, _my_class);

   /* FIXME: An hack, because our tree is not yet only Eobj */
   wd->bt = elm_button_add(eobj_evas_object_get(eobj_parent_get(obj)));
   evas_object_size_hint_align_set(wd->bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(wd->bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(wd->bt, "clicked", _btn_clicked, obj);

   eobj_evas_object_set(obj, wd->bt);
}

static void
_destructor(Eobj *obj)
{
   eobj_destructor_super(obj);

   //Widget_Data *wd = eobj_data_get(obj, _my_class);
   /* FIXME: Commented out because it's automatically done because our tree
    * is not made of only eobj */
   //evas_object_del(wd->bt);
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC_DESCRIPTION(ELW_BUTTON_ID(ELW_BUTTON_SUB_ID_TEXT_SET), _text_set),
        EOBJ_OP_FUNC_DESCRIPTION(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POSITION_SET), _position_set),
        EOBJ_OP_FUNC_DESCRIPTION_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
elw_button_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(ELW_BUTTON_SUB_ID_TEXT_SET, "s", "Text of a text supporting evas object."), // FIXME: This ID sholudn't really be defined here...
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Event_Description *event_desc[] = {
        SIG_CLICKED,
        NULL
   };

   static const Eobj_Class_Description class_desc = {
        "Elw Button",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(&ELW_BUTTON_BASE_ID, op_desc, ELW_BUTTON_SUB_ID_LAST),
        event_desc,
        sizeof(Widget_Data),
        _constructor,
        _destructor,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, EVAS_OBJ_CLASS, NULL);
}

