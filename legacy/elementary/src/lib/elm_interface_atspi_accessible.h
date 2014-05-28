
/**
 * ATSPI object state set.
 */
typedef uint64_t Elm_Atspi_State_Set;

/*
 * Sets a particilar state type for given state set.
 */
#define STATE_TYPE_SET(state_set, type)   (state_set|= (1 << type))

/**
 * Unsets a particilar state type for given state set.
 */
#define STATE_TYPE_UNSET(state_set, type) (state_set &= ~(1 << type))

/**
 * Gets value of a particilar state type for given state set.
 */
#define STATE_TYPE_GET(state_set, type)   (state_set & (1 << type))


typedef enum _Elm_Atspi_Role Elm_Atspi_Role;

/**
 * @enum _Elm_Atspi_Role
 * Describes the role of an object visible to AT-SPI Clients.
 */
enum _Elm_Atspi_Role
{
    ELM_ATSPI_ROLE_INVALID,
    ELM_ATSPI_ROLE_ACCELERATOR_LABEL,
    ELM_ATSPI_ROLE_ALERT,
    ELM_ATSPI_ROLE_ANIMATION,
    ELM_ATSPI_ROLE_ARROW,
    ELM_ATSPI_ROLE_CALENDAR,
    ELM_ATSPI_ROLE_CANVAS,
    ELM_ATSPI_ROLE_CHECK_BOX,
    ELM_ATSPI_ROLE_CHECK_MENU_ITEM,
    ELM_ATSPI_ROLE_COLOR_CHOOSER,
    ELM_ATSPI_ROLE_COLUMN_HEADER,
    ELM_ATSPI_ROLE_COMBO_BOX,
    ELM_ATSPI_ROLE_DATE_EDITOR,
    ELM_ATSPI_ROLE_DESKTOP_ICON,
    ELM_ATSPI_ROLE_DESKTOP_FRAME,
    ELM_ATSPI_ROLE_DIAL,
    ELM_ATSPI_ROLE_DIALOG,
    ELM_ATSPI_ROLE_DIRECTORY_PANE,
    ELM_ATSPI_ROLE_DRAWING_AREA,
    ELM_ATSPI_ROLE_FILE_CHOOSER,
    ELM_ATSPI_ROLE_FILLER,
    ELM_ATSPI_ROLE_FOCUS_TRAVERSABLE,
    ELM_ATSPI_ROLE_FONT_CHOOSER,
    ELM_ATSPI_ROLE_FRAME,
    ELM_ATSPI_ROLE_GLASS_PANE,
    ELM_ATSPI_ROLE_HTML_CONTAINER,
    ELM_ATSPI_ROLE_ICON,
    ELM_ATSPI_ROLE_IMAGE,
    ELM_ATSPI_ROLE_INTERNAL_FRAME,
    ELM_ATSPI_ROLE_LABEL,
    ELM_ATSPI_ROLE_LAYERED_PANE,
    ELM_ATSPI_ROLE_LIST,
    ELM_ATSPI_ROLE_LIST_ITEM,
    ELM_ATSPI_ROLE_MENU,
    ELM_ATSPI_ROLE_MENU_BAR,
    ELM_ATSPI_ROLE_MENU_ITEM,
    ELM_ATSPI_ROLE_OPTION_PANE,
    ELM_ATSPI_ROLE_PAGE_TAB,
    ELM_ATSPI_ROLE_PAGE_TAB_LIST,
    ELM_ATSPI_ROLE_PANEL,
    ELM_ATSPI_ROLE_PASSWORD_TEXT,
    ELM_ATSPI_ROLE_POPUP_MENU,
    ELM_ATSPI_ROLE_PROGRESS_BAR,
    ELM_ATSPI_ROLE_PUSH_BUTTON,
    ELM_ATSPI_ROLE_RADIO_BUTTON,
    ELM_ATSPI_ROLE_RADIO_MENU_ITEM,
    ELM_ATSPI_ROLE_ROOT_PANE,
    ELM_ATSPI_ROLE_ROW_HEADER,
    ELM_ATSPI_ROLE_SCROLL_BAR,
    ELM_ATSPI_ROLE_SCROLL_PANE,
    ELM_ATSPI_ROLE_SEPARATOR,
    ELM_ATSPI_ROLE_SLIDER,
    ELM_ATSPI_ROLE_SPIN_BUTTON,
    ELM_ATSPI_ROLE_SPLIT_PANE,
    ELM_ATSPI_ROLE_STATUS_BAR,
    ELM_ATSPI_ROLE_TABLE,
    ELM_ATSPI_ROLE_TABLE_CELL,
    ELM_ATSPI_ROLE_TABLE_COLUMN_HEADER,
    ELM_ATSPI_ROLE_TABLE_ROW_HEADER,
    ELM_ATSPI_ROLE_TEAROFF_MENU_ITEM,
    ELM_ATSPI_ROLE_TERMINAL,
    ELM_ATSPI_ROLE_TEXT,
    ELM_ATSPI_ROLE_TOGGLE_BUTTON,
    ELM_ATSPI_ROLE_TOOL_BAR,
    ELM_ATSPI_ROLE_TOOL_TIP,
    ELM_ATSPI_ROLE_TREE,
    ELM_ATSPI_ROLE_TREE_TABLE,
    ELM_ATSPI_ROLE_UNKNOWN,
    ELM_ATSPI_ROLE_VIEWPORT,
    ELM_ATSPI_ROLE_WINDOW,
    ELM_ATSPI_ROLE_EXTENDED,
    ELM_ATSPI_ROLE_HEADER,
    ELM_ATSPI_ROLE_FOOTER,
    ELM_ATSPI_ROLE_PARAGRAPH,
    ELM_ATSPI_ROLE_RULER,
    ELM_ATSPI_ROLE_APPLICATION,
    ELM_ATSPI_ROLE_AUTOCOMPLETE,
    ELM_ATSPI_ROLE_EDITBAR,
    ELM_ATSPI_ROLE_EMBEDDED,
    ELM_ATSPI_ROLE_ENTRY,
    ELM_ATSPI_ROLE_CHART,
    ELM_ATSPI_ROLE_CAPTION,
    ELM_ATSPI_ROLE_DOCUMENT_FRAME,
    ELM_ATSPI_ROLE_HEADING,
    ELM_ATSPI_ROLE_PAGE,
    ELM_ATSPI_ROLE_SECTION,
    ELM_ATSPI_ROLE_REDUNDANT_OBJECT,
    ELM_ATSPI_ROLE_FORM,
    ELM_ATSPI_ROLE_LINK,
    ELM_ATSPI_ROLE_INPUT_METHOD_WINDOW,
    ELM_ATSPI_ROLE_TABLE_ROW,
    ELM_ATSPI_ROLE_TREE_ITEM,
    ELM_ATSPI_ROLE_DOCUMENT_SPREADSHEET,
    ELM_ATSPI_ROLE_DOCUMENT_PRESENTATION,
    ELM_ATSPI_ROLE_DOCUMENT_TEXT,
    ELM_ATSPI_ROLE_DOCUMENT_WEB,
    ELM_ATSPI_ROLE_DOCUMENT_EMAIL,
    ELM_ATSPI_ROLE_COMMENT,
    ELM_ATSPI_ROLE_LIST_BOX,
    ELM_ATSPI_ROLE_GROUPING,
    ELM_ATSPI_ROLE_IMAGE_MAP,
    ELM_ATSPI_ROLE_NOTIFICATION,
    ELM_ATSPI_ROLE_INFO_BAR,
    ELM_ATSPI_ROLE_LAST_DEFINED,
};

