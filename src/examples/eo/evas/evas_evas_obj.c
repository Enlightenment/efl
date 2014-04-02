#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>

#include "Eo.h"
#include "evas_evas_obj.h"

#define MY_CLASS EXEVAS_OBJ_CLASS

EAPI Eo_Op EXEVAS_OBJ_BASE_ID = 0;

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
_color_get(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
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
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());

   /* Add type check. */
   Eo *parent = eo_parent_get(obj);
   if (parent)
      eo_do(parent, exevas_obj_child_add(obj));
}

static void
_destructor(Eo *obj, void *class_data, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());

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
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
        EO_OP_FUNC(EXEVAS_OBJ_ID(EXEVAS_OBJ_SUB_ID_POSITION_SET), _position_set),
        EO_OP_FUNC(EXEVAS_OBJ_ID(EXEVAS_OBJ_SUB_ID_SIZE_SET), _size_set),
        EO_OP_FUNC(EXEVAS_OBJ_ID(EXEVAS_OBJ_SUB_ID_COLOR_SET), _color_set),
        EO_OP_FUNC(EXEVAS_OBJ_ID(EXEVAS_OBJ_SUB_ID_COLOR_GET), _color_get),
        EO_OP_FUNC(EXEVAS_OBJ_ID(EXEVAS_OBJ_SUB_ID_VISIBILITY_SET), _visibility_set),
        EO_OP_FUNC(EXEVAS_OBJ_ID(EXEVAS_OBJ_SUB_ID_CHILD_ADD), _child_add),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EXEVAS_OBJ_SUB_ID_POSITION_SET, "Position of an evas object."),
     EO_OP_DESCRIPTION(EXEVAS_OBJ_SUB_ID_SIZE_SET, "Size of an evas object."),
     EO_OP_DESCRIPTION(EXEVAS_OBJ_SUB_ID_COLOR_SET, "Color of an evas object."),
     EO_OP_DESCRIPTION(EXEVAS_OBJ_SUB_ID_COLOR_GET, "Color of an evas object."),
     EO_OP_DESCRIPTION(EXEVAS_OBJ_SUB_ID_VISIBILITY_SET, "Visibility of an evas object."),
     EO_OP_DESCRIPTION(EXEVAS_OBJ_SUB_ID_CHILD_ADD, "Add a child eo."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Evas Object",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO_CLASS_DESCRIPTION_OPS(&EXEVAS_OBJ_BASE_ID, op_desc, EXEVAS_OBJ_SUB_ID_LAST),
     NULL,
     sizeof(Widget_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(evas_object_class_get, &class_desc, EO_CLASS, NULL)
