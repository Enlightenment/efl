#ifndef EFREET_DESKTOP_H
#define EFREET_DESKTOP_H

/**
 * @file efreet_desktop.h
 * @brief Contains the structures and methods used to support the
 *        FDO desktop entry specificiation.
 *
 * @internal
 * @defgroup Efreet_Desktop_Group Efreet_Desktop: The FDO Desktop Entry
 *           Specification functions and structures
 * @ingroup Efreet_Group
 *
 * @{
 */


EAPI extern int EFREET_DESKTOP_TYPE_APPLICATION;
EAPI extern int EFREET_DESKTOP_TYPE_LINK;
EAPI extern int EFREET_DESKTOP_TYPE_DIRECTORY;

/**
 * Event id for cache update. All users of efreet_desktop_get must listen to
 * this event and refetch. The old eet cache will be closed and mem will
 * be invalidated.
 */
EAPI extern int EFREET_EVENT_DESKTOP_CACHE_UPDATE;
/**
 * Event id for cache build complete.
 * @since 1.1.0
 */
EAPI extern int EFREET_EVENT_DESKTOP_CACHE_BUILD;

/**
 * Efreet_Desktop
 */
typedef struct _Efreet_Desktop Efreet_Desktop;

/**
 * A callback used with efreet_desktop_command_get()
 */
typedef void *(*Efreet_Desktop_Command_Cb) (void *data, Efreet_Desktop *desktop,
                                            char *command, int remaining);

/**
 * A callback used to get download progress of remote uris
 */
typedef int (*Efreet_Desktop_Progress_Cb) (void *data, Efreet_Desktop *desktop,
                                            char *uri, long int total, long int current);

/**
 * A callback used to parse data for custom types
 */
typedef void *(*Efreet_Desktop_Type_Parse_Cb) (Efreet_Desktop *desktop, Efreet_Ini *ini);

/**
 * A callback used to save data for custom types
 */
typedef void (*Efreet_Desktop_Type_Save_Cb) (Efreet_Desktop *desktop, Efreet_Ini *ini);

/**
 * A callback used to free data for custom types
 */
typedef void *(*Efreet_Desktop_Type_Free_Cb) (void *data);

/**
 * Efreet_Desktop
 * @brief a parsed representation of a .desktop file
 */
struct _Efreet_Desktop
{
    int type;               /**< type of desktop file */

    int ref;                /**< reference count - internal */

    char *version;          /**< version of spec file conforms to */

    char *orig_path;        /**< original path to .desktop file */
    long long load_time;    /**< modified time of .desktop on disk */

    char *name;             /**< Specific name of the application */
    char *generic_name;     /**< Generic name of the application */
    char *comment;          /**< Tooltip for the entry */
    char *icon;             /**< Icon to display in file manager, menus, etc */
    char *try_exec;         /**< Binary to determine if app is installed */
    char *exec;             /**< Program to execute */
    char *path;             /**< Working directory to run app in */
    char *startup_wm_class; /**< If specified will map at least one window with
                                the given string as it's WM class or WM name */
    char *url;              /**< URL to access if type is EFREET_TYPE_LINK */

    Eina_List  *only_show_in;   /**< list of environments that should
                                                    display the icon */
    Eina_List  *not_show_in;    /**< list of environments that shoudn't
                                                    display the icon */
    Eina_List  *categories;     /**< Categories in which item should be shown */
    Eina_List  *mime_types;     /**< The mime types supppored by this app */

    unsigned char no_display;        /**< Don't display this application in menus */
    unsigned char hidden;            /**< User delete the item */
    unsigned char terminal;          /**< Does the program run in a terminal */
    unsigned char startup_notify;    /**< The starup notify settings of the app */
    unsigned char eet:1;             /**< The desktop file is in eet cache */

    Eina_Hash *x; /**< Keep track of all user extensions, keys that begin with X- */
    void *type_data; /**< Type specific data for custom types */
};


/**
 * @brief Gets a reference to an Efreet_Desktop structure representing the
 * contents of @a file or NULL if @a file is not a valid .desktop file.
 *
 * By using efreet_desktop_get the Efreet_Desktop will be saved in an internal
 * cache for quicker loading.
 *
 * Users of this command should listen to EFREET_EVENT_DESKTOP_CACHE_UPDATE
 * event, if the application is to keep the reference. When the event fires
 * the Efreet_Desktop struct should be invalidated and reloaded from a new
 * cache file.
 *
 * @param[in] file The file to get the Efreet_Desktop from
 * @return Returns a reference to a cached Efreet_Desktop on success, NULL
 * on failure
 */
