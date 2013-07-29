#include "Eo.h"
#include "eo2_simple.h"

typedef struct
{
   int x;
} Private_Data;

static void
_inc(Eo *objid EINA_UNUSED, void *obj_data)
{
   Private_Data *data = (Private_Data *) obj_data;

   data->x += 1;
}
EAPI EO2_VOID_FUNC_BODY(eo2_inc);

static int
_get(Eo *objid EINA_UNUSED, void *obj_data)
{
   const Private_Data *data = (Private_Data *) obj_data;

   return data->x;
}
EAPI EO2_FUNC_BODY(eo2_get, int, 0);

static void
_set(Eo *objid EINA_UNUSED, void *obj_data, int x)
{
   Private_Data *data = (Private_Data *) obj_data;
   data->x = x;
}
EAPI EO2_VOID_FUNC_BODYV(eo2_set, EO2_FUNC_CALL(x), int x);

EAPI EO2_FUNC_BODYV(eo2_virtual, int, EO2_FUNC_CALL(x), 0, int x);

static void
_constructor(Eo *obj, void *obj_data)
{
   Private_Data *data = (Private_Data *) obj_data;

   eo2_do_super(obj, eo2_constructor());

   data->x = 66;
}

static void
_destructor(Eo *obj, void *obj_data EINA_UNUSED)
{
   eo2_do_super(obj, eo2_destructor());
}

static Eo2_Op_Description op_descs [] = {
       EO2_OP_FUNC_OVERRIDE(_constructor, eo2_constructor),
       EO2_OP_FUNC_OVERRIDE(_destructor, eo2_destructor),
       EO2_OP_FUNC(_inc, eo2_inc, "Inc X"),
       EO2_OP_FUNC(_get, eo2_get, "Get X"),
       EO2_OP_FUNC(_set, eo2_set, "Set X"),
       EO2_OP_FUNC_VIRTUAL(eo2_virtual, "Virtual Func"),
       EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Eo2 Simple",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs, OP_DESC_SIZE(op_descs)),
     NULL,
     sizeof(Private_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(eo2_simple_class_get, &class_desc, EO2_BASE_CLASS, NULL)

