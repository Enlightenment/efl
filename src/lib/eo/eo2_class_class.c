#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"

static Eo2_Op_Description op_descs [] = {
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Eo_Abstract_Class",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(eo2_class_class_get, &class_desc, NULL, NULL)
