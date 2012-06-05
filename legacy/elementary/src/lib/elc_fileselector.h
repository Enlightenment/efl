/**
 * @defgroup Fileselector File Selector
 * @ingroup Elementary
 *
 * @image html fileselector_inheritance_tree.png
 * @image latex fileselector_inheritance_tree.eps
 *
 * @image html img/widget/fileselector/preview-00.png
 * @image latex img/widget/fileselector/preview-00.eps
 *
 * A file selector is a widget that allows a user to navigate
 * through a file system, reporting file selections back via its
 * API.
 *
 * It contains shortcut buttons for home directory (@c ~) and to
 * jump one directory upwards (..), as well as cancel/ok buttons to
 * confirm/cancel a given selection. After either one of those two
 * former actions, the file selector will issue its @c "done" smart
 * callback.
 *
 * There's a text entry on it, too, showing the name of the current
 * selection. There's the possibility of making it editable, so it
 * is useful on file saving dialogs on applications, where one
 * gives a file name to save contents to, in a given directory in
 * the system. This custom file name will be reported on the @c
 * "done" smart callback (explained in sequence).
 *
 * Finally, it has a view to display file system items into in two
 * possible forms:
 * - list
 * - grid
 *
 * If Elementary is built with support of the Ethumb thumbnailing
 * library, the second form of view will display preview thumbnails
 * of files which it supports.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for file selector objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "selected" - the user has clicked on a file (when not in
 *      folders-only mode) or directory (when in folders-only mode)
 * - @c "directory,open" - the list has been populated with new
 *      content (@c event_info is a pointer to the directory's
 *      path, a @b stringshared string)
 * - @c "done" - the user has clicked on the "ok" or "cancel"
 *      buttons (@c event_info is a pointer to the selection's
 *      path, a @b stringshared string)
 *
 * Here is an example on its usage:
 * @li @ref fileselector_example
 */

/**
 * @addtogroup Fileselector
 * @{
 */

/**
 * Defines how a file selector widget is to layout its contents
 * (file system entries).
 */
typedef enum
{
   ELM_FILESELECTOR_LIST = 0, /**< layout as a list */
   ELM_FILESELECTOR_GRID, /**< layout as a grid */
   ELM_FILESELECTOR_LAST /**< sentinel (helper) value, not used */
} Elm_Fileselector_Mode;

/**
 * Add a new file selector widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new file selector widget handle or @c NULL, on errors
 *
 * This function inserts a new file selector widget on the canvas.
 *
 * @ingroup Fileselector
 */
EAPI Evas_Object          *elm_fileselector_add(Evas_Object *parent);

/**
 * Enable/disable the file name entry box where the user can type
 * in a name for a file, in a given file selector widget
 *
 * @param obj The file selector object
 * @param is_save @c EINA_TRUE to make the file selector a "saving
 * dialog", @c EINA_FALSE otherwise
 *
 * Having the entry editable is useful on file saving dialogs on
 * applications, where one gives a file name to save contents to,
 * in a given directory in the system. This custom file name will
 * be reported on the @c "done" smart callback.
 *
 * @see elm_fileselector_is_save_get()
 *
 * @ingroup Fileselector
 */
EAPI void                  elm_fileselector_is_save_set(Evas_Object *obj, Eina_Bool is_save);

/**
 * Get whether the given file selector is in "saving dialog" mode
 *
 * @param obj The file selector object
 * @return @c EINA_TRUE, if the file selector is in "saving dialog"
 * mode, @c EINA_FALSE otherwise (and on errors)
 *
 * @see elm_fileselector_is_save_set() for more details
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool             elm_fileselector_is_save_get(const Evas_Object *obj);

/**
 * Enable/disable folder-only view for a given file selector widget
 *
 * @param obj The file selector object
 * @param only @c EINA_TRUE to make @p obj only display
 * directories, @c EINA_FALSE to make files to be displayed in it
 * too
 *
 * If enabled, the widget's view will only display folder items,
 * naturally.
 *
 * @see elm_fileselector_folder_only_get()
 *
 * @ingroup Fileselector
 */
EAPI void                  elm_fileselector_folder_only_set(Evas_Object *obj, Eina_Bool only);

/**
 * Get whether folder-only view is set for a given file selector
 * widget
 *
 * @param obj The file selector object
 * @return only @c EINA_TRUE if @p obj is only displaying
 * directories, @c EINA_FALSE if files are being displayed in it
 * too (and on errors)
 *
 * @see elm_fileselector_folder_only_get()
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool             elm_fileselector_folder_only_get(const Evas_Object *obj);

/**
 * Enable/disable the "ok" and "cancel" buttons on a given file
 * selector widget
 *
 * @param obj The file selector object
 * @param buttons @c EINA_TRUE to show buttons, @c EINA_FALSE to hide.
 *
 * @note A file selector without those buttons will never emit the
 * @c "done" smart event, and is only usable if one is just hooking
 * to the other two events.
 *
 * @see elm_fileselector_buttons_ok_cancel_get()
 *
 * @ingroup Fileselector
 */
