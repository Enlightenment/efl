#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED

#include <Elementary.h>
#include "atspi/atspi-constants.h"
#include "elm_priv.h"
#include "elm_atspi_adaptor_common.h"

#define SIZE(x) sizeof(x)/sizeof(x[0])

static const int elm_roles_to_atspi_roles[][2] = {
   { EFL_ACCESS_ROLE_INVALID, ATSPI_ROLE_INVALID },
   { EFL_ACCESS_ROLE_ACCELERATOR_LABEL, ATSPI_ROLE_ACCELERATOR_LABEL },
   { EFL_ACCESS_ROLE_ALERT, ATSPI_ROLE_ALERT },
   { EFL_ACCESS_ROLE_ANIMATION, ATSPI_ROLE_ANIMATION },
   { EFL_ACCESS_ROLE_ARROW, ATSPI_ROLE_ARROW },
   { EFL_ACCESS_ROLE_CALENDAR, ATSPI_ROLE_CALENDAR },
   { EFL_ACCESS_ROLE_CANVAS, ATSPI_ROLE_CANVAS },
   { EFL_ACCESS_ROLE_CHECK_BOX, ATSPI_ROLE_CHECK_BOX },
   { EFL_ACCESS_ROLE_CHECK_MENU_ITEM, ATSPI_ROLE_CHECK_MENU_ITEM },
   { EFL_ACCESS_ROLE_COLOR_CHOOSER, ATSPI_ROLE_COLOR_CHOOSER },
   { EFL_ACCESS_ROLE_COLUMN_HEADER, ATSPI_ROLE_COLUMN_HEADER },
   { EFL_ACCESS_ROLE_COMBO_BOX, ATSPI_ROLE_COMBO_BOX },
   { EFL_ACCESS_ROLE_DATE_EDITOR, ATSPI_ROLE_DATE_EDITOR },
   { EFL_ACCESS_ROLE_DESKTOP_ICON, ATSPI_ROLE_DESKTOP_ICON },
   { EFL_ACCESS_ROLE_DESKTOP_FRAME, ATSPI_ROLE_DESKTOP_FRAME },
   { EFL_ACCESS_ROLE_DIAL, ATSPI_ROLE_DIAL },
   { EFL_ACCESS_ROLE_DIALOG, ATSPI_ROLE_DIALOG },
   { EFL_ACCESS_ROLE_DIRECTORY_PANE, ATSPI_ROLE_DIRECTORY_PANE },
   { EFL_ACCESS_ROLE_DRAWING_AREA, ATSPI_ROLE_DRAWING_AREA },
   { EFL_ACCESS_ROLE_FILE_CHOOSER, ATSPI_ROLE_FILE_CHOOSER },
   { EFL_ACCESS_ROLE_FILLER, ATSPI_ROLE_FILLER },
   { EFL_ACCESS_ROLE_FOCUS_TRAVERSABLE, ATSPI_ROLE_FOCUS_TRAVERSABLE },
   { EFL_ACCESS_ROLE_FONT_CHOOSER, ATSPI_ROLE_FONT_CHOOSER },
   { EFL_ACCESS_ROLE_FRAME, ATSPI_ROLE_FRAME },
   { EFL_ACCESS_ROLE_GLASS_PANE, ATSPI_ROLE_GLASS_PANE },
   { EFL_ACCESS_ROLE_HTML_CONTAINER, ATSPI_ROLE_HTML_CONTAINER },
   { EFL_ACCESS_ROLE_ICON, ATSPI_ROLE_ICON },
   { EFL_ACCESS_ROLE_IMAGE, ATSPI_ROLE_IMAGE },
   { EFL_ACCESS_ROLE_INTERNAL_FRAME, ATSPI_ROLE_INTERNAL_FRAME },
   { EFL_ACCESS_ROLE_LABEL, ATSPI_ROLE_LABEL },
   { EFL_ACCESS_ROLE_LAYERED_PANE, ATSPI_ROLE_LAYERED_PANE },
   { EFL_ACCESS_ROLE_LIST, ATSPI_ROLE_LIST },
   { EFL_ACCESS_ROLE_LIST_ITEM, ATSPI_ROLE_LIST_ITEM },
   { EFL_ACCESS_ROLE_MENU, ATSPI_ROLE_MENU },
   { EFL_ACCESS_ROLE_MENU_BAR, ATSPI_ROLE_MENU_BAR },
   { EFL_ACCESS_ROLE_MENU_ITEM, ATSPI_ROLE_MENU_ITEM },
   { EFL_ACCESS_ROLE_OPTION_PANE, ATSPI_ROLE_OPTION_PANE },
   { EFL_ACCESS_ROLE_PAGE_TAB, ATSPI_ROLE_PAGE_TAB },
   { EFL_ACCESS_ROLE_PAGE_TAB_LIST, ATSPI_ROLE_PAGE_TAB_LIST },
   { EFL_ACCESS_ROLE_PANEL, ATSPI_ROLE_PANEL },
   { EFL_ACCESS_ROLE_PASSWORD_TEXT, ATSPI_ROLE_PASSWORD_TEXT },
   { EFL_ACCESS_ROLE_POPUP_MENU, ATSPI_ROLE_POPUP_MENU },
   { EFL_ACCESS_ROLE_PROGRESS_BAR, ATSPI_ROLE_PROGRESS_BAR },
   { EFL_ACCESS_ROLE_PUSH_BUTTON, ATSPI_ROLE_PUSH_BUTTON },
   { EFL_ACCESS_ROLE_RADIO_BUTTON, ATSPI_ROLE_RADIO_BUTTON },
   { EFL_ACCESS_ROLE_RADIO_MENU_ITEM, ATSPI_ROLE_RADIO_MENU_ITEM },
   { EFL_ACCESS_ROLE_ROOT_PANE, ATSPI_ROLE_ROOT_PANE },
   { EFL_ACCESS_ROLE_ROW_HEADER, ATSPI_ROLE_ROW_HEADER },
   { EFL_ACCESS_ROLE_SCROLL_BAR, ATSPI_ROLE_SCROLL_BAR },
   { EFL_ACCESS_ROLE_SCROLL_PANE, ATSPI_ROLE_SCROLL_PANE },
   { EFL_ACCESS_ROLE_SEPARATOR, ATSPI_ROLE_SEPARATOR },
   { EFL_ACCESS_ROLE_SLIDER, ATSPI_ROLE_SLIDER },
   { EFL_ACCESS_ROLE_SPIN_BUTTON, ATSPI_ROLE_SPIN_BUTTON },
   { EFL_ACCESS_ROLE_SPLIT_PANE, ATSPI_ROLE_SPLIT_PANE },
   { EFL_ACCESS_ROLE_STATUS_BAR, ATSPI_ROLE_STATUS_BAR },
   { EFL_ACCESS_ROLE_TABLE, ATSPI_ROLE_TABLE },
   { EFL_ACCESS_ROLE_TABLE_CELL, ATSPI_ROLE_TABLE_CELL },
   { EFL_ACCESS_ROLE_TABLE_COLUMN_HEADER, ATSPI_ROLE_TABLE_COLUMN_HEADER },
   { EFL_ACCESS_ROLE_TABLE_ROW_HEADER, ATSPI_ROLE_TABLE_ROW_HEADER },
   { EFL_ACCESS_ROLE_TEAROFF_MENU_ITEM, ATSPI_ROLE_TEAROFF_MENU_ITEM },
   { EFL_ACCESS_ROLE_TERMINAL, ATSPI_ROLE_TERMINAL },
   { EFL_ACCESS_ROLE_TEXT, ATSPI_ROLE_TEXT },
   { EFL_ACCESS_ROLE_TOGGLE_BUTTON, ATSPI_ROLE_TOGGLE_BUTTON },
   { EFL_ACCESS_ROLE_TOOL_BAR, ATSPI_ROLE_TOOL_BAR },
   { EFL_ACCESS_ROLE_TOOL_TIP, ATSPI_ROLE_TOOL_TIP },
   { EFL_ACCESS_ROLE_TREE, ATSPI_ROLE_TREE },
   { EFL_ACCESS_ROLE_TREE_TABLE, ATSPI_ROLE_TREE_TABLE },
   { EFL_ACCESS_ROLE_UNKNOWN, ATSPI_ROLE_UNKNOWN },
   { EFL_ACCESS_ROLE_VIEWPORT, ATSPI_ROLE_VIEWPORT },
   { EFL_ACCESS_ROLE_WINDOW, ATSPI_ROLE_WINDOW },
   { EFL_ACCESS_ROLE_EXTENDED, ATSPI_ROLE_EXTENDED },
   { EFL_ACCESS_ROLE_HEADER, ATSPI_ROLE_HEADER },
   { EFL_ACCESS_ROLE_FOOTER, ATSPI_ROLE_FOOTER },
   { EFL_ACCESS_ROLE_PARAGRAPH, ATSPI_ROLE_PARAGRAPH },
   { EFL_ACCESS_ROLE_RULER, ATSPI_ROLE_RULER },
   { EFL_ACCESS_ROLE_APPLICATION, ATSPI_ROLE_APPLICATION },
   { EFL_ACCESS_ROLE_AUTOCOMPLETE, ATSPI_ROLE_AUTOCOMPLETE },
   { EFL_ACCESS_ROLE_EDITBAR, ATSPI_ROLE_EDITBAR },
   { EFL_ACCESS_ROLE_EMBEDDED, ATSPI_ROLE_EMBEDDED },
   { EFL_ACCESS_ROLE_ENTRY, ATSPI_ROLE_ENTRY },
   { EFL_ACCESS_ROLE_CHART, ATSPI_ROLE_CHART },
   { EFL_ACCESS_ROLE_CAPTION, ATSPI_ROLE_CAPTION },
   { EFL_ACCESS_ROLE_DOCUMENT_FRAME, ATSPI_ROLE_DOCUMENT_FRAME },
   { EFL_ACCESS_ROLE_HEADING, ATSPI_ROLE_HEADING },
   { EFL_ACCESS_ROLE_PAGE, ATSPI_ROLE_PAGE },
   { EFL_ACCESS_ROLE_SECTION, ATSPI_ROLE_SECTION },
   { EFL_ACCESS_ROLE_REDUNDANT_OBJECT, ATSPI_ROLE_REDUNDANT_OBJECT },
   { EFL_ACCESS_ROLE_FORM, ATSPI_ROLE_FORM },
   { EFL_ACCESS_ROLE_LINK, ATSPI_ROLE_LINK },
   { EFL_ACCESS_ROLE_INPUT_METHOD_WINDOW, ATSPI_ROLE_INPUT_METHOD_WINDOW },
   { EFL_ACCESS_ROLE_TABLE_ROW, ATSPI_ROLE_TABLE_ROW },
   { EFL_ACCESS_ROLE_TREE_ITEM, ATSPI_ROLE_TREE_ITEM },
   { EFL_ACCESS_ROLE_DOCUMENT_SPREADSHEET, ATSPI_ROLE_DOCUMENT_SPREADSHEET },
   { EFL_ACCESS_ROLE_DOCUMENT_PRESENTATION, ATSPI_ROLE_DOCUMENT_PRESENTATION },
   { EFL_ACCESS_ROLE_DOCUMENT_TEXT, ATSPI_ROLE_DOCUMENT_TEXT },
   { EFL_ACCESS_ROLE_DOCUMENT_WEB, ATSPI_ROLE_DOCUMENT_WEB },
   { EFL_ACCESS_ROLE_DOCUMENT_EMAIL, ATSPI_ROLE_DOCUMENT_EMAIL },
   { EFL_ACCESS_ROLE_COMMENT, ATSPI_ROLE_COMMENT },
   { EFL_ACCESS_ROLE_LIST_BOX, ATSPI_ROLE_LIST_BOX },
   { EFL_ACCESS_ROLE_GROUPING, ATSPI_ROLE_GROUPING },
   { EFL_ACCESS_ROLE_IMAGE_MAP, ATSPI_ROLE_IMAGE_MAP },
   { EFL_ACCESS_ROLE_NOTIFICATION, ATSPI_ROLE_NOTIFICATION },
   { EFL_ACCESS_ROLE_INFO_BAR, ATSPI_ROLE_INFO_BAR },
   { EFL_ACCESS_ROLE_LAST_DEFINED, ATSPI_ROLE_LAST_DEFINED },
};

