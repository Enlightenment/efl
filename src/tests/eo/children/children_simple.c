#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "children_simple.h"

#define MY_CLASS SIMPLE_CLASS

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EFL_CLASS_TYPE_REGULAR,
     0,
     NULL,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple_class_get, &class_desc, EFL_OBJECT_CLASS, NULL)