EAPI Efreet_Desktop   *efreet_desktop_get(const char *file);

/**
 * @brief Increases reference count on desktop
 *
 * @param[in] desktop The Efreet_Desktop to ref
 * @return Returns the new reference count
 */
EAPI int               efreet_desktop_ref(Efreet_Desktop *desktop);

/**
 * @brief Creates a new empty Efreet_Desktop structure or NULL on failure
 *
 * @param file The file to create the Efreet_Desktop from
 * @return Returns a new empty_Efreet_Desktop on success, NULL on failure
 */
EAPI Efreet_Desktop   *efreet_desktop_empty_new(const char *file);

/**
 * @brief Gets a reference to an Efreet_Desktop structure representing the
 * contents of @a file or NULL if @a file is not a valid .desktop file.
 *
 * Users of this command should listen to EFREET_EVENT_DESKTOP_CACHE_UPDATE
 * event, if the application is to keep the reference. When the event fires
 * the Efreet_Desktop struct should be invalidated and reloaded from a new
 * cache file.
 *
 * @param[in] file The file to get the Efreet_Desktop from
 * @return Returns a reference to a cached Efreet_Desktop on success, NULL
 * on failure
 */
EAPI Efreet_Desktop   *efreet_desktop_new(const char *file);

/**
 * @brief Creates a new Efreet_Desktop structure initialized from the
 * contents of @a file or NULL on failure
 *
 * By using efreet_desktop_uncached_new the Efreet_Desktop structure will be
 * read from disk, and not from any cache.
 *
 * Data in the structure is allocated with strdup, so use free and strdup to
 * change values.
 *
 * @param[in] file The file to create the Efreet_Desktop from
 * @return Returns a new Efreet_Desktop on success, NULL on failure
 */
EAPI Efreet_Desktop   *efreet_desktop_uncached_new(const char *file);

/**
 * @brief Frees the Efreet_Desktop structure and all of it's data
 *
 * @param[in] desktop The Efreet_Desktop to work with
 * @return Returns no value
 */
EAPI void              efreet_desktop_free(Efreet_Desktop *desktop);

/**
 * @def efreet_desktop_unref(desktop)
 * Alias for efreet_desktop_free(desktop)
 */
#define efreet_desktop_unref(desktop) efreet_desktop_free((desktop))


/**
 * @brief Saves any changes made to @a desktop back to the file on the
 * filesystem
 *
 * @param[in] desktop The desktop file to save
 * @return Returns 1 on success or 0 on failure
 */
EAPI int               efreet_desktop_save(Efreet_Desktop *desktop);

/**
 * @brief Saves @a desktop to @a file
 *
 * Please use efreet_desktop_uncached_new() on an existing file
 * before using efreet_desktop_save_as()
 *
 * @param[in] desktop The desktop file to save
 * @param[in] file The filename to save as
 * @return Returns 1 on success or 0 on failure
 */
EAPI int               efreet_desktop_save_as(Efreet_Desktop *desktop,
                                                const char *file);


/**
 * @brief Parses the @a desktop exec line and returns an Ecore_Exe.
 *
 * @param[in] desktop The desktop file to work with
 * @param[in] files The files to be substituted into the exec line
 * @param[in] data The data pointer to pass
 * @return Returns the Ecore_Exce for @a desktop
 */
EAPI void              efreet_desktop_exec(Efreet_Desktop *desktop,
                                           Eina_List *files, void *data);


/**
 * @brief sets the global desktop environment name
 *
 * @param[in] environment the environment name
 */
EAPI void              efreet_desktop_environment_set(const char *environment);

/**
 * @brief sets the global desktop environment name
 *
 * @return environment the environment name
 */
EAPI const char       *efreet_desktop_environment_get(void);

/**
 * @brief Get a command to use to execute a desktop entry, and receive progress
 * updates for downloading of remote URI's passed in.
 *
 * @param[in] desktop the desktop entry
 * @param[in] files an eina list of file names to execute, as either absolute paths,
 * relative paths, or uris
 * @param[in] cb_command a callback to call for each prepared command line
 * @param[in] cb_prog a callback to get progress for the downloads
 * @param[in] data user data passed to the callback
 * @return Returns 1 on success or 0 on failure
 */
EAPI void             *efreet_desktop_command_progress_get(Efreet_Desktop *desktop,
                                         Eina_List *files,
                                         Efreet_Desktop_Command_Cb cb_command,
                                         Efreet_Desktop_Progress_Cb cb_prog,
                                         void *data);