static const int elm_relation_to_atspi_relation_mapping[] = {
   [EFL_ACCESS_RELATION_NULL] =  ATSPI_RELATION_NULL,
   [EFL_ACCESS_RELATION_LABEL_FOR] =  ATSPI_RELATION_LABEL_FOR,
   [EFL_ACCESS_RELATION_LABELLED_BY] = ATSPI_RELATION_LABELLED_BY,
   [EFL_ACCESS_RELATION_CONTROLLER_FOR] = ATSPI_RELATION_CONTROLLER_FOR,
   [EFL_ACCESS_RELATION_CONTROLLED_BY] = ATSPI_RELATION_CONTROLLED_BY,
   [EFL_ACCESS_RELATION_MEMBER_OF] = ATSPI_RELATION_MEMBER_OF,
   [EFL_ACCESS_RELATION_TOOLTIP_FOR] = ATSPI_RELATION_TOOLTIP_FOR,
   [EFL_ACCESS_RELATION_NODE_CHILD_OF] = ATSPI_RELATION_NODE_CHILD_OF,
   [EFL_ACCESS_RELATION_NODE_PARENT_OF] = ATSPI_RELATION_NODE_PARENT_OF,
   [EFL_ACCESS_RELATION_EXTENDED] = ATSPI_RELATION_EXTENDED,
   [EFL_ACCESS_RELATION_FLOWS_TO] = ATSPI_RELATION_FLOWS_TO,
   [EFL_ACCESS_RELATION_FLOWS_FROM] = ATSPI_RELATION_FLOWS_FROM,
   [EFL_ACCESS_RELATION_SUBWINDOW_OF] = ATSPI_RELATION_SUBWINDOW_OF,
   [EFL_ACCESS_RELATION_EMBEDS] = ATSPI_RELATION_EMBEDS,
   [EFL_ACCESS_RELATION_EMBEDDED_BY] = ATSPI_RELATION_EMBEDDED_BY,
   [EFL_ACCESS_RELATION_POPUP_FOR] = ATSPI_RELATION_POPUP_FOR,
   [EFL_ACCESS_RELATION_PARENT_WINDOW_OF] = ATSPI_RELATION_PARENT_WINDOW_OF,
   [EFL_ACCESS_RELATION_DESCRIPTION_FOR] = ATSPI_RELATION_DESCRIPTION_FOR,
   [EFL_ACCESS_RELATION_DESCRIBED_BY] = ATSPI_RELATION_DESCRIBED_BY,
   [EFL_ACCESS_RELATION_LAST_DEFINED] = ATSPI_RELATION_LAST_DEFINED,
};

