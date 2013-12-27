#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Eo2_Abstract_Class",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO2_CLASS_DESCRIPTION_NOOPS(),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(eo2_class_class_get, &class_desc, NULL, NULL)
