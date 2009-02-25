/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

typedef struct Efreet_Cache_Fill        Efreet_Cache_Fill;
typedef struct Efreet_Cache_Fill_Dir    Efreet_Cache_Fill_Dir;
typedef struct Efreet_Cache_Search      Efreet_Cache_Search;
typedef struct Efreet_Cache_Search_List Efreet_Cache_Search_List;
typedef struct Efreet_Monitor           Efreet_Monitor;
typedef struct Efreet_Util_Desktop      Efreet_Util_Desktop;

struct Efreet_Cache_Fill
{
    Eina_List *dirs;
    Efreet_Cache_Fill_Dir *current;
    DIR  *files;
};

struct Efreet_Cache_Fill_Dir
{
    char *path;
    char *file_id;
    int   priority;
};

struct Efreet_Cache_Search
{
    Efreet_Util_Desktop *result;
    const char *what1;
    const char *what2;
};

struct Efreet_Cache_Search_List
{
    Eina_List *list;
    const char *what;
};

struct Efreet_Monitor
{
    char *file_id;
    Ecore_File_Monitor *monitor;
    int priority;
};

struct Efreet_Util_Desktop
{
    Efreet_Desktop *desktop;
    int priority;
};

static char *efreet_util_path_in_default(const char *section, const char *path);

static int  efreet_util_cache_fill(void *data);
static void efreet_util_cache_add(const char *path, const char *file_id, int priority, int event);
static void efreet_util_cache_remove(const char *path, const char *file_id, int priority);
static void efreet_util_cache_reload(const char *path, const char *file_id, int priority);
static void efreet_util_cache_dir_free(void *data);

static Eina_Bool efreet_util_cache_search_mime(const Eina_Hash *hash, void *value, void *fdata);
static Eina_Bool efreet_util_cache_search_wm_class(const Eina_Hash *hash, void *value, void *fdata);
static Eina_Bool efreet_util_cache_search_exec(const Eina_Hash *hash, void *value, void *fdata);
static Eina_Bool efreet_util_cache_search_name(const Eina_Hash *hash, void *value, void *fdata);
static Eina_Bool efreet_util_cache_search_generic_name(const Eina_Hash *hash, void *value, void *fdata);

static Eina_Bool efreet_util_cache_search_name_glob(const Eina_Hash *hash, void *value, void *fdata);
static Eina_Bool efreet_util_cache_search_exec_glob(const Eina_Hash *hash, void *value, void *fdata);
static Eina_Bool efreet_util_cache_search_generic_name_glob(const Eina_Hash *hash, void *value, void *fdata);
static Eina_Bool efreet_util_cache_search_comment_glob(const Eina_Hash *hash, void *value, void *fdata);

static int  efreet_util_glob_match(const char *str, const char *glob);

static void efreet_util_monitor(const char *path, const char *file_id, int priority);
static void efreet_util_monitor_cb(void *data, Ecore_File_Monitor *monitor,
                                    Ecore_File_Event event, const char *path);
static void efreet_util_monitor_free(void *data);
static void efreet_util_menus_find_helper(Eina_List *menus, const char *config_dir);

static void efreet_util_desktops_by_category_add(Efreet_Desktop *desktop);
static void efreet_util_desktops_by_category_remove(Efreet_Desktop *desktop);

static void efreet_util_desktop_free(Efreet_Util_Desktop *ud);
static void efreet_event_desktop_change_free(void *data, void *ev);

static Eina_Hash *desktop_by_file_id = NULL;
static Eina_Hash *file_id_by_desktop_path = NULL;
static Eina_Hash *desktops_by_category = NULL;

static Ecore_Idler       *idler = NULL;
static Efreet_Cache_Fill *fill = NULL;

static Eina_List *monitors = NULL;

static int init = 0;

EAPI int EFREET_EVENT_DESKTOP_LIST_CHANGE = 0;
EAPI int EFREET_EVENT_DESKTOP_CHANGE = 0;

