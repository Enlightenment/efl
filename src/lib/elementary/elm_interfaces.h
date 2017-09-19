#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_text.h"
#include "elm_interface_atspi_widget_action.h"
#include "efl_access_window.h"

#ifdef EFL_EO_API_SUPPORT
#include "elm_interface_scrollable.h"
#ifdef EFL_BETA_API_SUPPORT
#include "efl_access_action.eo.h"
#include "efl_access_component.eo.h"
#include "elm_interface_atspi_text_editable.eo.h"
#include "efl_access_image.eo.h"
#include "efl_access_selection.eo.h"
#include "efl_access_value.eo.h"
#endif
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#ifdef EFL_BETA_API_SUPPORT
#include "efl_access_action.eo.legacy.h"
#include "efl_access_component.eo.legacy.h"
#include "elm_interface_atspi_text_editable.eo.legacy.h"
#include "efl_access_image.eo.legacy.h"
#include "efl_access_selection.eo.legacy.h"
#include "efl_access_value.eo.legacy.h"
#endif
#endif
