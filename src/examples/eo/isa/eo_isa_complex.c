#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_isa_complex.h"

#define MY_CLASS COMPLEX_CLASS

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Complex",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_NOOPS(),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(complex_class_get, &class_desc, SIMPLE_CLASS, NULL);
