/**
 * @defgroup File_Selector_Entry File Selector Entry
 * @ingroup Elementary
 *
 * @image html fileselector_entry_inheritance_tree.png
 * @image latex fileselector_entry_inheritance_tree.eps
 *
 * @image html img/widget/fileselector_entry/preview-00.png
 * @image latex img/widget/fileselector_entry/preview-00.eps
 *
 * This is an entry made to be filled with or display a <b>file
 * system path string</b>. Besides the entry itself, the widget has
 * a @ref File_Selector_Button "file selector button" on its side,
 * which will raise an internal @ref Fileselector "file selector widget",
 * when clicked, for path selection aided by file system
 * navigation.
 *
 * This file selector may appear in an Elementary window or in an
 * inner window. When a file is chosen from it, the (inner) window
 * is closed and the selected file's path string is exposed both as
 * a smart event and as the new text on the entry.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for file selector entry objects
 * (@since 1.8).
 *
 * This widget encapsulates operations on its internal file
 * selector on its own API. There is less control over its file
 * selector than that one would have instantiating one directly.
 *
 * Smart callbacks one can register to:
 * - @c "changed" - The text within the entry was changed
 * - @c "activated" - The entry has had editing finished and
 *   changes are to be "committed"
 * - @c "press" - The entry has been clicked
 * - @c "longpressed" - The entry has been clicked (and held) for a
 *   couple seconds
 * - @c "clicked" - The entry has been clicked
 * - @c "clicked,double" - The entry has been double clicked
 * - @c "focused" - The entry has received focus
 * - @c "unfocused" - The entry has lost focus
 * - @c "selection,paste" - A paste action has occurred on the
 *   entry
 * - @c "selection,copy" - A copy action has occurred on the entry
 * - @c "selection,cut" - A cut action has occurred on the entry
 * - @c "unpressed" - The file selector entry's button was released
 *   after being pressed.
 * - @c "file,chosen" - The user has selected a path via the file
 *   selector entry's internal file selector, whose string pointer
 *   comes as the @c event_info data (a stringshared string)
 * - @c "language,changed" - the program's language changed
 *
 * Default text parts of the fileselector_button widget that you can use for
 * are:
 * @li "default" - Label of the fileselector_button
 *
 * Default content parts of the fileselector_entry widget that you can use for
 * are:
 * @li "button icon" - Button icon of the fileselector_entry
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 *
 * Here is an example on its usage:
 * @li @ref fileselector_entry_example
 *
 * @see @ref File_Selector_Button for a similar widget.
 * @{
 */

#define ELM_OBJ_FILESELECTOR_ENTRY_CLASS elm_obj_fileselector_entry_class_get()

const Eo_Class *elm_obj_fileselector_entry_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_FILESELECTOR_ENTRY_BASE_ID;

enum
{
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_SELECTED_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_SELECTED_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_PATH_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_PATH_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_EXPANDABLE_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_EXPANDABLE_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_FOLDER_ONLY_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_FOLDER_ONLY_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_IS_SAVE_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_IS_SAVE_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_LAST
};

#define ELM_OBJ_FILESELECTOR_ENTRY_ID(sub_id) (ELM_OBJ_FILESELECTOR_ENTRY_BASE_ID + sub_id)


/**
 * @def elm_obj_fileselector_entry_selected_set
 * @since 1.8
 *
 * Set the initial file system path for a given file selector entry
 *
 * @param[in] path
 *
 * @see elm_fileselector_entry_selected_set
 */
#define elm_obj_fileselector_entry_selected_set(path) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_SELECTED_SET), EO_TYPECHECK(const char *, path)

/**
 * @def elm_obj_fileselector_entry_selected_get
 * @since 1.8
 *
 * Get the parent directory's path to the latest file selection on
 * a given filer selector entry widget
 *
 * @param[out] ret
 *
 * @see elm_fileselector_entry_selected_get
 */
#define elm_obj_fileselector_entry_selected_get(ret) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_SELECTED_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_fileselector_entry_window_title_set
 * @since 1.8
 *
 * Set the title for a given file selector entry widget's window
 *
 * @param[in] title
 *
 * @see elm_fileselector_entry_window_title_set
 */
#define elm_obj_fileselector_entry_window_title_set(title) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_SET), EO_TYPECHECK(const char *, title)

/**
 * @def elm_obj_fileselector_entry_window_title_get
 * @since 1.8
 *
 * Get the title set for a given file selector entry widget's
 * window
 *
 * @param[out] ret
 *
 * @see elm_fileselector_entry_window_title_get
 */
