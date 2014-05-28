#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_ACTION_PROTECTED
#define ELM_INTERFACE_ATSPI_VALUE_PROTECTED
#define ELM_INTERFACE_ATSPI_IMAGE_PROTECTED
#include "atspi/atspi-constants.h"

#include <stdint.h>
#include <Elementary.h>
#include "elm_priv.h"
#include <assert.h>

#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_component.eo.h"
#include "elm_interface_atspi_window.eo.h"
#include "elm_interface_atspi_action.eo.h"
#include "elm_interface_atspi_value.eo.h"
#include "elm_interface_atspi_image.eo.h"

/*
 * Accessibility Bus info not defined in atspi-constants.h
 */
#define A11Y_DBUS_NAME "org.a11y.Bus"
#define A11Y_DBUS_PATH "/org/a11y/bus"
#define A11Y_DBUS_INTERFACE "org.a11y.Bus"
#define ATSPI_DBUS_INTERFACE_EVENT_WINDOW "org.a11y.atspi.Event.Window"

#define CACHE_ITEM_SIGNATURE "((so)(so)(so)a(so)assusau)"
#define CACHE_INTERFACE_PATH "/org/a11y/atspi/cache"

#define ELM_ACCESS_OBJECT_PATH_ROOT "root"
#define ELM_ACCESS_OBJECT_PATH_PREFIX  "/org/a11y/atspi/accessible/"
#define ELM_ACCESS_OBJECT_REFERENCE_TEMPLATE ELM_ACCESS_OBJECT_PATH_PREFIX "%llu"

#define SIZE(x) sizeof(x)/sizeof(x[0])

static int _init_count = 0;

static Eldbus_Connection *_a11y_bus = NULL;
static Eo *_root;
static Ecore_Idler *_cache_update_idler;
static Eina_List *_pending_objects;
static Eina_Hash *_cache;
static Eldbus_Service_Interface *_cache_interface = NULL;
static Eldbus_Signal_Handler *_register_hdl;
static Eldbus_Signal_Handler *_unregister_hdl;
static unsigned short _object_property_broadcast_mask;
static unsigned short _object_children_broadcast_mask;
static unsigned long long _object_state_broadcast_mask;
static unsigned long long _window_signal_broadcast_mask;

static Eina_Bool _state_changed_signal_send(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info);
static Eina_Bool _property_changed_signal_send(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info);
static Eina_Bool _children_changed_signal_send(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info);
static Eina_Bool _window_signal_send(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info);
static Eo * _access_object_from_path(const char *path);
static char * _path_from_access_object(Eo *eo);
static void _object_append_reference(Eldbus_Message_Iter *iter,  Eo *obj);
static void _object_append_desktop_reference(Eldbus_Message_Iter *iter);
static void _cache_build(void *obj);
static void _object_register(Eo *obj, char *path);

EO_CALLBACKS_ARRAY_DEFINE(_events_cb,
   { ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, _property_changed_signal_send},
   { ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_CHILDREN_CHANGED, _children_changed_signal_send},
   { ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_STATE_CHANGED, _state_changed_signal_send},
   { ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_VISIBLE_DATA_CHANGED, NULL },
   { ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_ACTIVE_DESCENDANTS_CHANGED, NULL }
);

EO_CALLBACKS_ARRAY_DEFINE(_window_cb,
   { ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_CREATED, _window_signal_send},
   { ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_ACTIVATED, _window_signal_send},
   { ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_DEACTIVATED, _window_signal_send}
);

enum _Atspi_Object_Child_Event_Type
{
   ATSPI_OBJECT_CHILD_ADDED = 0,
   ATSPI_OBJECT_CHILD_REMOVED
};

enum _Atspi_Object_Property
{
   ATSPI_OBJECT_PROPERTY_NAME = 0,
   ATSPI_OBJECT_PROPERTY_DESCRIPTION,
   ATSPI_OBJECT_PROPERTY_VALUE,
   ATSPI_OBJECT_PROPERTY_ROLE,
   ATSPI_OBJECT_PROPERTY_PARENT,
   ATSPI_OBJECT_PROPERTY_LAST
};

enum _Atspi_Object_Signals {
   ATSPI_OBJECT_EVENT_PROPERTY_CHANGED = 0,
   ATSPI_OBJECT_EVENT_BOUNDS_CHANGED,
   ATSPI_OBJECT_EVENT_LINK_SELECTED,
   ATSPI_OBJECT_EVENT_STATE_CHANGED,
   ATSPI_OBJECT_EVENT_CHILDREN_CHANGED,
   ATSPI_OBJECT_EVENT_VISIBLE_DATA_CHANGED,
   ATSPI_OBJECT_EVENT_SELECTION_CHANGED,
   ATSPI_OBJECT_EVENT_MODEL_CHANGED,
   ATSPI_OBJECT_EVENT_ACTIVE_DESCENDANT_CHANGED,
   ATSPI_OBJECT_EVENT_ROW_INSERTED,
   ATSPI_OBJECT_EVENT_ROW_REORDERED,
   ATSPI_OBJECT_EVENT_ROW_DELETED,
   ATSPI_OBJECT_EVENT_COLUMN_INSERTED,
   ATSPI_OBJECT_EVENT_COLUMN_REORDERED,
   ATSPI_OBJECT_EVENT_COLUMN_DELETED,
   ATSPI_OBJECT_EVENT_TEXT_BOUNDS_CHANGED,
   ATSPI_OBJECT_EVENT_TEXT_SELECTION_CHANGED,
   ATSPI_OBJECT_EVENT_TEXT_CHANGED,
   ATSPI_OBJECT_EVENT_TEXT_ATTRIBUTES_CHANGED,
   ATSPI_OBJECT_EVENT_TEXT_CARET_MOVED,
   ATSPI_OBJECT_EVENT_ATTRIBUTES_CHANGED,
};

enum _Atspi_Window_Signals
{
   ATSPI_WINDOW_EVENT_PROPERTY_CHANGE = 0,
   ATSPI_WINDOW_EVENT_MINIMIZE,
   ATSPI_WINDOW_EVENT_MAXIMIZE,
   ATSPI_WINDOW_EVENT_RESTORE,
   ATSPI_WINDOW_EVENT_CLOSE,
   ATSPI_WINDOW_EVENT_CREATE,
   ATSPI_WINDOW_EVENT_REPARENT,
   ATSPI_WINDOW_EVENT_DESKTOPCREATE,
   ATSPI_WINDOW_EVENT_DESKTOPDESTROY,
   ATSPI_WINDOW_EVENT_DESTROY,
   ATSPI_WINDOW_EVENT_ACTIVATE,
   ATSPI_WINDOW_EVENT_DEACTIVATE,
   ATSPI_WINDOW_EVENT_RAISE,
   ATSPI_WINDOW_EVENT_LOWER,
   ATSPI_WINDOW_EVENT_MOVE,
   ATSPI_WINDOW_EVENT_RESIZE,
   ATSPI_WINDOW_EVENT_SHADE,
   ATSPI_WINDOW_EVENT_UUSHADE,
   ATSPI_WINDOW_EVENT_RESTYLE,
};

