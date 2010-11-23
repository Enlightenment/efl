#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <fcntl.h>

#include "Efreet.h"
#include "efreet_private.h"

/* TODO: Listen for changes for icon dirs */

typedef struct _Efreet_Old_Cache Efreet_Old_Cache;

struct _Efreet_Old_Cache
{
    Eina_Hash *hash;
    Eet_File *ef;
};


/**
 * Data for cache files
 */
#ifdef ICON_CACHE
static Eet_Data_Descriptor *cache_icon_edd = NULL;
static Eet_Data_Descriptor *cache_icon_element_edd = NULL;
static Eet_Data_Descriptor *cache_icon_fallback_edd = NULL;

static Eet_File            *icon_cache = NULL;
static const char          *icon_cache_name = NULL;
static Eet_File            *icon_fallback_cache = NULL;
#endif

static Eet_Data_Descriptor *desktop_edd = NULL;

static Eet_File            *desktop_cache = NULL;
static const char          *desktop_cache_dirs = NULL;
static const char          *desktop_cache_file = NULL;

static Ecore_File_Monitor  *cache_monitor = NULL;

#ifdef ICON_CACHE
static Ecore_Timer         *cache_timer = NULL;
#endif

static Ecore_Event_Handler *cache_exe_handler = NULL;
#ifdef ICON_CACHE
static Ecore_Job           *icon_cache_job = NULL;
static Ecore_Exe           *icon_cache_exe = NULL;
static int                  icon_cache_exe_lock = -1;
#endif
static Ecore_Job           *desktop_cache_job = NULL;
static Ecore_Exe           *desktop_cache_exe = NULL;
static int                  desktop_cache_exe_lock = -1;

static Eina_List           *old_desktop_caches = NULL;

#ifdef ICON_CACHE
static void efreet_icon_edd_shutdown(void);
#endif
static void efreet_desktop_edd_shutdown(void);

static Eina_Bool cache_exe_cb(void *data, int type, void *event);
static void cache_update_cb(void *data, Ecore_File_Monitor *em,
                               Ecore_File_Event event, const char *path);

#ifdef ICON_CACHE
static void cache_timer_update(void);
static Eina_Bool cache_timer_cb(void *data);
#endif

#ifdef ICON_CACHE
static void icon_cache_close(void);
#endif

static void desktop_cache_update_cache_job(void *data);
#ifdef ICON_CACHE
static void icon_cache_update_cache_job(void *data);
#endif
static void desktop_cache_update_free(void *data, void *ev);

#ifdef ICON_CACHE
EAPI int EFREET_EVENT_ICON_CACHE_UPDATE = 0;
#endif
EAPI int EFREET_EVENT_DESKTOP_CACHE_UPDATE = 0;

int
efreet_cache_init(void)
{
    char buf[PATH_MAX];

#ifdef ICON_CACHE
    if (!efreet_icon_edd_init())
        goto error;
    if (!efreet_icon_fallback_edd_init())
        goto error;
#endif
    if (!efreet_desktop_edd_init())
        goto error;

#ifdef ICON_CACHE
    EFREET_EVENT_ICON_CACHE_UPDATE = ecore_event_type_new();
#endif
    EFREET_EVENT_DESKTOP_CACHE_UPDATE = ecore_event_type_new();

    snprintf(buf, sizeof(buf), "%s/.efreet", efreet_home_dir_get());
    if (!ecore_file_mkpath(buf)) goto error;

    if (efreet_cache_update)
    {
        cache_exe_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                                             cache_exe_cb, NULL);
        if (!cache_exe_handler) goto error;

        cache_monitor = ecore_file_monitor_add(buf,
                                               cache_update_cb,
                                               NULL);
        if (!cache_monitor) goto error;

#if 0
        efreet_icon_changes_listen();
#endif

#ifdef ICON_CACHE
        efreet_cache_icon_update();
#endif
        efreet_cache_desktop_update();
    }

    return 1;
error:
    if (cache_exe_handler) ecore_event_handler_del(cache_exe_handler);
    cache_exe_handler = NULL;
    if (cache_monitor) ecore_file_monitor_del(cache_monitor);
    cache_monitor = NULL;
#ifdef ICON_CACHE
    efreet_icon_edd_shutdown();
#endif
    efreet_desktop_edd_shutdown();
    return 0;
}

