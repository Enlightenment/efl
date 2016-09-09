#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "composite_objects_simple.h"

EAPI const Efl_Event_Description _EV_A_CHANGED =
        EFL_EVENT_DESCRIPTION("a,changed");

#define MY_CLASS SIMPLE_CLASS

static void
_a_set(Eo *obj, void *class_data, int a)
{
   Simple_Public_Data *pd = class_data;
   printf("%s %d\n", efl_class_name_get(MY_CLASS), a);
   pd->a = a;

   efl_event_callback_legacy_call(obj, EV_A_CHANGED, &pd->a);
}

static int
_a_get(Eo *obj EINA_UNUSED, void *class_data)
{
   const Simple_Public_Data *pd = class_data;
   return pd->a;
}

EAPI EFL_VOID_FUNC_BODYV(simple_a_set, EFL_FUNC_CALL(a), int a);
EAPI EFL_FUNC_BODY(simple_a_get, int, 0);

EAPI EFL_VOID_FUNC_BODYV(simple_a_set1, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set2, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set3, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set4, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set5, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set6, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set7, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set8, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set9, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set10, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set11, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set12, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set13, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set14, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set15, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set16, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set17, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set18, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set19, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set20, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set21, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set22, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set23, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set24, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set25, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set26, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set27, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set28, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set29, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set30, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set31, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_set32, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get1, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get2, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get3, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get4, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get5, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get6, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get7, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get8, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get9, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get10, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get11, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get12, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get13, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get14, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get15, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get16, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get17, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get18, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get19, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get20, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get21, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get22, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get23, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get24, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get25, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get26, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get27, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get28, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get29, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get30, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get31, EFL_FUNC_CALL(a), int a);
EAPI EFL_VOID_FUNC_BODYV(simple_a_get32, EFL_FUNC_CALL(a), int a);


static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   /* XXX: This is fragile, and emulates many IDs in order to go to the next
    * op id chain (assuming chain size is as it is at the moment, 32).
    * This is needed in order to properly test some edge cases (see commit message
    * for more info). */
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(simple_a_set1, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set2, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set3, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set4, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set5, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set6, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set7, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set8, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set9, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set10, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set11, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set12, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set13, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set14, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set15, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set16, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set17, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set18, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set19, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set20, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set21, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set22, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set23, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set24, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set25, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set26, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set27, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set28, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set29, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set30, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set31, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_set32, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_get1, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get2, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get3, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get4, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get5, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get6, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get7, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get8, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get9, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get10, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get11, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get12, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get13, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get14, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get15, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get16, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get17, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get18, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get19, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get20, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get21, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get22, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get23, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get24, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get25, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get26, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get27, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get28, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get29, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get30, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get31, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_get32, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_get, _a_get),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Simple_Public_Data),
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL);

