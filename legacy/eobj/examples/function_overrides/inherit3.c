#include "Eobj.h"
#include "simple.h"

#include "inherit2.h"
#include "inherit3.h"

#include "config.h"

#define MY_CLASS INHERIT3_CLASS

static void
_a_set(Eobj *obj, void *class_data EINA_UNUSED, va_list *list)
{
   int a;
   a = va_arg(*list, int);
   printf("%s %d\n", eobj_class_name_get(MY_CLASS), a);
   eobj_do_super(obj, simple_a_set(a + 1));
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

static const Eobj_Class_Description class_desc = {
     "Inherit3",
     EOBJ_CLASS_TYPE_REGULAR,
     EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     NULL,
     NULL,
     _class_constructor,
     NULL
};

EOBJ_DEFINE_CLASS(inherit3_class_get, &class_desc, INHERIT2_CLASS, NULL);

