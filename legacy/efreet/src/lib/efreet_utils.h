/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_UTILS_H
#define EFREET_UTILS_H

typedef enum Efreet_Desktop_Change Efreet_Desktop_Change;

enum Efreet_Desktop_Change
{
    EFREET_DESKTOP_CHANGE_ADD,
    EFREET_DESKTOP_CHANGE_REMOVE,
    EFREET_DESKTOP_CHANGE_UPDATE
};

typedef struct Efreet_Event_Desktop_Change Efreet_Event_Desktop_Change;

struct Efreet_Event_Desktop_Change
{
    Efreet_Desktop *current;
    Efreet_Desktop *previous;
    Efreet_Desktop_Change change;
};

EAPI int efreet_util_init(void);
EAPI int efreet_util_shutdown(void);

EAPI const char *efreet_util_path_to_file_id(const char *path);

EAPI Ecore_List *efreet_util_desktop_mime_list(const char *mime);

EAPI Efreet_Desktop *efreet_util_desktop_wm_class_find(const char *wmname, const char *wmclass);
EAPI Efreet_Desktop *efreet_util_desktop_file_id_find(const char *file_id);
EAPI Efreet_Desktop *efreet_util_desktop_exec_find(const char *exec);
EAPI Efreet_Desktop *efreet_util_desktop_name_find(const char *name);
EAPI Efreet_Desktop *efreet_util_desktop_generic_name_find(const char *generic_name);

EAPI Ecore_List *efreet_util_desktop_name_glob_list(const char *glob);
EAPI Ecore_List *efreet_util_desktop_exec_glob_list(const char *glob);
EAPI Ecore_List *efreet_util_desktop_generic_name_glob_list(const char *glob);
EAPI Ecore_List *efreet_util_desktop_comment_glob_list(const char *glob);

EAPI Ecore_List *efreet_util_desktop_categories_list(void);
EAPI Ecore_List *efreet_util_desktop_category_list(const char *category);

EAPI Ecore_List *efreet_util_menus_find(void);

EAPI extern int EFREET_EVENT_DESKTOP_LIST_CHANGE;
EAPI extern int EFREET_EVENT_DESKTOP_CHANGE;

#endif
