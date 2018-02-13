#ifndef EFL_UI_NSTATE_PRIVATE_H
#define EFL_UI_NSTATE_PRIVATE_H

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
 * @section efl-ui-nstate-class The Efl Ui Nstate Class
 *
 */

/**
 * Base widget smart data extended with nstate instance data.
 */
typedef struct _Efl_Ui_Nstate_Data
{
   int nstate;
   int state;
} Efl_Ui_Nstate_Data;
/**
 * @}
 */

#define EFL_UI_NSTATE_DATA_GET(o, sd) \
  Efl_Ui_Nstate_Data * sd = efl_data_scope_get(o, EFL_UI_NSTATE_CLASS)

#define EFL_UI_NSTATE_DATA_GET_OR_RETURN(o, ptr)     \
  EFL_UI_NSTATE_DATA_GET(o, ptr);                    \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_NSTATE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_NSTATE_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                \
    {                                                     \
       ERR("No widget data for object %p (%s)",           \
           o, evas_object_type_get(o));                   \
       return val;                                        \
    }

#define EFL_UI_NSTATE_CHECK(obj)                           \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_NSTATE_CLASS))) \
    return;

#endif
