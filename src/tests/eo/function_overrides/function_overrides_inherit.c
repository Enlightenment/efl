#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "function_overrides_simple.h"
#include "function_overrides_inherit.h"

#define MY_CLASS INHERIT_CLASS

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Inherit",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_NOOPS(),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(inherit_class_get, &class_desc, SIMPLE_CLASS, NULL);
