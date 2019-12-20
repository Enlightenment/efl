#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Efl.h"

typedef struct _Efl_Model_Provider_Data Efl_Model_Provider_Data;
struct _Efl_Model_Provider_Data
{
   Efl_Model *model;
};

static void
_efl_model_provider_efl_ui_view_model_set(Eo *obj, Efl_Model_Provider_Data *pd,
                                          Efl_Model *model)
{
   Efl_Model_Changed_Event ev;

   ev.previous = efl_ref(pd->model);
   ev.current = efl_ref(model);
   efl_replace(&pd->model, model);

   efl_event_callback_call(obj, EFL_UI_VIEW_EVENT_MODEL_CHANGED, &ev);

   efl_unref(ev.previous);
   efl_unref(ev.current);
}

static Efl_Model *
_efl_model_provider_efl_ui_view_model_get(const Eo *obj EINA_UNUSED,
                                          Efl_Model_Provider_Data *pd)
{
   return pd->model;
}

#include "efl_model_provider.eo.c"
