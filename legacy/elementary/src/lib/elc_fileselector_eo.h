/**
 * @ingroup Fileselector
 *
 * @{
 */
#define ELM_OBJ_FILESELECTOR_CLASS elm_obj_fileselector_class_get()

const Eo_Class *elm_obj_fileselector_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_FILESELECTOR_BASE_ID;

enum
{
   ELM_OBJ_FILESELECTOR_SUB_ID_IS_SAVE_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_IS_SAVE_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_FOLDER_ONLY_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_FOLDER_ONLY_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_EXPANDABLE_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_EXPANDABLE_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_PATH_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_PATH_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_MODE_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_MODE_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_MULTI_SELECT_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_MULTI_SELECT_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_PATHS_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_MIME_TYPES_FILTER_APPEND,
   ELM_OBJ_FILESELECTOR_SUB_ID_FILTERS_CLEAR,
   ELM_OBJ_FILESELECTOR_SUB_ID_HIDDEN_VISIBLE_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_HIDDEN_VISIBLE_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_LAST
};

#define ELM_OBJ_FILESELECTOR_ID(sub_id) (ELM_OBJ_FILESELECTOR_BASE_ID + sub_id)


/**
 * @def elm_obj_fileselector_is_save_set
 * @since 1.8
 *
 * Enable/disable the file name entry box where the user can type
 * in a name for a file, in a given file selector widget
 *
 * @param[in] is_save
 *
 * @see elm_fileselector_is_save_set
 */
#define elm_obj_fileselector_is_save_set(is_save) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_IS_SAVE_SET), EO_TYPECHECK(Eina_Bool, is_save)

/**
 * @def elm_obj_fileselector_is_save_get
 * @since 1.8
 *
 * Get whether the given file selector is in "saving dialog" mode
 *
 * @param[out] ret
 *
 * @see elm_fileselector_is_save_get
 */
#define elm_obj_fileselector_is_save_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_IS_SAVE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_folder_only_set
 * @since 1.8
 *
 * Enable/disable folder-only view for a given file selector widget
 *
 * @param[in] only
 *
 * @see elm_fileselector_folder_only_set
 */
#define elm_obj_fileselector_folder_only_set(only) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_FOLDER_ONLY_SET), EO_TYPECHECK(Eina_Bool, only)

/**
 * @def elm_obj_fileselector_folder_only_get
 * @since 1.8
 *
 * Get whether folder-only view is set for a given file selector
 *
 * @param[out] ret
 *
 * @see elm_fileselector_folder_only_get
 */
#define elm_obj_fileselector_folder_only_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_FOLDER_ONLY_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_buttons_ok_cancel_set
 * @since 1.8
 *
 * Enable/disable the "ok" and "cancel" buttons on a given file
 *
 * @param[in] visible
 *
 * @see elm_fileselector_buttons_ok_cancel_set
 */
#define elm_obj_fileselector_buttons_ok_cancel_set(visible) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_SET), EO_TYPECHECK(Eina_Bool, visible)

/**
 * @def elm_obj_fileselector_buttons_ok_cancel_get
 * @since 1.8
 *
 * Get whether the "ok" and "cancel" buttons on a given file
 *
 * @param[out] ret
 *
 * @see elm_fileselector_buttons_ok_cancel_get
 */
#define elm_obj_fileselector_buttons_ok_cancel_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_expandable_set
 * @since 1.8
 *
 * Enable/disable a tree view in the given file selector widget,
 * <b>if it's in @c #ELM_FILESELECTOR_LIST mode</b>
 *
 * @param[in] expand
 *
 * @see elm_fileselector_expandable_set
 */
#define elm_obj_fileselector_expandable_set(expand) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_EXPANDABLE_SET), EO_TYPECHECK(Eina_Bool, expand)

/**
 * @def elm_obj_fileselector_expandable_get
 * @since 1.8
 *
 * Get whether tree view is enabled for the given file selector
 *
 * @param[out] ret
 *
 * @see elm_fileselector_expandable_get
 */
#define elm_obj_fileselector_expandable_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_EXPANDABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_path_set
 * @since 1.8
 *
 * Set, programmatically, the directory that a given file
 * selector widget will display contents from
 *
 * @param[in] _path
 *
 * @see elm_fileselector_path_set
 */
