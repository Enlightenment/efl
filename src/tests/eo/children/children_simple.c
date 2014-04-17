#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "children_simple.h"

#define MY_CLASS SIMPLE_CLASS

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_NOOPS(),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL)

