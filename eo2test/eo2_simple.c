#include "Eo.h"
#include "eo2_simple.h"

typedef struct
{
   int a;
   int b;
   int x;
} Private_Data;

static void
_inc(Eo *objid EINA_UNUSED, void *obj_data)
{
   Private_Data *data = (Private_Data *) obj_data;

   data->x += 1;
}
EAPI EO2_VOID_FUNC_BODY(inc2);

static int
_get(Eo *objid EINA_UNUSED, void *obj_data)
{
   const Private_Data *data = (Private_Data *) obj_data;

   return data->x;
}
EAPI EO2_FUNC_BODY(get2, int, 0);

static void
_set(Eo *objid EINA_UNUSED, void *obj_data, int x)
{
   Private_Data *data = (Private_Data *) obj_data;
   data->x = x;
}
EAPI EO2_VOID_FUNC_BODYV(set2, EO2_FUNC_CALL(x), int x);

static void
_constructor(Eo *obj, void *obj_data, va_list *list EINA_UNUSED)
{
   Private_Data *data = (Private_Data *) obj_data;

   // FIXME
   eo_do_super(obj, EO2_SIMPLE_CLASS, eo_constructor());
   data->x = 66;
}

static void
_destructor(Eo *obj, void *obj_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   // FIXME
   eo_do_super(obj, EO2_SIMPLE_CLASS, eo_destructor());
}

/* should use macro and replace op_desc[], can't be const because of OP*/
Eo2_Op_Description op_descs [] = {
       EO2_OP_CONSTRUCTOR(_constructor),
       EO2_OP_DESTRUCTOR(_destructor),
       EO2_OP_FUNC(_inc, inc2, "Inc X"),
       EO2_OP_FUNC(_get, get2, "Get X"),
       EO2_OP_FUNC(_set, set2, "Set X"),
       EO2_OP_SENTINEL
};

static void
_class_constructor(Eo_Class *klass)
{
   eo2_class_funcs_set(klass);
   // now op_descs is sorted by api_func, and class _dich is feed

}

/*********************************************************************************/
/* can be removed when Eo2_Op_Description is used instead of Eo_Op_Description   */
/* Eo_Op_Func_Description could also be removed                                  */
EAPI Eo_Op TEST_BASE_ID = 0;

enum {
     TEST_SUB_ID_inst_func_set,
     TEST_SUB_ID_inst_func_get,
     TEST_SUB_ID_inst_func_inc,
     TEST_SUB_ID_LAST
};

#define TEST_ID(func_name) (TEST_BASE_ID + (TEST_SUB_ID_ ## func_name))

/* this should be replaced by Eo2_Op_Description */
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(TEST_SUB_ID_inst_func_set, "Get"),
     EO_OP_DESCRIPTION(TEST_SUB_ID_inst_func_get, "Set"),
     EO_OP_DESCRIPTION(TEST_SUB_ID_inst_func_inc, "Inc"),
     EO_OP_DESCRIPTION_SENTINEL
};
/*********************************************************************************/

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Test",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&TEST_BASE_ID, op_desc, TEST_SUB_ID_LAST),
     NULL,
     sizeof(Private_Data),
     _class_constructor,
     NULL,
     op_descs
};

EO_DEFINE_CLASS(eo2_simple_class_get, &class_desc, EO_BASE_CLASS, NULL)