#define elm_obj_fileselector_entry_window_title_get(ret) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_fileselector_entry_window_size_set
 * @since 1.8
 *
 * Set the size of a given file selector entry widget's window,
 * holding the file selector itself.
 *
 * @param[in] width
 * @param[in] height
 *
 * @see elm_fileselector_entry_window_size_set
 */
#define elm_obj_fileselector_entry_window_size_set(width, height) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_SET), EO_TYPECHECK(Evas_Coord, width), EO_TYPECHECK(Evas_Coord, height)

/**
 * @def elm_obj_fileselector_entry_window_size_get
 * @since 1.8
 *
 * Get the size of a given file selector entry widget's window,
 * holding the file selector itself.
 *
 * @param[out] width
 * @param[out] height
 *
 * @see elm_fileselector_entry_window_size_get
 */
#define elm_obj_fileselector_entry_window_size_get(width, height) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_GET), EO_TYPECHECK(Evas_Coord *, width), EO_TYPECHECK(Evas_Coord *, height)

/**
 * @def elm_obj_fileselector_entry_path_set
 * @since 1.8
 *
 * Set the initial file system path and the entry's path string for
 * a given file selector entry widget
 *
 * @param[in] path
 *
 * @see elm_fileselector_entry_path_set
 */
#define elm_obj_fileselector_entry_path_set(path) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_PATH_SET), EO_TYPECHECK(const char *, path)

/**
 * @def elm_obj_fileselector_entry_path_get
 * @since 1.8
 *
 * Get the entry's path string for a given file selector entry
 * widget
 *
 * @param[out] ret
 *
 * @see elm_fileselector_entry_path_get
 */
#define elm_obj_fileselector_entry_path_get(ret) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_PATH_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_fileselector_entry_expandable_set
 * @since 1.8
 *
 * Enable/disable a tree view in the given file selector entry
 *
 * @param[in] value
 *
 * @see elm_fileselector_entry_expandable_set
 */
#define elm_obj_fileselector_entry_expandable_set(value) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_EXPANDABLE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_fileselector_entry_expandable_get
 * @since 1.8
 *
 * Get whether tree view is enabled for the given file selector
 *
 * @param[out] ret
 *
 * @see elm_fileselector_entry_expandable_get
 */
#define elm_obj_fileselector_entry_expandable_get(ret) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_EXPANDABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_entry_folder_only_set
 * @since 1.8
 *
 * Set whether a given file selector entry widget's internal file
 * selector is to display folders only or the directory contents,
 * as well.
 *
 * @param[in] value
 *
 * @see elm_fileselector_entry_folder_only_set
 */
#define elm_obj_fileselector_entry_folder_only_set(value) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_FOLDER_ONLY_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_fileselector_entry_folder_only_get
 * @since 1.8
 *
 * Get whether a given file selector entry widget's internal file
 * selector is displaying folders only or the directory contents,
 * as well.
 *
 * @param[out] ret
 *
 * @see elm_fileselector_entry_folder_only_get
 */
#define elm_obj_fileselector_entry_folder_only_get(ret) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_FOLDER_ONLY_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_entry_is_save_set
 * @since 1.8
 *
 * Enable/disable the file name entry box where the user can type
 *
 * @param[in] value
 *
 * @see elm_fileselector_entry_is_save_set
 */
#define elm_obj_fileselector_entry_is_save_set(value) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_IS_SAVE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_fileselector_entry_is_save_get
 * @since 1.8
 *
 * Get whether the given file selector entry widget's internal
 *
 * @param[out] ret
 *
 * @see elm_fileselector_entry_is_save_get
 */
#define elm_obj_fileselector_entry_is_save_get(ret) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_IS_SAVE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_fileselector_entry_inwin_mode_set
 * @since 1.8
 *
 * Set whether a given file selector entry widget's internal file
 *
 * @param[in] value
 *
 * @see elm_fileselector_entry_inwin_mode_set
 */
#define elm_obj_fileselector_entry_inwin_mode_set(value) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_fileselector_entry_inwin_mode_get
 * @since 1.8
 *
 * Get whether a given file selector entry widget's internal file
 *
 * @param[out] ret
 *
 * @see elm_fileselector_entry_inwin_mode_get
 */
