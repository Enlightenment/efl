#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple.h"
#include "constructors_simple4.h"

#define MY_CLASS SIMPLE4_CLASS

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple4",
     EFL_CLASS_TYPE_REGULAR,
     0,
     NULL,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple4_class_get, &class_desc, SIMPLE_CLASS, MIXIN_CLASS, NULL);