static const Eldbus_Signal _event_obj_signals[] = {
   [ATSPI_OBJECT_EVENT_PROPERTY_CHANGED] = {"PropertyChange", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_BOUNDS_CHANGED] = {"BoundsChange", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_LINK_SELECTED] = {"LinkSelected", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_STATE_CHANGED] = {"StateChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_CHILDREN_CHANGED] = {"ChildrenChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_VISIBLE_DATA_CHANGED] = {"VisibleDataChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_SELECTION_CHANGED] = {"SelectionChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_MODEL_CHANGED] = {"ModelChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ACTIVE_DESCENDANT_CHANGED] = {"ActiveDescendantsChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ROW_INSERTED] = {"RowInserted", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ROW_REORDERED] = {"RowReordered", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ROW_DELETED] = {"RowDeleted", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_COLUMN_INSERTED] = {"ColumnInserted", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_COLUMN_REORDERED] = {"ColumnReordered", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_COLUMN_DELETED] = {"ColumnDeleted", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_BOUNDS_CHANGED] = {"TextBoundsChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_SELECTION_CHANGED] = {"SelectionChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_CHANGED] = {"TextChaged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_ATTRIBUTES_CHANGED] = {"TextAttributesChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_CARET_MOVED] = {"TextCaretMoved", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ATTRIBUTES_CHANGED] = {"AttributesChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
};

static const Eldbus_Signal _window_obj_signals[] = {
   [ATSPI_WINDOW_EVENT_PROPERTY_CHANGE] = {"PropertyChange", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_MINIMIZE] = {"Minimize", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_MAXIMIZE] = {"Maximize", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_RESTORE] = {"Restore", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_CLOSE] = {"Close", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_CREATE] = {"Create", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_REPARENT] = {"Reparent", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_DESKTOPCREATE] = {"DesktopCreate", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_DESKTOPDESTROY] = {"DesktopDestroy", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_DESTROY] = {"Destroy", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_ACTIVATE] = {"Activate", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_DEACTIVATE] = {"Deactivate", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_RAISE] = {"Raise", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_LOWER] = {"Lower", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_MOVE] = {"Move", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_RESIZE] = {"Resize", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_SHADE] = {"Shade", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_UUSHADE] = {"uUshade", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_RESTYLE] = {"Restyle", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   {NULL, ELDBUS_ARGS({NULL, NULL}), 0}
};


const int elm_roles_to_atspi_roles[][2] = {
   { ELM_ATSPI_ROLE_INVALID, ATSPI_ROLE_INVALID },
   { ELM_ATSPI_ROLE_ACCELERATOR_LABEL, ATSPI_ROLE_ACCELERATOR_LABEL },
   { ELM_ATSPI_ROLE_ALERT, ATSPI_ROLE_ALERT },
   { ELM_ATSPI_ROLE_ANIMATION, ATSPI_ROLE_ANIMATION },
   { ELM_ATSPI_ROLE_ARROW, ATSPI_ROLE_ARROW },
   { ELM_ATSPI_ROLE_CALENDAR, ATSPI_ROLE_CALENDAR },
   { ELM_ATSPI_ROLE_CANVAS, ATSPI_ROLE_CANVAS },
   { ELM_ATSPI_ROLE_CHECK_BOX, ATSPI_ROLE_CHECK_BOX },
   { ELM_ATSPI_ROLE_CHECK_MENU_ITEM, ATSPI_ROLE_CHECK_MENU_ITEM },
   { ELM_ATSPI_ROLE_COLOR_CHOOSER, ATSPI_ROLE_COLOR_CHOOSER },
   { ELM_ATSPI_ROLE_COLUMN_HEADER, ATSPI_ROLE_COLUMN_HEADER },
   { ELM_ATSPI_ROLE_COMBO_BOX, ATSPI_ROLE_COMBO_BOX },
   { ELM_ATSPI_ROLE_DATE_EDITOR, ATSPI_ROLE_DATE_EDITOR },
   { ELM_ATSPI_ROLE_DESKTOP_ICON, ATSPI_ROLE_DESKTOP_ICON },
   { ELM_ATSPI_ROLE_DESKTOP_FRAME, ATSPI_ROLE_DESKTOP_FRAME },
   { ELM_ATSPI_ROLE_DIAL, ATSPI_ROLE_DIAL },
   { ELM_ATSPI_ROLE_DIALOG, ATSPI_ROLE_DIALOG },
   { ELM_ATSPI_ROLE_DIRECTORY_PANE, ATSPI_ROLE_DIRECTORY_PANE },
   { ELM_ATSPI_ROLE_DRAWING_AREA, ATSPI_ROLE_DRAWING_AREA },
   { ELM_ATSPI_ROLE_FILE_CHOOSER, ATSPI_ROLE_FILE_CHOOSER },
   { ELM_ATSPI_ROLE_FILLER, ATSPI_ROLE_FILLER },
   { ELM_ATSPI_ROLE_FOCUS_TRAVERSABLE, ATSPI_ROLE_FOCUS_TRAVERSABLE },
   { ELM_ATSPI_ROLE_FONT_CHOOSER, ATSPI_ROLE_FONT_CHOOSER },
   { ELM_ATSPI_ROLE_FRAME, ATSPI_ROLE_FRAME },
   { ELM_ATSPI_ROLE_GLASS_PANE, ATSPI_ROLE_GLASS_PANE },
   { ELM_ATSPI_ROLE_HTML_CONTAINER, ATSPI_ROLE_HTML_CONTAINER },
   { ELM_ATSPI_ROLE_ICON, ATSPI_ROLE_ICON },
   { ELM_ATSPI_ROLE_IMAGE, ATSPI_ROLE_IMAGE },
   { ELM_ATSPI_ROLE_INTERNAL_FRAME, ATSPI_ROLE_INTERNAL_FRAME },
   { ELM_ATSPI_ROLE_LABEL, ATSPI_ROLE_LABEL },
   { ELM_ATSPI_ROLE_LAYERED_PANE, ATSPI_ROLE_LAYERED_PANE },
   { ELM_ATSPI_ROLE_LIST, ATSPI_ROLE_LIST },
   { ELM_ATSPI_ROLE_LIST_ITEM, ATSPI_ROLE_LIST_ITEM },
   { ELM_ATSPI_ROLE_MENU, ATSPI_ROLE_MENU },
   { ELM_ATSPI_ROLE_MENU_BAR, ATSPI_ROLE_MENU_BAR },
   { ELM_ATSPI_ROLE_MENU_ITEM, ATSPI_ROLE_MENU_ITEM },
   { ELM_ATSPI_ROLE_OPTION_PANE, ATSPI_ROLE_OPTION_PANE },
   { ELM_ATSPI_ROLE_PAGE_TAB, ATSPI_ROLE_PAGE_TAB },
   { ELM_ATSPI_ROLE_PAGE_TAB_LIST, ATSPI_ROLE_PAGE_TAB_LIST },
   { ELM_ATSPI_ROLE_PANEL, ATSPI_ROLE_PANEL },
   { ELM_ATSPI_ROLE_PASSWORD_TEXT, ATSPI_ROLE_PASSWORD_TEXT },
   { ELM_ATSPI_ROLE_POPUP_MENU, ATSPI_ROLE_POPUP_MENU },
   { ELM_ATSPI_ROLE_PROGRESS_BAR, ATSPI_ROLE_PROGRESS_BAR },
   { ELM_ATSPI_ROLE_PUSH_BUTTON, ATSPI_ROLE_PUSH_BUTTON },
   { ELM_ATSPI_ROLE_RADIO_BUTTON, ATSPI_ROLE_RADIO_BUTTON },
   { ELM_ATSPI_ROLE_RADIO_MENU_ITEM, ATSPI_ROLE_RADIO_MENU_ITEM },
   { ELM_ATSPI_ROLE_ROOT_PANE, ATSPI_ROLE_ROOT_PANE },
   { ELM_ATSPI_ROLE_ROW_HEADER, ATSPI_ROLE_ROW_HEADER },
   { ELM_ATSPI_ROLE_SCROLL_BAR, ATSPI_ROLE_SCROLL_BAR },
   { ELM_ATSPI_ROLE_SCROLL_PANE, ATSPI_ROLE_SCROLL_PANE },
   { ELM_ATSPI_ROLE_SEPARATOR, ATSPI_ROLE_SEPARATOR },
   { ELM_ATSPI_ROLE_SLIDER, ATSPI_ROLE_SLIDER },
   { ELM_ATSPI_ROLE_SPIN_BUTTON, ATSPI_ROLE_SPIN_BUTTON },
   { ELM_ATSPI_ROLE_SPLIT_PANE, ATSPI_ROLE_SPLIT_PANE },
   { ELM_ATSPI_ROLE_STATUS_BAR, ATSPI_ROLE_STATUS_BAR },
   { ELM_ATSPI_ROLE_TABLE, ATSPI_ROLE_TABLE },
   { ELM_ATSPI_ROLE_TABLE_CELL, ATSPI_ROLE_TABLE_CELL },
   { ELM_ATSPI_ROLE_TABLE_COLUMN_HEADER, ATSPI_ROLE_TABLE_COLUMN_HEADER },
   { ELM_ATSPI_ROLE_TABLE_ROW_HEADER, ATSPI_ROLE_TABLE_ROW_HEADER },
   { ELM_ATSPI_ROLE_TEAROFF_MENU_ITEM, ATSPI_ROLE_TEAROFF_MENU_ITEM },
   { ELM_ATSPI_ROLE_TERMINAL, ATSPI_ROLE_TERMINAL },
   { ELM_ATSPI_ROLE_TEXT, ATSPI_ROLE_TEXT },
   { ELM_ATSPI_ROLE_TOGGLE_BUTTON, ATSPI_ROLE_TOGGLE_BUTTON },
   { ELM_ATSPI_ROLE_TOOL_BAR, ATSPI_ROLE_TOOL_BAR },
   { ELM_ATSPI_ROLE_TOOL_TIP, ATSPI_ROLE_TOOL_TIP },
   { ELM_ATSPI_ROLE_TREE, ATSPI_ROLE_TREE },
   { ELM_ATSPI_ROLE_TREE_TABLE, ATSPI_ROLE_TREE_TABLE },
   { ELM_ATSPI_ROLE_UNKNOWN, ATSPI_ROLE_UNKNOWN },
   { ELM_ATSPI_ROLE_VIEWPORT, ATSPI_ROLE_VIEWPORT },
   { ELM_ATSPI_ROLE_WINDOW, ATSPI_ROLE_WINDOW },
   { ELM_ATSPI_ROLE_EXTENDED, ATSPI_ROLE_EXTENDED },
   { ELM_ATSPI_ROLE_HEADER, ATSPI_ROLE_HEADER },
   { ELM_ATSPI_ROLE_FOOTER, ATSPI_ROLE_FOOTER },
   { ELM_ATSPI_ROLE_PARAGRAPH, ATSPI_ROLE_PARAGRAPH },
   { ELM_ATSPI_ROLE_RULER, ATSPI_ROLE_RULER },
   { ELM_ATSPI_ROLE_APPLICATION, ATSPI_ROLE_APPLICATION },
   { ELM_ATSPI_ROLE_AUTOCOMPLETE, ATSPI_ROLE_AUTOCOMPLETE },
   { ELM_ATSPI_ROLE_EDITBAR, ATSPI_ROLE_EDITBAR },
   { ELM_ATSPI_ROLE_EMBEDDED, ATSPI_ROLE_EMBEDDED },
   { ELM_ATSPI_ROLE_ENTRY, ATSPI_ROLE_ENTRY },
   { ELM_ATSPI_ROLE_CHART, ATSPI_ROLE_CHART },
   { ELM_ATSPI_ROLE_CAPTION, ATSPI_ROLE_CAPTION },
   { ELM_ATSPI_ROLE_DOCUMENT_FRAME, ATSPI_ROLE_DOCUMENT_FRAME },
   { ELM_ATSPI_ROLE_HEADING, ATSPI_ROLE_HEADING },
   { ELM_ATSPI_ROLE_PAGE, ATSPI_ROLE_PAGE },
   { ELM_ATSPI_ROLE_SECTION, ATSPI_ROLE_SECTION },
   { ELM_ATSPI_ROLE_REDUNDANT_OBJECT, ATSPI_ROLE_REDUNDANT_OBJECT },
   { ELM_ATSPI_ROLE_FORM, ATSPI_ROLE_FORM },
   { ELM_ATSPI_ROLE_LINK, ATSPI_ROLE_LINK },
   { ELM_ATSPI_ROLE_INPUT_METHOD_WINDOW, ATSPI_ROLE_INPUT_METHOD_WINDOW },
   { ELM_ATSPI_ROLE_TABLE_ROW, ATSPI_ROLE_TABLE_ROW },
   { ELM_ATSPI_ROLE_TREE_ITEM, ATSPI_ROLE_TREE_ITEM },
   { ELM_ATSPI_ROLE_DOCUMENT_SPREADSHEET, ATSPI_ROLE_DOCUMENT_SPREADSHEET },
   { ELM_ATSPI_ROLE_DOCUMENT_PRESENTATION, ATSPI_ROLE_DOCUMENT_PRESENTATION },
   { ELM_ATSPI_ROLE_DOCUMENT_TEXT, ATSPI_ROLE_DOCUMENT_TEXT },
   { ELM_ATSPI_ROLE_DOCUMENT_WEB, ATSPI_ROLE_DOCUMENT_WEB },
   { ELM_ATSPI_ROLE_DOCUMENT_EMAIL, ATSPI_ROLE_DOCUMENT_EMAIL },
   { ELM_ATSPI_ROLE_COMMENT, ATSPI_ROLE_COMMENT },
   { ELM_ATSPI_ROLE_LIST_BOX, ATSPI_ROLE_LIST_BOX },
   { ELM_ATSPI_ROLE_GROUPING, ATSPI_ROLE_GROUPING },
   { ELM_ATSPI_ROLE_IMAGE_MAP, ATSPI_ROLE_IMAGE_MAP },
   { ELM_ATSPI_ROLE_NOTIFICATION, ATSPI_ROLE_NOTIFICATION },
   { ELM_ATSPI_ROLE_INFO_BAR, ATSPI_ROLE_INFO_BAR },
   { ELM_ATSPI_ROLE_LAST_DEFINED, ATSPI_ROLE_LAST_DEFINED },
}; 


const int elm_states_to_atspi_state[][2] = {
   { ELM_ATSPI_STATE_INVALID, ATSPI_STATE_INVALID },
   { ELM_ATSPI_STATE_ACTIVE, ATSPI_STATE_ACTIVE },
   { ELM_ATSPI_STATE_ARMED, ATSPI_STATE_ARMED },
   { ELM_ATSPI_STATE_BUSY, ATSPI_STATE_BUSY },
   { ELM_ATSPI_STATE_CHECKED, ATSPI_STATE_CHECKED },
   { ELM_ATSPI_STATE_COLLAPSED, ATSPI_STATE_COLLAPSED },
   { ELM_ATSPI_STATE_DEFUNCT, ATSPI_STATE_DEFUNCT },
   { ELM_ATSPI_STATE_EDITABLE, ATSPI_STATE_EDITABLE },
   { ELM_ATSPI_STATE_ENABLED, ATSPI_STATE_ENABLED },
   { ELM_ATSPI_STATE_EXPANDABLE, ATSPI_STATE_EXPANDABLE },
   { ELM_ATSPI_STATE_EXPANDED, ATSPI_STATE_EXPANDED },
   { ELM_ATSPI_STATE_FOCUSABLE, ATSPI_STATE_FOCUSABLE },
   { ELM_ATSPI_STATE_FOCUSED, ATSPI_STATE_FOCUSED },
   { ELM_ATSPI_STATE_HAS_TOOLTIP, ATSPI_STATE_HAS_TOOLTIP },
   { ELM_ATSPI_STATE_HORIZONTAL, ATSPI_STATE_HORIZONTAL },
   { ELM_ATSPI_STATE_ICONIFIED, ATSPI_STATE_ICONIFIED },
   { ELM_ATSPI_STATE_MODAL, ATSPI_STATE_MODAL },
   { ELM_ATSPI_STATE_MULTI_LINE, ATSPI_STATE_MULTI_LINE },
   { ELM_ATSPI_STATE_MULTISELECTABLE, ATSPI_STATE_MULTISELECTABLE },
   { ELM_ATSPI_STATE_OPAQUE, ATSPI_STATE_OPAQUE },
   { ELM_ATSPI_STATE_PRESSED, ATSPI_STATE_PRESSED },
   { ELM_ATSPI_STATE_RESIZABLE, ATSPI_STATE_RESIZABLE },
   { ELM_ATSPI_STATE_SELECTABLE, ATSPI_STATE_SELECTABLE },
   { ELM_ATSPI_STATE_SELECTED, ATSPI_STATE_SELECTED },
   { ELM_ATSPI_STATE_SENSITIVE, ATSPI_STATE_SENSITIVE },
   { ELM_ATSPI_STATE_SHOWING, ATSPI_STATE_SHOWING },
   { ELM_ATSPI_STATE_SINGLE_LINE, ATSPI_STATE_SINGLE_LINE },
   { ELM_ATSPI_STATE_STALE, ATSPI_STATE_STALE },
   { ELM_ATSPI_STATE_TRANSIENT, ATSPI_STATE_TRANSIENT },
   { ELM_ATSPI_STATE_VERTICAL, ATSPI_STATE_VERTICAL },
   { ELM_ATSPI_STATE_VISIBLE, ATSPI_STATE_VISIBLE },
   { ELM_ATSPI_STATE_MANAGES_DESCENDANTS, ATSPI_STATE_MANAGES_DESCENDANTS },
   { ELM_ATSPI_STATE_INDETERMINATE, ATSPI_STATE_INDETERMINATE },
   { ELM_ATSPI_STATE_REQUIRED, ATSPI_STATE_REQUIRED },
   { ELM_ATSPI_STATE_TRUNCATED, ATSPI_STATE_TRUNCATED },
   { ELM_ATSPI_STATE_ANIMATED, ATSPI_STATE_ANIMATED },
   { ELM_ATSPI_STATE_INVALID_ENTRY, ATSPI_STATE_INVALID_ENTRY },
   { ELM_ATSPI_STATE_SUPPORTS_AUTOCOMPLETION, ATSPI_STATE_SUPPORTS_AUTOCOMPLETION },
   { ELM_ATSPI_STATE_SELECTABLE_TEXT, ATSPI_STATE_SELECTABLE_TEXT },
   { ELM_ATSPI_STATE_IS_DEFAULT, ATSPI_STATE_IS_DEFAULT },
   { ELM_ATSPI_STATE_VISITED, ATSPI_STATE_VISITED },
   { ELM_ATSPI_STATE_LAST_DEFINED, ATSPI_STATE_LAST_DEFINED },
};

static Eldbus_Message *
_accessible_get_role(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   AtspiRole atspi_role = ATSPI_ROLE_INVALID;
   Elm_Atspi_Role role;

   eo_do(obj, role = elm_interface_atspi_accessible_role_get());

   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   atspi_role = role > ELM_ATSPI_ROLE_LAST_DEFINED ? ATSPI_ROLE_LAST_DEFINED : elm_roles_to_atspi_roles[role][1];
   eldbus_message_arguments_append(ret, "u", atspi_role);
   return ret;
}

static Eldbus_Message *
_accessible_get_role_name(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *role_name = NULL, *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);

   eo_do(obj, role_name = elm_interface_atspi_accessible_role_name_get());

   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   eldbus_message_arguments_append(ret, "s", role_name);

   return ret;
}

static Eldbus_Message *
_accessible_get_localized_role_name(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *l_role_name = NULL, *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);

   eo_do(obj, l_role_name = elm_interface_atspi_accessible_localized_role_name_get());

   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   eldbus_message_arguments_append(ret, "s", l_role_name);

   return ret;
}