static AtspiRelationType _elm_relation_to_atspi_relation(Efl_Access_Relation_Type type)
{
   if ((type < EFL_ACCESS_RELATION_LAST_DEFINED) && (type > EFL_ACCESS_RELATION_NULL))
     return elm_relation_to_atspi_relation_mapping[type];
   return ATSPI_RELATION_NULL;
}

struct atspi_state_desc
{
   Efl_Access_State_Type elm_state;
   AtspiStateType atspi_state;
   const char *name;
};

static const struct atspi_state_desc elm_states_to_atspi_state[] = {
   { EFL_ACCESS_STATE_INVALID, ATSPI_STATE_INVALID, "invalid" },
   { EFL_ACCESS_STATE_ACTIVE, ATSPI_STATE_ACTIVE, "active" },
   { EFL_ACCESS_STATE_ARMED, ATSPI_STATE_ARMED, "armed" },
   { EFL_ACCESS_STATE_BUSY, ATSPI_STATE_BUSY, "busy" },
   { EFL_ACCESS_STATE_CHECKED, ATSPI_STATE_CHECKED, "checked" },
   { EFL_ACCESS_STATE_COLLAPSED, ATSPI_STATE_COLLAPSED, "collapsed" },
   { EFL_ACCESS_STATE_DEFUNCT, ATSPI_STATE_DEFUNCT, "defunct" },
   { EFL_ACCESS_STATE_EDITABLE, ATSPI_STATE_EDITABLE, "editable" },
   { EFL_ACCESS_STATE_ENABLED, ATSPI_STATE_ENABLED, "enabled" },
   { EFL_ACCESS_STATE_EXPANDABLE, ATSPI_STATE_EXPANDABLE, "expandable" },
   { EFL_ACCESS_STATE_EXPANDED, ATSPI_STATE_EXPANDED, "expanded" },
   { EFL_ACCESS_STATE_FOCUSABLE, ATSPI_STATE_FOCUSABLE, "focusable" },
   { EFL_ACCESS_STATE_FOCUSED, ATSPI_STATE_FOCUSED, "focused" },
   { EFL_ACCESS_STATE_HAS_TOOLTIP, ATSPI_STATE_HAS_TOOLTIP, "has-tooltip" },
   { EFL_ACCESS_STATE_HORIZONTAL, ATSPI_STATE_HORIZONTAL, "horizontal" },
   { EFL_ACCESS_STATE_ICONIFIED, ATSPI_STATE_ICONIFIED, "iconified" },
   { EFL_ACCESS_STATE_MODAL, ATSPI_STATE_MODAL, "modal" },
   { EFL_ACCESS_STATE_MULTI_LINE, ATSPI_STATE_MULTI_LINE, "multi-line" },
   { EFL_ACCESS_STATE_MULTISELECTABLE, ATSPI_STATE_MULTISELECTABLE, "multiselectable" },
   { EFL_ACCESS_STATE_OPAQUE, ATSPI_STATE_OPAQUE, "opaque" },
   { EFL_ACCESS_STATE_PRESSED, ATSPI_STATE_PRESSED, "pressed" },
   { EFL_ACCESS_STATE_RESIZABLE, ATSPI_STATE_RESIZABLE, "resizable" },
   { EFL_ACCESS_STATE_SELECTABLE, ATSPI_STATE_SELECTABLE, "selectable" },
   { EFL_ACCESS_STATE_SELECTED, ATSPI_STATE_SELECTED, "selected" },
   { EFL_ACCESS_STATE_SENSITIVE, ATSPI_STATE_SENSITIVE, "sensitive" },
   { EFL_ACCESS_STATE_SHOWING, ATSPI_STATE_SHOWING, "showing" },
   { EFL_ACCESS_STATE_SINGLE_LINE, ATSPI_STATE_SINGLE_LINE, "single-line" },
   { EFL_ACCESS_STATE_STALE, ATSPI_STATE_STALE, "stale" },
   { EFL_ACCESS_STATE_TRANSIENT, ATSPI_STATE_TRANSIENT, "transient" },
   { EFL_ACCESS_STATE_VERTICAL, ATSPI_STATE_VERTICAL, "vertical" },
   { EFL_ACCESS_STATE_VISIBLE, ATSPI_STATE_VISIBLE, "visible" },
   { EFL_ACCESS_STATE_MANAGES_DESCENDANTS, ATSPI_STATE_MANAGES_DESCENDANTS, "manages-descendants" },
   { EFL_ACCESS_STATE_INDETERMINATE, ATSPI_STATE_INDETERMINATE, "indeterminate" },
   { EFL_ACCESS_STATE_REQUIRED, ATSPI_STATE_REQUIRED, "required" },
   { EFL_ACCESS_STATE_TRUNCATED, ATSPI_STATE_TRUNCATED, "truncated" },
   { EFL_ACCESS_STATE_ANIMATED, ATSPI_STATE_ANIMATED, "animated" },
   { EFL_ACCESS_STATE_INVALID_ENTRY, ATSPI_STATE_INVALID_ENTRY, "invalid-entry" },
   { EFL_ACCESS_STATE_SUPPORTS_AUTOCOMPLETION, ATSPI_STATE_SUPPORTS_AUTOCOMPLETION, "supports-autocompletion" },
   { EFL_ACCESS_STATE_SELECTABLE_TEXT, ATSPI_STATE_SELECTABLE_TEXT, "selectable-text" },
   { EFL_ACCESS_STATE_IS_DEFAULT, ATSPI_STATE_IS_DEFAULT, "is-default" },
   { EFL_ACCESS_STATE_VISITED, ATSPI_STATE_VISITED, "visited" },
   { EFL_ACCESS_STATE_LAST_DEFINED, ATSPI_STATE_LAST_DEFINED, "last-defined" },
};

