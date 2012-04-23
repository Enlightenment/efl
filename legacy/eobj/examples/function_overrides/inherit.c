#include "Eobj.h"
#include "simple.h"

#include "inherit.h"

#define MY_CLASS INHERIT_CLASS

static const Eobj_Class_Description class_desc = {
     "Inherit",
     EOBJ_CLASS_TYPE_REGULAR,
     EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL
};

EOBJ_DEFINE_CLASS(inherit_class_get, &class_desc, SIMPLE_CLASS, NULL);
