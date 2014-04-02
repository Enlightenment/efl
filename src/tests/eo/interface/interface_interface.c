#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "interface_interface.h"
#include "interface_simple.h"

#define MY_CLASS INTERFACE_CLASS

EO_FUNC_BODY(interface_ab_sum_get, int, 0);

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC(interface_ab_sum_get, NULL, "Get the sum of a and b."),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(interface_class_get, &class_desc, NULL, NULL)

