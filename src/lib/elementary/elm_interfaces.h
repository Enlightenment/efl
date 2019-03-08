#include "efl_access_object.h"
#include "efl_access_text.h"
#include "efl_access_widget_action.h"
#include "efl_access_window.h"

// FIXME FIXME FIXME -- Still required because of photocam
#ifdef EFL_EO_API_SUPPORT
# include "elm_interface_scrollable.h"
#endif

#ifdef EFL_BETA_API_SUPPORT
#ifdef EFL_EO_API_SUPPORT
#include "efl_access_action.eo.h"
#include "efl_access_component.eo.h"
#include "efl_access_editable_text.eo.h"
#include "efl_access_selection.eo.h"
#include "efl_access_value.eo.h"
#include "efl_ui_legacy.eo.h"
#endif

#ifndef EFL_NOLEGACY_API_SUPPORT
#ifndef _EFL_ACCESS_ACTION_EO_CLASS_TYPE
#define _EFL_ACCESS_ACTION_EO_CLASS_TYPE
typedef Eo Efl_Access_Action;
#endif
#ifndef _EFL_ACCESS_COMPONENT_EO_CLASS_TYPE
#define _EFL_ACCESS_COMPONENT_EO_CLASS_TYPE
typedef Eo Efl_Access_Component;
#endif
#ifndef _EFL_ACCESS_EDITABLE_TEXT_EO_CLASS_TYPE
#define _EFL_ACCESS_EDITABLE_TEXT_EO_CLASS_TYPE
typedef Eo Efl_Access_Editable_Text;
#endif
#ifndef _EFL_ACCESS_SELECTION_EO_CLASS_TYPE
#define _EFL_ACCESS_SELECTION_EO_CLASS_TYPE
typedef Eo Efl_Access_Selection;
#endif
#ifndef _EFL_ACCESS_VALUE_EO_CLASS_TYPE
#define _EFL_ACCESS_VALUE_EO_CLASS_TYPE
typedef Eo Efl_Access_Value;
#endif
#ifndef _EFL_UI_LEGACY_EO_CLASS_TYPE
#define _EFL_UI_LEGACY_EO_CLASS_TYPE
typedef Eo Efl_Ui_Legacy;
#endif
#endif

#endif