EAPI int
efreet_util_init(void)
{
    Eina_List *dirs;

    if (init++) return init;

    if (!EFREET_EVENT_DESKTOP_LIST_CHANGE)
        EFREET_EVENT_DESKTOP_LIST_CHANGE = ecore_event_type_new();
    if (!EFREET_EVENT_DESKTOP_CHANGE)
        EFREET_EVENT_DESKTOP_CHANGE = ecore_event_type_new();
    desktop_by_file_id = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_util_desktop_free));
    file_id_by_desktop_path = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));
    desktops_by_category = eina_hash_string_superfast_new(EINA_FREE_CB(eina_list_free));

    monitors = NULL;

    fill = NEW(Efreet_Cache_Fill, 1);
    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    if (dirs)
    {
        Efreet_Cache_Fill_Dir *dir;
        char *path;
        int priority = 0;

        while (dirs)
        {
            path = eina_list_data_get(dirs);
            dir = NEW(Efreet_Cache_Fill_Dir, 1);
            dir->path = path;
            dir->priority = priority++;
            fill->dirs = eina_list_append(fill->dirs, dir);
            dirs = eina_list_remove_list(dirs, dirs);
        }
    }
    idler = ecore_idler_add(efreet_util_cache_fill, NULL);
    return init;
}

EAPI int
efreet_util_shutdown(void)
{
    Efreet_Monitor *em;
    Efreet_Cache_Fill_Dir *dir;

    if (--init) return init;

    if (idler)
    {
        ecore_idler_del(idler);
        while (fill->dirs)
        {
            dir = eina_list_data_get(fill->dirs);
            efreet_util_cache_dir_free(dir);
            fill->dirs = eina_list_remove_list(fill->dirs, fill->dirs);
        }

        if (fill->current) efreet_util_cache_dir_free(fill->current);
        if (fill->files) closedir(fill->files);
        free(fill);
    }
    idler = NULL;

    IF_FREE_HASH(desktop_by_file_id);
    IF_FREE_HASH(file_id_by_desktop_path);

    while (monitors)
    {
        em = eina_list_data_get(monitors);
        efreet_util_monitor_free(em);
        monitors = eina_list_remove_list(monitors, monitors);
    }

    IF_FREE_HASH(desktops_by_category);

    return init;
}

static char *
efreet_util_path_in_default(const char *section, const char *path)
{
    Eina_List *dirs;
    char *ret = NULL;
    char *dir;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    section);

    EINA_LIST_FREE(dirs, dir)
    {
        size_t len;

        len = strlen(dir);
        if (!strncmp(path, dir, strlen(dir)))
        {
            ret = dir;
            break;
        }
	free(dir);
    }

    EINA_LIST_FREE(dirs, dir)
      if (ret != dir) free(dir);

    return ret;
}

EAPI const char *
efreet_util_path_to_file_id(const char *path)
{
    size_t len;
    char *tmp, *p;
    char *base;
    const char *file_id = NULL;

    if (!path) return NULL;
    file_id = eina_hash_find(file_id_by_desktop_path, path);
    if (file_id) return file_id;

    base = efreet_util_path_in_default("applications", path);
    if (!base) return NULL;

    len = strlen(base);
    if (strlen(path) <= len)
    {
        free(base);
        return NULL;
    }
    if (strncmp(path, base, len))
    {
        free(base);
        return NULL;
    }

    tmp = strdup(path + len + 1);
    p = tmp;
    while (*p)
    {
        if (*p == '/') *p = '-';
        p++;
    }
    free(base);
    file_id = eina_stringshare_add(tmp);
    free(tmp);
    eina_hash_del(file_id_by_desktop_path, path, NULL);
    eina_hash_add(file_id_by_desktop_path, path, (void *)file_id);
    return file_id;
}

EAPI Eina_List *
efreet_util_desktop_mime_list(const char *mime)
{
    Efreet_Cache_Search_List search;
    Eina_Iterator *it;

    search.list = NULL;
    search.what = eina_stringshare_add(mime);

    it = eina_hash_iterator_data_new(desktop_by_file_id);
    eina_iterator_foreach(it, EINA_EACH(efreet_util_cache_search_mime), &search);
    eina_iterator_free(it);

    eina_stringshare_del(search.what);

    return search.list;
}

EAPI Efreet_Desktop *
efreet_util_desktop_wm_class_find(const char *wmname, const char *wmclass)
{
    Efreet_Cache_Search search;
    Efreet_Util_Desktop *ud;
    Eina_Iterator *it;

    if ((!wmname) && (!wmclass)) return NULL;
    search.what1 = wmname;
    search.what2 = wmclass;
    search.result = NULL;

    it = eina_hash_iterator_data_new(desktop_by_file_id);
    eina_iterator_foreach(it, EINA_EACH(efreet_util_cache_search_wm_class), &search);
    eina_iterator_free(it);
    ud = search.result;

    if (ud)
      {
	 efreet_desktop_ref(ud->desktop);
	 return ud->desktop;
      }
    return NULL;
}

