/**
 * @defgroup App App
 * @ingroup Elementary
 * Provide information in order to make Elementary determine the @b
 * run time location of the software in question, so other data files
 * such as images, sound files, executable utilities, libraries,
 * modules and locale files can be found.
 */

/**
 * @addtogroup App
 * @{
 */

/**
 * Re-locate the application somewhere else after compilation, if the developer
 * wishes for easier distribution of pre-compiled binaries.
 *
 * @param mainfunc This is your application's main function name,
 *        whose binary's location is to be found. Providing @c NULL
 *        will make Elementary not to use it
 * @param dom This will be used as the application's "domain", in the
 *        form of a prefix to any environment variables that may
 *        override prefix detection and the directory name, inside the
 *        standard share or data directories, where the software's
 *        data files will be looked for.
 * @param checkfile This is an (optional) magic file's path to check
 *        for existence (and it must be located in the data directory,
 *        under the share directory provided above). Its presence will
 *        help determine the prefix found was correct. Pass @c NULL if
 *        the check is not to be done.
 *
 * The prefix system is designed to locate where the given software is
 * installed (under a common path prefix) at run time and then report
 * specific locations of this prefix and common directories inside
 * this prefix like the binary, library, data and locale directories,
 * through the @c elm_app_*_get() family of functions.
 *
 * Call elm_app_info_set() early on before you change working
 * directory or anything about @c argv[0], so it gets accurate
 * information.
 *
 * It will then try and trace back which file @p mainfunc comes from,
 * if provided, to determine the application's prefix directory.
 *
 * The @p dom parameter provides a string prefix to prepend before
 * environment variables, allowing a fallback to @b specific
 * environment variables to locate the software. You would most
 * probably provide a lowercase string there, because it will also
 * serve as directory domain, explained next. For environment
 * variables purposes, this string is made uppercase. For example if
 * @c "myapp" is provided as the prefix, then the program would expect
 * @c "MYAPP_PREFIX" as a master environment variable to specify the
 * exact install prefix for the software, or more specific environment
 * variables like @c "MYAPP_BIN_DIR", @c "MYAPP_LIB_DIR", @c
 * "MYAPP_DATA_DIR" and @c "MYAPP_LOCALE_DIR", which could be set by
 * the user or scripts before launching. If not provided (@c NULL),
 * environment variables will not be used to override compiled-in
 * defaults or auto detections.
 *
 * The @p dom string also provides a subdirectory inside the system
 * shared data directory for data files. For example, if the system
 * directory is @c /usr/local/share, then this directory name is
 * appended, creating @c /usr/local/share/myapp, if it @p was @c
 * "myapp". It is expected that the application installs data files in
 * this directory.
 *
 * The @p checkfile is a file name or path of something inside the
 * share or data directory to be used to test that the prefix
 * detection worked. For example, your app will install a wallpaper
 * image as @c /usr/local/share/myapp/images/wallpaper.jpg and so to
 * check that this worked, provide @c "images/wallpaper.jpg" as the @p
 * checkfile string.
 *
 * @see elm_app_compile_bin_dir_set()
 * @see elm_app_compile_lib_dir_set()
 * @see elm_app_compile_data_dir_set()
 * @see elm_app_compile_locale_set()
 * @see elm_app_prefix_dir_get()
 * @see elm_app_bin_dir_get()
 * @see elm_app_lib_dir_get()
 * @see elm_app_data_dir_get()
 * @see elm_app_locale_dir_get()
 *
 * @ingroup App
 */
EAPI void        elm_app_info_set(void *mainfunc, const char *dom, const char *checkfile);

/**
 * Set a formal name to be used with the elm application.
 *
 * @param name Application name.
 *
 * @ingroup App
 * @since 1.8
 */
EAPI void        elm_app_name_set(const char *name);

/**
 * Set the path to the '.desktop' file to be associated
 * with the elm application.
 *
 * @param path The path to the '.desktop' file
 *
 * @warning Since this path is very environment dependent,
 * this will hold whatever value is passed to it.
 *
 * @ingroup App
 * @since 1.8
 */
EAPI void        elm_app_desktop_entry_set(const char *path);

