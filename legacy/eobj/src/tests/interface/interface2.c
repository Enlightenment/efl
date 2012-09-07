#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "interface.h"
#include "interface2.h"
#include "simple.h"

EAPI Eo_Op INTERFACE2_BASE_ID = 0;

#define MY_CLASS INTERFACE2_CLASS

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(INTERFACE2_SUB_ID_AB_SUM_GET2, "Print the sum of a and b."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Interface2",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(&INTERFACE2_BASE_ID, op_desc, INTERFACE2_SUB_ID_LAST),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(interface2_class_get, &class_desc, INTERFACE_CLASS, NULL)

