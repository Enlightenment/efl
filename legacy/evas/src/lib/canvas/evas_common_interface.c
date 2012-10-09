#include "evas_common.h"
#include "evas_private.h"

#include "Eo.h"

EAPI Eo_Op EVAS_COMMON_BASE_ID = EO_NOOP;

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EVAS_COMMON_SUB_ID_EVAS_GET, "Return the evas parent attached to the object"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Evas_Common_Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(&EVAS_COMMON_BASE_ID, op_desc, EVAS_COMMON_SUB_ID_LAST),
     NULL,
     0,
     NULL,
     NULL,
};

EO_DEFINE_CLASS(evas_common_class_get, &class_desc, NULL, NULL)
