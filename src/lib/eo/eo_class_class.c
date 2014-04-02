#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Eo_Abstract_Class",
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO_CLASS_DESCRIPTION_NOOPS(),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(eo_class_class_get, &class_desc, NULL, NULL)