void
efreet_cache_shutdown(void)
{
    Efreet_Old_Cache *d;

#ifdef ICON_CACHE
    if (cache_timer) ecore_timer_del(cache_timer);
    cache_timer = NULL;
    icon_cache_close();
#endif
    if (desktop_cache) eet_close(desktop_cache);
    desktop_cache = NULL;
    IF_RELEASE(desktop_cache_file);
    IF_RELEASE(desktop_cache_dirs);

    if (cache_exe_handler) ecore_event_handler_del(cache_exe_handler);
    cache_exe_handler = NULL;
    if (cache_monitor) ecore_file_monitor_del(cache_monitor);
    cache_monitor = NULL;
#ifdef ICON_CACHE
    efreet_icon_edd_shutdown();
#endif
    efreet_desktop_edd_shutdown();
    if (desktop_cache_job)
    {
        ecore_job_del(desktop_cache_job);
        desktop_cache_job = NULL;
    }
#ifdef ICON_CACHE
    if (icon_cache_exe) ecore_exe_terminate(icon_cache_exe);
    if (icon_cache_exe_lock > 0)
    {
        close(icon_cache_exe_lock);
        icon_cache_exe_lock = -1;
    }
#endif
    if (desktop_cache_exe) ecore_exe_terminate(desktop_cache_exe);
    if (desktop_cache_exe_lock > 0)
    {
        close(desktop_cache_exe_lock);
        desktop_cache_exe_lock = -1;
    }

    EINA_LIST_FREE(old_desktop_caches, d)
    {
        eina_hash_free(d->hash);
        free(d);
    }
}

#ifdef ICON_CACHE
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
#endif

/*
 * Needs EAPI because of helper binaries
 */
EAPI const char *
efreet_desktop_cache_file(void)
{
    char tmp[PATH_MAX] = { '\0' };
    const char *home, *lang, *country, *modifier;

    if (desktop_cache_file) return desktop_cache_file;

    home = efreet_home_dir_get();
    lang = efreet_lang_get();
    country = efreet_lang_country_get();
    modifier = efreet_lang_modifier_get();

    if (lang && country && modifier)
        snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop_%s_%s@%s.cache", home, lang, country, modifier);
    else if (lang && country)
        snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop_%s_%s.cache", home, lang, country);
    else if (lang)
        snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop_%s.cache", home, lang);
    else
        snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop.cache", home);

    desktop_cache_file = eina_stringshare_add(tmp);
    return desktop_cache_file;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI const char *
efreet_desktop_cache_dirs(void)
{
    char tmp[PATH_MAX] = { '\0' };

    if (desktop_cache_dirs) return desktop_cache_dirs;

    snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop_dirs.cache", efreet_home_dir_get());

    desktop_cache_dirs = eina_stringshare_add(tmp);
    return desktop_cache_dirs;
}

#ifdef ICON_CACHE
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
#endif

/*
 * Needs EAPI because of helper binaries
 */
EAPI Eet_Data_Descriptor *
efreet_desktop_edd_init(void)
{
    if (!desktop_edd)
    {
        Eet_Data_Descriptor_Class eddc;

        EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Efreet_Desktop);
        desktop_edd = eet_data_descriptor_file_new(&eddc);
        if (!desktop_edd) return NULL;
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "type", type, EET_T_INT);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "version", version, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "orig_path", orig_path, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "load_time", load_time, EET_T_LONG_LONG);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "name", name, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "generic_name", generic_name, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "comment", comment, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "icon", icon, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "try_exec", try_exec, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "exec", exec, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "path", path, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "startup_wm_class", startup_wm_class, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "url", url, EET_T_STRING);
        eet_data_descriptor_element_add(desktop_edd, "only_show_in", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, only_show_in), 0, NULL, NULL);
        eet_data_descriptor_element_add(desktop_edd, "not_show_in", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, not_show_in), 0, NULL, NULL);
        eet_data_descriptor_element_add(desktop_edd, "categories", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, categories), 0, NULL, NULL);
        eet_data_descriptor_element_add(desktop_edd, "mime_types", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, mime_types), 0, NULL, NULL);
        eet_data_descriptor_element_add(desktop_edd, "x", EET_T_STRING, EET_G_HASH, offsetof(Efreet_Desktop, x), 0, NULL, NULL);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "no_display", no_display, EET_T_UCHAR);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "hidden", hidden, EET_T_UCHAR);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "terminal", terminal, EET_T_UCHAR);
        EET_DATA_DESCRIPTOR_ADD_BASIC(desktop_edd, Efreet_Desktop, "startup_notify", startup_notify, EET_T_UCHAR);
    }
    return desktop_edd;
}

