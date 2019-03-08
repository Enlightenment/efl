#ifndef ELM_INTERFACE_ATSPI_WIDGET_ACTION_H
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_H

#ifdef EFL_BETA_API_SUPPORT

struct _Efl_Access_Action_Data
{
   const char *name;
   const char *action;
   const char *param;
   Eina_Bool (*func)(Evas_Object *obj, const char *params);
};

typedef struct _Efl_Access_Action_Data Efl_Access_Action_Data;

#ifdef EFL_EO_API_SUPPORT
#include "efl_access_widget_action.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#ifndef _EFL_ACCESS_WIDGET_ACTION_EO_CLASS_TYPE
#define _EFL_ACCESS_WIDGET_ACTION_EO_CLASS_TYPE
typedef Eo Efl_Access_Widget_Action;
#endif
#endif

#endif
#endif