EAPI Efreet_Desktop *
efreet_util_desktop_file_id_find(const char *file_id)
{
    Efreet_Desktop *desktop = NULL;
    Efreet_Util_Desktop *ud = NULL;
    Eina_List *dirs, *l;
    const char *dir;
    int priority = 0;

    if (!file_id) return NULL;
    ud = eina_hash_find(desktop_by_file_id, file_id);
    if (ud) return ud->desktop;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    if (!dirs) return NULL;

    EINA_LIST_FOREACH(dirs, l, dir)
    {
        char *tmp, *p;
        char buf[PATH_MAX];

        tmp = strdup(file_id);
        p = tmp;

        while (p)
        {
            snprintf(buf, sizeof(buf), "%s/%s", dir, tmp);
            desktop = efreet_desktop_get(buf);
            if (desktop) break;
            p = strchr(p, '-');
            if (p) *p = '/';
        }
        free(tmp);
        if (desktop) break;
        priority++;
    }
    while (dirs)
    {
        free(eina_list_data_get(dirs));
        dirs = eina_list_remove_list(dirs, dirs);
    }
    if (desktop)
    {
        Efreet_Event_Desktop_Change *ev;

        ud = NEW(Efreet_Util_Desktop, 1);
        ud->priority = priority;
        ud->desktop = desktop;

        eina_hash_del(desktop_by_file_id, file_id, NULL);
        eina_hash_add(desktop_by_file_id, file_id, ud);
        efreet_util_desktops_by_category_add(desktop);

        ev = NEW(Efreet_Event_Desktop_Change, 1);
        ev->current = desktop;
        efreet_desktop_ref(ev->current);
        ev->change = EFREET_DESKTOP_CHANGE_ADD;
        ecore_event_add(EFREET_EVENT_DESKTOP_CHANGE, ev, efreet_event_desktop_change_free, NULL);
    }
    return desktop;
}

EAPI Efreet_Desktop *
efreet_util_desktop_exec_find(const char *exec)
{
    Efreet_Cache_Search search;
    Eina_Iterator *it;

    if (!exec) return NULL;
    search.what1 = exec;
    search.what2 = NULL;
    search.result = NULL;

    it = eina_hash_iterator_data_new(desktop_by_file_id);
    eina_iterator_foreach(it, EINA_EACH(efreet_util_cache_search_exec), &search);
    eina_iterator_free(it);

    if (!search.result) return NULL;

    efreet_desktop_ref(search.result->desktop);
    return search.result->desktop;
}

EAPI Efreet_Desktop *
efreet_util_desktop_name_find(const char *name)
{
    Efreet_Cache_Search search;
    Eina_Iterator *it;

    if (!name) return NULL;
    search.what1 = name;
    search.what2 = NULL;
    search.result = NULL;

    it = eina_hash_iterator_data_new(desktop_by_file_id);
    eina_iterator_foreach(it, EINA_EACH(efreet_util_cache_search_name), &search);
    eina_iterator_free(it);

    if (!search.result) return NULL;

    efreet_desktop_ref(search.result->desktop);
    return search.result->desktop;
}

EAPI Efreet_Desktop *
efreet_util_desktop_generic_name_find(const char *generic_name)
{
    Efreet_Cache_Search search;
    Eina_Iterator *it;

    if (!generic_name) return NULL;
    search.what1 = generic_name;
    search.what2 = NULL;
    search.result = NULL;

    it = eina_hash_iterator_data_new(desktop_by_file_id);
    eina_iterator_foreach(it, EINA_EACH(efreet_util_cache_search_generic_name), &search);
    eina_iterator_free(it);

    if (!search.result) return NULL;

    efreet_desktop_ref(search.result->desktop);
    return search.result->desktop;
}

EAPI Eina_List *
efreet_util_desktop_name_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;
    Eina_Iterator *it;

    search.list = NULL;
    search.what = glob;

    it = eina_hash_iterator_data_new(desktop_by_file_id);
    eina_iterator_foreach(it, EINA_EACH(efreet_util_cache_search_name_glob), &search);
    eina_iterator_free(it);

    return search.list;
}

EAPI Eina_List *
efreet_util_desktop_exec_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;
    Eina_Iterator *it;

    search.list = NULL;
    search.what = glob;

    it = eina_hash_iterator_data_new(desktop_by_file_id);
    eina_iterator_foreach(it, EINA_EACH(efreet_util_cache_search_exec_glob), &search);
    eina_iterator_free(it);

    return search.list;
}