static void
efreet_desktop_edd_shutdown(void)
{
    if (desktop_edd) eet_data_descriptor_free(desktop_edd);
    desktop_edd = NULL;
}

#ifdef ICON_CACHE
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
            cache_timer_update();
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
            cache_timer_update();
    }
    if (icon_fallback_cache)
        return eet_data_read(icon_fallback_cache, cache_icon_fallback_edd, icon);
    return NULL;
}
#endif

Efreet_Desktop *
efreet_cache_desktop_find(const char *file)
{
    Efreet_Desktop *desktop;
    char rp[PATH_MAX];

    if (!realpath(file, rp)) return NULL;

    if (!desktop_cache)
        desktop_cache = eet_open(efreet_desktop_cache_file(), EET_FILE_MODE_READ);
    if (!desktop_cache)
        return NULL;

    desktop = eet_data_read(desktop_cache, desktop_edd, rp);
    if (!desktop) return NULL;
    desktop->ref = 1;
    desktop->eet = 1;
    return desktop;
}

void
efreet_cache_desktop_update(void)
{
    if (!efreet_cache_update) return;

    /* TODO: Make sure we don't create a lot of execs, maybe use a timer? */
    if (desktop_cache_job) ecore_job_del(desktop_cache_job);
    desktop_cache_job = ecore_job_add(desktop_cache_update_cache_job, NULL);
}

#ifdef ICON_CACHE
void
efreet_cache_icon_update(void)
{
    if (!efreet_cache_update) return;

    /* TODO: Make sure we don't create a lot of execs, maybe use a timer? */
    if (icon_cache_job) ecore_job_del(icon_cache_job);
    icon_cache_job = ecore_job_add(icon_cache_update_cache_job, NULL);
}
#endif

void
efreet_cache_desktop_free(Efreet_Desktop *desktop)
{
    Efreet_Old_Cache *d;
    Efreet_Desktop *curr;
    Eina_List *l;

    if (!old_desktop_caches) return;

    EINA_LIST_FOREACH(old_desktop_caches, l, d)
    {
        curr = eina_hash_find(d->hash, desktop->orig_path);
        if (curr && curr == desktop)
        {
            eina_hash_del_by_key(d->hash, desktop->orig_path);
            if (eina_hash_population(d->hash) == 0)
            {
                eina_hash_free(d->hash);
                d->hash = NULL;
            }
            break;
        }
    }
}

static Eina_Bool
cache_exe_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
    Ecore_Exe_Event_Del *ev;

    ev = event;
    if (ev->exe == desktop_cache_exe)
    {
        if (desktop_cache_exe_lock > 0)
        {
            close(desktop_cache_exe_lock);
            desktop_cache_exe_lock = -1;
        }
        desktop_cache_exe = NULL;
    }
#ifdef ICON_CACHE
    else if (ev->exe == icon_cache_exe)
    {
        if (icon_cache_exe_lock > 0)
        {
            close(icon_cache_exe_lock);
            icon_cache_exe_lock = -1;
        }
        icon_cache_exe = NULL;
    }
#endif
    return ECORE_CALLBACK_RENEW;
}

static void
cache_update_cb(void *data __UNUSED__, Ecore_File_Monitor *em __UNUSED__,
                               Ecore_File_Event event, const char *path)
{
    const char *file;
    Efreet_Event_Cache_Update *ev = NULL;
    Eet_File *tmp = NULL;
    Efreet_Old_Cache *d = NULL;


    if (event != ECORE_FILE_EVENT_CREATED_FILE &&
        event != ECORE_FILE_EVENT_MODIFIED) return;

    file = ecore_file_file_get(path);
    if (!file) return;
    if (!strcmp(file, "desktop_data.update"))
    {
        printf("update desktop cache\n");
        tmp = eet_open(efreet_desktop_cache_file(), EET_FILE_MODE_READ);
        if (!tmp) return;
        ev = NEW(Efreet_Event_Cache_Update, 1);
        if (!ev) goto error;
        d = NEW(Efreet_Old_Cache, 1);
        if (!d) goto error;

        d->hash = efreet_desktop_cache;
        d->ef = desktop_cache;
        old_desktop_caches = eina_list_append(old_desktop_caches, d);

        efreet_desktop_cache = eina_hash_string_superfast_new(NULL);
        desktop_cache = tmp;

        efreet_util_desktop_cache_reload();
        ecore_event_add(EFREET_EVENT_DESKTOP_CACHE_UPDATE, ev, desktop_cache_update_free, d);
        return;
    }
#ifdef ICON_CACHE
    else if (!strcmp(file, "icon_data.update"))
    {
        printf("update icon cache\n");
        icon_cache_close();

        ev = NEW(Efreet_Event_Cache_Update, 1);
        if (!ev) return;
        ecore_event_add(EFREET_EVENT_ICON_CACHE_UPDATE, ev, NULL, NULL);
    }
#endif
error:
    IF_FREE(ev);
    IF_FREE(d);
    if(tmp) eet_close(tmp);
}

