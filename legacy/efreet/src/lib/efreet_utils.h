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
    Efreet_Desktop *desktop;
    Efreet_Desktop_Change change;
};

char           *efreet_util_path_in_default(const char *section, const char *path);
const char     *efreet_util_path_to_file_id(const char *path);

Ecore_List     *efreet_util_desktop_mime_list(const char *mime);

Efreet_Desktop *efreet_util_desktop_wm_class_find(const char *wmname, const char *wmclass);
Efreet_Desktop *efreet_util_desktop_file_id_find(const char *file_id);
Efreet_Desktop *efreet_util_desktop_exec_find(const char *exec);
Efreet_Desktop *efreet_util_desktop_name_find(const char *name);
Efreet_Desktop *efreet_util_desktop_generic_name_find(const char *generic_name);

Ecore_List     *efreet_util_desktop_name_glob_list(const char *glob);
Ecore_List     *efreet_util_desktop_exec_glob_list(const char *glob);
Ecore_List     *efreet_util_desktop_generic_name_glob_list(const char *glob);
Ecore_List     *efreet_util_desktop_comment_glob_list(const char *glob);

extern int EFREET_EVENT_UTIL_DESKTOP_LIST_CHANGE;
extern int EFREET_EVENT_UTIL_DESKTOP_CHANGE;

#endif
