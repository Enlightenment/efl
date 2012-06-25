#include "Eo.h"
#include "complex.h"

#include "config.h"

#define MY_CLASS COMPLEX_CLASS

static const Eo_Class_Description class_desc = {
     "Complex",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(complex_class_get, &class_desc, SIMPLE_CLASS, NULL);
