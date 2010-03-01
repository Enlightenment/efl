/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "Efreet_Mime.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

int
ef_mime_cb_get(void)
{
    const char *mime = NULL;
    int misses = 0, i = 0;
    struct
    {
        char *file;
        char *mime;
    } files[] = {
        {PACKAGE_DATA_DIR"/test/test_type.desktop", "application/x-desktop"},
        {PACKAGE_DATA_DIR"/test/entry.png", "image/png"},
        {PACKAGE_DATA_DIR"/test/entry", "image/png"},
        {PACKAGE_DATA_DIR"/test/sub", "inode/directory"},
        {NULL, NULL}
    };


    if (!efreet_mime_init())
    {
        printf("Could not init efreet\n");
        return 1;
    }

    for (i = 0; files[i].file != NULL; ++i)
    {
        mime = efreet_mime_type_get(files[i].file);
        if (!mime)
        {
            printf("Got %s as null instead of %s\n", files[i].file, files[i].mime);
            misses ++;
        }
        else if (strcmp(mime, files[i].mime))
        {
            printf("Got %s as %s instead of %s\n", files[i].file, mime, files[i].mime);
            misses ++;
        }
    }

    efreet_mime_shutdown();

    return !misses;
}