/**
 * Provide information on the @b fallback application's binaries
 * directory, in scenarios where they get overridden by
 * elm_app_info_set().
 *
 * @param dir The path to the default binaries directory (compile time
 * one)
 *
 * @note Elementary will as well use this path to determine actual
 * names of binaries' directory paths, maybe changing it to be @c
 * something/local/bin instead of @c something/bin, only, for
 * example.
 *
 * @warning You should call this function @b before
 * elm_app_info_set().
 *
 * @ingroup App
 */
EAPI void        elm_app_compile_bin_dir_set(const char *dir);

/**
 * Provide information on the @b fallback application's libraries
 * directory, on scenarios where they get overridden by
 * elm_app_info_set().
 *
 * @param dir The path to the default libraries directory (compile
 * time one)
 *
 * @note Elementary will as well use this path to determine actual
 * names of libraries' directory paths, maybe changing it to be @c
 * something/lib32 or @c something/lib64 instead of @c something/lib,
 * only, for example.
 *
 * @warning You should call this function @b before
 * elm_app_info_set().
 *
 * @ingroup App
 */
EAPI void        elm_app_compile_lib_dir_set(const char *dir);

/**
 * Provide information on the @b fallback application's data
 * directory, on scenarios where they get overridden by
 * elm_app_info_set().
 *
 * @param dir The path to the default data directory (compile time
 * one)
 *
 * @note Elementary will as well use this path to determine actual
 * names of data directory paths, maybe changing it to be @c
 * something/local/share instead of @c something/share, only, for
 * example.
 *
 * @warning You should call this function @b before
 * elm_app_info_set().
 *
 * @ingroup App
 */
EAPI void        elm_app_compile_data_dir_set(const char *dir);

/**
 * Provide information on the @b fallback application's locale
 * directory, on scenarios where they get overridden by
 * elm_app_info_set().
 *
 * @param dir The path to the default locale directory (compile time
 * one)
 *
 * @warning You should call this function @b before
 * elm_app_info_set().
 *
 * @ingroup App
 */
EAPI void        elm_app_compile_locale_set(const char *dir);

/**
 * Retrieve the application formal name, as set by elm_app_name_set().
 *
 * @return The application formal name.
 *
 * @ingroup App
 * @since 1.8
 */
EAPI const char *elm_app_name_get(void);

/**
 * Retrieve the path to the '.desktop' file, as set by
 * elm_app_desktop_entry_set().
 *
 * @return The '.desktop' file path.
 *
 * @ingroup App
 * @since 1.8
 */
EAPI const char *elm_app_desktop_entry_get(void);

/**
 * Retrieve the application's run time prefix directory, as set by
 * elm_app_info_set() and the way (environment) the application was
 * run from.
 *
 * @return The directory prefix the application is actually using.
 *
 * @ingroup App
 */
EAPI const char *elm_app_prefix_dir_get(void);

/**
 * Retrieve the application's run time binaries prefix directory, as
 * set by elm_app_info_set() and the way (environment) the application
 * was run from.
 *
 * @return The binaries directory prefix the application is actually
 * using.
 *
 * @ingroup App
 */
EAPI const char *elm_app_bin_dir_get(void);

/**
 * Retrieve the application's run time libraries prefix directory, as
 * set by elm_app_info_set() and the way (environment) the application
 * was run from.
 *
 * @return The libraries directory prefix the application is actually
 * using.
 *
 * @ingroup App
 */
EAPI const char *elm_app_lib_dir_get(void);

/**
 * Retrieve the application's run time data prefix directory, as
 * set by elm_app_info_set() and the way (environment) the application
 * was run from.
 *
 * @return The data directory prefix the application is actually
 * using.
 *
 * @ingroup App
 */
EAPI const char *elm_app_data_dir_get(void);

/**
 * Retrieve the application's run time locale prefix directory, as
 * set by elm_app_info_set() and the way (environment) the application
 * was run from.
 *
 * @return The locale directory prefix the application is actually
 * using.
 *
 * @ingroup App
 */
EAPI const char *elm_app_locale_dir_get(void);

/**
 * @}
 */
