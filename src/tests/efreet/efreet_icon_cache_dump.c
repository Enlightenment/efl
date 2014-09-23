#include "config.h" 

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <Eina.h>
#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>

#define EFREET_MODULE_LOG_DOM /* no logging in this file */

#include "Efreet.h"
#include "efreet_private.h"
#include "efreet_cache_private.h"

int verbose = 0;

static void
dump(Efreet_Icon_Theme *theme)
{
    Eet_File *ef;
    unsigned int count = 0;
    double start, avg;
    char **keys;
    int num, i;

    start = ecore_time_get();
    ef = eet_open(efreet_icon_cache_file(theme->name.internal), EET_FILE_MODE_READ);
    printf("open: %s %f\n", theme->name.internal, ecore_time_get() - start);

    start = ecore_time_get();
    keys = eet_list(ef, "*", &num);
    printf("list: %s %f\n", theme->name.internal, ecore_time_get() - start);
    if (!keys) return;

    start = ecore_time_get();
    for (i = 0; i < num; i++)
    {
        Efreet_Cache_Icon *icon;
        unsigned int j;

        icon = eet_data_read(ef, efreet_icon_edd(), keys[i]);
        if (!icon) continue;

        for (j = 0; j < icon->icons_count; ++j)
            count += icon->icons[j]->paths_count;
    }
    free(keys);

    start = ecore_time_get() - start;
    avg = 0;
    if (count > 0)
        avg = start / count;
    printf("read: %s - %u paths (time: %f) (avg %f)\n", theme->name.internal, count, start, avg);
    eet_close(ef);
    eet_clearcache();
}

int
main(int argc, char **argv)
{
    Eet_File *theme_ef;
    Eina_List *l = NULL;
    Efreet_Icon_Theme *theme;
    int i;

    efreet_cache_update = 0;

    if (!efreet_init()) return -1;

    theme_ef = eet_open(efreet_icon_theme_cache_file(), EET_FILE_MODE_READ);
    if (!theme_ef) return -1;

    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            theme = eet_data_read(theme_ef, efreet_icon_theme_edd(EINA_FALSE), argv[i]);
            if (theme) l = eina_list_append(l, theme);
        }
    }
    else
    {
        char **keys;
        int num;

        keys = eet_list(theme_ef, "*", &num);
        if (keys)
        {
            for (i = 0; i < num; i++)
            {
                theme = eet_data_read(theme_ef, efreet_icon_theme_edd(EINA_FALSE), keys[i]);
                if (theme) l = eina_list_append(l, theme);
            }
            free(keys);
        }
    }

    EINA_LIST_FREE(l, theme)
    {
        void *data;

        dump(theme);

        /* free theme */
        eina_list_free(theme->paths);
        eina_list_free(theme->inherits);
        EINA_LIST_FREE(theme->directories, data)
            free(data);
        free(theme);
    }

    efreet_shutdown();
    return 0;
}