/**
 * @brief Get a command to use to execute a desktop entry.
 *
 * @param[in] desktop the desktop entry
 * @param[in] files an eina list of file names to execute, as either absolute paths,
 * relative paths, or uris
 * @param[in] func a callback to call for each prepared command line
 * @param[in] data user data passed to the callback
 * @return Returns the return value of @p func on success or NULL on failure
 */
EAPI void              *efreet_desktop_command_get(Efreet_Desktop *desktop,
                                         Eina_List *files,
                                         Efreet_Desktop_Command_Cb func,
                                         void *data);

/**
 * @brief Get the command to use to execute a desktop entry
 *
 * The returned list and each of its elements must be freed.
 *
 * @param[in] desktop the desktop entry
 * @param[in] files an eina list of local files, as absolute paths, local paths, or file// uris (or NULL to get exec string with no files appended)
 * @return Returns an eina list of exec strings
 */
EAPI Eina_List *      efreet_desktop_command_local_get(Efreet_Desktop *desktop,
                                         Eina_List *files);


/**
 * @brief Retrieves the number of categories the given @a desktop belongs
 * too
 *
 * @param[in] desktop The desktop to work with
 * @return Returns the number of categories assigned to this desktop
 */
EAPI unsigned int      efreet_desktop_category_count_get(Efreet_Desktop *desktop);

/**
 * @brief add a category to a desktop
 *
 * @param[in] desktop the desktop
 * @param[in] category the category name
 */
EAPI void              efreet_desktop_category_add(Efreet_Desktop *desktop,
                                              const char *category);

/**
 * @brief removes a category from a desktop
 *
 * @param[in] desktop the desktop
 * @param[in] category the category name
 * @return 1 if the desktop had his category listed, 0 otherwise
 */
EAPI int               efreet_desktop_category_del(Efreet_Desktop *desktop,
                                              const char *category);


/**
 * @brief Adds the given type to the list of types in the system
 *
 * @param[in] type The type to add to the list of matching types
 * @param[in] parse_func a function to parse out custom fields
 * @param[in] save_func a function to save data returned from @a parse_func
 * @param[in] free_func a function to free data returned from @a parse_func
 * @return Returns the id of the new type
 */
EAPI int               efreet_desktop_type_add(const char *type,
                                    Efreet_Desktop_Type_Parse_Cb parse_func,
                                    Efreet_Desktop_Type_Save_Cb save_func,
                                    Efreet_Desktop_Type_Free_Cb free_func);

/**
 * @brief Add an alias for an existing desktop type.
 *
 * @param[in] from_type the type to alias (e.g. EFREE_DESKTOP_TYPE_APPLICATION)
 * @param[in] alias the alias
 * @return the new type id, or -1 if @p from_type was not valid
 *
 * This allows applications to add non-standard types that behave exactly as standard types.
 */
EAPI int               efreet_desktop_type_alias (int from_type,
                                             const char *alias);

/**
 * @brief get type specific data for custom desktop types
 *
 * @param[in] desktop the desktop
 * @return type specific data, or NULL if there is none
 */
EAPI void             *efreet_desktop_type_data_get(Efreet_Desktop *desktop);


/**
 * @brief Parse ';' separate list of strings according to the desktop spec
 *
 * @param[in] string the raw string list
 * @return an Eina_List of ecore string's
 */
EAPI Eina_List        *efreet_desktop_string_list_parse(const char *string);

/**
 * @brief Create a ';' separate list of strings according to the desktop spec
 *
 * @param[in] list Eina_List with strings
 * @return a raw string list
 */
EAPI char             *efreet_desktop_string_list_join(Eina_List *list);


/**
 * @brief Set the value for a X- field (Non spec) in the structure
 *
 * @param[in] desktop the desktop
 * @param[in] key the key name to set
 * @param[in] data the value to set
 * @return EINA_TRUE on success
 *
 * The key has to start with "X-"
 */
EAPI Eina_Bool         efreet_desktop_x_field_set(Efreet_Desktop *desktop, const char *key, const char *data);

/**
 * @brief Get the value for a X- field (Non spec) in the structure
 *
 * @param[in] desktop the desktop
 * @param[in] key the key
 * @return The value referenced by the key, or NULL if the key does not exist
 */
EAPI const char *      efreet_desktop_x_field_get(Efreet_Desktop *desktop, const char *key);

/**
 * @brief Delete the key and value for a X- field (Non spec) in the structure
 *
 * @param[in] desktop the desktop
 * @param[in] key the key
 * @return EINA_TRUE if the key existed
 */
EAPI Eina_Bool         efreet_desktop_x_field_del(Efreet_Desktop *desktop, const char *key);

/**
 * @}
 */

#endif
