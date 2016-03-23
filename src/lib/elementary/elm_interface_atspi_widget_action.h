#ifndef ELM_INTERFACE_ATSPI_WIDGET_ACTION_H
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_H

#ifdef EFL_BETA_API_SUPPORT

struct _Elm_Atspi_Action
{
   const char *name;
   const char *action;
   const char *param;
   Eina_Bool (*func)(Evas_Object *obj, const char *params);
};

typedef struct _Elm_Atspi_Action Elm_Atspi_Action;

#ifdef EFL_EO_API_SUPPORT
#include "elm_interface_atspi_widget_action.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_interface_atspi_widget_action.eo.legacy.h"
#endif

#endif
#endif
