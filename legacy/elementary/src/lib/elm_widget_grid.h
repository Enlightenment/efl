#ifndef ELM_WIDGET_GRID_H
#define ELM_WIDGET_GRID_H

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
 * @section elm-grid-class The Elementary Grid Class
 *
 * Elementary, besides having the @ref Grid widget, exposes its
 * foundation -- the Elementary Grid Class -- in order to create other
 * widgets which are a grid with some more logic on top.
 */

/**
 * @}
 */

#define ELM_GRID_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_GRID_CLASS))) \
    return

#endif
