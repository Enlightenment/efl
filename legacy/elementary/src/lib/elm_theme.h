/**
 * @defgroup Theme Theme
 * @ingroup Elementary
 *
 * Elementary uses Edje to theme its widgets, naturally. But for the most
 * part this is hidden behind a simpler interface that lets the user set
 * extensions and choose the style of widgets in a much easier way.
 *
 * Instead of thinking in terms of paths to Edje files and their groups
 * each time you want to change the appearance of a widget, Elementary
 * works so you can add any theme file with extensions or replace the
 * main theme at one point in the application, and then just set the style
 * of widgets with elm_object_style_set() and related functions. Elementary
 * will then look in its list of themes for a matching group and apply it,
 * and when the theme changes midway through the application, all widgets
 * will be updated accordingly.
 *
 * There are three concepts you need to know to understand how Elementary
 * theming works: default theme, extensions and overlays.
 *
 * Default theme, obviously enough, is the one that provides the default
 * look of all widgets. End users can change the theme used by Elementary
 * by setting the @c ELM_THEME environment variable before running an
 * application, or globally for all programs using the @c elementary_config
 * utility. Applications can change the default theme using elm_theme_set(),
 * but this can go against the user wishes, so it's not an advised practice.
 *
 * Ideally, applications should find everything they need in the already
 * provided theme, but there may be occasions when that's not enough and
 * custom styles are required to correctly express the idea. For this
 * cases, Elementary has extensions.
 *
 * Extensions allow the application developer to write styles of its own
 * to apply to some widgets. This requires knowledge of how each widget
 * is themed, as extensions will always replace the entire group used by
 * the widget, so important signals and parts need to be there for the
 * object to behave properly (see documentation of Edje for details).
 * Once the theme for the extension is done, the application needs to add
 * it to the list of themes Elementary will look into, using
 * elm_theme_extension_add(), and set the style of the desired widgets as
 * he would normally with elm_object_style_set().
 *
 * Overlays, on the other hand, can replace the look of all widgets by
 * overriding the default style. Like extensions, it's up to the application
 * developer to write the theme for the widgets it wants, the difference
 * being that when looking for the theme, Elementary will check first the
 * list of overlays, then the set theme and lastly the list of extensions,
 * so with overlays it's possible to replace the default view and every
 * widget will be affected. This is very much alike to setting the whole
 * theme for the application and will probably clash with the end user
 * options, not to mention the risk of ending up with not matching styles
 * across the program. Unless there's a very special reason to use them,
 * overlays should be avoided for the reasons exposed before.
 *
 * All these theme lists are handled by ::Elm_Theme instances. Elementary
 * keeps one default internally and every function that receives one of
 * these can be called with NULL to refer to this default (except for
 * elm_theme_free()). It's possible to create a new instance of a
 * ::Elm_Theme to set other theme for a specific widget (and all of its
 * children), but this is as discouraged, if not even more so, than using
 * overlays. Don't use this unless you really know what you are doing.
 *
 * But to be less negative about things, you can look at the following
 * examples:
 * @li @ref theme_example_01 "Using extensions"
 * @li @ref theme_example_02 "Using overlays"
 *
 * @{
 */
/**
 * @typedef Elm_Theme
 *
 * Opaque handler for the list of themes Elementary looks for when
 * rendering widgets.
 *
 * Stay out of this unless you really know what you are doing. For most
 * cases, sticking to the default is all a developer needs.
 */
typedef struct _Elm_Theme Elm_Theme;

/**
 * Create a new specific theme
 *
 * This creates an empty specific theme that only uses the default theme. A
 * specific theme has its own private set of extensions and overlays too
 * (which are empty by default). Specific themes do not fall back to themes
 * of parent objects. They are not intended for this use. Use styles, overlays
 * and extensions when needed, but avoid specific themes unless there is no
 * other way (example: you want to have a preview of a new theme you are
 * selecting in a "theme selector" window. The preview is inside a scroller
 * and should display what the theme you selected will look like, but not
 * actually apply it yet. The child of the scroller will have a specific
 * theme set to show this preview before the user decides to apply it to all
 * applications).
 *
 * @ingroup Theme
 */
EAPI Elm_Theme       *elm_theme_new(void);

