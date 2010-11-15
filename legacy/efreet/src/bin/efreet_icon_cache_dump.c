#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "Efreet.h"
#include "efreet_private.h"

static Eet_Data_Descriptor *edd = NULL;

int verbose = 0;

static void
dump(Efreet_Icon_Theme *theme)
{
    Eet_File *ef;
    char **keys;
    int i, num;
    double start, max, avg;

    start = ecore_time_get();
    ef = eet_open(efreet_icon_cache_file(theme->name.internal), EET_FILE_MODE_READ);
    printf("open: %s %f\n", theme->name.internal, ecore_time_get() - start);
    if (!ef) return;
    start = ecore_time_get();
    keys = eet_list(ef, "*", &num);
    printf("list: %s %d, %f\n", theme->name.internal, num, ecore_time_get() - start);
    if (!keys)
    {
        eet_close(ef);
        return;
    }
    start = ecore_time_get();
    for (i = 0; i < num; i++)
    {
        Efreet_Cache_Icon *icon;
        double dt;

        dt = ecore_time_get();
        icon = eet_data_read(ef, edd, keys[i]);
        if (!icon) continue;
        dt = ecore_time_get() - dt;
        if (dt > max)
            max = dt;
        efreet_cache_icon_free(icon);
    }
    start = ecore_time_get() - start;
    avg = start / num;
    printf("read: %s %f %f %f\n", theme->name.internal, start, avg, max);
    free(keys);
    eet_close(ef);
}

int
main(int argc, char **argv)
{
    Eina_List *l = NULL;
    Efreet_Icon_Theme *theme;
    int i;

    efreet_cache_update = 0;

    if (!efreet_init()) goto efreet_error;
    edd = efreet_icon_edd_init();
    if (!edd) goto edd_error;

    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            theme = efreet_icon_theme_find(argv[i]);
            if (theme) l = eina_list_append(l, theme);
        }
    }
    if (!l)
        l = efreet_icon_theme_list_get();
    EINA_LIST_FREE(l, theme)
    {
        dump(theme);
    }

    efreet_shutdown();
    return 0;
edd_error:
    efreet_shutdown();
efreet_error:
    return 1;
}
