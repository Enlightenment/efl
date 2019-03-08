#ifndef ELM_INTERFACE_ATSPI_TEXT_H
#define ELM_INTERFACE_ATSPI_TEXT_H

#ifdef EFL_BETA_API_SUPPORT

#ifdef EFL_EO_API_SUPPORT
#include "efl_access_text.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "efl_access_text_eo.legacy.h"
#endif

/**
 * @brief Free Efl_Access_Text_Attribute structure
 */
EAPI void elm_atspi_text_text_attribute_free(Efl_Access_Text_Attribute *attr);

/**
 * @brief Free Efl_Access_Text_Range structure
 */
EAPI void elm_atspi_text_text_range_free(Efl_Access_Text_Range *range);

#endif
#endif