/**
 * Free a specific theme
 *
 * @param th The theme to free
 *
 * This frees a theme created with elm_theme_new().
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_free(Elm_Theme *th);

/**
 * Copy the theme from the source to the destination theme
 *
 * @param th The source theme to copy from
 * @param thdst The destination theme to copy data to
 *
 * This makes a one-time static copy of all the theme config, extensions
 * and overlays from @p th to @p thdst. If @p th references a theme, then
 * @p thdst is also set to reference it, with all the theme settings,
 * overlays and extensions that @p th had.
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_copy(Elm_Theme *th, Elm_Theme *thdst);

/**
 * Tell the source theme to reference the ref theme
 *
 * @param th The theme that will do the referencing
 * @param thref The theme that is the reference source
 *
 * This clears @p th to be empty and then sets it to refer to @p thref
 * so @p th acts as an override to @p thref, but where its overrides
 * don't apply, it will fall through to @p thref for configuration.
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_ref_set(Elm_Theme *th, Elm_Theme *thref);

/**
 * Return the theme referred to
 *
 * @param th The theme to get the reference from
 * @return The referenced theme handle
 *
 * This gets the theme set as the reference theme by elm_theme_ref_set().
 * If no theme is set as a reference, NULL is returned.
 *
 * @ingroup Theme
 */
EAPI Elm_Theme       *elm_theme_ref_get(Elm_Theme *th);

/**
 * Return the default theme
 *
 * @return The default theme handle
 *
 * This returns the internal default theme setup handle that all widgets
 * use implicitly unless a specific theme is set. This is also often use
 * as a shorthand of NULL.
 *
 * @ingroup Theme
 */
EAPI Elm_Theme       *elm_theme_default_get(void);

/**
 * Prepends a theme overlay to the list of overlays
 *
 * @param th The theme to add to, or if NULL, the default theme
 * @param item The Edje file path to be used
 *
 * Use this if your application needs to provide some custom overlay theme
 * (An Edje file that replaces some default styles of widgets) where adding
 * new styles, or changing system theme configuration is not possible. Do
 * NOT use this instead of a proper system theme configuration. Use proper
 * configuration files, profiles, environment variables etc. to set a theme
 * so that the theme can be altered by simple configuration by a user. Using
 * this call to achieve that effect is abusing the API and will create lots
 * of trouble.
 *
 * @see elm_theme_extension_add()
 * @see elm_theme_overlay_mmap_add()
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_overlay_add(Elm_Theme *th, const char *item);

/**
 * Delete a theme overlay from the list of overlays
 *
 * @param th The theme to delete from, or if NULL, the default theme
 * @param item The name of the theme overlay
 *
 * @see elm_theme_overlay_add()
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_overlay_del(Elm_Theme *th, const char *item);

/**
 * Prepends a theme overlay to the list of overlays
 *
 * @param th The theme to add to, or if NULL, the default theme
 * @param f The Edje file handle to be used
 *
 * Use this if your application needs to provide some custom overlay theme
 * (An Edje file that replaces some default styles of widgets) where adding
 * new styles, or changing system theme configuration is not possible. Do
 * NOT use this instead of a proper system theme configuration. Use proper
 * configuration files, profiles, environment variables etc. to set a theme
 * so that the theme can be altered by simple configuration by a user. Using
 * this call to achieve that effect is abusing the API and will create lots
 * of trouble.
 *
 * @see elm_theme_extension_add()
 * @see elm_theme_overlay_add()
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_overlay_mmap_add(Elm_Theme *th, const Eina_File *f);

/**
 * Delete a theme overlay from the list of overlays
 *
 * @param th The theme to delete from, or if NULL, the default theme
 * @param f The file handle of the theme overlay
 *
 * @see elm_theme_overlay_mmap_add()
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_overlay_mmap_del(Elm_Theme *th, const Eina_File *f);

/**
 * Get the list of registered overlays for the given theme
 *
 * @param th The theme from which to get the overlays
 * @return List of theme overlays. Do not free it.
 *
 * @see elm_theme_overlay_add()
 *
 * @ingroup Theme
 */
EAPI const Eina_List *elm_theme_overlay_list_get(const Elm_Theme *th);

