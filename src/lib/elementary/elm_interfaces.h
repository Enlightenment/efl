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
#include "efl_access_action.eo.legacy.h"
#include "efl_access_component.eo.legacy.h"
#include "efl_access_editable_text.eo.legacy.h"
#include "efl_access_selection.eo.legacy.h"
#include "efl_access_value.eo.legacy.h"
#include "efl_ui_legacy.eo.legacy.h"
#endif
#endif
