/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "Efreet_Mime.h"
#include "efreet_private.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

int
ef_mime_cb_get(void)
{
    const char *mime = NULL;
    int misses = 0, i = 0;
    const char *files[] = {PACKAGE_DATA_DIR"/efreet/test/test_type.desktop",
                           PACKAGE_DATA_DIR"/efreet/test/test_type.desktop",
                           PACKAGE_DATA_DIR"/efreet/test/sub"};
    
    
    if (!efreet_mime_init())
    {
        printf("Could not init efreet\n");
        return 1;
    }

    for (i = 0; i < (sizeof(files) / sizeof(const char *)); ++i)
    {    
        mime = efreet_mime_type_get(files[i]);
        if (!mime)
        {
            printf("Missed %s\n", files[i]);
            misses ++;
        }
    }
    
    efreet_mime_shutdown();
    
    return !misses;
}
