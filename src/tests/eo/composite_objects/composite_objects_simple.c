#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "composite_objects_simple.h"

EAPI const Eo_Event_Description _EV_A_CHANGED =
        EO_EVENT_DESCRIPTION("a,changed", "Called when a has changed.");

#define MY_CLASS SIMPLE_CLASS

static void
_a_set(Eo *obj, void *class_data, int a)
{
   Simple_Public_Data *pd = class_data;
   printf("%s %d\n", eo_class_name_get(MY_CLASS), a);
   pd->a = a;

   eo_do(obj, eo_event_callback_call(EV_A_CHANGED, &pd->a));
}

static int
_a_get(Eo *obj EINA_UNUSED, void *class_data)
{
   const Simple_Public_Data *pd = class_data;
   return pd->a;
}

EAPI EO_VOID_FUNC_BODYV(simple_a_set, EO_FUNC_CALL(a), int a);
EAPI EO_FUNC_BODY(simple_a_get, int, 0);

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC(simple_a_set, _a_set, "Set property A"),
     EO_OP_FUNC(simple_a_get, _a_get, "Get property A"),
     EO_OP_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     EV_A_CHANGED,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     event_desc,
     sizeof(Simple_Public_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL);

