#ifndef ELM_WIDGET_TABLE_H
#define ELM_WIDGET_TABLE_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-table-class The Elementary Table Class
 *
 * Elementary, besides having the @ref Table widget, exposes its
 * foundation -- the Elementary Table Class -- in order to create
 * other widgets which are a table with some more logic on top.
 */

/**
 * @}
 */

#define ELM_TABLE_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_TABLE_CLASS))) \
    return

#endif
