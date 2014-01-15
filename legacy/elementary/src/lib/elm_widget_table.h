#ifndef ELM_WIDGET_TABLE_H
#define ELM_WIDGET_TABLE_H

#include "Elementary.h"

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
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_TABLE_CLASS))) \
    return

#endif