static Eldbus_Message *
_accessible_get_children(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   Eina_List *children_list = NULL, *l;
   Eldbus_Message *ret;

   Eldbus_Message_Iter *iter, *iter_array;
   Eo *children;

   eo_do(obj, children_list = elm_interface_atspi_accessible_children_get());

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', "(so)");
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   EINA_LIST_FOREACH(children_list, l, children)
      _object_append_reference(iter_array, children);

   eldbus_message_iter_container_close(iter, iter_array);
   eina_list_free(children_list);

   return ret;

fail:
   if (ret) eldbus_message_unref(ret);
   return NULL;
}

static Eldbus_Message *
_accessible_get_application(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   Eldbus_Message_Iter *iter = eldbus_message_iter_get(ret);
   _object_append_reference(iter, _root);

   return ret;
}

static Eldbus_Message *
_accessible_attributes_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eina_List *attrs, *l;
   Elm_Atspi_Attribute *attr;
   Eldbus_Message_Iter *iter, *iter_array;

   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);

   eo_do(obj, attrs = elm_interface_atspi_accessible_attributes_get());

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', "ss");

   EINA_LIST_FOREACH(attrs, l, attr)
     eldbus_message_iter_arguments_get(iter_array, "ss", attr->key, attr->value);

   eldbus_message_iter_container_close(iter, iter_array);
   elm_atspi_attributes_list_free(attrs);

   return ret;
}

static uint64_t 
_elm_atspi_state_set_to_atspi_state_set(Elm_Atspi_State_Set states)
{
   uint64_t ret = 0;
   unsigned int i = 0;

   for (i = 0; i < SIZE(elm_states_to_atspi_state); i++)
     {
        if (STATE_TYPE_GET(states, elm_states_to_atspi_state[i][0]))
          STATE_TYPE_SET(ret, elm_states_to_atspi_state[i][1]);
     }
   return ret;
}

static Eldbus_Message *
_accessible_get_state(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter, *iter_array;
   Elm_Atspi_State_Set states;
   uint64_t atspi_states = 0;

   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);

   if (!obj)
     {
        ERR("Atspi Object %s not found in cache!", obj_path);
        return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid object path.");
     }

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', "u");
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   eo_do(obj, states = elm_interface_atspi_accessible_state_set_get());

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
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   Eldbus_Message *ret;
   int idx = -1;

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eo_do(obj, idx = elm_interface_atspi_accessible_index_in_parent_get());

   eldbus_message_arguments_append(ret, "i", idx);

   return ret;
}

