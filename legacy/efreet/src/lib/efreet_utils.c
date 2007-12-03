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
    Ecore_List            *dirs;
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
    const char *what1;
    const char *what2;
};

struct Efreet_Cache_Search_List
{
    Ecore_List *list;
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

static void efreet_util_cache_search_mime(void *value, void *data);
static int  efreet_util_cache_search_wm_class(const void *value, const void *data);
static int  efreet_util_cache_search_exec(const void *value, const void *data);
static int  efreet_util_cache_search_name(const void *value, const void *data);
static int  efreet_util_cache_search_generic_name(const void *value, const void *data);

static void efreet_util_cache_search_name_glob(void *value, void *data);
static void efreet_util_cache_search_exec_glob(void *value, void *data);
static void efreet_util_cache_search_generic_name_glob(void *value, void *data);
static void efreet_util_cache_search_comment_glob(void *value, void *data);

static int  efreet_util_glob_match(const char *str, const char *glob);

static void efreet_util_monitor(const char *path, const char *file_id, int priority);
static void efreet_util_monitor_cb(void *data, Ecore_File_Monitor *monitor,
                                    Ecore_File_Event event, const char *path);
static void efreet_util_monitor_free(void *data);
static void efreet_util_menus_find_helper(Ecore_List *menus, const char *config_dir);

static void efreet_util_desktops_by_category_add(Efreet_Desktop *desktop);
static void efreet_util_desktops_by_category_remove(Efreet_Desktop *desktop);

static void efreet_util_desktop_free(Efreet_Util_Desktop *ud);
static void efreet_event_desktop_change_free(void *data, void *ev);

static Ecore_Hash *desktop_by_file_id = NULL;
static Ecore_Hash *file_id_by_desktop_path = NULL;
static Ecore_Hash *desktops_by_category = NULL;

static Ecore_Idler       *idler = NULL;
static Efreet_Cache_Fill *fill = NULL;

static Ecore_List *monitors = NULL;

static int init = 0;

EAPI int EFREET_EVENT_DESKTOP_LIST_CHANGE = 0;
EAPI int EFREET_EVENT_DESKTOP_CHANGE = 0;

EAPI int
efreet_util_init(void)
{
    Ecore_List *dirs;

    if (init++) return init;

    if (!EFREET_EVENT_DESKTOP_LIST_CHANGE)
        EFREET_EVENT_DESKTOP_LIST_CHANGE = ecore_event_type_new();
    if (!EFREET_EVENT_DESKTOP_CHANGE)
        EFREET_EVENT_DESKTOP_CHANGE = ecore_event_type_new();
    desktop_by_file_id = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(desktop_by_file_id, ECORE_FREE_CB(ecore_string_release));
    ecore_hash_free_value_cb_set(desktop_by_file_id, ECORE_FREE_CB(efreet_util_desktop_free));

    file_id_by_desktop_path = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(file_id_by_desktop_path, ECORE_FREE_CB(ecore_string_release));
    ecore_hash_free_value_cb_set(file_id_by_desktop_path, ECORE_FREE_CB(ecore_string_release));

    desktops_by_category = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(desktops_by_category, ECORE_FREE_CB(ecore_string_release));
    ecore_hash_free_value_cb_set(desktops_by_category, ECORE_FREE_CB(ecore_list_destroy));

    monitors = ecore_list_new();
    ecore_list_free_cb_set(monitors, efreet_util_monitor_free);

    fill = NEW(Efreet_Cache_Fill, 1);
    fill->dirs = ecore_list_new();
    ecore_list_free_cb_set(fill->dirs, efreet_util_cache_dir_free);
    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    if (dirs)
    {
        Efreet_Cache_Fill_Dir *dir;
        char *path;
        int priority = 0;

        while ((path = ecore_list_first_remove(dirs)))
        {
            dir = NEW(Efreet_Cache_Fill_Dir, 1);
            dir->path = path;
            dir->priority = priority++;
            ecore_list_append(fill->dirs, dir);
        }
        ecore_list_destroy(dirs);
    }
    idler = ecore_idler_add(efreet_util_cache_fill, NULL);
    return init;
}

EAPI int
efreet_util_shutdown(void)
{
    if (--init) return init;

    if (idler)
    {
        ecore_idler_del(idler);
        IF_FREE_LIST(fill->dirs);
        if (fill->current) efreet_util_cache_dir_free(fill->current);
        if (fill->files) closedir(fill->files);
        free(fill);
    }
    idler = NULL;

    IF_FREE_HASH(desktop_by_file_id);
    IF_FREE_HASH(file_id_by_desktop_path);

    IF_FREE_LIST(monitors);

    IF_FREE_HASH(desktops_by_category);

    return init;
}

static char *
efreet_util_path_in_default(const char *section, const char *path)
{
    Ecore_List *dirs;
    char *ret = NULL;
    char *dir;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    section);

