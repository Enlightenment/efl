#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple.h"
#include "inherit.h"
#include "inherit2.h"

#include "../eunit_tests.h"

EAPI Eo_Op INHERIT2_BASE_ID = 0;

#define MY_CLASS INHERIT2_CLASS

static void
_a_set(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   int a;
   a = va_arg(*list, int);
   printf("%s %d\n", eo_class_name_get(MY_CLASS), a);
   eo_do(obj, simple_a_print());
   eo_do_super(obj, simple_a_set(a + 1));

   fail_if(!eo_do_super(obj, simple_a_print()));
}

static void
_print(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   printf("Hey\n");
   fail_if(eo_do_super(obj, inherit2_print()));
}

static void
_print2(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   printf("Hey2\n");
}

static void
_class_print(const Eo_Class *klass, va_list *list)
{
   (void) list;
   printf("Print %s-%s\n", eo_class_name_get(klass), eo_class_name_get(MY_CLASS));
   fail_if(!eo_class_do_super(klass, simple_class_print()));
   fail_if(!eo_class_do_super(klass, simple_class_print2()));
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EO_OP_FUNC(INHERIT2_ID(INHERIT2_SUB_ID_PRINT), _print),
        EO_OP_FUNC(INHERIT2_ID(INHERIT2_SUB_ID_PRINT2), _print2),
        EO_OP_FUNC_CLASS(SIMPLE_ID(SIMPLE_SUB_ID_CLASS_PRINT), _class_print),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(INHERIT2_SUB_ID_PRINT, "Print hey"),
     EO_OP_DESCRIPTION(INHERIT2_SUB_ID_PRINT2, "Print hey2"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Inherit2",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&INHERIT2_BASE_ID, op_desc, INHERIT2_SUB_ID_LAST),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(inherit2_class_get, &class_desc, INHERIT_CLASS, NULL);

