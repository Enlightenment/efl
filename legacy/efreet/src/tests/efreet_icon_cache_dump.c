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

    eina_hash_free(cache->icons);
    free(cache);

    start = ecore_time_get() - start;
    avg = start / count;
    printf("read: %s - %u paths (time: %f) (avg %f)\n", theme->name.internal, count, start, avg);
}

int
main(int argc, char **argv)
{
    Eet_File *ef;
    Eina_List *l = NULL;
    Efreet_Icon_Theme *theme;
    int i;

    efreet_cache_update = 0;

    if (!efreet_init()) return -1;

    edd = efreet_icon_theme_edd(EINA_FALSE);
    if (!edd) return -1;

    if (argc > 1)
        for (i = 1; i < argc; i++)
        {
            theme = efreet_icon_theme_find(argv[i]);
            if (theme) l = eina_list_append(l, theme);
        }

    if (!l) l = efreet_icon_theme_list_get();

    ef = eet_open(efreet_icon_cache_file(), EET_FILE_MODE_READ);
    if (!ef) return -1;

    EINA_LIST_FREE(l, theme)
        dump(theme, ef);

    eet_close(ef);

    efreet_shutdown();
    return 0;
}