static uint64_t
_elm_atspi_state_set_to_atspi_state_set(Efl_Access_State_Set states)
{
   uint64_t ret = 0;
   unsigned int i = 0;

   for (i = 0; i < SIZE(elm_states_to_atspi_state); i++)
     {
        if (STATE_TYPE_GET(states, elm_states_to_atspi_state[i].elm_state))
          STATE_TYPE_SET(ret, elm_states_to_atspi_state[i].atspi_state);
     }
   return ret;
}

static Eldbus_Message*
_accessible_get_state(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter, *iter_array;
   Efl_Access_State_Set states;
   uint64_t atspi_states = 0;

   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', "u");
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   states = efl_access_state_set_get(obj);

   atspi_states = _elm_atspi_state_set_to_atspi_state_set(states);

   unsigned int s1 = atspi_states & 0xFFFFFFFF;
   unsigned int s2 = (atspi_states >> 32) & 0xFFFFFFFF;

   eldbus_message_iter_basic_append(iter_array, 'u', s1);
   eldbus_message_iter_basic_append(iter_array, 'u', s2);
   eldbus_message_iter_container_close(iter, iter_array);

   return ret;

fail:
   if (ret) eldbus_message_unref(ret);
   return NULL;
}

static Eldbus_Message *
_accessible_get_index_in_parent(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *ret;
   int idx = -1;

   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   idx = efl_access_index_in_parent_get(obj);

   eldbus_message_arguments_append(ret, "i", idx);

   return ret;
}