EAPI void                  elm_fileselector_buttons_ok_cancel_set(Evas_Object *obj, Eina_Bool buttons);

/**
 * Get whether the "ok" and "cancel" buttons on a given file
 * selector widget are being shown.
 *
 * @param obj The file selector object
 * @return @c EINA_TRUE if they are being shown, @c EINA_FALSE
 * otherwise (and on errors)
 *
 * @see elm_fileselector_buttons_ok_cancel_set() for more details
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool             elm_fileselector_buttons_ok_cancel_get(const Evas_Object *obj);

/**
 * Enable/disable a tree view in the given file selector widget,
 * <b>if it's in @c #ELM_FILESELECTOR_LIST mode</b>
 *
 * @param obj The file selector object
 * @param expand @c EINA_TRUE to enable tree view, @c EINA_FALSE to
 * disable
 *
 * In a tree view, arrows are created on the sides of directories,
 * allowing them to expand in place.
 *
 * @note If it's in other mode, the changes made by this function
 * will only be visible when one switches back to "list" mode.
 *
 * @see elm_fileselector_expandable_get()
 *
 * @ingroup Fileselector
 */
EAPI void                  elm_fileselector_expandable_set(Evas_Object *obj, Eina_Bool expand);

/**
 * Get whether tree view is enabled for the given file selector
 * widget
 *
 * @param obj The file selector object
 * @return @c EINA_TRUE if @p obj is in tree view, @c EINA_FALSE
 * otherwise (and or errors)
 *
 * @see elm_fileselector_expandable_set() for more details
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool             elm_fileselector_expandable_get(const Evas_Object *obj);

/**
 * Set, programmatically, the @b directory that a given file
 * selector widget will display contents from
 *
 * @param obj The file selector object
 * @param path The path to display in @p obj
 *
 * This will change the @b directory that @p obj is displaying. It
 * will also clear the text entry area on the @p obj object, which
 * displays select files' names.
 *
 * @see elm_fileselector_path_get()
 *
 * @ingroup Fileselector
 */
EAPI void                  elm_fileselector_path_set(Evas_Object *obj, const char *path);

/**
 * Get the parent directory's path that a given file selector
 * widget is displaying
 *
 * @param obj The file selector object
 * @return The (full) path of the directory the file selector is
 * displaying, a @b stringshared string
 *
 * @see elm_fileselector_path_set()
 *
 * @ingroup Fileselector
 */
EAPI const char           *elm_fileselector_path_get(const Evas_Object *obj);

/**
 * Set, programmatically, the currently selected file/directory in
 * the given file selector widget
 *
 * @param obj The file selector object
 * @param path The (full) path to a file or directory
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure. The
 * latter case occurs if the directory or file pointed to do not
 * exist.
 *
 * @see elm_fileselector_selected_get()
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool             elm_fileselector_selected_set(Evas_Object *obj, const char *path);

/**
 * Get the currently selected item's (full) path, in the given file
 * selector widget
 *
 * @param obj The file selector object
 * @return The absolute path of the selected item, a @b
 * stringshared string
 *
 * @note Custom editions on @p obj object's text entry, if made,
 * will appear on the return string of this function, naturally.
 *
 * @see elm_fileselector_selected_set() for more details
 *
 * @ingroup Fileselector
 */
EAPI const char           *elm_fileselector_selected_get(const Evas_Object *obj);

/**
 * Set the mode in which a given file selector widget will display
 * (layout) file system entries in its view
 *
 * @param obj The file selector object
 * @param mode The mode of the fileselector, being it one of #ELM_FILESELECTOR_LIST
 * (default) or #ELM_FILESELECTOR_GRID. The first one, naturally, will display
 * the files in a list. The latter will make the widget to display its entries
 * in a grid form.
 *
 * @note By using elm_fileselector_expandable_set(), the user may
 * trigger a tree view for that list.
 *
 * @note If Elementary is built with support of the Ethumb
 * thumbnailing library, the second form of view will display
 * preview thumbnails of files which it supports. You must have
 * elm_need_ethumb() called in your Elementary for thumbnailing to
 * work, though.
 *
 * @see elm_fileselector_expandable_set().
 * @see elm_fileselector_mode_get().
 *
 * @ingroup Fileselector
 */
EAPI void                  elm_fileselector_mode_set(Evas_Object *obj, Elm_Fileselector_Mode mode);

/**
 * Get the mode in which a given file selector widget is displaying
 * (layouting) file system entries in its view
 *
 * @param obj The fileselector object
 * @return The mode in which the fileselector is at
 *
 * @see elm_fileselector_mode_set() for more details
 *
 * @ingroup Fileselector
 */
EAPI Elm_Fileselector_Mode elm_fileselector_mode_get(const Evas_Object *obj);

/**
 * @}
 */
