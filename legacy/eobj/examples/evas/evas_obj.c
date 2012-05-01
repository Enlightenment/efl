#include <Elementary.h>

#include "Eo.h"
#include "evas_obj.h"

#include "config.h"

#define MY_CLASS EVAS_OBJ_CLASS

EAPI Eo_Op EVAS_OBJ_BASE_ID = 0;

typedef struct
{
   Eina_List *children;
} Widget_Data;

static void
_position_set(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   Evas_Object *evas_obj = eo_evas_object_get(obj);
   Evas_Coord x, y;
   x = va_arg(*list, Evas_Coord);
   y = va_arg(*list, Evas_Coord);
   evas_object_move(evas_obj, x, y);
}

static void
_size_set(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   Evas_Object *evas_obj = eo_evas_object_get(obj);
   Evas_Coord w, h;
   w = va_arg(*list, Evas_Coord);
   h = va_arg(*list, Evas_Coord);
   evas_object_resize(evas_obj, w, h);
}

static void
_color_set(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   Evas_Object *evas_obj = eo_evas_object_get(obj);
   int r, g, b, a;
   r = va_arg(*list, int);
   g = va_arg(*list, int);
   b = va_arg(*list, int);
   a = va_arg(*list, int);
   evas_object_color_set(evas_obj, r, g, b, a);
}

static void
_color_get(const Eo *obj, const void *class_data EINA_UNUSED, va_list *list)
{
   Evas_Object *evas_obj = eo_evas_object_get(obj);
   int *r, *g, *b, *a;
   r = va_arg(*list, int *);
   g = va_arg(*list, int *);
   b = va_arg(*list, int *);
   a = va_arg(*list, int *);
   evas_object_color_get(evas_obj, r, g, b, a);
}

static void
_visibility_set(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   Evas_Object *evas_obj = eo_evas_object_get(obj);
   Eina_Bool v;
   v = va_arg(*list, int);
   if (v) evas_object_show(evas_obj);
   else evas_object_hide(evas_obj);
}

static void
_child_add(Eo *obj, void *class_data, va_list *list)
{
   Widget_Data *wd = class_data;
   Eo *child;
   child = va_arg(*list, Eo *);
   wd->children = eina_list_append(wd->children, eo_xref(child, obj));
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_constructor_super(obj);

   /* Add type check. */
   Eo *parent = eo_parent_get(obj);
   if (parent)
      eo_do(parent, evas_obj_child_add(obj));
}

static void
_destructor(Eo *obj, void *class_data)
{
   eo_destructor_super(obj);

   Widget_Data *wd = class_data;

   Eo *child;
   EINA_LIST_FREE(wd->children, child)
     {
        eo_xunref(child, obj);
     }
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POSITION_SET), _position_set),
        EO_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_SET), _size_set),
        EO_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_SET), _color_set),
        EO_OP_FUNC_CONST(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_GET), _color_get),
        EO_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_VISIBILITY_SET), _visibility_set),
        EO_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CHILD_ADD), _child_add),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_POSITION_SET, "ii", "Position of an evas object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_SIZE_SET, "ii", "Size of an evas object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_COLOR_SET, "iiii", "Color of an evas object."),
     EO_OP_DESCRIPTION_CONST(EVAS_OBJ_SUB_ID_COLOR_GET, "iiii", "Color of an evas object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_VISIBILITY_SET, "b", "Visibility of an evas object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_SUB_ID_CHILD_ADD, "o", "Add a child eo."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     "Evas Object",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO_CLASS_DESCRIPTION_OPS(&EVAS_OBJ_BASE_ID, op_desc, EVAS_OBJ_SUB_ID_LAST),
     NULL,
     sizeof(Widget_Data),
     _constructor,
     _destructor,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(evas_object_class_get, &class_desc, EO_BASE_CLASS, NULL)