/**
 * Appends a theme extension to the list of extensions.
 *
 * @param th The theme to add to, or if NULL, the default theme
 * @param item The Edje file path to be used
 *
 * This is intended when an application needs more styles of widgets or new
 * widget themes that the default does not provide (or may not provide). The
 * application has "extended" usage by coming up with new custom style names
 * for widgets for specific uses, but as these are not "standard", they are
 * not guaranteed to be provided by a default theme. This means the
 * application is required to provide these extra elements itself in specific
 * Edje files. This call adds one of those Edje files to the theme search
 * path to be search after the default theme. The use of this call is
 * encouraged when default styles do not meet the needs of the application.
 * Use this call instead of elm_theme_overlay_add() for almost all cases.
 *
 * @see elm_object_style_set()
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_extension_add(Elm_Theme *th, const char *item);

/**
 * Deletes a theme extension from the list of extensions.
 *
 * @param th The theme to delete from, or if NULL, the default theme
 * @param item The name of the theme extension
 *
 * @see elm_theme_extension_add()
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_extension_del(Elm_Theme *th, const char *item);

/**
 * Appends a theme extension to the list of extensions.
 *
 * @param th The theme to add to, or if NULL, the default theme
 * @param f The Edje file handle to be used
 *
 * This is intended when an application needs more styles of widgets or new
 * widget themes that the default does not provide (or may not provide). The
 * application has "extended" usage by coming up with new custom style names
 * for widgets for specific uses, but as these are not "standard", they are
 * not guaranteed to be provided by a default theme. This means the
 * application is required to provide these extra elements itself in specific
 * Edje files. This call adds one of those Edje files to the theme search
 * path to be search after the default theme. The use of this call is
 * encouraged when default styles do not meet the needs of the application.
 * Use this call instead of elm_theme_overlay_add() for almost all cases.
 *
 * @see elm_object_style_set()
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_extension_mmap_add(Elm_Theme *th, const Eina_File *f);

/**
 * Deletes a theme extension from the list of extensions.
 *
 * @param th The theme to delete from, or if NULL, the default theme
 * @param f The file handle of the theme extension
 *
 * @see elm_theme_extension_add()
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_extension_mmap_del(Elm_Theme *th, const Eina_File *f);

/**
 * Get the list of registered extensions for the given theme
 *
 * @param th The theme from which to get the extensions
 * @return List of theme extensions. Do not free it.
 *
 * @see elm_theme_extension_add()
 *
 * @ingroup Theme
 */
EAPI const Eina_List *elm_theme_extension_list_get(const Elm_Theme *th);

/**
 * Set the theme search order for the given theme
 *
 * @param th The theme to set the search order, or if NULL, the default theme
 * @param theme Theme search string
 *
 * This sets the search string for the theme in path-notation from first
 * theme to search, to last, delimited by the : character. Example:
 *
 * "shiny:/path/to/file.edj:default"
 *
 * See the ELM_THEME environment variable for more information.
 *
 * @see elm_theme_get()
 * @see elm_theme_list_get()
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_set(Elm_Theme *th, const char *theme);

/**
 * Return the theme search order
 *
 * @param th The theme to get the search order, or if NULL, the default theme
 * @return The internal search order path
 *
 * This function returns a colon separated string of theme elements as
 * returned by elm_theme_list_get().
 *
 * @see elm_theme_set()
 * @see elm_theme_list_get()
 *
 * @ingroup Theme
 */
EAPI const char      *elm_theme_get(Elm_Theme *th);

/**
 * Return a list of theme elements to be used in a theme.
 *
 * @param th Theme to get the list of theme elements from.
 * @return The internal list of theme elements
 *
 * This returns the internal list of theme elements (will only be valid as
 * long as the theme is not modified by elm_theme_set() or theme is not
 * freed by elm_theme_free(). This is a list of strings which must not be
 * altered as they are also internal. If @p th is NULL, then the default
 * theme element list is returned.
 *
 * A theme element can consist of a full or relative path to a .edj file,
 * or a name, without extension, for a theme to be searched in the known
 * theme paths for Elementary.
 *
 * @see elm_theme_set()
 * @see elm_theme_get()
 *
 * @ingroup Theme
 */
EAPI const Eina_List *elm_theme_list_get(const Elm_Theme *th);

/**
 * Return the full path for a theme element
 *
 * @param f The theme element name
 * @param in_search_path Pointer to a boolean to indicate if item is in the search path or not
 * @return The full path to the file found.
 *
 * This returns a string you should free with free() on success, NULL on
 * failure. This will search for the given theme element, and if it is a
 * full or relative path element or a simple search-able name. The returned
 * path is the full path to the file, if searched, and the file exists, or it
 * is simply the full path given in the element or a resolved path if
 * relative to home. The @p in_search_path boolean pointed to is set to
 * EINA_TRUE if the file was a search-able file and is in the search path,
 * and EINA_FALSE otherwise.
 *
 * @ingroup Theme
 */
EAPI char            *elm_theme_list_item_path_get(const char *f, Eina_Bool *in_search_path);

/**
 * Flush the current theme.
 *
 * @param th Theme to flush
 *
 * This flushes caches that let elementary know where to find theme elements
 * in the given theme. If @p th is NULL, then the default theme is flushed.
 * Call this function if source theme data has changed in such a way as to
 * make any caches Elementary kept invalid.
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_flush(Elm_Theme *th);

/**
 * This flushes all themes (default and specific ones).
 *
 * This will flush all themes in the current application context, by calling
 * elm_theme_flush() on each of them.
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_full_flush(void);

/**
 * Return a list of theme elements in the theme search path
 *
 * @return A list of strings that are the theme element names.
 *
 * This lists all available theme files in the standard Elementary search path
 * for theme elements, and returns them in alphabetical order as theme
 * element names in a list of strings. Free this with
 * elm_theme_name_available_list_free() when you are done with the list.
 *
 * @ingroup Theme
 */
