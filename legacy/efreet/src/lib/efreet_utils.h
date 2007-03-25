/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_UTILS_H
#define EFREET_UTILS_H

char           *efreet_util_path_in_default(const char *section, const char *path);
char           *efreet_util_path_to_file_id(const char *path);
Efreet_Desktop *efreet_util_desktop_by_file_id_get(const char *file_id);

#endif
