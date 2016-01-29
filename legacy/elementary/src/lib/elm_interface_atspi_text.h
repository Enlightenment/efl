#ifndef ELM_INTERFACE_ATSPI_TEXT_H
#define ELM_INTERFACE_ATSPI_TEXT_H

#ifdef EFL_BETA_API_SUPPORT

#ifdef EFL_EO_API_SUPPORT
#include "elm_interface_atspi_text.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_interface_atspi_text.eo.legacy.h"
#endif

/**
 * @brief Free Elm_Atspi_Text_Attribute structure
 */
EAPI void elm_atspi_text_text_attribute_free(Elm_Atspi_Text_Attribute *attr);

#endif
#endif
