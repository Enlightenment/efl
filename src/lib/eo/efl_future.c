#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include "Eo.h"

// Efl.Future implementation is an opaque type in Ecore.
EOAPI const Efl_Event_Description _EFL_FUTURE_EVENT_FAILURE =
  EFL_EVENT_DESCRIPTION("future,failure");
EOAPI const Efl_Event_Description _EFL_FUTURE_EVENT_SUCCESS =
  EFL_EVENT_DESCRIPTION("future,success");
EOAPI const Efl_Event_Description _EFL_FUTURE_EVENT_PROGRESS =
  EFL_EVENT_DESCRIPTION("future,progress");

EOAPI EFL_FUNC_BODYV(efl_future_then, Efl_Future *, 0, EFL_FUNC_CALL(success, failure, progress, data), Efl_Event_Cb success, Efl_Event_Cb failure, Efl_Event_Cb progress, const void *data);
EOAPI EFL_VOID_FUNC_BODY(efl_future_cancel);

static Eina_Bool
_efl_future_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
                  EFL_OBJECT_OP_FUNC(efl_future_then, NULL),
                  EFL_OBJECT_OP_FUNC(efl_future_cancel, NULL));

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description _efl_future_class_desc = {
  EO_VERSION,
  "Efl_Future",
  EFL_CLASS_TYPE_REGULAR_NO_INSTANT,
  0,
  _efl_future_class_initializer,
  NULL,
  NULL
};

EFL_DEFINE_CLASS(efl_future_class_get, &_efl_future_class_desc, EFL_OBJECT_CLASS, NULL);

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
