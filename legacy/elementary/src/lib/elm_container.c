#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_container.h"

EAPI Eo_Op ELM_OBJ_CONTAINER_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_CONTAINER_CLASS

#define MY_CLASS_NAME "Elm_Container"

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET, "Set the content on part of a given container widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET, "Get the content on a part of a given container widget"),
     EO_OP_DESCRIPTION(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET, "Unset the content on a part of a given container widget"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_CONTAINER_BASE_ID, op_desc, ELM_OBJ_CONTAINER_SUB_ID_LAST),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(elm_obj_container_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, NULL);