EAPI Eina_List *
efreet_util_desktop_generic_name_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;
    Eina_Iterator *it;

    search.list = NULL;
    search.what = glob;

    it = eina_hash_iterator_data_new(desktop_by_file_id);
    eina_iterator_foreach(it, EINA_EACH(efreet_util_cache_search_generic_name_glob), &search);
    eina_iterator_free(it);

    return search.list;
}

EAPI Eina_List *
efreet_util_desktop_comment_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;
    Eina_Iterator *it;

    search.list = NULL;
    search.what = glob;

    it = eina_hash_iterator_data_new(desktop_by_file_id);
    eina_iterator_foreach(it, EINA_EACH(efreet_util_cache_search_comment_glob), &search);
    eina_iterator_free(it);

    return search.list;
}

static Eina_Bool
_hash_keys(Eina_Hash *hash, const void *key, void *fdata)
{
    Eina_List **l = fdata;

    *l = eina_list_append(*l, key);
    return EINA_TRUE;
}

/**
 * Find all desktop categories
 * This list must be freed using ecore_list_destroy()
 *
 * @return an Eina_List of category names (const char *)
 */
EAPI Eina_List *
efreet_util_desktop_categories_list(void)
{
    Eina_Iterator *it;
    Eina_List *list = NULL;

	it = eina_hash_iterator_key_new(desktops_by_category);
    if (it)
    {
        eina_iterator_foreach(it, EINA_EACH(_hash_keys), &list);
	eina_iterator_free(it);
      }

    return list;
}

/**
 * Find all desktops in a given category
 *
 * Note: this returns a pointer to an internal list. Do not add to, remove from, or delete this list.
 *
 * @param category the category name
 * @return a list of desktops
 */
EAPI Eina_List *
efreet_util_desktop_category_list(const char *category)
{
    return eina_hash_find(desktops_by_category, category);
}

#if 0
static Eina_Bool
dump(Eina_Hash *hash, const char *key, void *value, __UNUSED__ void *data)
{
    printf("%s -> %p\n", (char *)key, value);
}
#endif

static int
efreet_util_cache_fill(__UNUSED__ void *data)
{
    Efreet_Cache_Fill_Dir *dir;
    struct dirent *file = NULL;
    double start;
    char buf[PATH_MAX];

    if (!fill->dirs)
    {
        free(fill);
        idler = NULL;
        fill = NULL;
        efreet_cache_clear();
        ecore_event_add(EFREET_EVENT_DESKTOP_LIST_CHANGE, NULL, NULL, NULL);

        return 0;
    }
    if (!fill->current)
    {
        fill->current = eina_list_data_get(fill->dirs);
        fill->dirs = eina_list_remove_list(fill->dirs, fill->dirs);
        if (!fill->current)
        {
            while (fill->dirs)
            {
                dir = eina_list_data_get(fill->dirs);
                efreet_util_cache_dir_free(dir);
                fill->dirs = eina_list_remove_list(fill->dirs, fill->dirs);
            }

            free(fill);
            idler = NULL;
            fill = NULL;
#if 0
            eina_hash_foreach(desktop_by_file_id, dump, NULL);
            eina_hash_foreach(file_id_by_desktop_path, dump, NULL);
            printf("%d\n", eina_hash_population(desktop_by_file_id));
#endif
            efreet_cache_clear();
            ecore_event_add(EFREET_EVENT_DESKTOP_LIST_CHANGE, NULL, NULL, NULL);

            return 0;
        }
    }

    start = ecore_time_get();
    if (!fill->files) fill->files = opendir(fill->current->path);
    if (!fill->files)
    {
        /* Couldn't open this dir, continue to next */
        efreet_util_cache_dir_free(fill->current);
        fill->current = NULL;
    }
    else
    {
        do
        {
            char file_id[PATH_MAX];

            file = readdir(fill->files);
            if (!file) break;
            if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")) continue;

            snprintf(buf, PATH_MAX, "%s/%s", fill->current->path, file->d_name);
            if (fill->current->file_id)
                snprintf(file_id, PATH_MAX, "%s-%s", fill->current->file_id, file->d_name);
            else
                strcpy(file_id, file->d_name);

            if (ecore_file_is_dir(buf))
            {
                Efreet_Cache_Fill_Dir *dir;

                dir = NEW(Efreet_Cache_Fill_Dir, 1);
                dir->path = strdup(buf);
                dir->file_id = strdup(file_id);
                dir->priority = fill->current->priority;
                fill->dirs = eina_list_append(fill->dirs, dir);
            }
            else
                efreet_util_cache_add(buf, file_id, fill->current->priority, 0);
        } while ((ecore_time_get() - start) < 0.01);

        if (!file)
        {
            /* This dir has been search through */
            efreet_util_monitor(fill->current->path, fill->current->file_id,
                                fill->current->priority);
            efreet_util_cache_dir_free(fill->current);
            fill->current = NULL;
            closedir(fill->files);
            fill->files = NULL;
        }
    }

    return 1;
}

