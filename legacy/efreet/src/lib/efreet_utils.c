/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

char *
efreet_util_path_in_default(const char *section, const char *path)
{
    Ecore_List *dirs;
    char *ret = NULL;
    char *dir;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    section);

    ecore_list_goto_first(dirs);
    while ((dir = ecore_list_next(dirs)))
    {
        size_t len;

        len = strlen(dir);
        if (!strncmp(path, dir, strlen(dir)))
        {
            ret = strdup(dir);
            break;
        }
    }

    ecore_list_destroy(dirs);
    return ret;
}

char *
efreet_util_path_to_file_id(const char *base, const char *path)
{
    size_t len;
    char *id, *p;

    len = strlen(base);
    if (strlen(path) <= len) return NULL;
    if (strncmp(path, base, len)) return NULL;

    id = strdup(path + len + 1);
    p = id;
    while (*p)
    {
        if (*p == '/') *p = '-';
        p++;
    }
    return id;
}

Efreet_Desktop *
efreet_util_desktop_by_file_id_get(const char *file_id)
{
    Efreet_Desktop *desktop = NULL;
    Ecore_List *dirs;
    const char *dir;

    if (!file_id) return NULL;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    if (!dirs) return NULL;

    ecore_list_goto_first(dirs);
    while ((dir = ecore_list_next(dirs)))
    {
        char *tmp, *p;
        char buf[PATH_MAX];

        tmp = strdup(file_id);
        p = tmp;

        while (p)
        {
            snprintf(buf, sizeof(buf), "%s/%s", dir, file_id);
            desktop = efreet_desktop_get(buf);
            if (desktop) break;
            p = strchr(p, '-');
            if (p) *p = '/';
        }
        free(tmp);
        if (desktop) break;
    }
    ecore_list_destroy(dirs);
    return desktop;
}
