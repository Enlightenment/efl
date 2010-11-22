#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>

#include "Efreet.h"
#include "efreet_private.h"

/* TODO: Listen for changes for icon dirs */
/* TODO: Transfer some of desktop cache to this file, to share some infrastructure */

/**
 * Data for cache files
 */
static Eet_Data_Descriptor *cache_icon_edd = NULL;
static Eet_Data_Descriptor *cache_icon_element_edd = NULL;
static Eet_Data_Descriptor *cache_icon_fallback_edd = NULL;

static Eet_File            *icon_cache = NULL;
static const char          *icon_cache_name = NULL;
static Eet_File            *icon_fallback_cache = NULL;

static Ecore_File_Monitor  *icon_cache_monitor = NULL;
static Ecore_Timer         *icon_cache_timer = NULL;

static Ecore_Exe           *icon_cache_exe = NULL;
static int                  icon_cache_exe_lock = -1;
static Ecore_Event_Handler *icon_cache_exe_handler;

static void efreet_icon_edd_shutdown(void);

static Eina_Bool icon_cache_exe_cb(void *data, int type, void *event);
static void icon_cache_update_cb(void *data, Ecore_File_Monitor *em,
                               Ecore_File_Event event, const char *path);

static void icon_cache_timer_update(void);
static Eina_Bool icon_cache_timer_cb(void *data);

static void icon_cache_close(void);

EAPI int EFREET_EVENT_ICON_CACHE_UPDATE = 0;

int
efreet_cache_init(void)
{
    char buf[PATH_MAX];

    if (!efreet_icon_edd_init())
    {
        efreet_icon_edd_shutdown();
        return 0;
    }
    if (!efreet_icon_fallback_edd_init())
    {
        efreet_icon_edd_shutdown();
        return 0;
    }

    EFREET_EVENT_ICON_CACHE_UPDATE = ecore_event_type_new();

    snprintf(buf, sizeof(buf), "%s/.efreet", efreet_home_dir_get());
    if (!ecore_file_mkpath(buf)) goto cache_error;

    if (efreet_cache_update)
    {
        icon_cache_exe_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                                             icon_cache_exe_cb, NULL);
        if (!icon_cache_exe_handler) goto cache_error;

        icon_cache_monitor = ecore_file_monitor_add(buf,
                                               icon_cache_update_cb,
                                               NULL);
        if (!icon_cache_monitor) goto cache_error;

#if 0
        efreet_desktop_changes_listen();
#endif

        ecore_exe_run(PACKAGE_LIB_DIR "/efreet/efreet_icon_cache_create", NULL);
    }

    return 1;
cache_error:
    if (icon_cache_exe_handler) ecore_event_handler_del(icon_cache_exe_handler);
    if (icon_cache_monitor) ecore_file_monitor_del(icon_cache_monitor);
    return 0;
}

void
efreet_cache_shutdown(void)
{
    if (icon_cache_timer) ecore_timer_del(icon_cache_timer);
    icon_cache_close();

    if (icon_cache_exe_handler) ecore_event_handler_del(icon_cache_exe_handler);
    if (icon_cache_monitor) ecore_file_monitor_del(icon_cache_monitor);
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

/*
 * Needs EAPI because of helper binaries
 */
EAPI Eet_Data_Descriptor *
efreet_icon_fallback_edd_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (!cache_icon_fallback_edd)
    {
        EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Cache_Icon);
        cache_icon_fallback_edd = eet_data_descriptor_file_new(&eddc);
        if (!cache_icon_fallback_edd)
            goto error;

#if 0
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_fallback_edd, Efreet_Cache_Icon, "name", name, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_fallback_edd, Efreet_Cache_Icon, "theme", theme, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_fallback_edd, Efreet_Cache_Icon, "context", context, EET_T_INT);
#endif
        EET_DATA_DESCRIPTOR_ADD_BASIC(cache_icon_fallback_edd, Efreet_Cache_Icon, "fallback", fallback, EET_T_UCHAR);
        eet_data_descriptor_element_add(cache_icon_fallback_edd, "icons", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Cache_Icon, icons), 0, NULL, NULL);
    }
    return cache_icon_fallback_edd;
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
    if (cache_icon_fallback_edd) eet_data_descriptor_free(cache_icon_fallback_edd);
    cache_icon_fallback_edd = NULL;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI void
