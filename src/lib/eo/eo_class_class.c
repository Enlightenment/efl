#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"

EAPI Eo_Op EO_CLASS_CLASS_BASE_ID = 0;

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Eo_Abstract_Class",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO_CLASS_DESCRIPTION_OPS(&EO_CLASS_CLASS_BASE_ID, NULL, EO_CLASS_CLASS_SUB_ID_LAST),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(eo_class_class_get, &class_desc, NULL, NULL)
