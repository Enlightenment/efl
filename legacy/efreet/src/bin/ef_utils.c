/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

int
ef_cb_utils(void)
{
    Efreet_Desktop *desktop;
    char *tmp1;
    const char *tmp2;

    printf("\n");
    tmp1 = efreet_util_path_in_default("applications",
            "/usr/share/applications/test.desktop");
    if (tmp1)
    {
        printf("%s\n", tmp1);
        free(tmp1);
    }

    tmp2 = efreet_util_path_to_file_id("/usr/share/applications/this/tmp/test.desktop");
    if (tmp2) printf("%s\n", tmp2);

    desktop = efreet_util_desktop_file_id_find("kde-kresources.desktop");
    printf("kde-kresources.desktop: %p\n", desktop);
    desktop = efreet_util_desktop_file_id_find("mplayer.desktop");
    printf("mplayer.desktop: %p\n", desktop);
    desktop = efreet_util_desktop_file_id_find("nautilus-computer.desktop");
    printf("nautilus-computer.desktop: %p\n", desktop);
    return 1;
}