typedef enum _Elm_Atspi_State_Type Elm_Atspi_State_Type;

/**
 * @enum _Elm_Atspi_State_Type
 * Describes a possible states of an object visibile to AT-SPI clients.
 */
enum _Elm_Atspi_State_Type
{
    ELM_ATSPI_STATE_INVALID,
    ELM_ATSPI_STATE_ACTIVE,
    ELM_ATSPI_STATE_ARMED,
    ELM_ATSPI_STATE_BUSY,
    ELM_ATSPI_STATE_CHECKED,
    ELM_ATSPI_STATE_COLLAPSED,
    ELM_ATSPI_STATE_DEFUNCT,
    ELM_ATSPI_STATE_EDITABLE,
    ELM_ATSPI_STATE_ENABLED,
    ELM_ATSPI_STATE_EXPANDABLE,
    ELM_ATSPI_STATE_EXPANDED,
    ELM_ATSPI_STATE_FOCUSABLE,
    ELM_ATSPI_STATE_FOCUSED,
    ELM_ATSPI_STATE_HAS_TOOLTIP,
    ELM_ATSPI_STATE_HORIZONTAL,
    ELM_ATSPI_STATE_ICONIFIED,
    ELM_ATSPI_STATE_MODAL,
    ELM_ATSPI_STATE_MULTI_LINE,
    ELM_ATSPI_STATE_MULTISELECTABLE,
    ELM_ATSPI_STATE_OPAQUE,
    ELM_ATSPI_STATE_PRESSED,
    ELM_ATSPI_STATE_RESIZABLE,
    ELM_ATSPI_STATE_SELECTABLE,
    ELM_ATSPI_STATE_SELECTED,
    ELM_ATSPI_STATE_SENSITIVE,
    ELM_ATSPI_STATE_SHOWING,
    ELM_ATSPI_STATE_SINGLE_LINE,
    ELM_ATSPI_STATE_STALE,
    ELM_ATSPI_STATE_TRANSIENT,
    ELM_ATSPI_STATE_VERTICAL,
    ELM_ATSPI_STATE_VISIBLE,
    ELM_ATSPI_STATE_MANAGES_DESCENDANTS,
    ELM_ATSPI_STATE_INDETERMINATE,
    ELM_ATSPI_STATE_REQUIRED,
    ELM_ATSPI_STATE_TRUNCATED,
    ELM_ATSPI_STATE_ANIMATED,
    ELM_ATSPI_STATE_INVALID_ENTRY,
    ELM_ATSPI_STATE_SUPPORTS_AUTOCOMPLETION,
    ELM_ATSPI_STATE_SELECTABLE_TEXT,
    ELM_ATSPI_STATE_IS_DEFAULT,
    ELM_ATSPI_STATE_VISITED,
    ELM_ATSPI_STATE_LAST_DEFINED,
};