static void
efreet_util_cache_add(const char *path, const char *file_id, int priority, int event)
{
    Efreet_Desktop *desktop;
    Efreet_Util_Desktop *ud;
    char *ext;

    ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".desktop")) return;
    desktop = efreet_desktop_get(path);

    if (!desktop || desktop->type != EFREET_DESKTOP_TYPE_APPLICATION)
    {
        if (desktop) efreet_desktop_free(desktop);
        return;
    }

    if (!eina_hash_find(file_id_by_desktop_path, desktop->orig_path))
      {
	 eina_hash_add(file_id_by_desktop_path,
	       desktop->orig_path,
	       (void *)eina_stringshare_add(file_id));

      }

    ud = eina_hash_find(desktop_by_file_id, file_id);
    if (!ud)
    {
        ud = NEW(Efreet_Util_Desktop, 1);
        ud->priority = priority;
        ud->desktop = desktop;

        eina_hash_add(desktop_by_file_id, file_id, ud);
        efreet_util_desktops_by_category_add(desktop);

        if (event)
        {
            Efreet_Event_Desktop_Change *ev;

            ev = NEW(Efreet_Event_Desktop_Change, 1);
            ev->current = desktop;
            efreet_desktop_ref(ev->current);
            ev->change = EFREET_DESKTOP_CHANGE_ADD;
            efreet_cache_clear();
            ecore_event_add(EFREET_EVENT_DESKTOP_CHANGE, ev, efreet_event_desktop_change_free, NULL);
        }
    }
    else if (priority < ud->priority)
    {
        efreet_util_desktops_by_category_remove(ud->desktop);
        efreet_util_desktops_by_category_add(desktop);

        if (event)
        {
            Efreet_Event_Desktop_Change *ev;

            ev = NEW(Efreet_Event_Desktop_Change, 1);
            ev->current = desktop;
            efreet_desktop_ref(ev->current);
            ev->previous = ud->desktop;
            efreet_desktop_ref(ev->previous);
            ev->change = EFREET_DESKTOP_CHANGE_UPDATE;
            efreet_desktop_free(ud->desktop);
            ud->desktop = desktop;
            efreet_cache_clear();
            ecore_event_add(EFREET_EVENT_DESKTOP_CHANGE, ev, efreet_event_desktop_change_free, NULL);
        }
        else
        {
            efreet_desktop_free(ud->desktop);
            ud->desktop = desktop;
        }
    }
    else
        efreet_desktop_free(desktop);
}

static void
efreet_util_cache_remove(const char *path, const char *file_id, int priority)
{
    Efreet_Util_Desktop *ud;
    char *ext;

    ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".desktop")) return;
    ud = eina_hash_find(desktop_by_file_id, file_id);
    if (ud && (ud->priority >= priority))
    {
        Efreet_Event_Desktop_Change *ev;

        efreet_util_desktops_by_category_remove(ud->desktop);

        ev = NEW(Efreet_Event_Desktop_Change, 1);
        ev->current = ud->desktop;
        efreet_desktop_ref(ev->current);
        ev->change = EFREET_DESKTOP_CHANGE_REMOVE;
        efreet_cache_clear();
        ecore_event_add(EFREET_EVENT_DESKTOP_CHANGE, ev, efreet_event_desktop_change_free, NULL);

        eina_hash_del(desktop_by_file_id, file_id, ud);

        /* This call will search application dirs and add the file to cache if it
         * exists. */
        efreet_util_desktop_file_id_find(file_id);
    }
    eina_hash_del(file_id_by_desktop_path, path, eina_hash_find(file_id_by_desktop_path, path));
}

