#ifndef _EO2_BASE_CLASS_H
#define _EO2_BASE_CLASS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
/* #include "eo_ptr_indirection.h" */
/* #include "eo_private.h" */

EAPI void
data_set(const char *key, const void *data, eo_base_data_free_func free_func);

EAPI void
data_get(const char *key);

EAPI void
data_del(const char *key);

EAPI void
wref_add(Eo **wref);

EAPI void
wref_del(Eo **wref);

EAPI void
ev_cb_priority_add(const Eo_Event_Description *desc,
                   Eo_Callback_Priority priority, Eo_Event_Cb func,
                   const void *user_data);

EAPI void
ev_cb_del(const Eo_Event_Description *desc, Eo_Event_Cb func,
          const void *user_data);

EAPI void
ev_cb_array_priority_add(const Eo_Callback_Array_Item *array,
                         Eo_Callback_Priority priority, const void *user_data);

EAPI void
ev_cb_array_del(const Eo_Callback_Array_Item *array,
                const void *user_data);

EAPI Eina_Bool
ev_cb_call(const Eo_Event_Description *desc, void *event_info);

EAPI void
ev_cb_forwarder_add(const Eo_Event_Description *desc, Eo *new_obj);

EAPI void
ev_cb_forwarder_del(const Eo_Event_Description *desc, Eo *new_obj);

EAPI void
ev_freeze();

EAPI void
ev_thaw();

EAPI int
ev_freeze_get();

EAPI void
ev_global_freeze();

EAPI void
ev_global_thaw();

EAPI int
ev_global_freeze_get();

EAPI void
dbg_info_get();

EAPI void
eo2_dbg_info_free(Eo_Dbg_Info *info);

EAPI const Eo_Class *eo2_base_class_get(void);
#define EO2_BASE_CLASS eo2_base_class_get()

#endif /* _EO2_BASE_CLASS_H */
