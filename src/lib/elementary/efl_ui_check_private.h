#ifndef EFL_UI_CHECK_PRIVATE_H
#define EFL_UI_CHECK_PRIVATE_H

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
 * @section elm-check-class The Elementary Check Class
 *
 * Elementary, besides having the @ref Check widget, exposes its
 * foundation -- the Elementary Check Class -- in order to create other
 * widgets which are a check with some more logic on top.
 */

/**
 * Base layout smart data extended with check instance data.
 */
typedef struct _Efl_Ui_Check_Data Efl_Ui_Check_Data;
struct _Efl_Ui_Check_Data
{
   Eina_Bool            *statep;
   Eina_Bool            selected;
   Eina_Bool            legacy_changed_emitted_unselect : 1;
   Eina_Bool            legacy_changed_emitted_select : 1;
};

/**
 * @}
 */

#define EFL_UI_CHECK_DATA_GET(o, sd) \
  Efl_Ui_Check_Data * sd = efl_data_scope_get(o, EFL_UI_CHECK_CLASS)

#define EFL_UI_CHECK_DATA_GET_OR_RETURN(o, ptr)         \
  EFL_UI_CHECK_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_CHECK_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_CHECK_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define EFL_UI_CHECK_CHECK(obj)                       \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_CHECK_CLASS))) \
    return

#endif
