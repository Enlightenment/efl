#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_interface_fileselector.h"

EAPI Eo_Op ELM_FILESELECTOR_INTERFACE_BASE_ID = EO_NOOP;

#define MY_FILESELECTOR_INTERFACE_NAME "Elm_Interface_Fileselector"

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_IS_SAVE_SET, "Enable/disable the file name entry box where the user can type in a name for a file, in a given file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_IS_SAVE_GET, "Get whether the given file selector is in 'saving dialog' mode."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_FOLDER_ONLY_SET, "Enable/disable folder-only view for a given file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_FOLDER_ONLY_GET, "Get whether folder-only view is set for a given file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_EXPANDABLE_SET, "Enable/disable a tree view in the given file selector widget, if it's in #ELM_FILESELECTOR_LIST mode."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_EXPANDABLE_GET, "Get whether tree view is enabled for the given file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_PATH_SET, "Set, programmatically, the directory that a given file selector widget will display contents from."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_PATH_GET, "Get the parent directory's path that a given file selector widget is displaying."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_MODE_SET, "Set the mode in which a given file selector widget will display (layout) file system entries in its view."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_MODE_GET, "Get the mode in which a given file selector widget is displaying (layouting) file system entries in its view."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_MULTI_SELECT_SET, "Enable or disable multi-selection in the file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_MULTI_SELECT_GET, "Get if multi-selection in file selector widget is enabled or disabled."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_SELECTED_GET, "Get the currently selected item's (full) path, in the given file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_SELECTED_SET, "Set, programmatically, the currently selected file/directory in the given file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_SELECTED_PATHS_GET, "Get the currently selected item's (full) path, in the given file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_MIME_TYPES_FILTER_APPEND, "Append mime type filter"),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_CUSTOM_FILTER_APPEND, "Append custom filter"),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_FILTERS_CLEAR, "Clear filters"),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_HIDDEN_VISIBLE_SET, "Enable or disable visibility of hidden files/directories in the file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_HIDDEN_VISIBLE_GET, "Get if visibility of hidden files/directories in the file selector widget is enabled or disabled."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_THUMBNAIL_SIZE_SET, "Set the size for the thumbnail of the file selector widget's view, if it's in #ELM_FILESELECTOR_GRID mode"),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_THUMBNAIL_SIZE_GET, "Get the size for the thumbnails of a given file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_SORT_METHOD_SET, "Set the sort method of the file selector widget."),
     EO_OP_DESCRIPTION(ELM_FILESELECTOR_INTERFACE_SUB_ID_SORT_METHOD_GET, "Get the sort method of the file selector widget."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description _elm_fileselector_interface_desc = {
     EO_VERSION,
     MY_FILESELECTOR_INTERFACE_NAME,
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(&ELM_FILESELECTOR_INTERFACE_BASE_ID, op_desc, ELM_FILESELECTOR_INTERFACE_SUB_ID_LAST),
     NULL, 0, NULL, NULL
};

EO_DEFINE_CLASS(elm_fileselector_interface_get, &_elm_fileselector_interface_desc, NULL, NULL);
