#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "access_simple.h"
#include "access_simple_protected.h"

typedef struct
{
   Simple_Protected_Data protected;
   int a;
} Private_Data;

EAPI const Eo_Event_Description _EV_A_CHANGED =
        EO_EVENT_DESCRIPTION("a,changed", "Called when a has changed.");

#define MY_CLASS SIMPLE_CLASS

static void
_a_set(Eo *obj, void *class_data, int a)
{
   Private_Data *pd = class_data;
   pd->a = a;
   printf("%s %d\n", __func__, pd->a);

   pd->protected.protected_x1 = a + 1;
   pd->protected.public.public_x2 = a + 2;

   eo2_do(obj, eo2_event_callback_call(EV_A_CHANGED, &pd->a));
}

EAPI EO2_VOID_FUNC_BODYV(simple_a_set, EO2_FUNC_CALL(a), int a);

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC(simple_a_set, _a_set, "Set property A"),
     EO2_OP_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     EV_A_CHANGED,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     event_desc,
     sizeof(Private_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO2_BASE_CLASS, NULL)