EAPI Eina_List       *elm_theme_name_available_list_new(void);

/**
 * Free the list returned by elm_theme_name_available_list_new()
 *
 * This frees the list of themes returned by
 * elm_theme_name_available_list_new(). Once freed the list should no longer
 * be used. a new list mys be created.
 *
 * @ingroup Theme
 */
EAPI void             elm_theme_name_available_list_free(Eina_List *list);

/**
 * Set a specific theme to be used for this object and its children
 *
 * @param obj The object to set the theme on
 * @param th The theme to set
 *
 * This sets a specific theme that will be used for the given object and any
 * child objects it has. If @p th is NULL then the theme to be used is
 * cleared and the object will inherit its theme from its parent (which
 * ultimately will use the default theme if no specific themes are set).
 *
 * Use special themes with great care as this will annoy users and make
 * configuration difficult. Avoid any custom themes at all if it can be
 * helped.
 *
 * @ingroup Theme
 */
EAPI void             elm_object_theme_set(Evas_Object *obj, Elm_Theme *th);

/**
 * Get the specific theme to be used
 *
 * @param obj The object to get the specific theme from
 * @return The specific theme set.
 *
 * This will return a specific theme set, or NULL if no specific theme is
 * set on that object. It will not return inherited themes from parents, only
 * the specific theme set for that specific object. See elm_object_theme_set()
 * for more information.
 *
 * @ingroup Theme
 */
EAPI Elm_Theme       *elm_object_theme_get(const Evas_Object *obj);

/**
 * Get a data item from a theme
 *
 * @param th The theme, or NULL for default theme
 * @param key The data key to search with
 * @return The data value, or NULL on failure
 *
 * This function is used to return data items from edc in @p th, an overlay, or an extension.
 * It works the same way as edje_file_data_get() except that the return is stringshared.
 *
 * @ingroup Theme
 */
EAPI const char      *elm_theme_data_get(Elm_Theme *th, const char *key);

/**
 * Get the file path for an edje file for the group and theme given
 *
 * @param th The theme, or NULL for default theme
 * @param group The group in the edje file to look for
 * @return The full path to the file as a string
 *
 * This function looks up the given edje @p group in the set of theme edje
 * files configured for the theme @p th (which if NULL indicates the default
 * theme). If not found in any, NULL wil be returned. If found, the string
 * returned is internal and should not be freed, but will only be valid
 * until the theme is re-configured, or cache flushed, so if the string needs
 * to be kept, duplicate it and store that. The string will be a stringshare
 * string that is returned by functions like eina_stringshare_add() so it can
 * be just references via stringshare functions if desired.
 *
 * If group is NULL, then nothing can be looked up, so it is a non-sensical
 * request.
 *
 * @since 1.8
 * @ingroup Theme
 */
EAPI const char *elm_theme_group_path_find(Elm_Theme *th, const char *group);

/**
 * Get a list of groups that match the initial base string given within all themes
 *
 * @param th The theme, or NULL for default theme
 * @param base The base string group collection to look for
 * @return A list of collection names (sorted) or NULL if none found
 *
 * This function will walk all theme files configured in the theme @p th (or
 * NULL if its the default) and find all groups that BEGIN with the string
 * @p begin and have that string as at LEAST their start, and then add the
 * fulll group name that matches to the list and return that full group
 * group string.
 *
 * The list returned must be freed by the caller, with each string being a
 * stringshared string to be freed with eina_stringshare_del(). Not doing so
 * may result in a leak.
 *
 * @since 1.8
 * @ingroup Theme
 */
 EAPI Eina_List *elm_theme_group_base_list(Elm_Theme *th, const char *base);

/**
 * Get the file path where elementary system theme files are found
 *
 * @return A string that holds the path where system themes are
 *
 * This returns the location in the filesystem where the system themes are
 * to be found that elementary looks for. This is useful for something
 * that wishes toiterate over the files in this folder and display them, for
 * example a theme selector.
 *
 * @since 1.8
 * @ingroup Theme
 */
EAPI const char *elm_theme_system_dir_get(void);

/**
 * Get the file path where elementary user theme files are found
 *
 * @return A string that holds the path where user themes are
 *
 * This returns the location in the filesystem where the user themes are
 * to be found that elementary looks for. This is useful for something
 * that wishes toiterate over the files in this folder and display them, for
 * example a theme selector.
 *
 * User themes are always looked for before system themes. The user theme
 * directory is normally expected to be writable by the user.
 *
 * @since 1.8
 * @ingroup Theme
 */
EAPI const char *elm_theme_user_dir_get(void);

/**
 * @}
 */
