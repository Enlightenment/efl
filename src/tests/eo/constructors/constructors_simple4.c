#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple.h"
#include "constructors_simple4.h"

#define MY_CLASS SIMPLE4_CLASS

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple4",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_NOOPS(),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple4_class_get, &class_desc, SIMPLE_CLASS, MIXIN_CLASS, NULL);

