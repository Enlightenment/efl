#ifndef ELM_WIDGET_GRID_H
#define ELM_WIDGET_GRID_H

#include "Elementary.h"

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
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_GRID_CLASS))) \
    return

#endif
