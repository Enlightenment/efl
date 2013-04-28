#define ELM_OBJ_FILESELECTOR_BUTTON_CLASS elm_obj_fileselector_button_class_get()

const Eo_Class *elm_obj_fileselector_button_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_FILESELECTOR_BUTTON_BASE_ID;

enum
{
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_TITLE_SET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_TITLE_GET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_SIZE_SET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_SIZE_GET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_PATH_SET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_PATH_GET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_EXPANDABLE_SET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_EXPANDABLE_GET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_FOLDER_ONLY_SET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_FOLDER_ONLY_GET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_IS_SAVE_SET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_IS_SAVE_GET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_INWIN_MODE_SET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_INWIN_MODE_GET,
   ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_LAST
};

#define ELM_OBJ_FILESELECTOR_BUTTON_ID(sub_id) (ELM_OBJ_FILESELECTOR_BUTTON_BASE_ID + sub_id)


/**
 * @def elm_obj_fileselector_button_window_title_set
 * @since 1.8
 *
 * Set the title for a given file selector button widget's window
 *
 * @param[in] title
 *
 * @see elm_fileselector_button_window_title_set
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_window_title_set(title) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_TITLE_SET), EO_TYPECHECK(const char *, title)

/**
 * @def elm_obj_fileselector_button_window_title_get
 * @since 1.8
 *
 * Get the title for a given file selector button widget's
 *
 * @param[out] ret
 *
 * @see elm_fileselector_button_window_title_get
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_window_title_get(ret) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_TITLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_fileselector_button_window_size_set
 * @since 1.8
 *
 * Set the size of a given file selector button widget's window,
 * holding the file selector itself.
 *
 * @param[in] width
 * @param[in] height
 *
 * @see elm_fileselector_button_window_size_set
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_window_size_set(width, height) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_SIZE_SET), EO_TYPECHECK(Evas_Coord, width), EO_TYPECHECK(Evas_Coord, height)

/**
 * @def elm_obj_fileselector_button_window_size_get
 * @since 1.8
 *
 * Get the size of a given file selector button widget's window,
 * holding the file selector itself.
 *
 * @param[out] width
 * @param[out] height
 *
 * @see elm_fileselector_button_window_size_get
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_window_size_get(width, height) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_WINDOW_SIZE_GET), EO_TYPECHECK(Evas_Coord *, width), EO_TYPECHECK(Evas_Coord *, height)

/**
 * @def elm_obj_fileselector_button_path_set
 * @since 1.8
 *
 * Set the initial file system path for a given file selector
 *
 * @param[in] path
 *
 * @see elm_fileselector_button_path_set
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_path_set(path) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_PATH_SET), EO_TYPECHECK(const char *, path)

/**
 * @def elm_obj_fileselector_button_path_get
 * @since 1.8
 *
 * Get the initial file system path set for a given file selector
 *
 * @param[out] ret
 *
 * @see elm_fileselector_button_path_get
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_path_get(ret) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_PATH_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_fileselector_button_expandable_set
 * @since 1.8
 *
 * Enable/disable a tree view in the given file selector button
 *
 * @param[in] value
 *
 * @see elm_fileselector_button_expandable_set
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_expandable_set(value) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_EXPANDABLE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_fileselector_button_expandable_get
 * @since 1.8
 *
 * Get whether tree view is enabled for the given file selector
 *
 * @param[out] ret
 *
 * @see elm_fileselector_button_expandable_get
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_expandable_get(ret) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_EXPANDABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_button_folder_only_set
 * @since 1.8
 *
 * Set whether a given file selector button widget's internal file
 * selector is to display folders only or the directory contents,
 * as well.
 *
 * @param[in] value
 *
 * @see elm_fileselector_button_folder_only_set
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_folder_only_set(value) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_FOLDER_ONLY_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_fileselector_button_folder_only_get
 * @since 1.8
 *
 * Get whether a given file selector button widget's internal file
 * selector is displaying folders only or the directory contents,
 * as well.
 *
 * @param[out] ret
 *
 * @see elm_fileselector_button_folder_only_get
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_folder_only_get(ret) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_FOLDER_ONLY_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_button_is_save_set
 * @since 1.8
 *
 * Enable/disable the file name entry box where the user can type
 *
 * @param[in] value
 *
 * @see elm_fileselector_button_is_save_set
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_is_save_set(value) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_IS_SAVE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_fileselector_button_is_save_get
 * @since 1.8
 *
 * Get whether the given file selector button widget's internal
 * file selector is in "saving dialog" mode
 *
 * @param[out] ret
 *
 * @see elm_fileselector_button_is_save_get
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_is_save_get(ret) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_IS_SAVE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_button_inwin_mode_set
 * @since 1.8
 *
 * Set whether a given file selector button widget's internal file
 * selector will raise an Elementary "inner window", instead of a
 * dedicated Elementary window. By default, it won't.
 *
 * @param[in] value
 *
 * @see elm_fileselector_button_inwin_mode_set
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_inwin_mode_set(value) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_INWIN_MODE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_fileselector_button_inwin_mode_get
 * @since 1.8
 *
 * Get whether a given file selector button widget's internal file
 * selector will raise an Elementary "inner window", instead of a
 * dedicated Elementary window.
 *
 * @param[out] ret
 *
 * @see elm_fileselector_button_inwin_mode_get
 *
 * @ingroup File_Selector_Button
 */
#define elm_obj_fileselector_button_inwin_mode_get(ret) ELM_OBJ_FILESELECTOR_BUTTON_ID(ELM_OBJ_FILESELECTOR_BUTTON_SUB_ID_INWIN_MODE_GET), EO_TYPECHECK(Eina_Bool *, ret)
