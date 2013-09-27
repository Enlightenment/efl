#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_test_class_simple.h"

#define MY_CLASS SIMPLE_CLASS

EAPI Eo_Op SIMPLE_BASE_ID = 0;

EAPI const Eo_Event_Description _EV_A_CHANGED =
        EO_EVENT_DESCRIPTION("a,changed", "Called when a has changed.");

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Simple_Public_Data *pd = class_data;
   int a;
   a = va_arg(*list, int);
   printf("%s %d\n", eo_class_name_get(MY_CLASS), a);
   pd->a = a;

   eo_do(obj, eo_event_callback_call(EV_A_CHANGED, &pd->a, NULL));
}

static void
_a_print(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   const Simple_Public_Data *pd = class_data;
   (void) list;
   printf("Print %s %d\n", eo_class_name_get(MY_CLASS), pd->a);
}

static void
_class_hi_print(Eo_Class *klass, void *data EINA_UNUSED, va_list *list)
{
   (void) list;
   printf("Hi Print %s\n", eo_class_name_get(klass));
}

static void
_dbg_info_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eo_Dbg_Info *root = (Eo_Dbg_Info *) va_arg(*list, Eo_Dbg_Info *);
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, "Test list");
   EO_DBG_INFO_APPEND(group, "Test", EINA_VALUE_TYPE_INT, 8);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DBG_INFO_GET), _dbg_info_get),
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_PRINT), _a_print),
        EO_OP_FUNC_CLASS(SIMPLE_ID(SIMPLE_SUB_ID_CLASS_HI_PRINT), _class_hi_print),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(SIMPLE_SUB_ID_A_SET, "Set property A"),
     EO_OP_DESCRIPTION(SIMPLE_SUB_ID_A_PRINT, "Print property A"),
     EO_OP_DESCRIPTION_CLASS(SIMPLE_SUB_ID_CLASS_HI_PRINT, "Print Hi"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&SIMPLE_BASE_ID, op_desc, SIMPLE_SUB_ID_LAST),
     NULL,
     sizeof(Simple_Public_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_BASE_CLASS, NULL)