    ecore_list_first_goto(dirs);
    while ((dir = ecore_list_next(dirs)))
    {
        size_t len;

        len = strlen(dir);
        if (!strncmp(path, dir, strlen(dir)))
        {
            ret = strdup(dir);
            break;
        }
    }

    ecore_list_destroy(dirs);
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
    file_id = ecore_hash_get(file_id_by_desktop_path, path);
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
    file_id = ecore_string_instance(tmp);
    free(tmp);
    ecore_hash_set(file_id_by_desktop_path, (void *)ecore_string_instance(path),
                                                        (void *)file_id);
    return file_id;
}

EAPI Ecore_List *
efreet_util_desktop_mime_list(const char *mime)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = ecore_string_instance(mime);

    ecore_hash_for_each_node(desktop_by_file_id, efreet_util_cache_search_mime, &search);
    ecore_string_release(search.what);

    if (ecore_list_empty_is(search.list)) IF_FREE_LIST(search.list);
    return search.list;
}

EAPI Efreet_Desktop *
efreet_util_desktop_wm_class_find(const char *wmname, const char *wmclass)
{
    Efreet_Cache_Search search;
    Efreet_Util_Desktop *ud;

    if ((!wmname) && (!wmclass)) return NULL;
    search.what1 = wmname;
    search.what2 = wmclass;
    ud = ecore_hash_find(desktop_by_file_id, efreet_util_cache_search_wm_class, &search);
    if (ud) return ud->desktop;
    return NULL;
}

EAPI Efreet_Desktop *
efreet_util_desktop_file_id_find(const char *file_id)
{
    Efreet_Desktop *desktop = NULL;
    Efreet_Util_Desktop *ud = NULL;
    Ecore_List *dirs;
    const char *dir;
    int priority = 0;

    if (!file_id) return NULL;
    ud = ecore_hash_get(desktop_by_file_id, file_id);
    if (ud) return ud->desktop;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    if (!dirs) return NULL;

    ecore_list_first_goto(dirs);
    while ((dir = ecore_list_next(dirs)))
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
    ecore_list_destroy(dirs);
    if (desktop)
    {
        Efreet_Event_Desktop_Change *ev;

        ud = NEW(Efreet_Util_Desktop, 1);
        ud->priority = priority;
        ud->desktop = desktop;

        ecore_hash_set(desktop_by_file_id, (void *)ecore_string_instance(file_id), ud);
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
    Efreet_Util_Desktop *ud;

    if (!exec) return NULL;
    search.what1 = exec;
    search.what2 = NULL;
    ud = ecore_hash_find(desktop_by_file_id, efreet_util_cache_search_exec, &search);
    if (ud) return ud->desktop;
    return NULL;
}

EAPI Efreet_Desktop *
efreet_util_desktop_name_find(const char *name)
{
    Efreet_Cache_Search search;
    Efreet_Util_Desktop *ud;

    if (!name) return NULL;
    search.what1 = name;
    search.what2 = NULL;
    ud = ecore_hash_find(desktop_by_file_id, efreet_util_cache_search_name, &search);
    if (ud) return ud->desktop;
    return NULL;
}

EAPI Efreet_Desktop *
efreet_util_desktop_generic_name_find(const char *generic_name)
{
    Efreet_Cache_Search search;
    Efreet_Util_Desktop *ud;

    if (!generic_name) return NULL;
    search.what1 = generic_name;
    search.what2 = NULL;
    ud = ecore_hash_find(desktop_by_file_id, efreet_util_cache_search_generic_name, &search);
    if (ud) return ud->desktop;
    return NULL;
}

EAPI Ecore_List *
efreet_util_desktop_name_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = glob;

    ecore_hash_for_each_node(desktop_by_file_id, efreet_util_cache_search_name_glob, &search);

    if (ecore_list_empty_is(search.list)) IF_FREE_LIST(search.list);
    return search.list;
}