efreet_cache_icon_free(Efreet_Cache_Icon *icon)
{
    void *data;

    if (!icon) return;

    if (icon->free)
    {
#if 0
        eina_stringshare_del(icon->name);
#endif
        eina_stringshare_del(icon->theme);
    }

    EINA_LIST_FREE(icon->icons, data)
    {
        const char *path;

        if (icon->fallback)
        {
            if (icon->free)
                eina_stringshare_del(data);
        }
        else
        {
            Efreet_Cache_Icon_Element *elem;

            elem = data;
            if (icon->free)
            {
                EINA_LIST_FREE(elem->paths, path)
                    eina_stringshare_del(path);
            }
            else
                eina_list_free(elem->paths);
            free(elem);
        }
    }
    free(icon);
}

Efreet_Cache_Icon *
efreet_cache_icon_find(Efreet_Icon_Theme *theme, const char *icon)
{
    if (icon_cache)
    {
        if (!strcmp(icon_cache_name, theme->name.internal))
        {
            eet_close(icon_cache);
            eina_stringshare_del(icon_cache_name);
            icon_cache = NULL;
            icon_cache_name = NULL;
        }
    }
    if (!icon_cache)
    {
        const char *path;

        path = efreet_icon_cache_file(theme->name.internal);
        icon_cache = eet_open(path, EET_FILE_MODE_READ);
        if (icon_cache)
        {
            icon_cache_name = eina_stringshare_add(theme->name.internal);
            icon_cache_timer_update();
        }
    }
    if (icon_cache)
        return eet_data_read(icon_cache, cache_icon_edd, icon);
    return NULL;
}

Efreet_Cache_Icon *
efreet_cache_icon_fallback_find(const char *icon)
{
    if (!icon_fallback_cache)
    {
        const char *path;

        path = efreet_icon_cache_file("_fallback");
        icon_fallback_cache = eet_open(path, EET_FILE_MODE_READ);
        if (icon_fallback_cache)
            icon_cache_timer_update();
    }
    if (icon_fallback_cache)
        return eet_data_read(icon_fallback_cache, cache_icon_fallback_edd, icon);
    return NULL;
}

static Eina_Bool
icon_cache_exe_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
    Ecore_Exe_Event_Del *ev;

    ev = event;
    if (ev->exe != icon_cache_exe) return ECORE_CALLBACK_RENEW;
    if (icon_cache_exe_lock > 0)
    {
        close(icon_cache_exe_lock);
        icon_cache_exe_lock = -1;
    }
    return ECORE_CALLBACK_RENEW;
}

static void
icon_cache_update_cb(void *data __UNUSED__, Ecore_File_Monitor *em __UNUSED__,
                               Ecore_File_Event event, const char *path)
{
    const char *file, *ext;
    Efreet_Event_Cache_Update *ev = NULL;

    if (event != ECORE_FILE_EVENT_CREATED_FILE &&
        event != ECORE_FILE_EVENT_MODIFIED) return;
    file = ecore_file_file_get(path);
    if (!file || strncmp(file, "icon_", 5)) return;
    ext = strrchr(file, '.');
    if (!ext || strcmp(ext, ".cache")) return;

    icon_cache_close();

    ev = NEW(Efreet_Event_Cache_Update, 1);
    if (!ev) return;
    ecore_event_add(EFREET_EVENT_ICON_CACHE_UPDATE, ev, NULL, NULL);
}

static void
icon_cache_timer_update(void)
{
    if (icon_cache_timer)
        ecore_timer_interval_set(icon_cache_timer, 60.0);
    else
        icon_cache_timer = ecore_timer_add(60.0, icon_cache_timer_cb, NULL);
}

static Eina_Bool
icon_cache_timer_cb(void *data __UNUSED__)
{
    icon_cache_timer = NULL;

    icon_cache_close();
    return ECORE_CALLBACK_DONE;
}

static void
icon_cache_close(void)
{
    if (icon_cache) eet_close(icon_cache);
    icon_cache = NULL;
    if (icon_fallback_cache) eet_close(icon_fallback_cache);
    icon_fallback_cache = NULL;
    IF_RELEASE(icon_cache_name);
}
