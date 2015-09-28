#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "composite_objects_simple.h"

EAPI const Eo_Event_Description _EV_A_CHANGED =
        EO_EVENT_DESCRIPTION("a,changed");

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

EAPI EO_VOID_FUNC_BODYV(simple_a_set1, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set2, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set3, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set4, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set5, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set6, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set7, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set8, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set9, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set10, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set11, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set12, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set13, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set14, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set15, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set16, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set17, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set18, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set19, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set20, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set21, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set22, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set23, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set24, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set25, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set26, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set27, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set28, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set29, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set30, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set31, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_set32, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get1, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get2, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get3, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get4, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get5, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get6, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get7, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get8, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get9, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get10, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get11, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get12, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get13, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get14, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get15, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get16, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get17, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get18, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get19, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get20, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get21, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get22, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get23, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get24, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get25, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get26, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get27, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get28, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get29, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get30, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get31, EO_FUNC_CALL(a), int a);
EAPI EO_VOID_FUNC_BODYV(simple_a_get32, EO_FUNC_CALL(a), int a);

/* XXX: This is fragile, and emulates many IDs in order to go to the next
 * op id chain (assuming chain size is as it is at the moment, 32).
 * This is needed in order to properly test some edge cases (see commit message
 * for more info). */
static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC(simple_a_set1, _a_set),
     EO_OP_FUNC(simple_a_set2, _a_set),
     EO_OP_FUNC(simple_a_set3, _a_set),
     EO_OP_FUNC(simple_a_set4, _a_set),
     EO_OP_FUNC(simple_a_set5, _a_set),
     EO_OP_FUNC(simple_a_set6, _a_set),
     EO_OP_FUNC(simple_a_set7, _a_set),
     EO_OP_FUNC(simple_a_set8, _a_set),
     EO_OP_FUNC(simple_a_set9, _a_set),
     EO_OP_FUNC(simple_a_set10, _a_set),
     EO_OP_FUNC(simple_a_set11, _a_set),
     EO_OP_FUNC(simple_a_set12, _a_set),
     EO_OP_FUNC(simple_a_set13, _a_set),
     EO_OP_FUNC(simple_a_set14, _a_set),
     EO_OP_FUNC(simple_a_set15, _a_set),
     EO_OP_FUNC(simple_a_set16, _a_set),
     EO_OP_FUNC(simple_a_set17, _a_set),
     EO_OP_FUNC(simple_a_set18, _a_set),
     EO_OP_FUNC(simple_a_set19, _a_set),
     EO_OP_FUNC(simple_a_set20, _a_set),
     EO_OP_FUNC(simple_a_set21, _a_set),
     EO_OP_FUNC(simple_a_set22, _a_set),
     EO_OP_FUNC(simple_a_set23, _a_set),
     EO_OP_FUNC(simple_a_set24, _a_set),
     EO_OP_FUNC(simple_a_set25, _a_set),
     EO_OP_FUNC(simple_a_set26, _a_set),
     EO_OP_FUNC(simple_a_set27, _a_set),
     EO_OP_FUNC(simple_a_set28, _a_set),
     EO_OP_FUNC(simple_a_set29, _a_set),
     EO_OP_FUNC(simple_a_set30, _a_set),
     EO_OP_FUNC(simple_a_set31, _a_set),
     EO_OP_FUNC(simple_a_set32, _a_set),
     EO_OP_FUNC(simple_a_get1, _a_get),
     EO_OP_FUNC(simple_a_get2, _a_get),
     EO_OP_FUNC(simple_a_get3, _a_get),
     EO_OP_FUNC(simple_a_get4, _a_get),
     EO_OP_FUNC(simple_a_get5, _a_get),
     EO_OP_FUNC(simple_a_get6, _a_get),
     EO_OP_FUNC(simple_a_get7, _a_get),
     EO_OP_FUNC(simple_a_get8, _a_get),
     EO_OP_FUNC(simple_a_get9, _a_get),
     EO_OP_FUNC(simple_a_get10, _a_get),
     EO_OP_FUNC(simple_a_get11, _a_get),
     EO_OP_FUNC(simple_a_get12, _a_get),
     EO_OP_FUNC(simple_a_get13, _a_get),
     EO_OP_FUNC(simple_a_get14, _a_get),
     EO_OP_FUNC(simple_a_get15, _a_get),
     EO_OP_FUNC(simple_a_get16, _a_get),
     EO_OP_FUNC(simple_a_get17, _a_get),
     EO_OP_FUNC(simple_a_get18, _a_get),
     EO_OP_FUNC(simple_a_get19, _a_get),
     EO_OP_FUNC(simple_a_get20, _a_get),
     EO_OP_FUNC(simple_a_get21, _a_get),
     EO_OP_FUNC(simple_a_get22, _a_get),
     EO_OP_FUNC(simple_a_get23, _a_get),
     EO_OP_FUNC(simple_a_get24, _a_get),
     EO_OP_FUNC(simple_a_get25, _a_get),
     EO_OP_FUNC(simple_a_get26, _a_get),
     EO_OP_FUNC(simple_a_get27, _a_get),
     EO_OP_FUNC(simple_a_get28, _a_get),
     EO_OP_FUNC(simple_a_get29, _a_get),
     EO_OP_FUNC(simple_a_get30, _a_get),
     EO_OP_FUNC(simple_a_get31, _a_get),
     EO_OP_FUNC(simple_a_get32, _a_get),
     EO_OP_FUNC(simple_a_set, _a_set),
     EO_OP_FUNC(simple_a_get, _a_get),
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

