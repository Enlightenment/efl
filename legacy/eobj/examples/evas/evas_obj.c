#include <Elementary.h>

#include "Eobj.h"
#include "evas_obj.h"

#include "config.h"

static Eobj_Class *_my_class = NULL;

EAPI Eobj_Op EVAS_OBJ_BASE_ID = 0;

typedef struct
{
   Eina_List *children;
} Widget_Data;

static void
_position_set(Eobj *obj, void *class_data __UNUSED__, va_list *list)
{
   Evas_Object *evas_obj = eobj_evas_object_get(obj);
   Evas_Coord x, y;
   x = va_arg(*list, Evas_Coord);
   y = va_arg(*list, Evas_Coord);
   evas_object_move(evas_obj, x, y);
}

static void
_size_set(Eobj *obj, void *class_data __UNUSED__, va_list *list)
{
   Evas_Object *evas_obj = eobj_evas_object_get(obj);
   Evas_Coord w, h;
   w = va_arg(*list, Evas_Coord);
   h = va_arg(*list, Evas_Coord);
   evas_object_resize(evas_obj, w, h);
}

static void
_color_set(Eobj *obj, void *class_data __UNUSED__, va_list *list)
{
   Evas_Object *evas_obj = eobj_evas_object_get(obj);
   int r, g, b, a;
   r = va_arg(*list, int);
   g = va_arg(*list, int);
   b = va_arg(*list, int);
   a = va_arg(*list, int);
   evas_object_color_set(evas_obj, r, g, b, a);
}

static void
_color_get(Eobj *obj, void *class_data __UNUSED__, va_list *list)
{
   Evas_Object *evas_obj = eobj_evas_object_get(obj);
   int *r, *g, *b, *a;
   r = va_arg(*list, int *);
   g = va_arg(*list, int *);
   b = va_arg(*list, int *);
   a = va_arg(*list, int *);
   evas_object_color_get(evas_obj, r, g, b, a);
}

static void
_visibility_set(Eobj *obj, void *class_data __UNUSED__, va_list *list)
{
   Evas_Object *evas_obj = eobj_evas_object_get(obj);
   Eina_Bool v;
   v = va_arg(*list, int);
   if (v) evas_object_show(evas_obj);
   else evas_object_hide(evas_obj);
}

static void
_child_add(Eobj *obj __UNUSED__, void *class_data, va_list *list)
{
   Widget_Data *wd = class_data;
   Eobj *child;
   child = va_arg(*list, Eobj *);
   wd->children = eina_list_append(wd->children, eobj_ref(child));
}

static void
_constructor(Eobj *obj, void *class_data __UNUSED__)
{
   eobj_constructor_super(obj);

   /* Add type check. */
   Eobj *parent = eobj_parent_get(obj);
   if (parent)
      eobj_do(parent, EVAS_OBJ_CHILD_ADD(obj));
}

static void
_destructor(Eobj *obj, void *class_data)
{
   eobj_destructor_super(obj);

   Widget_Data *wd = class_data;

   Eobj *child;
   EINA_LIST_FREE(wd->children, child)
     {
        eobj_del(child);
     }
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POSITION_SET), _position_set),
        EOBJ_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_SET), _size_set),
        EOBJ_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_SET), _color_set),
        EOBJ_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_GET), _color_get),
        EOBJ_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_VISIBILITY_SET), _visibility_set),
        EOBJ_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CHILD_ADD), _child_add),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
evas_object_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_POSITION_SET, "ii", "Position of an evas object."),
        EOBJ_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_SIZE_SET, "ii", "Size of an evas object."),
        EOBJ_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_COLOR_SET, "iiii", "Color of an evas object."),
        EOBJ_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_COLOR_GET, "iiii", "Color of an evas object."),
        EOBJ_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_VISIBILITY_SET, "b", "Visibility of an evas object."),
        EOBJ_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_CHILD_ADD, "o", "Add a child eobj."),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Class_Description class_desc = {
        "Evas Object",
        EOBJ_CLASS_TYPE_REGULAR_NO_INSTANT,
        EOBJ_CLASS_DESCRIPTION_OPS(&EVAS_OBJ_BASE_ID, op_desc, EVAS_OBJ_SUB_ID_LAST),
        NULL,
        sizeof(Widget_Data),
        _constructor,
        _destructor,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, EOBJ_CLASS_BASE, NULL);
}
