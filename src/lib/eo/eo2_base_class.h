#ifndef _EO2_BASE_CLASS_H
#define _EO2_BASE_CLASS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
/* #include "eo_ptr_indirection.h" */
/* #include "eo_private.h" */

EAPI void
data_set(eo2_a, const char *key, const void *data, eo_base_data_free_func free_func);

EAPI void
data_get(eo2_a, const char *key);

EAPI void
data_del(eo2_a, const char *key);

EAPI void
wref_add(eo2_a, Eo **wref);

EAPI void
wref_del(eo2_a, Eo **wref);

EAPI void
ev_cb_priority_add(eo2_a, const Eo_Event_Description *desc,
                   Eo_Callback_Priority priority, Eo_Event_Cb func,
                   const void *user_data);

EAPI void
ev_cb_del(eo2_a, const Eo_Event_Description *desc, Eo_Event_Cb func,
          const void *user_data);

EAPI void
ev_cb_array_priority_add(eo2_a, const Eo_Callback_Array_Item *array,
                         Eo_Callback_Priority priority, const void *user_data);

EAPI void
ev_cb_array_del(eo2_a, const Eo_Callback_Array_Item *array,
                const void *user_data);

EAPI Eina_Bool
ev_cb_call(eo2_a, const Eo_Event_Description *desc, void *event_info);

EAPI void
ev_cb_forwarder_add(eo2_a, const Eo_Event_Description *desc, Eo *new_obj);

EAPI void
ev_cb_forwarder_del(eo2_a, const Eo_Event_Description *desc, Eo *new_obj);

EAPI void
ev_freeze(eo2_a);

EAPI void
ev_thaw(eo2_a);

EAPI int
ev_freeze_get(eo2_a);

EAPI void
ev_global_freeze(eo2_a);

EAPI void
ev_global_thaw(eo2_a);

EAPI int
ev_global_freeze_get(eo2_a);

EAPI void
dbg_info_get(eo2_a);

EAPI void
eo2_dbg_info_free(Eo_Dbg_Info *info);

EAPI const Eo_Class *eo2_base_class_get(void);
#define EO2_BASE_CLASS eo2_base_class_get()

#endif /* _EO2_BASE_CLASS_H */