static Eldbus_Message *
_accessible_child_at_index(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   Eo *child = NULL;
   Eina_List *children = NULL;
   int idx;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter;

   if (!eldbus_message_arguments_get(msg, "i", &idx))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   eo_do(obj, children = elm_interface_atspi_accessible_children_get());

   child = eina_list_nth(children, idx);
   _object_append_reference(iter, child);
   eina_list_free(children);

   return ret;
}

static Eldbus_Message *
_accessible_get_relation_set(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.NotSupported", "Relation states not implemented.");
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
   { NULL, NULL, NULL, NULL, 0 }
};

static Eldbus_Message *
_action_description_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *description, *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int idx;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "i", &idx))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eo_do(obj, description = elm_interface_atspi_action_description_get(idx));
   description = description ? description : "";
   eldbus_message_arguments_append(ret, "s", description);

   return ret;
}

static Eldbus_Message *
_action_name_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *name, *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int idx;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "i", &idx))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eo_do(obj, name = elm_interface_atspi_action_name_get(idx));
   name = name ? name : "";
   eldbus_message_arguments_append(ret, "s", name);

   return ret;
}

static Eldbus_Message *
_action_localized_name_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *name, *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int idx;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "i", &idx))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eo_do(obj, name = elm_interface_atspi_action_localized_name_get(idx));
   name = name ? name : "";
   eldbus_message_arguments_append(ret, "s", name);

   return ret;
}

static Eldbus_Message *
_action_key_binding_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *key, *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int idx;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "i", &idx))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eo_do(obj, key = elm_interface_atspi_action_keybinding_get(idx));
   key = key ? key : "";
   eldbus_message_arguments_append(ret, "s", key);

   return ret;
}

static Eldbus_Message *
_action_actions_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *action, *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   Eina_List *actions;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter, *iter_array;

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iter, NULL);

   iter_array = eldbus_message_iter_container_new(iter, 'a', "sss");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iter_array, NULL);

   eo_do(obj, actions = elm_interface_atspi_action_actions_get());

   int id = 0;
   EINA_LIST_FREE(actions, action)
     {
        const char *key, *descr;
        eo_do(obj, key = elm_interface_atspi_action_keybinding_get(id));
        key = key ? key : "";
        eo_do(obj, descr = elm_interface_atspi_action_description_get(id));
        descr = descr ? descr : "";
        eldbus_message_iter_arguments_append(iter_array, "sss", action, descr, key);
        id++;
     }

  eldbus_message_iter_container_close(iter, iter_array);

   return ret;
}

static Eldbus_Message *
_action_action_do(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int idx;
   Eldbus_Message *ret;
   Eina_Bool result;

   if (!eldbus_message_arguments_get(msg, "i", &idx))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eo_do(obj, result = elm_interface_atspi_action_action_do(idx));
   eldbus_message_arguments_append(ret, "b", result);

   return ret;
}

