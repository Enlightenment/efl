#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple.h"
#include "inherit2.h"
#include "inherit3.h"

#define MY_CLASS INHERIT3_CLASS

static void
_a_set(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   int a;
   a = va_arg(*list, int);
   printf("%s %d\n", eo_class_name_get(MY_CLASS), a);
   eo_do_super(obj, simple_a_set(a + 1));
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Inherit3",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(inherit3_class_get, &class_desc, INHERIT2_CLASS, NULL);