static Eldbus_Message *
_accessible_child_at_index(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eo *child = NULL;
   Eina_List *children = NULL;
   int idx;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter;

   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   if (!eldbus_message_arguments_get(msg, "i", &idx))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   children = efl_access_children_get(obj);
   child = eina_list_nth(children, idx);

   eldbus_message_iter_efl_access_reference_append(iter, eldbus_service_connection_get(iface), child);
   eina_list_free(children);

   return ret;
}

static Eldbus_Message *
_accessible_get_relation_set(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eo *rel_obj;
   Eldbus_Message *ret = NULL;
   Eldbus_Message_Iter *iter = NULL, *iter_array = NULL, *iter_array2 = NULL, *iter_struct;
   Efl_Access_Relation *rel;
   Eina_List *l, *l2;
   Efl_Access_Relation_Set rels;

   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', "(ua(so))");
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   rels = efl_access_relation_set_get(obj);

   EINA_LIST_FOREACH(rels, l, rel)
     {
        iter_struct = eldbus_message_iter_container_new(iter_array, 'r', NULL);
        eldbus_message_iter_basic_append(iter_struct, 'u', _elm_relation_to_atspi_relation(rel->type));
        eldbus_message_iter_efl_access_reference_array_append(iter_struct, eldbus_service_connection_get(iface), rel->objects);
        eldbus_message_iter_container_close(iter_array, iter_struct);
     }
   efl_access_relation_set_free(rels);
   eldbus_message_iter_container_close(iter, iter_array);

   return ret;

fail:
   eldbus_message_unref(ret);
   return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.Failed", "Unable to get relation set.");
}

