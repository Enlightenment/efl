#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include "Eo.h"

// Efl.Future implementation is an opaque type in Ecore.
EOAPI const Eo_Event_Description _EFL_FUTURE_EVENT_FAILURE =
  EO_EVENT_DESCRIPTION("future,failure");
EOAPI const Eo_Event_Description _EFL_FUTURE_EVENT_SUCCESS =
  EO_EVENT_DESCRIPTION("future,success");
EOAPI const Eo_Event_Description _EFL_FUTURE_EVENT_PROGRESS =
  EO_EVENT_DESCRIPTION("future,progress");

EOAPI EO_FUNC_BODYV(efl_future_then, Efl_Future *, 0, EO_FUNC_CALL(success, failure, progress, data), Eo_Event_Cb success, Eo_Event_Cb failure, Eo_Event_Cb progress, const void *data);
EOAPI EO_VOID_FUNC_BODY(efl_future_cancel);

static const Eo_Op_Description _efl_future_op_desc[] = {
  EO_OP_FUNC(efl_future_then, NULL),
  EO_OP_FUNC(efl_future_cancel, NULL),
};

static const Eo_Event_Description *_efl_future_event_desc[] = {
  EFL_FUTURE_EVENT_FAILURE,
  EFL_FUTURE_EVENT_SUCCESS,
  EFL_FUTURE_EVENT_PROGRESS,
  NULL
};

static const Eo_Class_Description _efl_future_class_desc = {
  EO_VERSION,
  "Efl_Future",
  EO_CLASS_TYPE_REGULAR_NO_INSTANT,
  EO_CLASS_DESCRIPTION_OPS(_efl_future_op_desc),
  _efl_future_event_desc,
  0,
  NULL,
  NULL
};

EO_DEFINE_CLASS(efl_future_class_get, &_efl_future_class_desc, EO_BASE_CLASS, NULL);

static const char EINA_ERROR_FUTURE_CANCEL_STR[] = "Future cancelled";
EAPI Eina_Error EINA_ERROR_FUTURE_CANCEL;

Eina_Bool
efl_future_init(void)
{
   EINA_ERROR_FUTURE_CANCEL = eina_error_msg_static_register(EINA_ERROR_FUTURE_CANCEL_STR);

   return EINA_TRUE;
}

Eina_Bool
efl_future_shutdown(void)
{
   return EINA_TRUE;
}
