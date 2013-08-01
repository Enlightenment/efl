#include "Eo.h"
#include "eo2_simple.h"
#include "eo2_inherit.h"

typedef struct
{
   int y;
} Private_Data;

static void
_inc(Eo *objid, void *obj_data)
{
   Private_Data *data = (Private_Data *) obj_data;

   eo2_do_super(objid, eo2_inc());
   data->y += 1;
}

static int
_get(Eo *objid EINA_UNUSED, void *obj_data)
{
   const Private_Data *data = (Private_Data *) obj_data;

   return data->y;
}
EAPI EO2_FUNC_BODY(eo2_inherit_get, int, -2);

static void
_class_hello(const Eo_Class *klass, int a)
{
   printf("Hello %d - body %s - EAPI %s\n", a, eo_class_name_get(klass), eo_class_name_get(EO2_SIMPLE_CLASS)); 

   eo2_class_super_do(klass, eo2_class_hello(a * 2));
}

static int
_virtual(Eo *objid EINA_UNUSED, void *obj_data EINA_UNUSED, int in)
{
   return (in * 2);
}

static void
_constructor(Eo *obj, void *obj_data)
{
   Private_Data *data = (Private_Data *) obj_data;

   eo2_do_super(obj, eo2_simple_constructor(66));

   data->y = 68;
}

static void
_destructor(Eo *obj, void *obj_data EINA_UNUSED)
{
   eo2_do_super(obj, eo2_destructor());
}

static Eo2_Op_Description op_descs [] = {
       EO2_OP_FUNC_OVERRIDE(_constructor, eo2_constructor),
       EO2_OP_FUNC_OVERRIDE(_destructor, eo2_destructor),
       EO2_OP_FUNC_OVERRIDE(_inc, eo2_inc),
       EO2_OP_FUNC_OVERRIDE(_virtual, eo2_virtual),
       EO2_OP_CLASS_FUNC_OVERRIDE(_class_hello, eo2_class_hello),
       EO2_OP_FUNC(_get, eo2_inherit_get, "Get Y"),
       EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Eo2 Inherit",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     sizeof(Private_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(eo2_inherit_class_get, &class_desc, EO2_SIMPLE_CLASS, NULL)