static Eldbus_Message *
_accessible_get_role(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   AtspiRole atspi_role = ATSPI_ROLE_INVALID;
   Efl_Access_Role role;

   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   role = efl_access_role_get(obj);

   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   atspi_role = role > EFL_ACCESS_ROLE_LAST_DEFINED ? ATSPI_ROLE_LAST_DEFINED : elm_roles_to_atspi_roles[role][1];
   eldbus_message_arguments_append(ret, "u", atspi_role);
   return ret;
}

static Eldbus_Message *
_accessible_get_role_name(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   const char *role_name = efl_access_role_name_get(obj);

   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   eldbus_message_arguments_append(ret, "s", role_name);

   return ret;
}

static Eldbus_Message *
_accessible_get_localized_role_name(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   const char *l_role_name = efl_access_localized_role_name_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(l_role_name, NULL);

   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   eldbus_message_arguments_append(ret, "s", l_role_name);

   return ret;
}

static Eldbus_Message *
_accessible_get_children(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eina_List *children_list = NULL, *l;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter, *iter_array;
   Eo *child;

   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   children_list = efl_access_children_get(obj);

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   eldbus_message_iter_efl_access_reference_array_append(iter, eldbus_service_connection_get(iface), children_list);
   eina_list_free(children_list);

   return ret;

fail:
   if (ret) eldbus_message_unref(ret);
   return NULL;
}