EAPI Ecore_List *
efreet_util_desktop_exec_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = glob;

    ecore_hash_for_each_node(desktop_by_file_id, efreet_util_cache_search_exec_glob, &search);

    if (ecore_list_empty_is(search.list)) IF_FREE_LIST(search.list);
    return search.list;
}

EAPI Ecore_List *
efreet_util_desktop_generic_name_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = glob;

    ecore_hash_for_each_node(desktop_by_file_id, efreet_util_cache_search_generic_name_glob, &search);

    if (ecore_list_empty_is(search.list)) IF_FREE_LIST(search.list);
    return search.list;
}

EAPI Ecore_List *
efreet_util_desktop_comment_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = glob;

    ecore_hash_for_each_node(desktop_by_file_id, efreet_util_cache_search_comment_glob, &search);

    if (ecore_list_empty_is(search.list)) IF_FREE_LIST(search.list);
    return search.list;
}

/**
 * Find all desktop categories
 * This list must be freed using ecore_list_destroy()
 *
 * @return an Ecore_List of category names (const char *)
 */
EAPI Ecore_List *
efreet_util_desktop_categories_list(void)
{
    return ecore_hash_keys(desktops_by_category);
}

/**
 * Find all desktops in a given category
 *
 * Note: this returns a pointer to an internal list. Do not add to, remove from, or delete this list.
 *
 * @param category the category name
 * @return a list of desktops
 */
EAPI Ecore_List *
efreet_util_desktop_category_list(const char *category)
{
    return ecore_hash_get(desktops_by_category, category);
}

#if 0
static void
dump(void *value, void *data __UNUSED__)
{
    Ecore_Hash_Node *node;
    node = value;
    printf("%s -> %p\n", (char *)node->key, node->value);
}
#endif