static void
efreet_util_cache_reload(const char *path, const char *file_id, int priority)
{
    Efreet_Desktop *desktop;
    Efreet_Util_Desktop *ud;
    char *ext;

    ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".desktop")) return;
    desktop = efreet_desktop_get(path);

    if (!desktop || desktop->type != EFREET_DESKTOP_TYPE_APPLICATION)
    {
        if (desktop) efreet_desktop_free(desktop);
        return;
    }

    ud = eina_hash_find(desktop_by_file_id, file_id);
    if (ud)
    {
        Efreet_Event_Desktop_Change *ev;

        if (ud->priority < priority)
        {
            efreet_desktop_free(desktop);
            return;
        }

        efreet_util_desktops_by_category_remove(ud->desktop);
        efreet_util_desktops_by_category_add(desktop);

        ev = NEW(Efreet_Event_Desktop_Change, 1);
        ev->current = desktop;
        efreet_desktop_ref(ev->current);
        ev->previous = ud->desktop;
        efreet_desktop_ref(ev->previous);
        ev->change = EFREET_DESKTOP_CHANGE_UPDATE;
        efreet_cache_clear();
        ecore_event_add(EFREET_EVENT_DESKTOP_CHANGE, ev, efreet_event_desktop_change_free, NULL);

        efreet_desktop_free(ud->desktop);
        ud->desktop = desktop;
    }
    else
    {
        Efreet_Event_Desktop_Change *ev;

        ud = NEW(Efreet_Util_Desktop, 1);
        ud->priority = priority;
        ud->desktop = desktop;

        eina_hash_add(desktop_by_file_id, file_id, ud);
        efreet_util_desktops_by_category_remove(ud->desktop);
        efreet_util_desktops_by_category_add(desktop);

        ev = NEW(Efreet_Event_Desktop_Change, 1);
        ev->current = desktop;
        efreet_desktop_ref(ev->current);
        ev->change = EFREET_DESKTOP_CHANGE_ADD;
        efreet_cache_clear();
        ecore_event_add(EFREET_EVENT_DESKTOP_CHANGE, ev, efreet_event_desktop_change_free, NULL);
    }
}

static void
efreet_util_cache_dir_free(void *data)
{
    Efreet_Cache_Fill_Dir *dir;

    dir = data;
    IF_FREE(dir->path);
    IF_FREE(dir->file_id);
    free(dir);
}

static Eina_Bool
efreet_util_cache_search_mime(__UNUSED__ const Eina_Hash *hash, void *value, void *fdata)
{
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;
    Eina_List                *l;
    const char               *mime;

    search = fdata;
    ud = value;

    if (!ud->desktop->mime_types) return EINA_FALSE;
    EINA_LIST_FOREACH(ud->desktop->mime_types, l, mime)
        if (search->what == mime)
        {
	    efreet_desktop_ref(ud->desktop);
            search->list = eina_list_append(search->list, ud->desktop);
            break;
        }
    return EINA_TRUE;
}

static Eina_Bool
efreet_util_cache_search_wm_class(__UNUSED__ const Eina_Hash *hash, void *value, void *fdata)
{
    Efreet_Cache_Search *search;
    Efreet_Util_Desktop *ud;

    ud = value;
    search = fdata;

    if (!ud->desktop) return EINA_TRUE;
    if (!ud->desktop->startup_wm_class) return EINA_TRUE;
    if ((search->what2) && (!strcmp(ud->desktop->startup_wm_class, search->what2)))
      {
	  search->result = ud;
          return EINA_FALSE;
      }
    else if ((search->what1) && (!strcmp(ud->desktop->startup_wm_class, search->what1)))
      {
	  search->result = ud;
	  return EINA_FALSE;
      }
    return EINA_TRUE;
}

static Eina_Bool
efreet_util_cache_search_exec(__UNUSED__ const Eina_Hash *hash, void *value, void *fdata)
{
    Efreet_Cache_Search *search;
    Efreet_Util_Desktop *ud;
    char                      *exec;
    const char                *file;

    ud = value;
    search = fdata;

    if (!ud->desktop->exec) return EINA_TRUE;
    exec = ecore_file_app_exe_get(ud->desktop->exec);
    if (!exec) return EINA_TRUE;
    if (!strcmp(exec, search->what1))
    {
       free(exec);
       search->result = ud;
       return EINA_FALSE;
    }

    file = ecore_file_file_get(exec);
    if (file && !strcmp(file, search->what1))
    {
       free(exec);
       search->result = ud;
       return EINA_FALSE;
    }
    free(exec);
    return EINA_TRUE;
}