static Eldbus_Message *
_accessible_get_application(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eo *root;
   Eldbus_Message *ret;

   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   Eldbus_Message_Iter *iter = eldbus_message_iter_get(ret);
   root = efl_access_root_get(EFL_ACCESS_MIXIN);
   eldbus_message_iter_efl_access_reference_append(iter, eldbus_service_connection_get(iface), root);

   return ret;
}

static Eldbus_Message *
_accessible_attributes_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eina_List *attrs = NULL, *l;
   Efl_Access_Attribute *attr;
   Eldbus_Message_Iter *iter, *iter_dict = NULL, *iter_entry;
   Eldbus_Message *ret;

   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   ret = eldbus_message_method_return_new(msg);
   if (!ret) goto error;

   attrs = efl_access_attributes_get(obj);

   iter = eldbus_message_iter_get(ret);
   if (!iter) goto error;

   iter_dict = eldbus_message_iter_container_new(iter, 'a', "{ss}");
   if (!iter_dict) goto error;

   EINA_LIST_FOREACH(attrs, l, attr)
     {
        iter_entry = eldbus_message_iter_container_new(iter_dict, 'e', NULL);
        if (!iter_entry) goto error;
        eldbus_message_iter_arguments_append(iter_entry, "ss", attr->key, attr->value);
        eldbus_message_iter_container_close(iter_dict, iter_entry);
     }

   eldbus_message_iter_container_close(iter, iter_dict);
   efl_access_attributes_list_free(attrs);
   return ret;

error:
   if (iter_dict) eldbus_message_iter_container_close(iter, iter_dict);
   if (ret) eldbus_message_unref(ret);
   efl_access_attributes_list_free(attrs);
   return NULL;
}

