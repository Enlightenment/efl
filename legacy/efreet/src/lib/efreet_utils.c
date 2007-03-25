/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

typedef struct _Efreet_Cache_Fill        Efreet_Cache_Fill;
typedef struct _Efreet_Cache_Fill_Dir    Efreet_Cache_Fill_Dir;
typedef struct _Efreet_Cache_Search      Efreet_Cache_Search;
typedef struct _Efreet_Cache_Search_List Efreet_Cache_Search_List;

struct _Efreet_Cache_Fill
{
    Ecore_List            *dirs;
    Efreet_Cache_Fill_Dir *current;
    DIR  *files;
};

struct _Efreet_Cache_Fill_Dir
{
    char *path;
    char *file_id;
};

struct _Efreet_Cache_Search
{
    const char *what1;
    const char *what2;
};

struct _Efreet_Cache_Search_List
{
    Ecore_List *list;
    const char *what;
};

static int  _efreet_util_cache_fill(void *data);
static void _efreet_util_cache_dir_free(void *data);

static void _efreet_util_cache_search_mime(void *value, void *data);
static int  _efreet_util_cache_search_wm_class(const void *value, const void *data);
static int  _efreet_util_cache_search_name(const void *value, const void *data);
static int  _efreet_util_cache_search_generic_name(const void *value, const void *data);

static void _efreet_util_cache_search_name_glob(void *value, void *data);
static void _efreet_util_cache_search_exec_glob(void *value, void *data);
static void _efreet_util_cache_search_generic_name_glob(void *value, void *data);
static void _efreet_util_cache_search_comment_glob(void *value, void *data);

static int  _efreet_util_glob_match(const char *str, const char *glob);

static Ecore_Hash *desktop_by_file_id = NULL;
static Ecore_Hash *desktop_by_exec = NULL;
static Ecore_Hash *file_id_by_desktop_path = NULL;

static Ecore_Idler *idler = NULL;

int
efreet_util_init(void)
{
    Efreet_Cache_Fill *fill;
    Ecore_List *dirs;

    desktop_by_file_id = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_set_free_key(desktop_by_file_id, ECORE_FREE_CB(ecore_string_release));
    desktop_by_exec = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_set_free_key(desktop_by_exec, ECORE_FREE_CB(ecore_string_release));
    file_id_by_desktop_path = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_set_free_key(file_id_by_desktop_path, ECORE_FREE_CB(ecore_string_release));
    ecore_hash_set_free_value(file_id_by_desktop_path, ECORE_FREE_CB(ecore_string_release));

    fill = NEW(Efreet_Cache_Fill, 1);
    fill->dirs = ecore_list_new();
    ecore_list_set_free_cb(fill->dirs, _efreet_util_cache_dir_free);
    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    if (dirs)
    {
        Efreet_Cache_Fill_Dir *dir;
        char *path;

        while ((path = ecore_list_remove_first(dirs)))
        {
            dir = NEW(Efreet_Cache_Fill_Dir, 1);
            dir->path = path;
            ecore_list_append(fill->dirs, dir);
        }
        ecore_list_destroy(dirs);
        ecore_list_goto_first(fill->dirs);
    }
    idler = ecore_idler_add(_efreet_util_cache_fill, fill);
    return 1;
}

void
efreet_util_shutdown(void)
{
    if (idler)
    {
        Efreet_Cache_Fill *fill;
        fill = ecore_idler_del(idler);
        IF_FREE_LIST(fill->dirs);
        if (fill->current) _efreet_util_cache_dir_free(fill->current);
        if (fill->files) closedir(fill->files);
        free(fill);
    }
    idler = NULL;

    IF_FREE_HASH(desktop_by_file_id);
    IF_FREE_HASH(desktop_by_exec);
    IF_FREE_HASH(file_id_by_desktop_path);
}

char *
efreet_util_path_in_default(const char *section, const char *path)
{
    Ecore_List *dirs;
    char *ret = NULL;
    char *dir;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    section);

    ecore_list_goto_first(dirs);
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

const char *
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

Ecore_List *
efreet_util_desktop_mime_list(const char *mime)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = mime;

    ecore_hash_for_each_node(desktop_by_exec, _efreet_util_cache_search_mime, &search);

    if (ecore_list_is_empty(search.list)) IF_FREE_LIST(search.list);
    return search.list;
}

Efreet_Desktop *
efreet_util_desktop_wm_class_find(const char *wmname, const char *wmclass)
{
    Efreet_Cache_Search search;

    if ((!wmname) && (!wmclass)) return NULL;
    search.what1 = wmname;
    search.what2 = wmclass;
    return ecore_hash_find(desktop_by_exec, _efreet_util_cache_search_wm_class, &search);
}