static Eina_Bool
efreet_util_cache_search_name(__UNUSED__ const Eina_Hash *hash, void *value, void *fdata)
{
    Efreet_Cache_Search *search;
    Efreet_Util_Desktop *ud;

    ud = value;
    search = fdata;

    if (!ud->desktop->name) return EINA_TRUE;
    if (!strcmp(ud->desktop->name, search->what1))
      {
	 search->result = ud;
	 return EINA_FALSE;
      }
    return EINA_TRUE;
}

static Eina_Bool
efreet_util_cache_search_generic_name(__UNUSED__ const Eina_Hash *hash, void *value, void *fdata)
{
    Efreet_Cache_Search *search;
    Efreet_Util_Desktop *ud;

    ud = value;
    search = fdata;

    if (!ud->desktop->generic_name) return EINA_TRUE;
    if (!strcmp(ud->desktop->generic_name, search->what1))
      {
	 search->result = ud;
	 return EINA_FALSE;
      }
    return EINA_TRUE;
}

static Eina_Bool
efreet_util_cache_search_name_glob(__UNUSED__ const Eina_Hash *hash, void *value, void *fdata)
{
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;

    search = fdata;
    ud = value;

    if (!ud->desktop) return EINA_TRUE;
    if (efreet_util_glob_match(ud->desktop->name, search->what))
      {
	 efreet_desktop_ref(ud->desktop);
	 search->list = eina_list_append(search->list, ud->desktop);
      }
    return EINA_TRUE;
}

static Eina_Bool
efreet_util_cache_search_exec_glob(__UNUSED__ const Eina_Hash *hash, void *value, void *fdata)
{
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;
    char                     *exec;

    search = fdata;
    ud = value;

    if (!ud->desktop->exec) return EINA_FALSE;
    exec = ecore_file_app_exe_get(ud->desktop->exec);
    if (!exec) return EINA_TRUE;

        if (efreet_util_glob_match(exec, search->what))
      {
	 efreet_desktop_ref(ud->desktop);
	 search->list = eina_list_append(search->list, ud->desktop);
    }
    free(exec);
    return EINA_TRUE;
}

static Eina_Bool
efreet_util_cache_search_generic_name_glob(__UNUSED__ const Eina_Hash *hash, void *value, void *fdata)
{
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;

    search = fdata;
    ud = value;

    if (efreet_util_glob_match(ud->desktop->generic_name, search->what))
      {
	 efreet_desktop_ref(ud->desktop);
	 search->list = eina_list_append(search->list, ud->desktop);
      }
    return EINA_TRUE;
}

static Eina_Bool
efreet_util_cache_search_comment_glob(__UNUSED__ const Eina_Hash *hash, void *value, void *fdata)
{
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;

    search = fdata;
    ud = value;

    if (efreet_util_glob_match(ud->desktop->comment, search->what))
      {
	 efreet_desktop_ref(ud->desktop);
	 search->list = eina_list_append(search->list, ud->desktop);
      }
    return EINA_TRUE;
}

static int
efreet_util_glob_match(const char *str, const char *glob)
{
    if (!str || !glob)
        return 0;
    if (glob[0] == '\0')
    {
        if (str[0] == '\0') return 1;
        return 0;
    }
    if (!strcmp(glob, "*")) return 1;
    if (!fnmatch(glob, str, 0)) return 1;
    return 0;
}

static void
efreet_util_monitor(const char *path, const char *file_id, int priority)
{
    Efreet_Monitor *em;

    em = NEW(Efreet_Monitor, 1);
    em->monitor = ecore_file_monitor_add(path, efreet_util_monitor_cb, em);
    if (file_id) em->file_id = strdup(file_id);
    em->priority = priority;
    monitors = eina_list_append(monitors, em);
}

