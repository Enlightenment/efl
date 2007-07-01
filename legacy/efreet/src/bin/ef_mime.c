/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "Efreet_Mime.h"
#include "efreet_private.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

double
calc_time(const struct timeval *start, const struct timeval *end)
{
    double s, e;
    
    s = ((double)start->tv_sec) + ((double)start->tv_usec / 1000000.0);
    e = ((double)end->tv_sec) + ((double)end->tv_usec / 1000000.0);
    
    return e - s;
}

int
ef_mime_cb_get(void)
{
    Efreet_Mime_Method flags = EFREET_MIME_FLAG_GLOB;
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
        mime = efreet_mime_get(files[i], flags);
        if (!mime)
        {
            printf("Missed %s\n", files[i]);
            misses ++;
        }
        flags <<= 1;
    }
    
    efreet_mime_shutdown();
    
    return !misses;
}