#ifdef ICON_CACHE
static void
cache_timer_update(void)
{
    if (cache_timer)
        ecore_timer_interval_set(cache_timer, 60.0);
    else
        cache_timer = ecore_timer_add(60.0, cache_timer_cb, NULL);
}

static Eina_Bool
cache_timer_cb(void *data __UNUSED__)
{
    cache_timer = NULL;

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
#endif

static void
desktop_cache_update_cache_job(void *data __UNUSED__)
{
    char file[PATH_MAX];
    struct flock fl;

    desktop_cache_job = NULL;

    /* TODO: Retry update cache later */
    if (desktop_cache_exe_lock > 0) return;

    if (!efreet_desktop_write_cache_dirs_file()) return;

    snprintf(file, sizeof(file), "%s/.efreet/desktop_exec.lock", efreet_home_dir_get());

    desktop_cache_exe_lock = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (desktop_cache_exe_lock < 0) return;
    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(desktop_cache_exe_lock, F_SETLK, &fl) < 0) goto error;
    desktop_cache_exe = ecore_exe_run(PACKAGE_LIB_DIR "/efreet/efreet_desktop_cache_create", NULL);
    if (!desktop_cache_exe) goto error;

    return;

error:
    if (desktop_cache_exe_lock > 0)
    {
        close(desktop_cache_exe_lock);
        desktop_cache_exe_lock = -1;
    }
}

#ifdef ICON_CACHE
static void
icon_cache_update_cache_job(void *data __UNUSED__)
{
    char file[PATH_MAX];
    struct flock fl;

    icon_cache_job = NULL;

    /* TODO: Retry update cache later */
    if (icon_cache_exe_lock > 0) return;

    snprintf(file, sizeof(file), "%s/.efreet/icon_exec.lock", efreet_home_dir_get());

    icon_cache_exe_lock = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (icon_cache_exe_lock < 0) return;
    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(icon_cache_exe_lock, F_SETLK, &fl) < 0) goto error;
    icon_cache_exe = ecore_exe_run(PACKAGE_LIB_DIR "/efreet/efreet_icon_cache_create", NULL);
    if (!icon_cache_exe) goto error;

    return;

error:
    if (icon_cache_exe_lock > 0)
    {
        close(icon_cache_exe_lock);
        icon_cache_exe_lock = -1;
    }
}
#endif

static void
desktop_cache_update_free(void *data, void *ev)
{
   Efreet_Old_Cache *d;
   int dangling = 0;

   d = data;
   /*
    * All users should now had the chance to update their pointers, so we can now
    * free the old cache
    */
   if (d->hash)
     {
        Eina_Iterator *it;
        Eina_Hash_Tuple *tuple;

        it = eina_hash_iterator_tuple_new(d->hash);
        EINA_ITERATOR_FOREACH(it, tuple)
          {
             printf("Efreet: %d:%s still in cache on cache close!\n",
                    ((Efreet_Desktop *)tuple->data)->ref, (char *)tuple->key);
             dangling++;
          }
        eina_iterator_free(it);

        eina_hash_free(d->hash);
     }
   /*
    * If there are dangling references the eet file won't be closed - to
    * avoid crashes, but this will leak instead.
    */
   if (dangling == 0)
     {
        if (d->ef) eet_close(d->ef);
     }
   else
     {
        printf("Efreet: ERROR. There are still %i desktop files with old\n"
               "dangling references to desktop files. This application\n"
               "has not handled the EFREET_EVENT_DESKTOP_CACHE_UPDATE\n"
               "fully and released its references. Please fix the application\n"
               "so it does this.\n",
               dangling);
     }
   old_desktop_caches = eina_list_remove(old_desktop_caches, d);
   free(d);
   free(ev);
}

