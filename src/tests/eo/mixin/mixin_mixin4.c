#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin_mixin.h"
#include "mixin_mixin4.h"
#include "mixin_simple.h"

#include "../eunit_tests.h"

#define MY_CLASS MIXIN4_CLASS

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Mixin4",
     EFL_CLASS_TYPE_MIXIN,
     0,
     NULL,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(mixin4_class_get, &class_desc, NULL, NULL);