#define elm_obj_fileselector_entry_inwin_mode_get(ret) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_GET), EO_TYPECHECK(Eina_Bool *, ret)
/**
 * Add a new file selector entry widget to the given parent
 * Elementary (container) object
 *
 * @param parent The parent object
 * @return a new file selector entry widget handle or @c NULL, on
 * errors
 *
 * @ingroup File_Selector_Entry
 */
EAPI Evas_Object                *elm_fileselector_entry_add(Evas_Object *parent);

/**
 * Set the title for a given file selector entry widget's window
 *
 * @param obj The file selector entry widget
 * @param title The title string
 *
 * This will change the window's title, when the file selector pops
 * out after a click on the entry's button. Those windows have the
 * default (unlocalized) value of @c "Select a file" as titles.
 *
 * @note It will only take any effect if the file selector
 * entry widget is @b not under "inwin mode".
 *
 * @see elm_fileselector_entry_window_title_get()
 *
 * @ingroup File_Selector_Entry
 */
EAPI void                        elm_fileselector_entry_window_title_set(Evas_Object *obj, const char *title);

/**
 * Get the title set for a given file selector entry widget's
 * window
 *
 * @param obj The file selector entry widget
 * @return Title of the file selector entry's window
 *
 * @see elm_fileselector_entry_window_title_get() for more details
 *
 * @ingroup File_Selector_Entry
 */
EAPI const char                 *elm_fileselector_entry_window_title_get(const Evas_Object *obj);

/**
 * Set the size of a given file selector entry widget's window,
 * holding the file selector itself.
 *
 * @param obj The file selector entry widget
 * @param width The window's width
 * @param height The window's height
 *
 * @note it will only take any effect if the file selector entry
 * widget is @b not under "inwin mode". The default size for the
 * window (when applicable) is 400x400 pixels.
 *
 * @see elm_fileselector_entry_window_size_get()
 *
 * @ingroup File_Selector_Entry
 */
EAPI void                        elm_fileselector_entry_window_size_set(Evas_Object *obj, Evas_Coord width, Evas_Coord height);

/**
 * Get the size of a given file selector entry widget's window,
 * holding the file selector itself.
 *
 * @param obj The file selector entry widget
 * @param width Pointer into which to store the width value
 * @param height Pointer into which to store the height value
 *
 * @note Use @c NULL pointers on the size values you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_fileselector_entry_window_size_set(), for more details
 *
 * @ingroup File_Selector_Entry
 */
EAPI void                        elm_fileselector_entry_window_size_get(const Evas_Object *obj, Evas_Coord *width, Evas_Coord *height);

/**
 * Set the initial file system path and the entry's path string for
 * a given file selector entry widget
 *
 * @param obj The file selector entry widget
 * @param path The path string
 *
 * It must be a <b>directory</b> path, which will have the contents
 * displayed initially in the file selector's view, when invoked
 * from @p obj. The default initial path is the @c "HOME"
 * environment variable's value.
 *
 * @see elm_fileselector_entry_path_get()
 *
 * @ingroup File_Selector_Entry
 */
EAPI void                        elm_fileselector_entry_path_set(Evas_Object *obj, const char *path);

/**
 * Get the entry's path string for a given file selector entry
 * widget
 *
 * @param obj The file selector entry widget
 * @return path The path string
 *
 * @see elm_fileselector_entry_path_set() for more details
 *
 * @ingroup File_Selector_Entry
 */
EAPI const char                 *elm_fileselector_entry_path_get(const Evas_Object *obj);

/**
 * Enable/disable a tree view in the given file selector entry
 * widget's internal file selector
 *
 * @param obj The file selector entry widget
 * @param value @c EINA_TRUE to enable tree view, @c EINA_FALSE to disable
 *
 * This has the same effect as elm_fileselector_expandable_set(),
 * but now applied to a file selector entry's internal file
 * selector.
 *
 * @note There's no way to put a file selector entry's internal
 * file selector in "grid mode", as one may do with "pure" file
 * selectors.
 *
 * @see elm_fileselector_expandable_get()
 *
 * @ingroup File_Selector_Entry
 */
EAPI void                        elm_fileselector_entry_expandable_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether tree view is enabled for the given file selector
 * entry widget's internal file selector
 *
 * @param obj The file selector entry widget
 * @return @c EINA_TRUE if @p obj widget's internal file selector
 * is in tree view, @c EINA_FALSE otherwise (and or errors)
 *
 * @see elm_fileselector_expandable_set() for more details
 *
 * @ingroup File_Selector_Entry
 */
EAPI Eina_Bool                   elm_fileselector_entry_expandable_get(const Evas_Object *obj);

