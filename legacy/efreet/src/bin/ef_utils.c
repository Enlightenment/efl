/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

int
ef_cb_utils(void)
{
    Efreet_Desktop *desktop;
    char *tmp;

    tmp = efreet_util_path_in_default("applications",
            "/usr/share/applications/test.desktop");
    if (!tmp || strcmp(tmp, "/usr/share/applications"))
    {
        if (tmp) free(tmp);
        return 0;
    }
    if (tmp) free(tmp);

    tmp = efreet_util_path_to_file_id("/usr/share/applications",
            "/usr/share/applications/this/tmp/test.desktop");
    if (!tmp || strcmp(tmp, "this-tmp-test.desktop"))
    {
        if (tmp) free(tmp);
        return 0;
    }
    if (tmp) free(tmp);

    desktop = efreet_util_desktop_by_file_id_get("kde-kresources.desktop");
    printf("kde-kresources.desktop: %p\n", desktop);
    desktop = efreet_util_desktop_by_file_id_get("mplayer.desktop");
    printf("mplayer.desktop: %p\n", desktop);
    desktop = efreet_util_desktop_by_file_id_get("nautilus-computer.desktop");
    printf("nautilus-computer.desktop: %p\n", desktop);
    return 1;
}
