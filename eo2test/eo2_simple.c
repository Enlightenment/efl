#include "Eo.h"
#include "eo2_simple.h"

EAPI const Eo_Event_Description _EO2_EV_X_CHANGED =
   EO_EVENT_DESCRIPTION("x,changed", "Called when x has changed.");

typedef struct
{
   int x;
} Private_Data;

static void
_inc(Eo *obj EINA_UNUSED, void *obj_data)
{
   Private_Data *data = (Private_Data *) obj_data;

   data->x += 1;
}
EAPI EO2_VOID_FUNC_BODY(eo2_inc);

static int
_get(Eo *obj EINA_UNUSED, void *obj_data)
{
   const Private_Data *data = (Private_Data *) obj_data;

   return data->x;
}
EAPI EO2_FUNC_BODY(eo2_get, int, -1);

static void
_set(Eo *obj EINA_UNUSED, void *obj_data, int x)
{
   Private_Data *data = (Private_Data *) obj_data;
   data->x = x;
}
EAPI EO2_VOID_FUNC_BODYV(eo2_set, EO2_FUNC_CALL(x), int x);

static void
_set_evt(Eo *obj, void *obj_data, int x)
{
   Private_Data *data = (Private_Data *) obj_data;
   data->x = x;
   eo2_do(obj, eo2_event_callback_call(EO2_EV_X_CHANGED, &data->x); );
}
EAPI EO2_VOID_FUNC_BODYV(eo2_set_evt, EO2_FUNC_CALL(x), int x);

static void
_class_hello(const Eo_Class *klass, int a)
{
   printf("Hello %d - body %s - EAPI %s\n", a, eo_class_name_get(klass), eo_class_name_get(EO2_SIMPLE_CLASS)); 
}
EAPI EO2_VOID_CLASS_FUNC_BODYV(eo2_class_hello, EO2_CLASS_FUNC_CALL(a), int a);

EAPI EO2_FUNC_BODYV(eo2_virtual, int, -1, EO2_FUNC_CALL(x), int x);

static void
_constructor(Eo *obj, void *obj_data, int x)
{
   Private_Data *data = (Private_Data *) obj_data;

   eo2_do_super(obj, eo2_constructor());

   data->x = x;
}
EAPI EO2_VOID_FUNC_BODYV(eo2_simple_constructor, EO2_FUNC_CALL(x), int x);

static void
_destructor(Eo *obj, void *obj_data EINA_UNUSED)
{
   eo2_do_super(obj, eo2_destructor());
}

static Eo2_Op_Description op_descs [] = {
       EO2_OP_FUNC(_constructor, eo2_simple_constructor, "Simple constructor"),
       EO2_OP_FUNC_OVERRIDE(_destructor, eo2_destructor),
       EO2_OP_FUNC(_inc, eo2_inc, "Inc X"),
       EO2_OP_FUNC(_get, eo2_get, "Get X"),
       EO2_OP_FUNC(_set, eo2_set, "Set X"),
       EO2_OP_FUNC(_set_evt, eo2_set_evt, "Set X + event"),
       EO2_OP_CLASS_FUNC(_class_hello, eo2_class_hello, "Class says hello"),
       EO2_OP_FUNC(NULL, eo2_virtual, "Virtual Func"),
       EO2_OP_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     EO2_EV_X_CHANGED,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Eo2 Simple",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     event_desc,
     sizeof(Private_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(eo2_simple_class_get, &class_desc, EO2_BASE_CLASS, NULL)

