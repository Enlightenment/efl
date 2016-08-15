#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "interface_interface.h"
#include "interface_interface2.h"
#include "interface_simple.h"

#define MY_CLASS INTERFACE2_CLASS

EFL_FUNC_BODY(interface2_ab_sum_get2, int, 0);

static Efl_Op_Description op_descs[] = {
     EFL_OBJECT_OP_FUNC(interface2_ab_sum_get2, NULL),
};

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Interface2",
     EFL_CLASS_TYPE_INTERFACE,
     EFL_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(interface2_class_get, &class_desc, INTERFACE_CLASS, NULL)

