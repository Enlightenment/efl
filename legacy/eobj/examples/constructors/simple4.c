#include "Eobj.h"
#include "mixin.h"
#include "simple.h"
#include "simple4.h"

#define MY_CLASS SIMPLE4_CLASS

static const Eobj_Class_Description class_desc = {
     "Simple4",
     EOBJ_CLASS_TYPE_REGULAR,
     EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL
};

EOBJ_DEFINE_CLASS(simple4_class_get, &class_desc, SIMPLE_CLASS, MIXIN_CLASS, NULL);

