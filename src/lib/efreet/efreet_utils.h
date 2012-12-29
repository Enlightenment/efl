#ifndef EFREET_UTILS_H
#define EFREET_UTILS_H

/**
 * @file efreet_utils.h
 * @brief Contains utility functions to ease usage of Efreet.
 *        FDO desktop entry specificiation.
 * @addtogroup Efreet_Utils Efreet utilities for FDO
 * @ingroup Efreet
 *
 * @{
 */


/**
 * Returns the fdo file id for a given path. If the file isn't inside
 * a default fdo path it will return NULL.
 *
 * @param path The path to find the file id for
 *
 * @return File id for path or NULL
 */
EAPI const char *efreet_util_path_to_file_id(const char *path);


/**
 * Find all desktops for a given mime type
 *
 * This list must be freed using EINA_LIST_FREE / efreet_desktop_free
 *
 * @param mime the mime type
 * @return a list of desktops
 */
EAPI Eina_List *efreet_util_desktop_mime_list(const char *mime);


/**
 * Find all desktops for a given wm class
 *
 * This list must be freed using EINA_LIST_FREE / efreet_desktop_free
 *
 * @param wmname the wm name
 * @param wmclass the wm class
 * @return a list of desktops
 */
EAPI Efreet_Desktop *efreet_util_desktop_wm_class_find(const char *wmname, const char *wmclass);

/**
 * Find a desktop by file id
 *
 * return value must be freed by efreet_desktop_free
 *
 * @param file_id the file id
 * @return a desktop
 */
EAPI Efreet_Desktop *efreet_util_desktop_file_id_find(const char *file_id);

/**
 * Find a desktop by exec
 *
 * return value must be freed by efreet_desktop_free
 *
 * @param exec the exec name
 * @return a desktop
 */
EAPI Efreet_Desktop *efreet_util_desktop_exec_find(const char *exec);

/**
 * Find a desktop by name
 *
 * return value must be freed by efreet_desktop_free
 *
 * @param name the name
 * @return a desktop
 */
EAPI Efreet_Desktop *efreet_util_desktop_name_find(const char *name);

/**
 * Find a desktop by generic name
 *
 * return value must be freed by efreet_desktop_free
 *
 * @param generic_name the generic name
 * @return a desktop
 */
EAPI Efreet_Desktop *efreet_util_desktop_generic_name_find(const char *generic_name);


/**
 * Find all desktops where name matches a glob pattern
 *
 * This list must be freed using EINA_LIST_FREE / efreet_desktop_free
 *
 * @param glob the pattern to match
 * @return a list of desktops
 */
EAPI Eina_List *efreet_util_desktop_name_glob_list(const char *glob);

/**
 * Find all desktops where exec matches a glob pattern
 *
 * This list must be freed using EINA_LIST_FREE / efreet_desktop_free
 *
 * @param glob the pattern to match
 * @return a list of desktops
 */
EAPI Eina_List *efreet_util_desktop_exec_glob_list(const char *glob);

/**
 * Find all desktops where generic name matches a glob pattern
 *
 * This list must be freed using EINA_LIST_FREE / efreet_desktop_free
 *
 * @param glob the pattern to match
 * @return a list of desktops
 */
EAPI Eina_List *efreet_util_desktop_generic_name_glob_list(const char *glob);

/**
 * Find all desktops where comment matches a glob pattern
 *
 * This list must be freed using EINA_LIST_FREE / efreet_desktop_free
 *
 * @param glob the pattern to match
 * @return a list of desktops
 */
EAPI Eina_List *efreet_util_desktop_comment_glob_list(const char *glob);


/**
 * Find all desktop categories
 * This list must be freed using EINA_LIST_FREE
 *
 * @return an Eina_List of category names (const char *)
 */
EAPI Eina_List *efreet_util_desktop_categories_list(void);

/**
 * Find all desktops in a given category
 *
 * This list must be freed using EINA_LIST_FREE / efreet_desktop_free
 *
 * @param category the category name
 * @return a list of desktops
 */
EAPI Eina_List *efreet_util_desktop_category_list(const char *category);


/**
 * Returns a list of .menu files found in the various config dirs.
 * @return An eina list of menu file paths (const char *). This must be freed with EINA_LIST_FREE.
 */
EAPI Eina_List *efreet_util_menus_find(void);

/**
 * @}
 */
#endif