typedef enum _Elm_Atspi_Relation_Type Elm_Atspi_Relation_Type;

enum _Elm_Atspi_Relation_Type {
	ELM_ATSPI_RELATION_NULL,
	ELM_ATSPI_RELATION_LABEL_FOR,
	ELM_ATSPI_RELATION_LABELLED_BY,
	ELM_ATSPI_RELATION_CONTROLLER_FOR,
	ELM_ATSPI_RELATION_CONTROLLED_BY,
	ELM_ATSPI_RELATION_MEMBER_OF,
	ELM_ATSPI_RELATION_TOOLTIP_FOR,
	ELM_ATSPI_RELATION_NODE_CHILD_OF,
	ELM_ATSPI_RELATION_NODE_PARENT_OF,
	ELM_ATSPI_RELATION_EXTENDED,
	ELM_ATSPI_RELATION_FLOWS_TO,
	ELM_ATSPI_RELATION_FLOWS_FROM,
	ELM_ATSPI_RELATION_SUBWINDOW_OF,
	ELM_ATSPI_RELATION_EMBEDS,
	ELM_ATSPI_RELATION_EMBEDDED_BY,
	ELM_ATSPI_RELATION_POPUP_FOR,
	ELM_ATSPI_RELATION_PARENT_WINDOW_OF,
	ELM_ATSPI_RELATION_DESCRIPTION_FOR,
	ELM_ATSPI_RELATION_DESCRIBED_BY,
	ELM_ATSPI_RELATION_LAST_DEFINED,
};


