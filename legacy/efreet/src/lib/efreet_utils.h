/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_UTILS_H
#define EFREET_UTILS_H

char           *efreet_util_path_in_default(const char *section, const char *path);
const char     *efreet_util_path_to_file_id(const char *path);

Efreet_Desktop *efreet_util_desktop_wmclass_find(const char *wmname, const char *wmclass);
Efreet_Desktop *efreet_util_desktop_file_id_find(const char *file_id);
Efreet_Desktop *efreet_util_desktop_exec_find(const char *exec);
Efreet_Desktop *efreet_util_desktop_name_find(const char *name);
Efreet_Desktop *efreet_util_desktop_generic_name_find(const char *generic_name);


#endif
