#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "interface_interface.h"
#include "interface_simple.h"

#define MY_CLASS INTERFACE_CLASS

EFL_FUNC_BODY(interface_ab_sum_get, int, 0);

static Efl_Op_Description op_descs[] = {
     EFL_OBJECT_OP_FUNC(interface_ab_sum_get, NULL),
};

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Interface",
     EFL_CLASS_TYPE_INTERFACE,
     EFL_CLASS_DESCRIPTION_OPS(op_descs),
     0,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(interface_class_get, &class_desc, NULL, NULL)