Efreet_Desktop *
efreet_util_desktop_file_id_find(const char *file_id)
{
    Efreet_Desktop *desktop = NULL;
    Ecore_List *dirs;
    const char *dir;

    if (!file_id) return NULL;
    desktop = ecore_hash_get(desktop_by_file_id, file_id);
    if (desktop) return desktop;
    desktop = NULL;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    if (!dirs) return NULL;

    ecore_list_goto_first(dirs);
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
    }
    ecore_list_destroy(dirs);
    if (desktop) ecore_hash_set(desktop_by_file_id, (void *)ecore_string_instance(file_id), desktop);
    return desktop;
}

Efreet_Desktop *
efreet_util_desktop_exec_find(const char *exec)
{
    Efreet_Desktop *desktop = NULL;

    if (!exec) return NULL;
    desktop = ecore_hash_get(desktop_by_exec, exec);
    if (desktop) return desktop;
    desktop = NULL;

    /* TODO: Try to search for the .desktop file. But if it isn't in the cache it will be
     * timeconsuming.*/

    if (desktop) ecore_hash_set(desktop_by_exec, (void *)ecore_string_instance(exec), desktop);
    return desktop;
}

Efreet_Desktop *
efreet_util_desktop_name_find(const char *name)
{
    Efreet_Cache_Search search;

    if (!name) return NULL;
    search.what1 = name;
    search.what2 = NULL;
    return ecore_hash_find(desktop_by_exec, _efreet_util_cache_search_name, &search);
}

Efreet_Desktop *
efreet_util_desktop_generic_name_find(const char *generic_name)
{
    Efreet_Cache_Search search;

    if (!generic_name) return NULL;
    search.what1 = generic_name;
    search.what2 = NULL;
    return ecore_hash_find(desktop_by_exec, _efreet_util_cache_search_generic_name, &search);
}

Ecore_List *
efreet_util_desktop_name_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = glob;

    ecore_hash_for_each_node(desktop_by_exec, _efreet_util_cache_search_name_glob, &search);

    if (ecore_list_is_empty(search.list)) IF_FREE_LIST(search.list);
    return search.list;
}

Ecore_List *
efreet_util_desktop_exec_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = glob;

    ecore_hash_for_each_node(desktop_by_exec, _efreet_util_cache_search_exec_glob, &search);

    if (ecore_list_is_empty(search.list)) IF_FREE_LIST(search.list);
    return search.list;
}

Ecore_List *
efreet_util_desktop_generic_name_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = glob;

    ecore_hash_for_each_node(desktop_by_exec, _efreet_util_cache_search_generic_name_glob, &search);

    if (ecore_list_is_empty(search.list)) IF_FREE_LIST(search.list);
    return search.list;
}

Ecore_List *
efreet_util_desktop_comment_glob_list(const char *glob)
{
    Efreet_Cache_Search_List search;

    search.list = ecore_list_new();
    search.what = glob;

    ecore_hash_for_each_node(desktop_by_exec, _efreet_util_cache_search_comment_glob, &search);

    if (ecore_list_is_empty(search.list)) IF_FREE_LIST(search.list);
    return search.list;
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
_efreet_util_cache_fill(void *data)
{
    Efreet_Cache_Fill *fill;
    struct dirent *file = NULL;
    double start;
    char buf[PATH_MAX];

    fill = data;
    if (!fill->dirs)
    {
        free(fill);
        idler = NULL;
        return 0;
    }
    if (!fill->current)
    {
        fill->current = ecore_list_remove_first(fill->dirs);
        if (!fill->current)
        {
            IF_FREE_LIST(fill->dirs);
            free(fill);
            idler = NULL;
#if 0
            ecore_hash_for_each_node(desktop_by_file_id, dump, NULL);
            ecore_hash_for_each_node(desktop_by_exec, dump, NULL);
            ecore_hash_for_each_node(file_id_by_desktop_path, dump, NULL);
            printf("%d\n", ecore_hash_count(desktop_by_file_id));
#endif
            return 0;
        }
    }

    start = ecore_time_get();
    if (!fill->files) fill->files = opendir(fill->current->path);
    if (!fill->files)
    {
        /* Couldn't open this dir, continue to next */
        fill->current = NULL;
    }
    else
    {
        while ((ecore_time_get() - start) < 0.01)
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
                ecore_list_append(fill->dirs, dir);
            }
            else
            {
                Efreet_Desktop *desktop;
                char *ext;
                char *exec;

                ext = strrchr(buf, '.');
                if (!ext || strcmp(ext, ".desktop")) continue;
                desktop = efreet_desktop_get(buf);

                if (!desktop || desktop->type != EFREET_DESKTOP_TYPE_APPLICATION) continue;
                if (!ecore_hash_get(desktop_by_file_id, file_id))
                    ecore_hash_set(desktop_by_file_id, (void *)ecore_string_instance(file_id),
                                                            desktop);
                exec = ecore_file_app_exe_get(desktop->exec);
                if (exec)
                {
                    /* TODO: exec can be with and without full path, we should handle that */
                    if (!ecore_hash_get(desktop_by_exec, exec))
                        ecore_hash_set(desktop_by_exec, (void *)ecore_string_instance(exec),
                                                            desktop);
                    free(exec);
                }
                if (!ecore_hash_get(file_id_by_desktop_path, desktop->orig_path))
                    ecore_hash_set(file_id_by_desktop_path,
                                    (void *)ecore_string_instance(desktop->orig_path),
                                    (void *)ecore_string_instance(file_id));
            }
        }
        if (!file)
        {
            /* This dir has been search through */
            _efreet_util_cache_dir_free(fill->current);
            fill->current = NULL;
            closedir(fill->files);
            fill->files = NULL;
        }
    }

    return 1;
}

