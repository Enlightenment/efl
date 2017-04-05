#ifndef ELM_INTERFACE_ATSPI_ACCESSIBLE_H
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_H

#ifdef EFL_BETA_API_SUPPORT

#ifdef EFL_EO_API_SUPPORT
#include "elm_interface_atspi_accessible.eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_interface_atspi_accessible.eo.legacy.h"
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
 * Free Elm_Atspi_Attributes_List
 */
EAPI void elm_atspi_attributes_list_free(Eina_List *list);

/**
 * Frees relation.
 */
EAPI void elm_atspi_relation_free(Elm_Atspi_Relation *relation);

/**
 * Clones relation.
 */
EAPI Elm_Atspi_Relation * elm_atspi_relation_clone(const Elm_Atspi_Relation *relation);

/**
 * Appends relation to relation set
 */
EAPI Eina_Bool elm_atspi_relation_set_relation_append(Elm_Atspi_Relation_Set *set, Elm_Atspi_Relation_Type type, const Eo *rel_obj);

/**
 * Removes relation from relation set
 */
EAPI void elm_atspi_relation_set_relation_remove(Elm_Atspi_Relation_Set *set, Elm_Atspi_Relation_Type type, const Eo *rel_obj);

/**
 * Removes all relation from relation set of a given type
 */
EAPI void elm_atspi_relation_set_relation_type_remove(Elm_Atspi_Relation_Set *set, Elm_Atspi_Relation_Type type);

/**
 * Frees Elm_Atspi_Relation_Set
 */
EAPI void elm_atspi_relation_set_free(Elm_Atspi_Relation_Set set);

/**
 * Clones Elm_Atspi_Relation_Set
 */
EAPI Elm_Atspi_Relation_Set elm_atspi_relation_set_clone(const Elm_Atspi_Relation_Set set);

#ifdef EFL_EO_API_SUPPORT

/**
 * Emits ATSPI 'StateChanged' event.
 */
#define elm_interface_atspi_accessible_state_changed_signal_emit(obj, tp, nvl) \
   { do { \
      Elm_Atspi_Event_State_Changed_Data evinfo; \
      evinfo.type = (tp); \
      evinfo.new_value = (nvl); \
      efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_STATE_CHANGED, (void*)&evinfo); \
   } while(0); }

/**
 * Emits ATSPI 'BoundsChanged' event.
 */
#define elm_interface_atspi_accessible_bounds_changed_signal_emit(obj, x, y, width, height) \
   do { \
         Elm_Atspi_Event_Geometry_Changed_Data evinfo = { x, y, width, height }; \
         efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_BOUNDS_CHANGED, (void*)&evinfo); \
   } while(0);

/**
 * Emits ATSPI 'PropertyChanged' event for 'Name' property.
 */
#define elm_interface_atspi_accessible_name_changed_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, "name");

/**
 * Emits ATSPI 'PropertyChanged' event for 'Description' property.
 */
#define elm_interface_atspi_accessible_description_changed_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, "description");

/**
 * Emits ATSPI 'PropertyChanged' event for 'Parent' property.
 */
#define elm_interface_atspi_accessible_parent_changed_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, "parent");

/**
 * Emits ATSPI 'PropertyChanged' event for 'Role' property.
 */
#define elm_interface_atspi_accessible_role_changed_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, "role");

/**
 * Emits ATSPI 'PropertyChanged' event for 'Value' property.
 */
#define elm_interface_atspi_accessible_value_changed_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, "value");

/**
 * Emits ATSPI 'ChildrenChanged' event with added child as argument.
 */
#define elm_interface_atspi_accessible_children_changed_added_signal_emit(obj, child) \
   do { \
      Elm_Atspi_Event_Children_Changed_Data atspi_data = { EINA_TRUE, child }; \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_CHILDREN_CHANGED, &atspi_data); \
   } while(0);

/**
 * Emits ATSPI 'ChildrenChanged' event with deleted child as argument.
 */
#define elm_interface_atspi_accessible_children_changed_del_signal_emit(obj, child) \
   do { \
      Elm_Atspi_Event_Children_Changed_Data atspi_data = { EINA_FALSE, child }; \
      efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_CHILDREN_CHANGED, &atspi_data); \
   } while(0);

/**
 * Emits ATSPI 'ActiveDescendantChanged' event.
 */
#define elm_interface_atspi_accessible_active_descendant_changed_signal_emit(obj, child) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_ACTIVE_DESCENDANT_CHANGED, child);

/**
 * Emits ATSPI 'VisibleDataChanged' event.
 */
#define elm_interface_atspi_accessible_visible_data_changed_signal_emit(obj) \
   efl_event_callback_call(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_VISIBLE_DATA_CHANGED, NULL);

#endif

#endif
#endif
