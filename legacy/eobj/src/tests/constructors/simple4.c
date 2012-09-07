#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin.h"
#include "simple.h"
#include "simple4.h"

#define MY_CLASS SIMPLE4_CLASS

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple4",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple4_class_get, &class_desc, SIMPLE_CLASS, MIXIN_CLASS, NULL);