static Eldbus_Message *
_accessible_interfaces_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter;

   Efl_Access *obj = efl_access_unmarshal(msg);
   ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, msg);

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   eldbus_message_iter_efl_access_interfaces_append(iter, obj);

   return ret;
}

static const Eldbus_Method accessible_methods[] = {
   { "GetChildAtIndex", ELDBUS_ARGS({"i", "index"}), ELDBUS_ARGS({"(so)", "Accessible"}), _accessible_child_at_index, 0 },
   { "GetChildren", NULL, ELDBUS_ARGS({"a(so)", "children"}), _accessible_get_children, 0 },
   { "GetIndexInParent", NULL, ELDBUS_ARGS({"i", "index"}), _accessible_get_index_in_parent, 0 },
   { "GetRelationSet", NULL, ELDBUS_ARGS({"a(ua(so))", NULL}), _accessible_get_relation_set, 0 },
   { "GetRole", NULL, ELDBUS_ARGS({"u", "Role"}), _accessible_get_role, 0 },
   { "GetRoleName", NULL, ELDBUS_ARGS({"s", "Name"}), _accessible_get_role_name, 0 },
   { "GetLocalizedRoleName", NULL, ELDBUS_ARGS({"s", "LocalizedName"}), _accessible_get_localized_role_name, 0},
   { "GetState", NULL, ELDBUS_ARGS({"au", NULL}), _accessible_get_state, 0},
   { "GetApplication", NULL, ELDBUS_ARGS({"(so)", NULL}), _accessible_get_application, 0},
   { "GetAttributes", NULL, ELDBUS_ARGS({"a{ss}", NULL}), _accessible_attributes_get, 0},
   { "GetInterfaces", NULL, ELDBUS_ARGS({"as", NULL}), _accessible_interfaces_get, 0},
   { NULL, NULL, NULL, NULL, 0 }
};

static Eina_Bool
_accessible_property_get(const Eldbus_Service_Interface *interface, const char *property,
                         Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg,
                         Eldbus_Message **error)
{
   const char *ret = NULL;
   Eo *ret_obj = NULL;

   Efl_Access *obj = efl_access_unmarshal(request_msg);
   ELM_ATSPI_PROPERTY_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, EFL_ACCESS_MIXIN, request_msg, error);

   if (!strcmp(property, "Name"))
     {
        ret = efl_access_name_get(obj);
        if (!ret)
          ret = "";
        eldbus_message_iter_basic_append(iter, 's', ret);
        return EINA_TRUE;
     }
   else if (!strcmp(property, "Description"))
     {
        ret = efl_access_description_get(obj);
        if (!ret)
          ret = "";
        eldbus_message_iter_basic_append(iter, 's', ret);
        return EINA_TRUE;
     }
   else if (!strcmp(property, "Parent"))
     {
       ret_obj = efl_access_parent_get(obj);
       eldbus_message_iter_efl_access_reference_append(iter, eldbus_service_connection_get(interface), ret_obj);
       return EINA_TRUE;
     }
   else if (!strcmp(property, "ChildCount"))
     {
        Eina_List *l = NULL;
        l = efl_access_children_get(obj);
        eldbus_message_iter_basic_append(iter, 'i', eina_list_count(l));
        eina_list_free(l);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static const Eldbus_Property accessible_properties[] = {
   { "Name", "s", NULL, NULL, 0 },
   { "Description", "s", NULL, NULL, 0 },
   { "Parent", "(so)", NULL, NULL, 0 },
   { "ChildCount", "i", NULL, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc accessible_iface_desc = {
   ATSPI_DBUS_INTERFACE_ACCESSIBLE,
   accessible_methods,
   NULL,
   accessible_properties,
   _accessible_property_get,
   NULL
};

Eldbus_Service_Interface*
_elm_atspi_access_adaptor_register(Eldbus_Connection *conn)
{
   return eldbus_service_interface_fallback_register(conn, ELM_ACCESS_OBJECT_PATH_PREFIX2, &accessible_iface_desc);
}
