/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

int
ef_cb_utils(void)
{
    char *tmp;

    tmp = efreet_util_path_in_default("applications",
            "/usr/share/applications/test.desktop");
    if (strcmp(tmp, "/usr/share/applications"))
    {
        free(tmp);
        return 0;
    }
    free(tmp);

    tmp = efreet_util_path_to_file_id("/usr/share/applications",
            "/usr/share/applications/this/tmp/test.desktop");
    if (strcmp(tmp, "this-tmp-test.desktop"))
    {
        free(tmp);
        return 0;
    }
    free(tmp);
    return 1;
}
