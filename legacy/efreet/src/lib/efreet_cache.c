#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Efreet.h"
#include "efreet_private.h"

/**
 * Data for cache files
 */
static Eet_Data_Descriptor *cache_icon_edd = NULL;
static Eet_Data_Descriptor *cache_icon_element_edd = NULL;

static void efreet_icon_edd_shutdown(void);

int
efreet_cache_init(void)
{
    if (!efreet_icon_edd_init()) return 0;
    return 1;
}

void
efreet_cache_shutdown(void)
{
    efreet_icon_edd_shutdown();
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI const char *
efreet_icon_cache_file(const char *theme)
{
    static char cache_file[PATH_MAX] = { '\0' };
    const char *home;

    home = efreet_home_dir_get();

    snprintf(cache_file, sizeof(cache_file), "%s/.efreet/icon_%s.cache", home, theme);

    return cache_file;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI Eet_Data_Descriptor *
efreet_icon_edd_init(void)
{
    Eet_Data_Descriptor_Class iconeddc;
    Eet_Data_Descriptor_Class elemeddc;

    if (!cache_icon_edd)
    {
        EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&iconeddc, Efreet_Cache_Icon);
        EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&elemeddc, Efreet_Cache_Icon_Element);
        cache_icon_edd = eet_data_descriptor_file_new(&iconeddc);
        if (!cache_icon_edd)
            goto error;
        cache_icon_element_edd = eet_data_descriptor_file_new(&elemeddc);
        if (!cache_icon_element_edd)
            goto error;

#if 0
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_edd, Efreet_Cache_Icon, "name", name, EET_T_STRING);
#endif
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_edd, Efreet_Cache_Icon, "theme", theme, EET_T_STRING);
#if 0
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_edd, Efreet_Cache_Icon, "context", context, EET_T_INT);
#endif
        EET_DATA_DESCRIPTOR_ADD_LIST(cache_icon_edd, Efreet_Cache_Icon, "icons", icons, cache_icon_element_edd);
#if 0
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_element_edd, Efreet_Cache_Icon_Element, "type", type, EET_T_INT);
#endif
        eet_data_descriptor_element_add(cache_icon_element_edd, "paths", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Cache_Icon_Element, paths), 0, NULL, NULL);
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_element_edd, Efreet_Cache_Icon_Element, "size.normal", size.normal, EET_T_USHORT);
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_element_edd, Efreet_Cache_Icon_Element, "size.min", size.min, EET_T_USHORT);
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_element_edd, Efreet_Cache_Icon_Element, "size.max", size.max, EET_T_USHORT);
    }
    return cache_icon_edd;
error:
    efreet_icon_edd_shutdown();
    return NULL;
}

static void
efreet_icon_edd_shutdown(void)
{
    if (cache_icon_edd) eet_data_descriptor_free(cache_icon_edd);
    cache_icon_edd = NULL;
    if (cache_icon_element_edd) eet_data_descriptor_free(cache_icon_element_edd);
    cache_icon_element_edd = NULL;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI void
efreet_cache_icon_free(Efreet_Cache_Icon *icon)
{
    Efreet_Cache_Icon_Element *elem;
    if (icon->free)
    {
#if 0
        eina_stringshare_del(icon->name);
#endif
        eina_stringshare_del(icon->theme);
    }

    EINA_LIST_FREE(icon->icons, elem)
    {
        const char *path;

        if (icon->free)
            EINA_LIST_FREE(elem->paths, path)
                eina_stringshare_del(path);
        else
            eina_list_free(elem->paths);
        free(elem);
    }
    free(icon);
}