static void
_efreet_util_cache_dir_free(void *data)
{
    Efreet_Cache_Fill_Dir *dir;

    dir = data;
    IF_FREE(dir->path);
    IF_FREE(dir->file_id);
    free(dir);
}

static void
_efreet_util_cache_search_mime(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Desktop           *desktop;
    const char               *mime;

    node = value;
    search = data;
    desktop = node->value;

    if (!desktop->mime_types) return;
    ecore_list_goto_first(desktop->mime_types);
    while ((mime = ecore_list_next(desktop->mime_types)))
    {
        if (!strcmp(search->what, mime))
        {
            ecore_list_append(search->list, desktop);
            break;
        }
    }
}

static int
_efreet_util_cache_search_wm_class(const void *value, const void *data)
{
    const Efreet_Cache_Search *search;
    const Efreet_Desktop      *desktop;

    desktop = value;
    search = data;

    if (!desktop->startup_wm_class) return 1;
    if ((search->what1) && (!strcmp(desktop->startup_wm_class, search->what1)))
        return 0;
    else if ((search->what2) && (!strcmp(desktop->startup_wm_class, search->what2)))
        return 0;
    return 1;
}

static int
_efreet_util_cache_search_name(const void *value, const void *data)
{
    const Efreet_Cache_Search *search;
    const Efreet_Desktop      *desktop;

    desktop = value;
    search = data;

    if (!desktop->name) return 1;
    return (!strcmp(desktop->name, search->what1));
}

static int
_efreet_util_cache_search_generic_name(const void *value, const void *data)
{
    const Efreet_Cache_Search *search;
    const Efreet_Desktop      *desktop;

    desktop = value;
    search = data;

    if (!desktop->generic_name) return 0;
    return (!strcmp(desktop->generic_name, search->what1));
}

static void
_efreet_util_cache_search_name_glob(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Desktop           *desktop;

    node = value;
    search = data;
    desktop = node->value;

    if (_efreet_util_glob_match(desktop->name, search->what))
        ecore_list_append(search->list, desktop);
}

static void
_efreet_util_cache_search_exec_glob(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Desktop           *desktop;
    char                     *exec;

    node = value;
    search = data;
    desktop = node->value;

    exec = ecore_file_app_exe_get(desktop->exec);
    if (exec)
    {
        if (_efreet_util_glob_match(exec, search->what))
            ecore_list_append(search->list, desktop);
        free(exec);
    }
}

static void
_efreet_util_cache_search_generic_name_glob(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Desktop           *desktop;

    node = value;
    search = data;
    desktop = node->value;

    if (_efreet_util_glob_match(desktop->generic_name, search->what))
        ecore_list_append(search->list, desktop);
}

static void
_efreet_util_cache_search_comment_glob(void *value, void *data)
{
    Ecore_Hash_Node          *node;
    Efreet_Cache_Search_List *search;
    Efreet_Desktop           *desktop;

    node = value;
    search = data;
    desktop = node->value;

    if (_efreet_util_glob_match(desktop->comment, search->what))
        ecore_list_append(search->list, desktop);
}

static int
_efreet_util_glob_match(const char *str, const char *glob)
{
    if (!str || !glob)
        return 0;
    if (glob[0] == 0)
    {
        if (str[0] == 0) return 1;
        return 0;
    }
    if (!strcmp(glob, "*")) return 1;
    if (!fnmatch(glob, str, 0)) return 1;
    return 0;
}