typedef struct _Elm_Atspi_Event_State_Changed_Data Elm_Atspi_Event_State_Changed_Data;

struct _Elm_Atspi_Event_State_Changed_Data
{
   Elm_Atspi_State_Type type;
   Eina_Bool new_value;
};

typedef struct _Elm_Atspi_Event_Children_Changed_Data Elm_Atspi_Event_Children_Changed_Data;

struct _Elm_Atspi_Event_Children_Changed_Data
{
   Eina_Bool is_added;
   Eo *child;
};

typedef struct _Elm_Atspi_Attribute Elm_Atspi_Attribute;

struct _Elm_Atspi_Attribute
{
   const char *key;
   const char *value;
};

typedef struct _Elm_Atspi_Relation Elm_Atspi_Relation;

struct _Elm_Atspi_Relation
{
   Elm_Atspi_Relation_Type type;
   const Eo *obj;
};

/**
 * Free Elm_Atspi_Attributes_List
 */
void elm_atspi_attributes_list_free(Eina_List *list);

/**
 * Emits ATSPI 'StateChanged' dbus signal.
 */
#define elm_interface_atspi_accessible_state_changed_signal_emit(obj, tp, nvl) \
   { do { \
      Elm_Atspi_Event_State_Changed_Data evinfo; \
      evinfo.type = (tp); \
      evinfo.new_value = (nvl); \
      eo_do(obj, eo_event_callback_call(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_STATE_CHANGED, (void*)&evinfo)); \
   } while(0); }

/**
 * Emits ATSPI 'PropertyChanged' dbus signal for 'Name' property.
 */
#define elm_interface_atspi_accessible_name_changed_signal_emit(obj) \
   eo_do(obj, eo_event_callback_call(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, "name"));

/**
 * Emits ATSPI 'PropertyChanged' dbus signal for 'Description' property.
 */
#define elm_interface_atspi_accessible_description_changed_signal_emit(obj) \
   eo_do(obj, eo_event_callback_call(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, "description"));

/**
 * Emits ATSPI 'PropertyChanged' dbus signal for 'Parent' property.
 */
#define elm_interface_atspi_accessible_parent_changed_signal_emit(obj) \
   eo_do(obj, eo_event_callback_call(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, "parent"));

/**
 * Emits ATSPI 'PropertyChanged' dbus signal for 'Role' property.
 */
#define elm_interface_atspi_accessible_role_changed_signal_emit(obj) \
   eo_do(obj, eo_event_callback_call(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_PROPERTY_CHANGED, "role"));

/**
 * Emits ATSPI 'ChildrenChanged' dbus signal with added child as argument.
 */
#define elm_interface_atspi_accessible_children_changed_added_signal_emit(obj, child) \
   do { \
      Elm_Atspi_Event_Children_Changed_Data data = { EINA_TRUE, child }; \
      eo_do(obj, eo_event_callback_call(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_CHILDREN_CHANGED, &data)); \
   } while(0);

/**
 * Emits ATSPI 'ChildrenChanged' dbus signal with deleted child as argument.
 */
#define elm_interface_atspi_accessible_children_changed_del_signal_emit(obj, child) \
   do { \
      Elm_Atspi_Event_Children_Changed_Data data = { EINA_FALSE, child }; \
      eo_do(obj, eo_event_callback_call(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_CHILDREN_CHANGED, &data)); \
   } while(0);

/**
 * Emits ATSPI 'ActiveDescendantsChanged' dbus signal with deleted child as argument.
 */
#define elm_interface_atspi_accessible_active_descendants_changed_signal_emit(obj, desc) \
   eo_do(obj, eo_event_callback_call(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_ACTIVE_DESCENDANT_CHANGED, desc));
