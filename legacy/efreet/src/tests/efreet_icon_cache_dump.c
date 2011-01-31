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
#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "Efreet.h"
#include "efreet_private.h"
#include "efreet_cache_private.h"

static Eet_Data_Descriptor *edd = NULL;

int verbose = 0;

static void
dump(Efreet_Icon_Theme *theme, Eet_File *ef)
{
    Efreet_Cache_Icons *cache;
    Eina_Iterator *it;
    const char *key;
    unsigned int count = 0;
    double start, avg;

    start = ecore_time_get();

    printf("open: %s\n", theme->name.internal);

    cache = eet_data_read(ef, edd, theme->name.internal);

    printf("read: %s %f\n", theme->name.internal, ecore_time_get() - start);

    if (!cache || !cache->icons) return ;

    it = eina_hash_iterator_key_new(cache->icons);

    EINA_ITERATOR_FOREACH(it, key)
    {
        Efreet_Cache_Icon *icon;
        unsigned int i;

        icon = eina_hash_find(cache->icons, key);

        for (i = 0; i < icon->icons_count; ++i)
            count += icon->icons[i]->paths_count;
    }

    eina_iterator_free(it);

    efreet_hash_free(cache->icons, EINA_FREE_CB(efreet_cache_icon_free));
    free(cache);

    start = ecore_time_get() - start;
    avg = start / count;
    printf("read: %s - %u paths (time: %f) (avg %f)\n", theme->name.internal, count, start, avg);
}

int
main(int argc, char **argv)
{
    Eet_File *icons_ef;
    Eet_File *theme_ef;
    Eina_List *l = NULL;
    Efreet_Icon_Theme *theme;
    int i;

    efreet_cache_update = 0;

    if (!efreet_init()) return -1;

    theme_ef = eet_open(efreet_icon_theme_cache_file(), EET_FILE_MODE_READ);
    if (!theme_ef) return -1;

    edd = efreet_icons_edd(EINA_FALSE);
    if (!edd) return -1;

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

    icons_ef = eet_open(efreet_icon_cache_file(), EET_FILE_MODE_READ);
    if (!icons_ef) return -1;

    EINA_LIST_FREE(l, theme)
    {
        void *data;

        dump(theme, icons_ef);

        /* free theme */
        eina_list_free(theme->paths);
        eina_list_free(theme->inherits);
        EINA_LIST_FREE(theme->directories, data)
            free(data);
        free(theme);
    }

    eet_close(icons_ef);

    efreet_shutdown();
    return 0;
}
