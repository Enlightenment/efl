#include "efl_ui_view_private.h"

#define MY_CLASS EFL_UI_VIEW_CLASS
#define MY_CLASS_NAME "Efl.Ui.View"

static const char SIG_MODEL_CHANGED[] = "model,changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_MODEL_CHANGED, ""},
   {NULL, NULL}
};

static Eo *
_efl_ui_view_model_get(Eo* obj, Efl_Ui_View_Data)
{
   return pd->model;
}

static void *
_efl_ui_view_model_set(Eo* obj, Efl_Ui_View_Data *pd, Eo *model)
{
   pd->model = model;
}