#define elm_obj_fileselector_path_set(_path) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_PATH_SET), EO_TYPECHECK(const char *, _path)

/**
 * @def elm_obj_fileselector_path_get
 * @since 1.8
 *
 * Get the parent directory's path that a given file selector
 * selector widget will display contents from
 *
 * @param[out] ret
 *
 * @see elm_fileselector_path_get
 */
#define elm_obj_fileselector_path_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_PATH_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_fileselector_mode_set
 * @since 1.8
 *
 * Set the mode in which a given file selector widget will display
 * (layout) file system entries in its view
 *
 * @param[in] mode
 *
 * @see elm_fileselector_mode_set
 */
#define elm_obj_fileselector_mode_set(mode) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_MODE_SET), EO_TYPECHECK(Elm_Fileselector_Mode, mode)

/**
 * @def elm_obj_fileselector_mode_get
 * @since 1.8
 *
 * Get the mode in which a given file selector widget is displaying
 *
 * @param[out] ret
 *
 * @see elm_fileselector_mode_get
 */
#define elm_obj_fileselector_mode_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_MODE_GET), EO_TYPECHECK(Elm_Fileselector_Mode *, ret)

/**
 * @def elm_obj_fileselector_multi_select_set
 * @since 1.8
 *
 * Enable or disable multi-selection in the fileselector
 *
 * @param[in] multi
 *
 * @see elm_fileselector_multi_select_set
 */
#define elm_obj_fileselector_multi_select_set(multi) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_MULTI_SELECT_SET), EO_TYPECHECK(Eina_Bool, multi)

/**
 * @def elm_obj_fileselector_multi_select_get
 * @since 1.8
 *
 * Gets if multi-selection in fileselector is enabled or disabled.
 *
 * @param[out] multi
 *
 * @see elm_fileselector_multi_select_get
 */
#define elm_obj_fileselector_multi_select_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_MULTI_SELECT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_selected_get
 * @since 1.8
 *
 * Get the currently selected item's (full) path, in the given file
 * the given file selector widget
 *
 * @param[out] ret
 *
 * @see elm_fileselector_selected_get
 */
#define elm_obj_fileselector_selected_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_fileselector_selected_set
 * @since 1.8
 *
 * Set, programmatically, the currently selected file/directory in
 * the given file selector widget
 *
 * @param[in] _path
 * @param[out] ret
 *
 * @see elm_fileselector_selected_set
 */
#define elm_obj_fileselector_selected_set(_path, ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_SET), EO_TYPECHECK(const char *, _path), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_selected_paths_get
 * @since 1.8
 *
 * Get a list of selected paths in the fileselector.
 *
 * @param[out] ret
 *
 * @see elm_fileselector_selected_paths_get
 */
#define elm_obj_fileselector_selected_paths_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_PATHS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_fileselector_mime_types_filter_append
 * @since 1.8
 *
 * Append mime type based filter into filter list
 *
 * @param[in] mime_types
 * @param[in] filter_name
 * @param[out] ret
 *
 * @see elm_fileselector_mime_types_filter_append
 */
#define elm_obj_fileselector_mime_types_filter_append(mime_types, filter_name, ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_MIME_TYPES_FILTER_APPEND), EO_TYPECHECK(const char *, mime_types), EO_TYPECHECK(const char *, filter_name), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_filters_clear
 * @since 1.8
 *
 * Clear all filters registered
 *
 *
 * @see elm_fileselector_mime_type_filter_append
 */
#define elm_obj_fileselector_filters_clear() ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_FILTERS_CLEAR)

/**
 * @def elm_obj_fileselector_hidden_visible_set
 * @since 1.8
 *
 * Enable or disable visibility of hidden files/directories
 * in the file selector widget.
 *
 * @param[in] visible
 *
 * @see elm_fileselector_hidden_visible_get
 */
#define elm_obj_fileselector_hidden_visible_set(multi) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_HIDDEN_VISIBLE_SET), EO_TYPECHECK(Eina_Bool, multi)

/**
 * @def elm_obj_fileselector_hidden_visible_get
 * @since 1.8
 *
 * Get if hiden files/directories in the file selector are visible or not.
 *
 * @param[out] visible
 *
 * @see elm_fileselector_hidden_visible_set
 */
#define elm_obj_fileselector_hidden_visible_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_HIDDEN_VISIBLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */
