/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

int
efreet_util_path_in_default(const char *section, const char *path)
{
    Ecore_List *dirs;
    int ret = 0;
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
            ret = 1;
            break;
        }
    }

    ecore_list_destroy(dirs);
    return ret;
}
