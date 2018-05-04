#ifndef EFL_ACCESS_H
#define EFL_ACCESS_H

#ifdef EFL_BETA_API_SUPPORT

#ifdef EFL_EO_API_SUPPORT
#include "efl_access_object.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "efl_access_object.eo.legacy.h"
#endif

/*
 * Sets a particilar state type for given state set.
 */
#define STATE_TYPE_SET(state_set, type)   (state_set|= (1ULL << type))

/**
 * Unsets a particilar state type for given state set.
 */
#define STATE_TYPE_UNSET(state_set, type) (state_set &= ~(1ULL << type))

/**
 * Gets value of a particilar state type for given state set.
 */
#define STATE_TYPE_GET(state_set, type)   (state_set & (1ULL << type))

/**
 * Free Efl_Access_Attributes_List
 */
EAPI void efl_access_attributes_list_free(Eina_List *list);

#ifdef EFL_EO_API_SUPPORT

/**
 * Emits Accessible 'StateChanged' signal.
 */
#define efl_access_state_changed_signal_emit(obj, tp, nvl) \
   { do { \
      Efl_Access_Event_State_Changed_Data evinfo; \
      evinfo.type = (tp); \
      evinfo.new_value = (nvl); \
      efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_STATE_CHANGED, (void*)&evinfo); \
   } while(0); }

/**
 * Emits Accessible 'BoundsChanged' signal.
 */
#define efl_access_bounds_changed_signal_emit(obj, x, y, width, height) \
   do { \
         Efl_Access_Event_Geometry_Changed_Data evinfo = { x, y, width, height }; \
         efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_BOUNDS_CHANGED, (void*)&evinfo); \
   } while(0);

/**
 * Emits Accessible 'PropertyChanged' signal for 'Name' property.
 */
#define efl_access_i18n_name_changed_signal_emit(obj) \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_PROPERTY_CHANGED, "i18n_name");

/**
 * Emits Accessible 'PropertyChanged' signal for 'Description' property.
 */
#define efl_access_description_changed_signal_emit(obj) \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_PROPERTY_CHANGED, "description");

/**
 * Emits Accessible 'PropertyChanged' signal for 'Parent' property.
 */
#define efl_access_parent_changed_signal_emit(obj) \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_PROPERTY_CHANGED, "parent");

/**
 * Emits Accessible 'PropertyChanged' signal for 'Role' property.
 */
#define efl_access_role_changed_signal_emit(obj) \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_PROPERTY_CHANGED, "role");

/**
 * Emits Accessible 'PropertyChanged' signal for 'Value' property.
 */
#define efl_access_value_changed_signal_emit(obj) \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_PROPERTY_CHANGED, "value");

/**
 * Emits Accessible 'ChildrenChanged' signal with added child as argument.
 */
#define efl_access_children_changed_added_signal_emit(obj, child) \
   do { \
      Efl_Access_Event_Children_Changed_Data atspi_data = { EINA_TRUE, child }; \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_CHILDREN_CHANGED, &atspi_data); \
   } while(0);

/**
 * Emits Accessible 'ChildrenChanged' signal with deleted child as argument.
 */
#define efl_access_children_changed_del_signal_emit(obj, child) \
   do { \
      Efl_Access_Event_Children_Changed_Data atspi_data = { EINA_FALSE, child }; \
      efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_CHILDREN_CHANGED, &atspi_data); \
   } while(0);

/**
 * Emits Accessible 'ActiveDescendantChanged' signal.
 */
#define efl_access_active_descendant_changed_signal_emit(obj, child) \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_ACTIVE_DESCENDANT_CHANGED, child);

/**
 * Emits Accessible 'VisibleDataChanged' signal.
 */
#define efl_access_visible_data_changed_signal_emit(obj) \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_VISIBLE_DATA_CHANGED, NULL);

/**
 * Emits Accessible 'AddAccessible' signal.
 */
#define efl_access_added(obj) \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_ADDED, NULL);

/**
 * Emits Accessible 'RemoveAccessible' signal.
 */
#define efl_access_removed(obj) \
   efl_access_object_event_emit(EFL_ACCESS_OBJECT_MIXIN, obj, EFL_ACCESS_OBJECT_EVENT_REMOVED, NULL);

#endif

#endif
#endif
