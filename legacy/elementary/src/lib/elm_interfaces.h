#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_text.h"
#include "elm_interface_atspi_widget_action.h"

#ifdef EFL_EO_API_SUPPORT
#include "elm_interface_scrollable.h"
#ifdef EFL_BETA_API_SUPPORT
#include "elm_interface_atspi_action.eo.h"
#include "elm_interface_atspi_component.eo.h"
#include "elm_interface_atspi_editable_text.eo.h"
#include "elm_interface_atspi_image.eo.h"
#include "elm_interface_atspi_selection.eo.h"
#include "elm_interface_atspi_value.eo.h"
#include "elm_interface_atspi_window.eo.h"
#endif
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#ifdef EFL_BETA_API_SUPPORT
#include "elm_interface_atspi_action.eo.legacy.h"
#include "elm_interface_atspi_component.eo.legacy.h"
#include "elm_interface_atspi_editable_text.eo.legacy.h"
#include "elm_interface_atspi_image.eo.legacy.h"
#include "elm_interface_atspi_selection.eo.legacy.h"
#include "elm_interface_atspi_value.eo.legacy.h"
#include "elm_interface_atspi_window.eo.legacy.h"
#endif
#endif