static void
efreet_util_monitor_cb(void *data, Ecore_File_Monitor *monitor __UNUSED__,
                        Ecore_File_Event event, const char *path)
{
    Efreet_Monitor *em;
    char file_id[PATH_MAX];

    em = data;
    if (em->file_id)
        snprintf(file_id, sizeof(file_id), "%s-%s", em->file_id, ecore_file_file_get(path));
    else
        strcpy(file_id, ecore_file_file_get(path));
    switch (event)
    {
        case ECORE_FILE_EVENT_NONE:
            /* Ignore */
            break;
        case ECORE_FILE_EVENT_CREATED_FILE:
            efreet_util_cache_add(path, file_id, em->priority, 1);
            break;
        case ECORE_FILE_EVENT_CREATED_DIRECTORY:
            {
                Efreet_Cache_Fill_Dir *dir;

                if (!fill)
                {
                    fill = NEW(Efreet_Cache_Fill, 1);
                }

                dir = NEW(Efreet_Cache_Fill_Dir, 1);
                dir->path = strdup(path);
                dir->file_id = strdup(file_id);
                dir->priority = em->priority;
                fill->dirs = eina_list_append(fill->dirs, dir);

                if (!idler)
                    idler = ecore_idler_add(efreet_util_cache_fill, NULL);
            }
            break;
        case ECORE_FILE_EVENT_DELETED_FILE:
            efreet_util_cache_remove(path, file_id, em->priority);
            break;
        case ECORE_FILE_EVENT_DELETED_DIRECTORY:
            /* Ignore, we should already have a monitor on any subdir */
            break;
        case ECORE_FILE_EVENT_DELETED_SELF:
            if (eina_list_data_find(monitors, em))
                eina_list_remove(monitors, em);
            efreet_util_monitor_free(em);
            break;
        case ECORE_FILE_EVENT_MODIFIED:
            efreet_util_cache_reload(path, file_id, em->priority);
            break;
    }
}

static void
efreet_util_monitor_free(void *data)
{
    Efreet_Monitor *em;

    em = data;
    if (em->monitor) ecore_file_monitor_del(em->monitor);
    IF_FREE(em->file_id);
    free(em);
}

/**
 * Returns a list of .menu files found in the various config dirs.
 * @return An ecore list of menu file paths (const char *). This must be freed with ecore_list_destroy().
 */
EAPI Eina_List *
efreet_util_menus_find(void)
{
    Eina_List *menus = NULL;
    Eina_List *dirs, *l;
    const char *dir;

    efreet_util_menus_find_helper(menus, efreet_config_home_get());

    dirs = efreet_config_dirs_get();
    EINA_LIST_FOREACH(dirs, l, dir)
        efreet_util_menus_find_helper(menus, dir);

    return menus;
}

static void
efreet_util_menus_find_helper(Eina_List *menus, const char *config_dir)
{
    DIR *files = NULL;
    struct dirent *file = NULL;
    char dbuf[PATH_MAX], fbuf[PATH_MAX];

    snprintf(dbuf, sizeof(dbuf), "%s/menus", config_dir);
    files = opendir(dbuf);
    if (!files) return;
    while ((file = readdir(files))) {
        const char *exten;
        exten = strrchr(file->d_name, '.');
        if (!exten) continue;
        if (strcmp(".menu", exten)) continue;

        snprintf(fbuf, PATH_MAX, "%s/%s", dbuf, file->d_name);
        if (ecore_file_is_dir(fbuf)) continue;

        menus = eina_list_append(menus, strdup(fbuf));
    }
    closedir(files);
}

static void
efreet_util_desktops_by_category_add(Efreet_Desktop *desktop)
{
    Eina_List *l;
    const char *category;

    if (!desktop->categories) return;

    EINA_LIST_FOREACH(desktop->categories, l, category)
    {
        Eina_List *list;
        list = eina_hash_find(desktops_by_category, category);
        if (!eina_list_data_find(list, desktop))
            list = eina_list_append(list, desktop);
        eina_hash_modify(desktops_by_category, category, list);
    }
}

static void
efreet_util_desktops_by_category_remove(Efreet_Desktop *desktop)
{
    Eina_List *l;
    const char *category;

    if (!desktop->categories) return;

    EINA_LIST_FOREACH(desktop->categories, l, category)
    {
        Eina_List *list;
        list = eina_hash_find(desktops_by_category, category);
        if (!list) continue;
        if (eina_list_data_find(list, desktop))
            list = eina_list_remove(list, desktop);
        if (!list)
	  eina_hash_del(desktops_by_category, category, list);
        else
            eina_hash_modify(desktops_by_category, category, list);
    }
}

static void
efreet_util_desktop_free(Efreet_Util_Desktop *ud)
{
    if (!ud) return;

    efreet_desktop_free(ud->desktop);
    free(ud);
}

static void
efreet_event_desktop_change_free(void *data __UNUSED__, void *ev)
{
    Efreet_Event_Desktop_Change *event;

    event = ev;
    if (event->current) efreet_desktop_free(event->current);
    if (event->previous) efreet_desktop_free(event->previous);

    free(event);
}
