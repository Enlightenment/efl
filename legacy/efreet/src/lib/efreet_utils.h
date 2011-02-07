#ifndef EFREET_UTILS_H
#define EFREET_UTILS_H

/**
 * @file efreet_utils.h
 * @brief Contains utility functions to ease usage of Efreet.
 *        FDO desktop entry specificiation.
 * @addtogroup Efreet_Utils Efreet utilities for FDO
 *
 * @{
 */

EAPI const char *efreet_util_path_to_file_id(const char *path);

EAPI Eina_List *efreet_util_desktop_mime_list(const char *mime);

EAPI Efreet_Desktop *efreet_util_desktop_wm_class_find(const char *wmname, const char *wmclass);
EAPI Efreet_Desktop *efreet_util_desktop_file_id_find(const char *file_id);
EAPI Efreet_Desktop *efreet_util_desktop_exec_find(const char *exec);
EAPI Efreet_Desktop *efreet_util_desktop_name_find(const char *name);
EAPI Efreet_Desktop *efreet_util_desktop_generic_name_find(const char *generic_name);

EAPI Eina_List *efreet_util_desktop_name_glob_list(const char *glob);
EAPI Eina_List *efreet_util_desktop_exec_glob_list(const char *glob);
EAPI Eina_List *efreet_util_desktop_generic_name_glob_list(const char *glob);
EAPI Eina_List *efreet_util_desktop_comment_glob_list(const char *glob);

EAPI Eina_List *efreet_util_desktop_categories_list(void);
EAPI Eina_List *efreet_util_desktop_category_list(const char *category);

EAPI Eina_List *efreet_util_menus_find(void);

/**
 * @}
 */
#endif