static int
efreet_util_cache_fill(void *data __UNUSED__)
{
    struct dirent *file = NULL;
    double start;
    char buf[PATH_MAX];

    if (!fill->dirs)
    {
        free(fill);
        idler = NULL;
        fill = NULL;
        ecore_event_add(EFREET_EVENT_DESKTOP_LIST_CHANGE, NULL, NULL, NULL);

        return 0;
    }
    if (!fill->current)
    {
        fill->current = ecore_list_first_remove(fill->dirs);
        if (!fill->current)
        {
            IF_FREE_LIST(fill->dirs);
            free(fill);
            idler = NULL;
            fill = NULL;
#if 0
            ecore_hash_for_each_node(desktop_by_file_id, dump, NULL);
            ecore_hash_for_each_node(file_id_by_desktop_path, dump, NULL);
            printf("%d\n", ecore_hash_count(desktop_by_file_id));
#endif
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
                ecore_list_append(fill->dirs, dir);
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

    if (!ecore_hash_get(file_id_by_desktop_path, desktop->orig_path))
        ecore_hash_set(file_id_by_desktop_path,
                        (void *)ecore_string_instance(desktop->orig_path),
                        (void *)ecore_string_instance(file_id));

    ud = ecore_hash_get(desktop_by_file_id, file_id);
    if (!ud)
    {
        ud = NEW(Efreet_Util_Desktop, 1);
        ud->priority = priority;
        ud->desktop = desktop;

        ecore_hash_set(desktop_by_file_id, (void *)ecore_string_instance(file_id), ud);
        efreet_util_desktops_by_category_add(desktop);

        if (event)
        {
            Efreet_Event_Desktop_Change *ev;

            ev = NEW(Efreet_Event_Desktop_Change, 1);
            ev->current = desktop;
            efreet_desktop_ref(ev->current);
            ev->change = EFREET_DESKTOP_CHANGE_ADD;
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
    ud = ecore_hash_get(desktop_by_file_id, file_id);
    if (ud && (ud->priority >= priority))
    {
        Efreet_Event_Desktop_Change *ev;

        ecore_hash_remove(desktop_by_file_id, file_id);
        efreet_util_desktops_by_category_remove(ud->desktop);

        ev = NEW(Efreet_Event_Desktop_Change, 1);
        ev->current = ud->desktop;
        efreet_desktop_ref(ev->current);
        ev->change = EFREET_DESKTOP_CHANGE_REMOVE;
        ecore_event_add(EFREET_EVENT_DESKTOP_CHANGE, ev, efreet_event_desktop_change_free, NULL);

        efreet_util_desktop_free(ud);

        /* This call will search application dirs and add the file to cache if it
         * exists. */
        efreet_util_desktop_file_id_find(file_id);
    }
    ecore_hash_remove(file_id_by_desktop_path, path);
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

    ud = ecore_hash_get(desktop_by_file_id, file_id);
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

        ecore_hash_set(desktop_by_file_id, (void *)ecore_string_instance(file_id), ud);
        efreet_util_desktops_by_category_remove(ud->desktop);
        efreet_util_desktops_by_category_add(desktop);

        ev = NEW(Efreet_Event_Desktop_Change, 1);
        ev->current = desktop;
        efreet_desktop_ref(ev->current);
        ev->change = EFREET_DESKTOP_CHANGE_ADD;
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

static void
efreet_util_cache_search_mime(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;
    const char               *mime;

    node = value;
    search = data;
    ud = node->value;

    if (!ud->desktop->mime_types) return;
    ecore_list_first_goto(ud->desktop->mime_types);
    while ((mime = ecore_list_next(ud->desktop->mime_types)))
    {
        if (search->what == mime)
        {
            ecore_list_append(search->list, ud->desktop);
            break;
        }
    }
}

static int
efreet_util_cache_search_wm_class(const void *value, const void *data)
{
    const Efreet_Cache_Search *search;
    const Efreet_Util_Desktop *ud;

    ud = value;
    search = data;

    if (!ud->desktop->startup_wm_class) return 1;
    if ((search->what2) && (!strcmp(ud->desktop->startup_wm_class, search->what2)))
        return 0;
    else if ((search->what1) && (!strcmp(ud->desktop->startup_wm_class, search->what1)))
        return 0;
    return 1;
}

static int
efreet_util_cache_search_exec(const void *value, const void *data)
{
    const Efreet_Cache_Search *search;
    const Efreet_Util_Desktop *ud;
    char                      *exec;
    const char                *file;

    ud = value;
    search = data;

    if (!ud->desktop->exec) return 1;
    exec = ecore_file_app_exe_get(ud->desktop->exec);
    if (!exec) return 1;
    if (!strcmp(exec, search->what1))
    {
       free(exec);
       return 0;
    }

    file = ecore_file_file_get(exec);
    if (file && !strcmp(file, search->what1))
    {
       free(exec);
       return 0;
    }
    free(exec);
    return 1;
}

static int
efreet_util_cache_search_name(const void *value, const void *data)
{
    const Efreet_Cache_Search *search;
    const Efreet_Util_Desktop *ud;

    ud = value;
    search = data;

    if (!ud->desktop->name) return 1;
    return strcmp(ud->desktop->name, search->what1);
}

static int
efreet_util_cache_search_generic_name(const void *value, const void *data)
{
    const Efreet_Cache_Search *search;
    const Efreet_Util_Desktop *ud;

    ud = value;
    search = data;

    if (!ud->desktop->generic_name) return 1;
    return strcmp(ud->desktop->generic_name, search->what1);
}

static void
efreet_util_cache_search_name_glob(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;

    node = value;
    search = data;
    ud = node->value;

    if (efreet_util_glob_match(ud->desktop->name, search->what))
        ecore_list_append(search->list, ud->desktop);
}

static void
efreet_util_cache_search_exec_glob(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;
    char                     *exec;

    node = value;
    search = data;
    ud = node->value;

    if (!ud->desktop->exec) return;
    exec = ecore_file_app_exe_get(ud->desktop->exec);
    if (exec)
    {
        if (efreet_util_glob_match(exec, search->what))
            ecore_list_append(search->list, ud->desktop);
        free(exec);
    }
}

static void
efreet_util_cache_search_generic_name_glob(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;

    node = value;
    search = data;
    ud = node->value;

    if (efreet_util_glob_match(ud->desktop->generic_name, search->what))
        ecore_list_append(search->list, ud->desktop);
}

static void
efreet_util_cache_search_comment_glob(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Util_Desktop      *ud;

    node = value;
    search = data;
    ud = node->value;

    if (efreet_util_glob_match(ud->desktop->comment, search->what))
        ecore_list_append(search->list, ud->desktop);
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
    ecore_list_append(monitors, em);
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
                    fill->dirs = ecore_list_new();
                    ecore_list_free_cb_set(fill->dirs, efreet_util_cache_dir_free);
                }

                dir = NEW(Efreet_Cache_Fill_Dir, 1);
                dir->path = strdup(path);
                dir->file_id = strdup(file_id);
                dir->priority = em->priority;
                ecore_list_append(fill->dirs, dir);

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
            if (ecore_list_goto(monitors, em))
                ecore_list_remove(monitors);
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
EAPI Ecore_List *
efreet_util_menus_find(void)
{
    Ecore_List *menus, *dirs;
    const char *dir;

    menus = ecore_list_new();
    ecore_list_free_cb_set(menus, ECORE_FREE_CB(free));

    efreet_util_menus_find_helper(menus, efreet_config_home_get());

    dirs = efreet_config_dirs_get();
    ecore_list_first_goto(dirs);
    while ((dir = ecore_list_next(dirs)))
        efreet_util_menus_find_helper(menus, dir);

    return menus;
}

static void
efreet_util_menus_find_helper(Ecore_List *menus, const char *config_dir)
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

        ecore_list_append(menus, strdup(fbuf));
    }
    closedir(files);
}

static void
efreet_util_desktops_by_category_add(Efreet_Desktop *desktop)
{
    const char *category;

    if (!desktop->categories) return;

    ecore_list_first_goto(desktop->categories);
    while ((category = ecore_list_next(desktop->categories)))
    {
        Ecore_List *list;
        list = ecore_hash_get(desktops_by_category, category);
        if (!list)
        {
            list = ecore_list_new();
            ecore_hash_set(desktops_by_category,
                    (void *)ecore_string_instance(category), list);
        }
        if (!ecore_list_goto(list, desktop))
            ecore_list_append(list, desktop);
    }
}

static void
efreet_util_desktops_by_category_remove(Efreet_Desktop *desktop)
{
    const char *category;

    if (!desktop->categories) return;

    ecore_list_first_goto(desktop->categories);
    while ((category = ecore_list_next(desktop->categories)))
    {
        Ecore_List *list;
        list = ecore_hash_get(desktops_by_category, category);
        if (!list) continue;
        if (ecore_list_goto(list, desktop))
            ecore_list_remove(list);
        if (ecore_list_empty_is(list))
        {
            ecore_hash_remove(desktops_by_category, category);
            ecore_list_destroy(list);
        }
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