static const Eldbus_Method action_methods[] = {
   { "GetDescription", ELDBUS_ARGS({"i", "index"}), ELDBUS_ARGS({"s", "description"}), _action_description_get, 0 },
   { "GetName", ELDBUS_ARGS({"i", "index"}), ELDBUS_ARGS({"s", "name"}), _action_name_get, 0 },
   { "GetLocalizedName", ELDBUS_ARGS({"i", "index"}), ELDBUS_ARGS({"s", "name"}), _action_localized_name_get, 0 },
   { "GetKeyBinding", ELDBUS_ARGS({"i", "index"}), ELDBUS_ARGS({"s", "key"}), _action_key_binding_get, 0 },
   { "GetActions", NULL, ELDBUS_ARGS({"a(sss)", "actions"}), _action_actions_get, 0 },
   { "DoAction", ELDBUS_ARGS({"i", "index"}), ELDBUS_ARGS({"b", "result"}), _action_action_do, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static Eldbus_Message *
_image_extents_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   AtspiCoordType type;
   Eldbus_Message *ret;
   const char *obj_path = eldbus_service_object_path_get(iface);
   int x, y, w, h;
   Eina_Bool screen_coords;
   Eo *obj = _access_object_from_path(obj_path);

   x = y = w = h = -1;

   if (!eldbus_message_arguments_get(msg, "u", &type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   screen_coords = type == ATSPI_COORD_TYPE_SCREEN ? EINA_TRUE : EINA_FALSE;
   eo_do(obj, elm_interface_atspi_image_extents_get(screen_coords, &x, &y, &w, &h));
   eldbus_message_arguments_append(ret, "iiii", x, y, w, h);

   return ret;
}

static Eldbus_Message *
_image_position_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   AtspiCoordType type;
   Eldbus_Message *ret;
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int x = -1, y = -1;
   Eina_Bool screen_coords;

   if (!eldbus_message_arguments_get(msg, "u", &type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   screen_coords = type == ATSPI_COORD_TYPE_SCREEN ? EINA_TRUE : EINA_FALSE;
   eo_do(obj, elm_interface_atspi_image_extents_get(screen_coords, &x, &y, NULL, NULL));
   eldbus_message_arguments_append(ret, "i", x);
   eldbus_message_arguments_append(ret, "i", y);

   return ret;
}

static Eldbus_Message *
_image_size_get(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eldbus_Message *ret;
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int w = -1, h = -1;

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eo_do(obj, elm_interface_atspi_image_extents_get(EINA_FALSE, NULL, NULL, &w, &h));
   eldbus_message_arguments_append(ret, "i", w);
   eldbus_message_arguments_append(ret, "i", h);

   return ret;
}

static const Eldbus_Method image_methods[] = {
   { "GetImageExtents", ELDBUS_ARGS({"u", "coordType"}), ELDBUS_ARGS({"iiii", "extents"}), _image_extents_get, 0 },
   { "GetImagePosition", ELDBUS_ARGS({"u", "coordType"}), ELDBUS_ARGS({"i", "x"}, {"i", "y"}), _image_position_get, 0 },
   { "GetImageSize", NULL, ELDBUS_ARGS({"i", "width"}, {"i", "height"}), _image_size_get, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static Eo *
_access_object_from_path(const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   unsigned long long eo_ptr = 0;
   Eo *eo = NULL;
   const char *tmp = path;

   int len = strlen(ELM_ACCESS_OBJECT_PATH_PREFIX);

   if (strncmp(path, ELM_ACCESS_OBJECT_PATH_PREFIX, len))
     return NULL;

   tmp = path + len; /* Skip over the prefix */
   if (!strcmp(ELM_ACCESS_OBJECT_PATH_ROOT, tmp))
     return _root;

   sscanf(tmp, "%llu", &eo_ptr);
   eo = (Eo *) (uintptr_t) eo_ptr;
   return eo_isa(eo, ELM_INTERFACE_ATSPI_ACCESSIBLE_CLASS) ? eo : NULL;
}

static char *
_path_from_access_object(Eo *eo)
{
   char path[256];

   EINA_SAFETY_ON_NULL_RETURN_VAL(eo, strdup(ATSPI_DBUS_PATH_NULL));
   if (eo == _root)
     snprintf(path, sizeof(path), "%s%s", ELM_ACCESS_OBJECT_PATH_PREFIX, ELM_ACCESS_OBJECT_PATH_ROOT);
   else
     snprintf(path, sizeof(path), ELM_ACCESS_OBJECT_REFERENCE_TEMPLATE, (unsigned long long) (uintptr_t)eo);
   return strdup(path);
}

static Eina_Bool
_accessible_property_get(const Eldbus_Service_Interface *interface, const char *property,
                         Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED,
                         Eldbus_Message **error EINA_UNUSED)
{
   const char *ret = NULL, *obj_path = eldbus_service_object_path_get(interface);
   Eo *ret_obj = NULL, *obj = _access_object_from_path(obj_path);

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   if (!strcmp(property, "Name"))
     {
        eo_do(obj, ret = elm_interface_atspi_accessible_name_get());
        if (!ret)
          ret = "";
        eldbus_message_iter_basic_append(iter, 's', ret);
        return EINA_TRUE;
     }
   else if (!strcmp(property, "Description"))
     {
        eo_do(obj, ret = elm_interface_atspi_accessible_description_get());
        if (!ret)
          ret = "";
        eldbus_message_iter_basic_append(iter, 's', ret);
        return EINA_TRUE;
     }
   else if (!strcmp(property, "Parent"))
     {
       eo_do(obj, ret_obj = elm_interface_atspi_accessible_parent_get());
       Elm_Atspi_Role role = ELM_ATSPI_ROLE_INVALID;
       eo_do(obj, role = elm_interface_atspi_accessible_role_get());
       if ((!ret_obj) && (ELM_ATSPI_ROLE_APPLICATION == role))
         _object_append_desktop_reference(iter);
       else
         _object_append_reference(iter, ret_obj);
       return EINA_TRUE;
     }
   else if (!strcmp(property, "ChildCount"))
     {
        Eina_List *l = NULL;
        eo_do(obj, l = elm_interface_atspi_accessible_children_get());
        eldbus_message_iter_basic_append(iter, 'i', eina_list_count(l));
        eina_list_free(l);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_action_property_get(const Eldbus_Service_Interface *interface, const char *property,
                         Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED,
                         Eldbus_Message **error EINA_UNUSED)
{
   Eina_List *actions;
   const char *obj_path = eldbus_service_object_path_get(interface);
   Eo *obj = _access_object_from_path(obj_path);

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   if (!strcmp(property, "NActions"))
     {
        eo_do(obj, actions = elm_interface_atspi_action_actions_get());
        eldbus_message_iter_basic_append(iter, 'i', eina_list_count(actions));
        eina_list_free(actions);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eldbus_Message*
_value_properties_set(const Eldbus_Service_Interface *interface, const char *property,
                         Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg)
{
   double value;
   Eina_Bool ret;
   const char *obj_path = eldbus_service_object_path_get(interface);
   Eo *obj = _access_object_from_path(obj_path);

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   if (!eldbus_message_iter_arguments_get(iter, "d", &value))
     {
       return eldbus_message_error_new(request_msg, "org.freedesktop.DBus.Error.InvalidArgs", "Expected value of type: double.");
     }

   if (!strcmp(property, "CurrentValue"))
     {
        eo_do(obj, ret = elm_interface_atspi_value_value_and_text_set(value, NULL));
        Eldbus_Message *answer = eldbus_message_method_return_new(request_msg);
        eldbus_message_arguments_append(answer, "b", ret);
        return answer;
     }

   return NULL;
}

static Eina_Bool
_value_properties_get(const Eldbus_Service_Interface *interface, const char *property,
                         Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED,
                         Eldbus_Message **error EINA_UNUSED)
{
   double value;
   const char *obj_path = eldbus_service_object_path_get(interface);
   Eo *obj = _access_object_from_path(obj_path);

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   if (!strcmp(property, "CurrentValue"))
     {
        eo_do(obj, elm_interface_atspi_value_value_and_text_get(&value, NULL));
        eldbus_message_iter_basic_append(iter, 'd', value);
        return EINA_TRUE;
     }
   if (!strcmp(property, "MinimumValue"))
     {
        eo_do(obj, elm_interface_atspi_value_range_get(&value, NULL, NULL));
        eldbus_message_iter_basic_append(iter, 'd', value);
        return EINA_TRUE;
     }
   if (!strcmp(property, "MaximumValue"))
     {
        eo_do(obj, elm_interface_atspi_value_range_get(NULL, &value, NULL));
        eldbus_message_iter_basic_append(iter, 'd', value);
        return EINA_TRUE;
     }
   if (!strcmp(property, "MinimumIncrement"))
     {
        eo_do(obj, value = elm_interface_atspi_value_increment_get());
        eldbus_message_iter_basic_append(iter, 'd', value);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_image_properties_get(const Eldbus_Service_Interface *interface, const char *property,
                         Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED,
                         Eldbus_Message **error EINA_UNUSED)
{
   const char *value;
   const char *obj_path = eldbus_service_object_path_get(interface);
   Eo *obj = _access_object_from_path(obj_path);

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   if (!strcmp(property, "ImageDescription"))
     {
        eo_do(obj, value = elm_interface_atspi_image_description_get());
        value = value ? value : "";
        eldbus_message_iter_basic_append(iter, 's', value);
        return EINA_TRUE;
     }
   if (!strcmp(property, "ImageLocale"))
     {
        eo_do(obj, value = elm_interface_atspi_image_locale_get());
        value = value ? value : "";
        eldbus_message_iter_basic_append(iter, 's', value);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static const Eldbus_Property accessible_properties[] = {
   { "Name", "s", _accessible_property_get, NULL, 0 },
   { "Description", "s", _accessible_property_get, NULL, 0 },
   { "Parent", "(so)", _accessible_property_get, NULL, 0 },
   { "ChildCount", "i", _accessible_property_get, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Property action_properties[] = {
   { "NActions", "i", _action_property_get, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Property value_properties[] = {
   { "MinimumValue", "d", NULL, NULL, 0 },
   { "MaximumValue", "d", NULL, NULL, 0 },
   { "MinimumIncrement", "d", NULL, NULL, 0 },
   { "CurrentValue", "d", NULL, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Property image_properties[] = {
   { "ImageDescription", "s", NULL, NULL, 0 },
   { "ImageLocale", "s", NULL, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc accessible_iface_desc = {
   ATSPI_DBUS_INTERFACE_ACCESSIBLE, accessible_methods, NULL, accessible_properties, _accessible_property_get, NULL
};

static const Eldbus_Service_Interface_Desc event_iface_desc = {
   ATSPI_DBUS_INTERFACE_EVENT_OBJECT, NULL, _event_obj_signals, NULL, NULL, NULL
};

static const Eldbus_Service_Interface_Desc window_iface_desc = {
   ATSPI_DBUS_INTERFACE_EVENT_WINDOW, NULL, _window_obj_signals, NULL, NULL, NULL
};

static const Eldbus_Service_Interface_Desc action_iface_desc = {
   ATSPI_DBUS_INTERFACE_ACTION, action_methods, NULL, action_properties, NULL, NULL
};

static const Eldbus_Service_Interface_Desc value_iface_desc = {
   ATSPI_DBUS_INTERFACE_VALUE, NULL, NULL, value_properties, _value_properties_get, _value_properties_set
};

static const Eldbus_Service_Interface_Desc image_iface_desc = {
   ATSPI_DBUS_INTERFACE_IMAGE, image_methods, NULL, image_properties, _image_properties_get, NULL
};

static void
_object_append_reference(Eldbus_Message_Iter *iter, Eo *obj)
{
  Eldbus_Message_Iter *iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
  EINA_SAFETY_ON_NULL_RETURN(iter);
  char *path = _path_from_access_object(obj);
  eldbus_message_iter_basic_append(iter_struct, 's', eldbus_connection_unique_name_get(_a11y_bus));
  eldbus_message_iter_basic_append(iter_struct, 'o', path);
  eldbus_message_iter_container_close(iter, iter_struct);
  free(path);
}

static void
_object_append_desktop_reference(Eldbus_Message_Iter *iter)
{
  Eldbus_Message_Iter *iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
  EINA_SAFETY_ON_NULL_RETURN(iter);

  eldbus_message_iter_basic_append(iter_struct, 's', ATSPI_DBUS_NAME_REGISTRY);
  eldbus_message_iter_basic_append(iter_struct, 'o', ATSPI_DBUS_PATH_ROOT);
  eldbus_message_iter_container_close(iter, iter_struct);
}

static Eina_Bool
_append_item_fn(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
  if (!eo_ref_get(data) || eo_destructed_is(data))
    return EINA_TRUE;

  Eldbus_Message_Iter *iter_struct, *iter_sub_array;
  Eldbus_Message_Iter *iter_array = fdata;
  Elm_Atspi_State_Set states;
  Elm_Atspi_Role role;

  eo_do(data, role = elm_interface_atspi_accessible_role_get());

  iter_struct = eldbus_message_iter_container_new(iter_array, 'r', NULL);
  EINA_SAFETY_ON_NULL_RETURN_VAL(iter_struct, EINA_TRUE);

  /* Marshall object path */
  _object_append_reference(iter_struct, data);

  /* Marshall application */
  _object_append_reference(iter_struct, _root);

  Eo *parent = NULL;
  eo_do(data, parent = elm_interface_atspi_accessible_parent_get());
  /* Marshall parent */
  if ((!parent) && (ELM_ATSPI_ROLE_APPLICATION == role))
    _object_append_desktop_reference(iter_struct);
  else
    _object_append_reference(iter_struct, parent);

  /* Marshall children  */
  Eina_List *children_list = NULL, *l;
  Eo *child;

  eo_do(data, children_list = elm_interface_atspi_accessible_children_get());
  iter_sub_array = eldbus_message_iter_container_new(iter_struct, 'a', "(so)");
  EINA_SAFETY_ON_NULL_GOTO(iter_sub_array, fail);

  EINA_LIST_FOREACH(children_list, l, child)
     _object_append_reference(iter_sub_array, child);

  eldbus_message_iter_container_close(iter_struct, iter_sub_array);
  eina_list_free(children_list);

  /* Marshall interfaces */
  iter_sub_array = eldbus_message_iter_container_new(iter_struct, 'a', "s");
  EINA_SAFETY_ON_NULL_GOTO(iter_sub_array, fail);

  eldbus_message_iter_basic_append(iter_sub_array, 's', ATSPI_DBUS_INTERFACE_ACCESSIBLE);
  if (eo_isa(data, ELM_INTERFACE_ATSPI_COMPONENT_CLASS))
    eldbus_message_iter_basic_append(iter_sub_array, 's', ATSPI_DBUS_INTERFACE_COMPONENT);
  if (eo_isa(data, ELM_INTERFACE_ATSPI_ACTION_CLASS))
    eldbus_message_iter_basic_append(iter_sub_array, 's', ATSPI_DBUS_INTERFACE_ACTION);
  if (eo_isa(data, ELM_INTERFACE_ATSPI_VALUE_CLASS))
    eldbus_message_iter_basic_append(iter_sub_array, 's', ATSPI_DBUS_INTERFACE_VALUE);
  if (eo_isa(data, ELM_INTERFACE_ATSPI_IMAGE_CLASS))
    eldbus_message_iter_basic_append(iter_sub_array, 's', ATSPI_DBUS_INTERFACE_IMAGE);

  eldbus_message_iter_container_close(iter_struct, iter_sub_array);

  /* Marshall name */
  const char *name = NULL;
  eo_do(data, name = elm_interface_atspi_accessible_name_get());
  if (!name)
    name = "";
  eldbus_message_iter_basic_append(iter_struct, 's', name);

  /* Marshall role */
  eldbus_message_iter_basic_append(iter_struct, 'u', role);

  /* Marshall description */
  const char* descritpion = NULL;
  eo_do(data, descritpion = elm_interface_atspi_accessible_description_get());
  if (!descritpion)
    descritpion = "";
  eldbus_message_iter_basic_append(iter_struct, 's', descritpion);

  /* Marshall state set */
  iter_sub_array = eldbus_message_iter_container_new(iter_struct, 'a', "u");
  EINA_SAFETY_ON_NULL_GOTO(iter_sub_array, fail);

  eo_do(data, states = elm_interface_atspi_accessible_state_set_get());

  unsigned int s1 = states & 0xFFFFFFFF;
  unsigned int s2 = (states >> 32) & 0xFFFFFFFF;
  eldbus_message_iter_basic_append(iter_sub_array, 'u', s1);
  eldbus_message_iter_basic_append(iter_sub_array, 'u', s2);

  eldbus_message_iter_container_close(iter_struct, iter_sub_array);
  eldbus_message_iter_container_close(iter_array, iter_struct);

  return EINA_TRUE;

fail:
  if (iter_struct) eldbus_message_iter_del(iter_struct);
  return EINA_TRUE;
}

static Eldbus_Message *
_cache_get_items(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message_Iter *iter, *iter_array;
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', CACHE_ITEM_SIGNATURE);
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   eina_hash_foreach(_cache, _append_item_fn, iter_array);
   eldbus_message_iter_container_close(iter, iter_array);

   return ret;
fail:
   if (ret) eldbus_message_unref(ret);
   return NULL;
}

static const Eldbus_Method cache_methods[] = {
   { "GetItems", NULL, ELDBUS_ARGS({CACHE_ITEM_SIGNATURE, "items"}), _cache_get_items, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Signal cache_signals[] = {
  [ATSPI_OBJECT_CHILD_ADDED] = { "AddAccessible", ELDBUS_ARGS({"((so)(so)a(so)assusau)", "added"}), 0},
  [ATSPI_OBJECT_CHILD_REMOVED] = { "RemoveAccessible", ELDBUS_ARGS({ "(so)", "removed" }), 0},
  {NULL, NULL, 0}
};

static const Eldbus_Service_Interface_Desc cache_iface_desc = {
   ATSPI_DBUS_INTERFACE_CACHE, cache_methods, cache_signals, NULL, NULL, NULL
};

// Component interface
static Eldbus_Message *
_component_contains(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int x, y;
   Eina_Bool contains = EINA_FALSE;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "iiu", &x, &y, &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   Eina_Bool type = coord_type == ATSPI_COORD_TYPE_SCREEN ? EINA_TRUE : EINA_FALSE;
   eo_do(obj, contains = elm_interface_atspi_component_contains(type, x, y));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eldbus_message_arguments_append(ret, "b", contains);

   return ret;
}

static Eldbus_Message *
_component_get_accessible_at_point(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int x, y;
   Eo *accessible = NULL;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter;

   if (!eldbus_message_arguments_get(msg, "iiu", &x, &y, &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   Eina_Bool type = coord_type == ATSPI_COORD_TYPE_SCREEN ? EINA_TRUE : EINA_FALSE;
   eo_do(obj, accessible = elm_interface_atspi_component_accessible_at_point_get(type, x, y));
   _object_append_reference(iter, accessible);

   return ret;
}

static Eldbus_Message *
_component_get_extents(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int x, y, w, h;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter, *iter_struct;
   if (!eldbus_message_arguments_get(msg, "u", &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);

   Eina_Bool type = coord_type == ATSPI_COORD_TYPE_SCREEN ? EINA_TRUE : EINA_FALSE;
   eo_do(obj, elm_interface_atspi_component_extents_get(type, &x, &y, &w, &h));
   iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
   EINA_SAFETY_ON_NULL_GOTO(iter_struct, fail);

   eldbus_message_iter_basic_append(iter_struct, 'i', x);
   eldbus_message_iter_basic_append(iter_struct, 'i', y);
   eldbus_message_iter_basic_append(iter_struct, 'i', w);
   eldbus_message_iter_basic_append(iter_struct, 'i', h);

   eldbus_message_iter_container_close(iter, iter_struct);

   return ret;
fail:
   if (ret) eldbus_message_unref(ret);
   return NULL;
}

static Eldbus_Message *
_component_get_position(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int x, y;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "u", &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   Eina_Bool type = coord_type == ATSPI_COORD_TYPE_SCREEN ? EINA_TRUE : EINA_FALSE;
   eo_do(obj, elm_interface_atspi_component_position_get(type, &x, &y));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eldbus_message_arguments_append(ret, "i", x);
   eldbus_message_arguments_append(ret, "i", y);

   return ret;
}

static Eldbus_Message *
_component_get_size(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int x, y;
   Eldbus_Message *ret;

   eo_do(obj, elm_interface_atspi_component_size_get(&x, &y));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eldbus_message_arguments_append(ret, "i", x);
   eldbus_message_arguments_append(ret, "i", y);

   return ret;
}

static AtspiComponentLayer
_elm_layer_2_atspi_layer(int layer)
{
   if (layer <= ELM_OBJECT_LAYER_BACKGROUND) return ATSPI_LAYER_CANVAS;
   if (layer < ELM_OBJECT_LAYER_FOCUS) return ATSPI_LAYER_WIDGET;
   if (layer <= ELM_OBJECT_LAYER_TOOLTIP) return ATSPI_LAYER_POPUP;

   return ATSPI_LAYER_OVERLAY;
}

static Eldbus_Message *
_component_get_layer(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int layer = 0;
   Eldbus_Message *ret;
   AtspiComponentLayer atspi_layer;

   eo_do(obj, layer = elm_interface_atspi_component_layer_get());

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   atspi_layer = _elm_layer_2_atspi_layer(layer);
   eldbus_message_arguments_append(ret, "u", atspi_layer);

   return ret;
}

static Eldbus_Message *
_component_grab_focus(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   Eldbus_Message *ret;
   Eina_Bool focus = EINA_FALSE;

   eo_do(obj, focus = elm_interface_atspi_component_focus_grab());

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eldbus_message_arguments_append(ret, "b", focus);

   return ret;
}

static Eldbus_Message *
_component_get_alpha(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   Eldbus_Message *ret;
   double alpha = 0;

   eo_do(obj, alpha = elm_interface_atspi_component_alpha_get());

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eldbus_message_arguments_append(ret, "d", alpha);

   return ret;
}

static Eldbus_Message *
_component_set_extends(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int x, y, w, h;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;
   Eina_Bool result = EINA_FALSE;

   if (!eldbus_message_arguments_get(msg, "iiiiu", &x, &y, &w, &h, &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   Eina_Bool type = coord_type == ATSPI_COORD_TYPE_SCREEN ? EINA_TRUE : EINA_FALSE;
   eo_do(obj, result = elm_interface_atspi_component_extents_set(type, x, y, w, h));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eldbus_message_arguments_append(ret, "b", result);

   return ret;
}

static Eldbus_Message *
_component_set_position(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int x, y;
   Eina_Bool result = EINA_FALSE;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "iiu", &x, &y, &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   Eina_Bool type = coord_type == ATSPI_COORD_TYPE_SCREEN ? EINA_TRUE : EINA_FALSE;
   eo_do(obj, result = elm_interface_atspi_component_position_set(type, x, y));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eldbus_message_arguments_append(ret, "b", result);

   return ret;
}

static Eldbus_Message *
_component_set_size(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Eo *obj = _access_object_from_path(obj_path);
   int w, h;
   Eina_Bool result;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "ii", &w, &h))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   eo_do(obj, result = elm_interface_atspi_component_size_set(w, h));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eldbus_message_arguments_append(ret, "b", result);

   return ret;
}

static const Eldbus_Method component_methods[] = {
   { "Contains", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"u", "coord_type"}), ELDBUS_ARGS({"b", "contains"}), _component_contains, 0 },
   { "GetAccessibleAtPoint", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"u", "coord_type"}), ELDBUS_ARGS({"(so)", "accessible"}), _component_get_accessible_at_point, 0 },
   { "GetExtents", ELDBUS_ARGS({"u", "coord_type"}), ELDBUS_ARGS({"(iiii)", "extents"}), _component_get_extents, 0 },
   { "GetPosition", ELDBUS_ARGS({"u", "coord_type"}), ELDBUS_ARGS({"i", "x"}, {"i","y"}), _component_get_position, 0 },
   { "GetSize", NULL, ELDBUS_ARGS({"i", "w"}, {"i", "h"}), _component_get_size, 0 },
   { "GetLayer", NULL, ELDBUS_ARGS({"u", "layer"}), _component_get_layer, 0 },
//   { "GetMDIZOrder", NULL, ELDBUS_ARGS({"n", "MDIZOrder"}), _component_get_mdizorder, 0 },
   { "GrabFocus", NULL, ELDBUS_ARGS({"b", "focus"}), _component_grab_focus, 0 },
   { "GetAlpha", NULL, ELDBUS_ARGS({"d", "alpha"}), _component_get_alpha, 0 },
   { "SetExtents", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"i", "width"}, {"i", "height"}, {"u", "coord_type"}), ELDBUS_ARGS({"b", "result"}), _component_set_extends, 0 },
   { "SetPosition", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"u", "coord_type"}), ELDBUS_ARGS({"b", "result"}), _component_set_position, 0 },
   { "SetSize", ELDBUS_ARGS({"i", "width"}, {"i", "height"}), ELDBUS_ARGS({"b", "result"}), _component_set_size, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc component_iface_desc = {
   ATSPI_DBUS_INTERFACE_COMPONENT, component_methods, NULL, NULL, NULL, NULL
};

static void
_on_elm_atspi_bridge_app_register(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }
   DBG("Application successfuly registered at ATSPI2 bus.");
}

EAPI Eina_Bool
_elm_atspi_bridge_app_register(void)
{
   if (!_a11y_bus || !_root) return EINA_FALSE;
   Eldbus_Message *message = eldbus_message_method_call_new(ATSPI_DBUS_NAME_REGISTRY,
                                    ATSPI_DBUS_PATH_ROOT,
                                    ATSPI_DBUS_INTERFACE_SOCKET,
                                    "Embed");
   Eldbus_Message_Iter *iter = eldbus_message_iter_get(message);

   _object_append_reference(iter, _root);
   eldbus_connection_send(_a11y_bus, message, _on_elm_atspi_bridge_app_register, NULL, -1);

   return EINA_TRUE;
}

EAPI Eina_Bool
_elm_atspi_bridge_app_unregister(void)
{
   if (!_a11y_bus || !_root) return EINA_FALSE;
   Eldbus_Message *message = eldbus_message_method_call_new(ATSPI_DBUS_NAME_REGISTRY,
                                    ATSPI_DBUS_PATH_ROOT,
                                    ATSPI_DBUS_INTERFACE_SOCKET,
                                    "Unembed");
   Eldbus_Message_Iter *iter = eldbus_message_iter_get(message);

   _object_append_reference(iter, _root);
   eldbus_connection_send(_a11y_bus, message, NULL, NULL, -1);

   return EINA_TRUE;
}

static void
_cache_register(void)
{
   _cache_interface = eldbus_service_interface_register(_a11y_bus, CACHE_INTERFACE_PATH, &cache_iface_desc);
}

static void
_set_broadcast_flag(const char *event)
{
   char **tokens;

   tokens = eina_str_split(event, ":", 3);

   if (!tokens) return;

   if (!strcmp(tokens[0], "Object"))
     {
        if (!tokens[1] || *tokens[1] == '\0') return; // do not handle "Object:*"
        else if (!strcmp(tokens[1], "StateChanged"))
          {
             if (!tokens[2] || *tokens[2] == '\0')
               _object_state_broadcast_mask = -1; // broadcast all
             else if (!strcmp(tokens[2], "Focused"))
               STATE_TYPE_SET(_object_state_broadcast_mask, ATSPI_STATE_FOCUSED);
             else if (!strcmp(tokens[2], "Showing"))
               STATE_TYPE_SET(_object_state_broadcast_mask, ATSPI_STATE_SHOWING);
          }
        else if (!strcmp(tokens[1], "PropertyChange"))
          {
             if (!tokens[2] || *tokens[2] == '\0')
               _object_property_broadcast_mask = -1; //broadcast all
             else if (!strcmp(tokens[2], "AccessibleValue"))
               STATE_TYPE_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_VALUE);
             else if (!strcmp(tokens[2], "AccessibleName"))
               STATE_TYPE_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_NAME);
             else if (!strcmp(tokens[2], "AccessibleDescription"))
               STATE_TYPE_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_DESCRIPTION);
             else if (!strcmp(tokens[2], "AccessibleParent"))
               STATE_TYPE_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_PARENT);
             else if (!strcmp(tokens[2], "AccessibleRole"))
               STATE_TYPE_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_ROLE);
          }
        else if (!strcmp(tokens[1], "ChildrenChanged"))
          {
             if (!tokens[2] || *tokens[2] == '\0')
               _object_children_broadcast_mask = -1; // broadcast all
             else if (!strcmp(tokens[2], "add"))
               STATE_TYPE_SET(_object_children_broadcast_mask, ATSPI_OBJECT_CHILD_ADDED);
             else if (!strcmp(tokens[2], "remove"))
               STATE_TYPE_SET(_object_children_broadcast_mask, ATSPI_OBJECT_CHILD_ADDED);
          }
     }
   else if (!strcmp(tokens[0], "Window"))
     {
        if (!tokens[1] || *tokens[1] == '\0')
          _window_signal_broadcast_mask = -1; // broadcast all
        else if (!strcmp(tokens[1], "Create"))
          STATE_TYPE_SET(_window_signal_broadcast_mask, ATSPI_WINDOW_EVENT_CREATE);
        else if (!strcmp(tokens[1], "Activate"))
          STATE_TYPE_SET(_window_signal_broadcast_mask, ATSPI_WINDOW_EVENT_ACTIVATE);
        else if (!strcmp(tokens[1], "Deactivate"))
          STATE_TYPE_SET(_window_signal_broadcast_mask, ATSPI_WINDOW_EVENT_DEACTIVATE);
     }

   free(tokens[0]);
   free(tokens);
}

static void
_registered_listeners_get(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   DBG("Updating registered ATSPI signals list.");
   _object_children_broadcast_mask = 0;
   _object_property_broadcast_mask = 0;
   _object_state_broadcast_mask = 0;
   _window_signal_broadcast_mask = 0;

   const char *event, *bus;
   Eldbus_Message_Iter *iter, *siter;
   if (!eldbus_message_arguments_get(msg, "a(ss)", &iter))
     {
        ERR("Invalid answer type from GetRegisteredEvents method call!");
        return;
     }
   while (eldbus_message_iter_get_and_next(iter, 'r', &siter))
     {
        eldbus_message_iter_arguments_get(siter, "ss", &bus, &event);
        _set_broadcast_flag(event);
     }
}

static void
_registered_events_list_update(void)
{
   Eldbus_Message *msg;
   msg = eldbus_message_method_call_new(ATSPI_DBUS_NAME_REGISTRY, ATSPI_DBUS_PATH_REGISTRY, ATSPI_DBUS_INTERFACE_REGISTRY, "GetRegisteredEvents");
   eldbus_connection_send(_a11y_bus, msg, _registered_listeners_get, NULL, -1);
}

static void
_handle_listener_change(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *bus, *event;
   if (!eldbus_message_arguments_get(msg, "ss", &bus, &event))
     {
        ERR("Invalid org.a11y.Registry signal message args.");
        return;
     }
   _registered_events_list_update();
}

static Eina_Bool
_state_changed_signal_send(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *viter;
   Eldbus_Service_Interface *events = data;
   Elm_Atspi_Event_State_Changed_Data *state_data = event_info;
   char *type_desc;

   if (!STATE_TYPE_GET(_object_state_broadcast_mask, state_data->type))
     {
        DBG("Masking event: %d", state_data->type);
        return EINA_FALSE;
     }

   if (!events)
     {
        ERR("Atspi object does not have event interface!");
        return EINA_FALSE;
     }

   switch (state_data->type) {
        case ELM_ATSPI_STATE_FOCUSED:
         type_desc = "focused";
         break;
        case ELM_ATSPI_STATE_SHOWING:
         type_desc = "showing";
         break;
        case ELM_ATSPI_STATE_VISIBLE:
         type_desc = "visible";
         break;
        default:
         return EINA_FALSE;
   }

   msg = eldbus_service_signal_new(events, ATSPI_OBJECT_EVENT_STATE_CHANGED);
   iter = eldbus_message_iter_get(msg);

   eldbus_message_iter_arguments_append(iter, "sii", type_desc, state_data->new_value, 0);

   viter = eldbus_message_iter_container_new(iter, 'v', "i");
   EINA_SAFETY_ON_NULL_RETURN_VAL(viter, EINA_FALSE);

   eldbus_message_iter_arguments_append(viter, "i", 0);
   eldbus_message_iter_container_close(iter, viter);

   _object_append_reference(iter, obj);

   eldbus_service_signal_send(events, msg);
   DBG("signal sent StateChanged:%s:%d", type_desc, state_data->new_value);

   return EINA_TRUE;
}

static Eina_Bool
_property_changed_signal_send(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *siter, *viter;
   Eldbus_Service_Interface *events = data;
   const char *property = event_info;
   char *path, *atspi_desc;
   enum _Atspi_Object_Property prop = ATSPI_OBJECT_PROPERTY_LAST;

   if (!events)
     {
        ERR("Atspi object does not have event interface!");
        return EINA_FALSE;
     }

   if (!strcmp(property, "parent"))
     {
        prop = ATSPI_OBJECT_PROPERTY_PARENT;
        atspi_desc = "accessible-parent";
     }
   else if (!strcmp(property, "name"))
     {
        prop = ATSPI_OBJECT_PROPERTY_NAME;
        atspi_desc = "accessible-name";
     }
   else if (!strcmp(property, "description"))
     {
        prop = ATSPI_OBJECT_PROPERTY_PARENT;
        atspi_desc = "accessible-description";
     }
   else if (!strcmp(property, "role"))
     {
        prop = ATSPI_OBJECT_PROPERTY_PARENT;
        atspi_desc = "accessible-role";
     }

   if (prop == ATSPI_OBJECT_PROPERTY_LAST)
     {
        ERR("Unrecognized property name!");
        return EINA_FALSE;
     }
   if (!STATE_TYPE_GET(_object_property_broadcast_mask, prop))
     {
        DBG("Masking property %s changed event.", property);
        return EINA_FALSE;
     }

   msg = eldbus_service_signal_new(events, ATSPI_OBJECT_EVENT_PROPERTY_CHANGED);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, EINA_FALSE);

   iter = eldbus_message_iter_get(msg);
   siter = eldbus_message_iter_container_new(iter, 'r', NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(siter, EINA_FALSE);

   eldbus_message_iter_arguments_append(siter, "suu", atspi_desc, 0, 0);

   viter = eldbus_message_iter_container_new(siter, 'v', "s");
   EINA_SAFETY_ON_NULL_RETURN_VAL(viter, EINA_FALSE);

   path = _path_from_access_object(obj);
   eldbus_message_iter_arguments_append(viter, "s", path);
   free(path);

   eldbus_message_iter_arguments_append(siter, "v", viter);
   eldbus_message_iter_container_close(siter, viter);

   eldbus_message_iter_container_close(iter, siter);
   eldbus_service_signal_send(events, msg);
   DBG("signal sent PropertyChanged:%s", property);

   return EINA_TRUE;
}

static Eina_Bool
_idler_cb(void *data EINA_UNUSED)
{
   Eo *obj;
   EINA_LIST_FREE(_pending_objects, obj)
      _cache_build(obj);
   _pending_objects = NULL;
   _cache_update_idler = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_children_changed_signal_send(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Eldbus_Service_Interface *events = data;
   Eldbus_Message_Iter *iter, *viter;
   Eldbus_Message *msg;
   const char *atspi_desc = NULL;
   Elm_Atspi_Event_Children_Changed_Data *ev_data = event_info;
   int idx;
   enum _Atspi_Object_Child_Event_Type type;

   type = ev_data->is_added ? ATSPI_OBJECT_CHILD_ADDED : ATSPI_OBJECT_CHILD_REMOVED;

   // update cached objects
   if (ev_data->is_added)
     {
        _pending_objects = eina_list_append(_pending_objects, obj);
        if (!_cache_update_idler)
          _cache_update_idler = ecore_idler_add(_idler_cb, NULL);
     }

   if (!STATE_TYPE_GET(_object_children_broadcast_mask, type))
     return EINA_FALSE;

   if (!events)
     {
        ERR("Atspi object does not have event interface! %p %s", obj, eo_class_name_get(eo_class_get(obj)));
        return EINA_FALSE;
     }

   switch (type)
    {
     case ATSPI_OBJECT_CHILD_ADDED:
        atspi_desc = "add";
        eo_do(ev_data->child, idx = elm_interface_atspi_accessible_index_in_parent_get());
        break;
     case ATSPI_OBJECT_CHILD_REMOVED:
        atspi_desc = "remove";
        idx = -1;
        break;
    }

   if (!atspi_desc) return EINA_FALSE;

   msg = eldbus_service_signal_new(events, ATSPI_OBJECT_EVENT_CHILDREN_CHANGED);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, EINA_FALSE);

   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_arguments_append(iter, "sii", atspi_desc, idx, 0);

   viter = eldbus_message_iter_container_new(iter, 'v', "(so)");
   EINA_SAFETY_ON_NULL_RETURN_VAL(viter, EINA_FALSE);

   _object_append_reference(viter, ev_data->child);
   eldbus_message_iter_container_close(iter, viter);

   _object_append_reference(iter, _root);

   eldbus_service_signal_send(events, msg);
   DBG("signal sent childrenChanged:%s:%d", atspi_desc, idx);

   return EINA_TRUE;
}

static Eina_Bool
_window_signal_send(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info EINA_UNUSED)
{
   const char *event_desc;
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *viter;
   Eldbus_Service_Interface *window = data;
   enum _Atspi_Window_Signals type;

   if (desc == ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_CREATED)
     {
        event_desc = "Created";
        type = ATSPI_WINDOW_EVENT_CREATE;
     }
   else if (desc == ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_DEACTIVATED)
     {
        event_desc = "Deactivate";
        type = ATSPI_WINDOW_EVENT_DEACTIVATE;
     }
   else if (desc == ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_ACTIVATED)
     {
        event_desc = "Activate";
        type = ATSPI_WINDOW_EVENT_ACTIVATE;
     }
   else
     {
        WRN("ATSPI Window event not handled");
        return EINA_FALSE;
     }

   if (!STATE_TYPE_GET(_window_signal_broadcast_mask, type))
     return EINA_FALSE;

   if (!window || !_a11y_bus)
     {
        ERR("A11Y connection closed. Unable to send ATSPI event.");
        return EINA_FALSE;
     }

   msg = eldbus_service_signal_new(window, type);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, EINA_FALSE);

   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_arguments_append(iter, "sii", event_desc, 0, 0);

   viter = eldbus_message_iter_container_new(iter, 'v', "i");
   EINA_SAFETY_ON_NULL_RETURN_VAL(viter, EINA_FALSE);

   eldbus_message_iter_arguments_append(viter, "i", 0);
   eldbus_message_iter_container_close(iter, viter);

   _object_append_reference(iter, obj);

   eldbus_service_signal_send(window, msg);
   DBG("signal sent Window:%s", event_desc);
   return EINA_TRUE;
}

static void
_event_handlers_register(void)
{
   _registered_events_list_update();

   // register signal handlers in order to update list of registered listeners of ATSPI-Clients
   _register_hdl = eldbus_signal_handler_add(_a11y_bus, ATSPI_DBUS_NAME_REGISTRY, ATSPI_DBUS_PATH_REGISTRY, ATSPI_DBUS_INTERFACE_REGISTRY, "EventListenerRegistered", _handle_listener_change, NULL);
   _unregister_hdl = eldbus_signal_handler_add(_a11y_bus, ATSPI_DBUS_NAME_REGISTRY, ATSPI_DBUS_PATH_REGISTRY, ATSPI_DBUS_INTERFACE_REGISTRY, "EventListenerDeregistered", _handle_listener_change, NULL);
}

static Eina_Bool
_on_cache_item_del(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *event EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char *path;
   path = _path_from_access_object(obj);
   eina_hash_del(_cache, path, obj);
   free(path);

   return EINA_TRUE;
}

static void
_cache_build(void *obj)
{
   Eina_List *children;
   Eo *child;
   char *path = NULL;

   if (!eo_isa(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_CLASS))
     return;

   path = _path_from_access_object(obj);
   if (!eina_hash_find(_cache, path))
     {
        eina_hash_add(_cache, path, obj);
        _object_register(obj, path);
        eo_do(obj, eo_event_callback_add(EO_EV_DEL, _on_cache_item_del, NULL));
     }
   free(path);

   eo_do(obj, children = elm_interface_atspi_accessible_children_get());
   EINA_LIST_FREE(children, child)
      _cache_build(child);
}

static void
_a11y_bus_initialize(const char *socket_addr)
{
   _a11y_bus = eldbus_address_connection_get(socket_addr);

   _cache_register();
   _event_handlers_register();
   _elm_atspi_bridge_app_register();
   _cache_build(_root);
}

static void
_a11y_bus_address_get(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg, *sock_addr = NULL;
   Eldbus_Connection *session_bus = data;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        goto end;
     }

   if (!eldbus_message_arguments_get(msg, "s", &sock_addr) || !sock_addr)
     {
        ERR("Could not get A11Y Bus socket address.");
        goto end;
     }

   _a11y_bus_initialize(sock_addr);

end:
   eldbus_connection_unref(session_bus);
}

static void _object_register(Eo *obj, char *path)
{
   Eldbus_Service_Interface *infc = NULL;

   if (!eo_isa(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_CLASS))
     {
        WRN("Unable to register class w/o Elm_Interface_Atspi_Accessible!");
        return;
     }

   if (_a11y_bus)
     {
        eldbus_service_interface_register(_a11y_bus, path, &accessible_iface_desc);

        infc = eldbus_service_interface_register(_a11y_bus, path, &event_iface_desc);
        eo_do(obj, eo_key_data_set("event_interface", infc, NULL));

        eo_do(obj, eo_event_callback_array_add(_events_cb(), infc));

        if (eo_isa(obj, ELM_INTERFACE_ATSPI_COMPONENT_CLASS))
          {
             infc = eldbus_service_interface_register(_a11y_bus, path, &component_iface_desc);
             eo_do(obj, eo_key_data_set("component_interface", infc, NULL));
          }

        if (eo_isa(obj, ELM_INTERFACE_ATSPI_WINDOW_CLASS))
          {
             infc = eldbus_service_interface_register(_a11y_bus, path, &window_iface_desc);
             eo_do(obj, eo_key_data_set("window_interface", infc, NULL));

             eo_do(obj, eo_event_callback_array_add(_window_cb(), infc));
          }
        if (eo_isa(obj, ELM_INTERFACE_ATSPI_ACTION_CLASS))
          eldbus_service_interface_register(_a11y_bus, path, &action_iface_desc);
        if (eo_isa(obj, ELM_INTERFACE_ATSPI_VALUE_CLASS))
          eldbus_service_interface_register(_a11y_bus, path, &value_iface_desc);
        if (eo_isa(obj, ELM_INTERFACE_ATSPI_IMAGE_CLASS))
          eldbus_service_interface_register(_a11y_bus, path, &image_iface_desc);
     }
}

static void _object_unregister(void *obj)
{
   Eldbus_Service_Interface *infc = NULL;

   eo_do(obj, infc = eo_key_data_get("event_interface"));
   if (_a11y_bus && infc)
     {
        eldbus_service_object_unregister(infc);
        eo_do(obj, eo_key_data_set("event_interface", NULL, NULL));
     }

   eo_do(obj, eo_event_callback_del(EO_EV_DEL, _on_cache_item_del, NULL));

   if (eo_isa(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_CLASS))
      eo_do(obj, eo_event_callback_array_del(_events_cb(), infc));

   if (eo_isa(obj, ELM_INTERFACE_ATSPI_WINDOW_CLASS))
     {
        eo_do(obj, infc = eo_key_data_get("window_interface"));
        eo_do(obj, eo_event_callback_array_del(_window_cb(), infc));
     }
}

void
_elm_atspi_bridge_init(void)
{
   Eldbus_Message *msg;
   Eldbus_Connection *session_bus;

   if (!_init_count && (_elm_config->atspi_mode != ELM_ATSPI_MODE_OFF))
     {
        _elm_atspi_init();

        _root = _elm_atspi_root_get();
        if (!_root)
          {
             ERR("Unable to get root object");
             return;
          }

        session_bus = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
        EINA_SAFETY_ON_NULL_RETURN(session_bus);
        msg = eldbus_message_method_call_new(A11Y_DBUS_NAME, A11Y_DBUS_PATH, A11Y_DBUS_INTERFACE, "GetAddress");
        if (!msg)
          {
             eldbus_connection_unref(session_bus);
             return;
          }
        eldbus_connection_send(session_bus, msg, _a11y_bus_address_get, session_bus, -1);
        _cache = eina_hash_string_superfast_new(_object_unregister);
        _init_count = 1;
     }
}

void
_elm_atspi_bridge_shutdown(void)
{
   if (_init_count)
     {
        _elm_atspi_shutdown();

        if (_cache_update_idler)
          ecore_idler_del(_cache_update_idler);
        _cache_update_idler = NULL;

        if (_pending_objects)
          eina_list_free(_pending_objects);
        _pending_objects = NULL;

        if (_cache)
          eina_hash_free(_cache);
        _cache = NULL;

        if (_a11y_bus)
          eldbus_connection_unref(_a11y_bus);
        _a11y_bus = NULL;

        _init_count = 0;
        _root = NULL;
     }
}
