#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_test_class_simple.h"

#define MY_CLASS SIMPLE_CLASS

EAPI const Eo_Event_Description _EV_A_CHANGED =
        EO_EVENT_DESCRIPTION("a,changed", "Called when a has changed.");

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, int a)
{
   Simple_Public_Data *pd = class_data;
   printf("%s %d\n", eo_class_name_get(MY_CLASS), a);
   pd->a = a;

   eo2_do(obj, eo2_event_callback_call(EV_A_CHANGED, &pd->a));
}

static int
_a_get(Eo *obj EINA_UNUSED, void *class_data)
{
   Simple_Public_Data *pd = class_data;

   return pd->a;
}

static Eina_Bool
_a_print(Eo *obj EINA_UNUSED, void *class_data)
{
   const Simple_Public_Data *pd = class_data;
   printf("Print %s %d\n", eo_class_name_get(MY_CLASS), pd->a);

   return EINA_TRUE;
}

static Eina_Bool
_class_hi_print(Eo_Class *klass, void *data EINA_UNUSED)
{
   printf("Hi Print %s\n", eo_class_name_get(klass));

   return EINA_TRUE;
}

static void
_dbg_info_get(Eo *eo_obj, void *_pd EINA_UNUSED, Eo_Dbg_Info *root)
{
   eo2_do_super(eo_obj, MY_CLASS, eo2_dbg_info_get(root));
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, "Test list");
   EO_DBG_INFO_APPEND(group, "Test", EINA_VALUE_TYPE_INT, 8);
}

EO2_VOID_FUNC_BODYV(simple_a_set, EO2_FUNC_CALL(a), int a);
EO2_FUNC_BODY(simple_a_get, int, 0);
EO2_FUNC_BODY(simple_a_print, Eina_Bool, EINA_FALSE);
EO2_FUNC_BODY(simple_class_hi_print, Eina_Bool, EINA_FALSE);

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC_OVERRIDE(_dbg_info_get, eo2_dbg_info_get),
     EO2_OP_FUNC(_a_set, simple_a_set, "Set property a"),
     EO2_OP_FUNC(_a_get, simple_a_get, "Get property a"),
     EO2_OP_FUNC(_a_print, simple_a_print, "Print property a"),
     EO2_OP_CLASS_FUNC(_class_hi_print, simple_class_hi_print, "Print property a"),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     sizeof(Simple_Public_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO2_BASE_CLASS, NULL)