/**
 * Set whether a given file selector entry widget's internal file
 * selector is to display folders only or the directory contents,
 * as well.
 *
 * @param obj The file selector entry widget
 * @param value @c EINA_TRUE to make @p obj widget's internal file
 * selector only display directories, @c EINA_FALSE to make files
 * to be displayed in it too
 *
 * This has the same effect as elm_fileselector_folder_only_set(),
 * but now applied to a file selector entry's internal file
 * selector.
 *
 * @see elm_fileselector_folder_only_get()
 *
 * @ingroup File_Selector_Entry
 */
EAPI void                        elm_fileselector_entry_folder_only_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether a given file selector entry widget's internal file
 * selector is displaying folders only or the directory contents,
 * as well.
 *
 * @param obj The file selector entry widget
 * @return @c EINA_TRUE if @p obj widget's internal file
 * selector is only displaying directories, @c EINA_FALSE if files
 * are being displayed in it too (and on errors)
 *
 * @see elm_fileselector_entry_folder_only_set() for more details
 *
 * @ingroup File_Selector_Entry
 */
EAPI Eina_Bool                   elm_fileselector_entry_folder_only_get(const Evas_Object *obj);

/**
 * Enable/disable the file name entry box where the user can type
 * in a name for a file, in a given file selector entry widget's
 * internal file selector.
 *
 * @param obj The file selector entry widget
 * @param value @c EINA_TRUE to make @p obj widget's internal
 * file selector a "saving dialog", @c EINA_FALSE otherwise
 *
 * This has the same effect as elm_fileselector_is_save_set(),
 * but now applied to a file selector entry's internal file
 * selector.
 *
 * @see elm_fileselector_is_save_get()
 *
 * @ingroup File_Selector_Entry
 */
EAPI void                        elm_fileselector_entry_is_save_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether the given file selector entry widget's internal
 * file selector is in "saving dialog" mode
 *
 * @param obj The file selector entry widget
 * @return @c EINA_TRUE, if @p obj widget's internal file selector
 * is in "saving dialog" mode, @c EINA_FALSE otherwise (and on
 * errors)
 *
 * @see elm_fileselector_entry_is_save_set() for more details
 *
 * @ingroup File_Selector_Entry
 */
EAPI Eina_Bool                   elm_fileselector_entry_is_save_get(const Evas_Object *obj);

/**
 * Set whether a given file selector entry widget's internal file
 * selector will raise an Elementary "inner window", instead of a
 * dedicated Elementary window. By default, it won't.
 *
 * @param obj The file selector entry widget
 * @param value @c EINA_TRUE to make it use an inner window, @c
 * EINA_FALSE to make it use a dedicated window
 *
 * @see elm_win_inwin_add() for more information on inner windows
 * @see elm_fileselector_entry_inwin_mode_get()
 *
 * @ingroup File_Selector_Entry
 */
EAPI void                        elm_fileselector_entry_inwin_mode_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether a given file selector entry widget's internal file
 * selector will raise an Elementary "inner window", instead of a
 * dedicated Elementary window.
 *
 * @param obj The file selector entry widget
 * @return @c EINA_TRUE if will use an inner window, @c EINA_TRUE
 * if it will use a dedicated window
 *
 * @see elm_fileselector_entry_inwin_mode_set() for more details
 *
 * @ingroup File_Selector_Entry
 */
EAPI Eina_Bool                   elm_fileselector_entry_inwin_mode_get(const Evas_Object *obj);

/**
 * Set the initial file system path for a given file selector entry
 * widget
 *
 * @param obj The file selector entry widget
 * @param path The path string
 *
 * It must be a <b>directory</b> path, which will have the contents
 * displayed initially in the file selector's view, when invoked
 * from @p obj. The default initial path is the @c "HOME"
 * environment variable's value.
 *
 * @see elm_fileselector_entry_path_get()
 *
 * @ingroup File_Selector_Entry
 */
EAPI void                        elm_fileselector_entry_selected_set(Evas_Object *obj, const char *path);

/**
 * Get the parent directory's path to the latest file selection on
 * a given filer selector entry widget
 *
 * @param obj The file selector object
 * @return The (full) path of the directory of the last selection
 * on @p obj widget, a @b stringshared string
 *
 * @see elm_fileselector_entry_path_set()
 *
 * @ingroup File_Selector_Entry
 */
EAPI const char                 *elm_fileselector_entry_selected_get(const Evas_Object *obj);

/**
 * @}
 */
