#include "Efreet.h"
#include "Efreet_Mime.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <Ecore.h>

#define THEME "Tango"
#define SIZE 128

int
ef_mime_cb_get(void)
{
    const char *mime = NULL, *icon;
    int misses = 0, i = 0;
    struct
    {
        char *file;
        char *mime;
    } files[] = {
        {PKG_DATA_DIR"/test/test_type.desktop", "application/x-desktop"},
        {PKG_DATA_DIR"/test/entry.png", "image/png"},
        {PKG_DATA_DIR"/test/entry", "image/png"},
        {PKG_DATA_DIR"/test/sub", "inode/directory"},
        {NULL, NULL}
    };
    double start;

    if (!efreet_mime_init())
    {
        printf("Could not init efreet\n");
        return 1;
    }

    for (i = 0; files[i].file; ++i)
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
        start = ecore_time_get();
        icon = efreet_mime_type_icon_get(files[i].mime, THEME, SIZE);
        printf("mime icon: %s %s %f\n", files[i].mime, icon, ecore_time_get() - start);
    }

    efreet_mime_shutdown();

    return !misses;
}
