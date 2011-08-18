#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <Eina.h>
#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>

#define EFREET_MODULE_LOG_DOM _efreet_icon_cache_log_dom
static int _efreet_icon_cache_log_dom = -1;

#include "Efreet.h"
#include "efreet_private.h"
#include "efreet_cache_private.h"

/* TODO:
 * - Need to handle programs using different exts
 */

static Eina_Array *exts = NULL;
static Eina_Array *extra_dirs = NULL;
static Eina_Array *strs = NULL;
static Eina_Hash *icon_themes = NULL;

static Eina_Bool
cache_directory_modified(Eina_Hash *dirs, const char *dir)
{
    Efreet_Cache_Directory *dcache;
    struct stat st;

    if (!dirs) return EINA_TRUE;

    if (stat(dir, &st) < 0) return EINA_FALSE;
    dcache = eina_hash_find(dirs, dir);
    if (!dcache)
    {
        dcache = malloc(sizeof (Efreet_Cache_Directory));
        if (!dcache) return EINA_TRUE;

        dcache->modified_time = (long long) st.st_mtime;
        eina_hash_add(dirs, dir, dcache);
    }
    else if (dcache->modified_time == (long long) st.st_mtime) return EINA_FALSE;
    dcache->modified_time = st.st_mtime;

    return EINA_TRUE;
}

static Eina_Bool
cache_extension_lookup(const char *ext)
{
    unsigned int i;

    for (i = 0; i < exts->count; ++i)
        if (!strcmp(exts->data[i], ext))
            return EINA_TRUE;
    return EINA_FALSE;
}

static Eina_Bool
cache_fallback_scan_dir(Eina_Hash *icons, Eina_Hash *dirs, const char *dir)
{
    Eina_Iterator *it;
    Eina_File_Direct_Info *entry;

    if (!cache_directory_modified(dirs, dir)) return EINA_TRUE;

    it = eina_file_stat_ls(dir);
    if (!it) return EINA_TRUE;

    EINA_ITERATOR_FOREACH(it, entry)
    {
        Efreet_Cache_Fallback_Icon *icon;
        char *name;
        char *ext;
        unsigned int i;

        if (entry->type == EINA_FILE_DIR)
            continue;

        ext = strrchr(entry->path + entry->name_start, '.');
        if (!ext || !cache_extension_lookup(ext))
            continue;

        /* icon with known extension */
        name = entry->path + entry->name_start;
        *ext = '\0';

        icon = eina_hash_find(icons, name);
        if (!icon)
        {
            icon = NEW(Efreet_Cache_Fallback_Icon, 1);
            icon->theme = NULL;
            eina_hash_add(icons, name, icon);
        }

        *ext = '.';

        for (i = 0; i < icon->icons_count; ++i)
            if (!strcmp(icon->icons[i], entry->path))
                break;

        if (i != icon->icons_count)
            continue;

        icon->icons = realloc(icon->icons, sizeof (char *) * (icon->icons_count + 1));
        icon->icons[icon->icons_count] = eina_stringshare_add(entry->path);
        eina_array_push(strs, icon->icons[icon->icons_count++]);
    }

    eina_iterator_free(it);

    return EINA_TRUE;
}

static Eina_Bool
cache_fallback_scan(Eina_Hash *icons, Eina_Hash *dirs)
{
    unsigned int i;
    Eina_List *xdg_dirs, *l;
    const char *dir;
    char path[PATH_MAX];

    for (i = 0; i < extra_dirs->count; i++)
        cache_fallback_scan_dir(icons, dirs, extra_dirs->data[i]);

    cache_fallback_scan_dir(icons, dirs, efreet_icon_deprecated_user_dir_get());
    cache_fallback_scan_dir(icons, dirs, efreet_icon_user_dir_get());

    xdg_dirs = efreet_data_dirs_get();
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/icons", dir);
        cache_fallback_scan_dir(icons, dirs, path);
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/pixmaps", dir);
        cache_fallback_scan_dir(icons, dirs, path);
    }
#endif

    cache_fallback_scan_dir(icons, dirs, "/usr/share/pixmaps");

    return EINA_TRUE;
}

static Eina_Bool
check_fallback_changed(Efreet_Cache_Icon_Theme *theme)
{
    unsigned int i;
    Eina_List *xdg_dirs, *l;
    const char *dir;
    char path[PATH_MAX];

    /* Check if the dirs we have cached are changed */
    if (theme->dirs)
    {
        Eina_Iterator *it;
        Eina_Bool changed = EINA_FALSE;

        it = eina_hash_iterator_key_new(theme->dirs);
        EINA_ITERATOR_FOREACH(it, dir)
        {
            changed = !ecore_file_exists(dir);
            if (changed) break;
            changed = cache_directory_modified(theme->dirs, dir);
            if (changed) break;
        }
        eina_iterator_free(it);
        if (changed) return EINA_TRUE;
    }

    /* Check if spec dirs have changed */
    for (i = 0; i < extra_dirs->count; i++)
        if (cache_directory_modified(theme->dirs, extra_dirs->data[i])) return EINA_TRUE;

    if (cache_directory_modified(theme->dirs, efreet_icon_deprecated_user_dir_get())) return EINA_TRUE;
    if (cache_directory_modified(theme->dirs, efreet_icon_user_dir_get())) return EINA_TRUE;

    xdg_dirs = efreet_data_dirs_get();
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/icons", dir);
        if (cache_directory_modified(theme->dirs, path)) return EINA_TRUE;
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/pixmaps", dir);
        if (cache_directory_modified(theme->dirs, path)) return EINA_TRUE;
    }
#endif

    if (cache_directory_modified(theme->dirs, "/usr/share/pixmaps")) return EINA_TRUE;
    return EINA_FALSE;
}

static Eina_Bool
cache_scan_path_dir(Efreet_Icon_Theme *theme,
                    const char *path,
                    Efreet_Icon_Theme_Directory *dir,
                    Eina_Hash *icons)
{
    Eina_Iterator *it;
    char buf[PATH_MAX];
    Eina_File_Direct_Info *entry;

    snprintf(buf, sizeof(buf), "%s/%s", path, dir->name);

    it = eina_file_stat_ls(buf);
    if (!it) return EINA_TRUE;

    EINA_ITERATOR_FOREACH(it, entry)
    {
        Efreet_Cache_Icon *icon;
        char *name;
        char *ext;
        unsigned int i;

        if (entry->type == EINA_FILE_DIR)
            continue;

        ext = strrchr(entry->path + entry->name_start, '.');
        if (!ext || !cache_extension_lookup(ext))
            continue;

        /* icon with known extension */
        name = entry->path + entry->name_start;
        *ext = '\0';

        icon = eina_hash_find(icons, name);
        if (!icon)
        {
            icon = NEW(Efreet_Cache_Icon, 1);
            icon->theme = eina_stringshare_add(theme->name.internal);
            eina_array_push(strs, icon->theme);
            eina_hash_add(icons, name, icon);
        }
        else if (icon->theme && strcmp(icon->theme, theme->name.internal))
        {
            /* We got this icon from a parent theme */
            continue;
        }

        /* find if we have the same icon in another type */
        for (i = 0; i < icon->icons_count; ++i)
        {
            if ((icon->icons[i]->type == dir->type) &&
                (icon->icons[i]->normal == dir->size.normal) &&
                (icon->icons[i]->max == dir->size.max) &&
                (icon->icons[i]->min == dir->size.min))
                break;
        }

        *ext = '.';

        if (i != icon->icons_count)
        {
            unsigned int j;

            /* check if the path already exist */
            for (j = 0; j < icon->icons[i]->paths_count; ++j)
                if (!strcmp(icon->icons[i]->paths[j], entry->path))
                    break;

            if (j != icon->icons[i]->paths_count)
                continue;
        }
        /* no icon match so add a new one */
        else
        {
            icon->icons = realloc(icon->icons,
                                  sizeof (Efreet_Cache_Icon_Element*) * (++icon->icons_count));
            icon->icons[i] = NEW(Efreet_Cache_Icon_Element, 1);
            icon->icons[i]->type = dir->type;
            icon->icons[i]->normal = dir->size.normal;
            icon->icons[i]->min = dir->size.min;
            icon->icons[i]->max = dir->size.max;
            icon->icons[i]->paths = NULL;
            icon->icons[i]->paths_count = 0;
        }

        /* and finally store the path */
        icon->icons[i]->paths = realloc(icon->icons[i]->paths,
                                        sizeof (char*) * (icon->icons[i]->paths_count + 1));
        icon->icons[i]->paths[icon->icons[i]->paths_count] = eina_stringshare_add(entry->path);
        eina_array_push(strs, icon->icons[i]->paths[icon->icons[i]->paths_count++]);
    }

    eina_iterator_free(it);

    return EINA_TRUE;
}

static Eina_Bool
cache_scan_path(Efreet_Icon_Theme *theme, Eina_Hash *icons, const char *path)
{
    Eina_List *l;
    Efreet_Icon_Theme_Directory *dir;

    EINA_LIST_FOREACH(theme->directories, l, dir)
        if (!cache_scan_path_dir(theme, path, dir, icons)) return EINA_FALSE;

    return EINA_TRUE;
}

static Eina_Bool
cache_scan(Efreet_Icon_Theme *theme, Eina_Hash *themes, Eina_Hash *icons)
{
    Eina_List *l;
    const char *path;
    const char *name;

    if (!theme) return EINA_TRUE;
    if (eina_hash_find(themes, theme->name.internal)) return EINA_TRUE;
    eina_hash_direct_add(themes, theme->name.internal, theme);

    /* scan theme */
    EINA_LIST_FOREACH(theme->paths, l, path)
        if (!cache_scan_path(theme, icons, path)) return EINA_FALSE;

    /* scan inherits */
    if (theme->inherits)
    {
        EINA_LIST_FOREACH(theme->inherits, l, name)
        {
            Efreet_Icon_Theme *inherit;

            inherit = eina_hash_find(icon_themes, name);
            if (!inherit)
                INF("Theme `%s` not found for `%s`.",
                    name, theme->name.internal);
            if (!cache_scan(inherit, themes, icons)) return EINA_FALSE;
        }
    }
    else if (strcmp(theme->name.internal, "hicolor"))
    {
        theme = eina_hash_find(icon_themes, "hicolor");
        if (!cache_scan(theme, themes, icons)) return EINA_FALSE;
    }

    return EINA_TRUE;
}

static Eina_Bool
check_changed(Efreet_Cache_Icon_Theme *theme)
{
    Eina_List *l;
    const char *name;

    if (!theme) return EINA_FALSE;

    if (theme->changed) return EINA_TRUE;
    if (theme->theme.inherits)
    {
        EINA_LIST_FOREACH(theme->theme.inherits, l, name)
        {
            Efreet_Cache_Icon_Theme *inherit;

            inherit = eina_hash_find(icon_themes, name);
            if (!inherit)
                INF("Theme `%s` not found for `%s`.",
                        name, theme->theme.name.internal);
            if (check_changed(inherit)) return EINA_TRUE;
        }
    }
    else if (strcmp(theme->theme.name.internal, "hicolor"))
    {
        theme = eina_hash_find(icon_themes, "hicolor");
        if (check_changed(theme)) return EINA_TRUE;
    }
    return EINA_FALSE;
}

static Efreet_Icon_Theme_Directory *
icon_theme_directory_new(Efreet_Ini *ini, const char *name)
{
    Efreet_Icon_Theme_Directory *dir;
    int val;
    const char *tmp;

    if (!ini) return NULL;

    dir = NEW(Efreet_Icon_Theme_Directory, 1);
    if (!dir) return NULL;
    dir->name = eina_stringshare_add(name);
    eina_array_push(strs, dir->name);

    efreet_ini_section_set(ini, name);

    tmp = efreet_ini_string_get(ini, "Context");
    if (tmp)
    {
        if (!strcasecmp(tmp, "Actions"))
            dir->context = EFREET_ICON_THEME_CONTEXT_ACTIONS;

        else if (!strcasecmp(tmp, "Devices"))
            dir->context = EFREET_ICON_THEME_CONTEXT_DEVICES;

        else if (!strcasecmp(tmp, "FileSystems"))
            dir->context = EFREET_ICON_THEME_CONTEXT_FILESYSTEMS;

        else if (!strcasecmp(tmp, "MimeTypes"))
            dir->context = EFREET_ICON_THEME_CONTEXT_MIMETYPES;
    }

    /* Threshold is fallback  */
    dir->type = EFREET_ICON_SIZE_TYPE_THRESHOLD;

    tmp = efreet_ini_string_get(ini, "Type");
    if (tmp)
    {
        if (!strcasecmp(tmp, "Fixed"))
            dir->type = EFREET_ICON_SIZE_TYPE_FIXED;

        else if (!strcasecmp(tmp, "Scalable"))
            dir->type = EFREET_ICON_SIZE_TYPE_SCALABLE;
    }

    dir->size.normal = efreet_ini_int_get(ini, "Size");

    if (dir->type == EFREET_ICON_SIZE_TYPE_THRESHOLD)
    {
        val = efreet_ini_int_get(ini, "Threshold");
        if (val < 0) val = 2;
        dir->size.max = dir->size.normal + val;
        dir->size.min = dir->size.normal - val;
    }
    else if (dir->type == EFREET_ICON_SIZE_TYPE_SCALABLE)
    {
        val = efreet_ini_int_get(ini, "MinSize");
        if (val < 0) dir->size.min = dir->size.normal;
        else dir->size.min = val;

        val = efreet_ini_int_get(ini, "MaxSize");
        if (val < 0) dir->size.max = dir->size.normal;
        else dir->size.max = val;
    }

    return dir;
}

static Eina_Bool
icon_theme_index_read(Efreet_Cache_Icon_Theme *theme, const char *path)
{
    Efreet_Ini *ini;
    Efreet_Icon_Theme_Directory *dir;
    const char *tmp;
    struct stat st;
    char rp[PATH_MAX];

    if (!theme || !path) return EINA_FALSE;

    if (!realpath(path, rp)) return EINA_FALSE;

    if (stat(rp, &st) < 0) return EINA_FALSE;
    if (theme->path && !strcmp(theme->path, rp) && theme->last_cache_check >= (long long) st.st_mtime)
    {
        /* no change */
        theme->valid = 1;
        return EINA_TRUE;
    }
    if (!theme->path || strcmp(theme->path, rp))
    {
        theme->path = eina_stringshare_add(rp);
        eina_array_push(strs, theme->path);
    }
    if ((long long) st.st_mtime > theme->last_cache_check)
        theme->last_cache_check = (long long) st.st_mtime;
    theme->changed = 1;

    ini = efreet_ini_new(path);
    if (!ini) return EINA_FALSE;
    if (!ini->data)
    {
        efreet_ini_free(ini);
        return EINA_FALSE;
    }

    efreet_ini_section_set(ini, "Icon Theme");
    tmp = efreet_ini_localestring_get(ini, "Name");
    if (tmp)
    {
        theme->theme.name.name = eina_stringshare_add(tmp);
        eina_array_push(strs, theme->theme.name.name);
    }

    tmp = efreet_ini_localestring_get(ini, "Comment");
    if (tmp)
    {
        theme->theme.comment = eina_stringshare_add(tmp);
        eina_array_push(strs, theme->theme.comment);
    }

    tmp = efreet_ini_string_get(ini, "Example");
    if (tmp)
    {
        theme->theme.example_icon = eina_stringshare_add(tmp);
        eina_array_push(strs, theme->theme.example_icon);
    }

    theme->hidden = efreet_ini_boolean_get(ini, "Hidden");

    theme->valid = 1;

    /* Check the inheritance. If there is none we inherit from the hicolor theme */
    tmp = efreet_ini_string_get(ini, "Inherits");
    if (tmp)
    {
        char *t, *s, *p;
        const char *i;
        size_t len;

        len = strlen(tmp) + 1;
        t = alloca(len);
        memcpy(t, tmp, len);
        s = t;
        p = strchr(s, ',');

        while (p)
        {
            *p = '\0';

            i = eina_stringshare_add(s);
            theme->theme.inherits = eina_list_append(theme->theme.inherits, i);
            eina_array_push(strs, i);
            s = ++p;
            p = strchr(s, ',');
        }
        i = eina_stringshare_add(s);
        theme->theme.inherits = eina_list_append(theme->theme.inherits, i);
        eina_array_push(strs, i);
    }

    /* make sure this one is done last as setting the directory will change
     * the ini section ... */
    tmp = efreet_ini_string_get(ini, "Directories");
    if (tmp)
    {
        char *t, *s, *p;
        size_t len;

        len = strlen(tmp) + 1;
        t = alloca(len);
        memcpy(t, tmp, len);
        s = t;
        p = s;

        while (p)
        {
            p = strchr(s, ',');

            if (p) *p = '\0';

            dir = icon_theme_directory_new(ini, s);
            if (!dir) goto error;
            theme->theme.directories = eina_list_append(theme->theme.directories, dir);

            if (p) s = ++p;
        }
    }

error:
    efreet_ini_free(ini);

    return EINA_TRUE;
}

static Eina_Bool
cache_theme_scan(const char *dir)
{
    Eina_Iterator *it;
    Eina_File_Direct_Info *entry;

    it = eina_file_stat_ls(dir);
    if (!it) return EINA_TRUE;

    EINA_ITERATOR_FOREACH(it, entry)
    {
        Efreet_Cache_Icon_Theme *theme;
        const char *name;
        const char *path;
        char buf[PATH_MAX];
        struct stat st;

        if (stat(entry->path, &st) < 0) continue;

        if ((entry->type != EINA_FILE_DIR) &&
            (entry->type != EINA_FILE_LNK))
            continue;

        name = entry->path + entry->name_start;
        theme = eina_hash_find(icon_themes, name);

        if (!theme)
        {
            theme = NEW(Efreet_Cache_Icon_Theme, 1);
            theme->theme.name.internal = eina_stringshare_add(name);
            eina_array_push(strs, theme->theme.name.internal);
            eina_hash_direct_add(icon_themes,
                          (void *)theme->theme.name.internal, theme);
            theme->changed = 1;
        }
        if ((long long) st.st_mtime > theme->last_cache_check)
        {
            theme->last_cache_check = (long long) st.st_mtime;
            theme->changed = 1;
        }

        /* TODO: We need to handle change in order of included paths */
        if (!eina_list_search_unsorted(theme->theme.paths, EINA_COMPARE_CB(strcmp), entry->path))
        {
            path = eina_stringshare_add(entry->path);
            theme->theme.paths = eina_list_append(theme->theme.paths, path);
            eina_array_push(strs, path);
            theme->changed = 1;
        }

        /* we're already valid so no reason to check for an index.theme file */
        if (theme->valid) continue;

        /* if the index.theme file exists we parse it into the theme */
        memcpy(buf, entry->path, entry->path_length);
        memcpy(buf + entry->path_length, "/index.theme", sizeof("/index.theme"));
        if (ecore_file_exists(buf))
        {
            if (!icon_theme_index_read(theme, buf))
                theme->valid = 0;
        }
    }
    eina_iterator_free(it);
    return EINA_TRUE;
}

static int
cache_lock_file(void)
{
    char file[PATH_MAX];
    struct flock fl;
    int lockfd;

    snprintf(file, sizeof(file), "%s/efreet/icon_data.lock", efreet_cache_home_get());
    lockfd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (lockfd < 0) return -1;
    efreet_fsetowner(lockfd);

    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(lockfd, F_SETLK, &fl) < 0)
    {
        WRN("LOCKED! You may want to delete %s if this persists", file);
        close(lockfd);
        return -1;
    }

    return lockfd;
}

static void
icon_theme_free(Efreet_Cache_Icon_Theme *theme)
{
    void *data;

    eina_list_free(theme->theme.paths);
    eina_list_free(theme->theme.inherits);
    EINA_LIST_FREE(theme->theme.directories, data)
        free(data);
    if (theme->dirs) efreet_hash_free(theme->dirs, free);
    free(theme);
}

/**
 * @internal
 * @return EINA_TRUE if data adds new
 */
static Eina_Bool
add_data(Eet_File *ef, Eina_Array *data, const char *key)
{
    Efreet_Cache_Array_String *add;
    unsigned int i, j;
    Eina_Bool added = EINA_FALSE;

    add = eet_data_read(ef, efreet_array_string_edd(), key);
    if (!add) return EINA_TRUE;
    /* loop once to check added */
    for (i = 0; i < data->count; i++)
    {
        int found = 0;
        for (j = 0; j < add->array_count; ++j)
        {
            if (!strcmp(add->array[j], data->data[i]))
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            added = EINA_TRUE;
            break;
        }
    }
    /* loop again to add all data */
    for (i = 0; i < add->array_count; i++)
    {
        int found = 0;
        for (j = 0; j < data->count; ++j)
        {
            if (!strcmp(add->array[i], data->data[j]))
            {
                found = 1;
                break;
            }
        }
        if (!found)
            eina_array_push(data, add->array[i]);
    }
    IF_FREE(add->array);
    free(add);

    return added;
}


static void
save_data(Eet_File *ef, Eina_Array *data, const char *key)
{
    Efreet_Cache_Array_String *save;
    unsigned int i;

    if (!data || !data->count) return;

    save = NEW(Efreet_Cache_Array_String, 1);
    save->array = NEW(char *, data->count);
    save->array_count = 0;
    for (i = 0; i < data->count; ++i)
        save->array[save->array_count++] = data->data[i];
    eet_data_write(ef, efreet_array_string_edd(), key, save, 1);
    IF_FREE(save->array);
    free(save);
}

int
main(int argc, char **argv)
{
    /* TODO:
     * - Add file monitor on files, so that we catch changes on files
     *   during whilst this program runs.
     * - Maybe linger for a while to reduce number of cache re-creates.
     */
    Eina_Iterator *it;
    Efreet_Cache_Version *icon_version;
    Efreet_Cache_Version *theme_version;
    Efreet_Cache_Icon_Theme *theme;
    Eet_Data_Descriptor *theme_edd;
    Eet_Data_Descriptor *icon_edd;
    Eet_Data_Descriptor *fallback_edd;
    Eet_File *icon_ef;
    Eet_File *theme_ef;
    Eina_List *xdg_dirs = NULL;
    Eina_List *l = NULL;
    char file[PATH_MAX];
    const char *path;
    char *dir = NULL;
    Eina_Bool changed = EINA_FALSE;
    Eina_Bool flush = EINA_FALSE;
    int lockfd = -1;
    int tmpfd = -1;
    char **keys;
    int num, i;

    /* init external subsystems */
    if (!eina_init()) return -1;
    _efreet_icon_cache_log_dom =
        eina_log_domain_register("efreet_icon_cache", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_icon_cache_log_dom < 0)
    {
        EINA_LOG_ERR("Efreet: Could not create a log domain for efreet_icon_cache.");
        return -1;
    }

    eina_log_domain_level_set("efreet_icon_cache", EINA_LOG_LEVEL_ERR);

    exts = eina_array_new(10);
    extra_dirs = eina_array_new(10);

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-v"))
            eina_log_domain_level_set("efreet_icon_cache", EINA_LOG_LEVEL_DBG);
        else if ((!strcmp(argv[i], "-h")) ||
                 (!strcmp(argv[i], "-help")) ||
                 (!strcmp(argv[i], "--h")) ||
                 (!strcmp(argv[i], "--help")))
        {
            printf("Options:\n");
            printf("  -v              Verbose mode\n");
            printf("  -e .ext1 .ext2  Extensions\n");
            printf("  -d dir1 dir2    Extra dirs\n");
            exit(0);
        }
        else if (!strcmp(argv[i], "-e"))
        {
            while ((i < (argc - 1)) && (argv[(i + 1)][0] != '-'))
                eina_array_push(exts, argv[++i]);
        }
        else if (!strcmp(argv[i], "-d"))
        {
            while ((i < (argc - 1)) && (argv[(i + 1)][0] != '-'))
                eina_array_push(extra_dirs, argv[++i]);
        }
    }

    if (!eet_init()) return -1;
    if (!ecore_init()) return -1;

    efreet_cache_update = 0;
    /* finish efreet init */
    if (!efreet_init()) goto on_error;

    strs = eina_array_new(32);

    /* create homedir */
    snprintf(file, sizeof(file), "%s/efreet", efreet_cache_home_get());
    if (!ecore_file_exists(file))
    {
        if (!ecore_file_mkpath(file)) return -1;
        efreet_setowner(file);
    }

    /* lock process, so that we only run one copy of this program */
    lockfd = cache_lock_file();
    if (lockfd == -1) goto on_error;

    /* Need to init edd's, so they are like we want, not like userspace wants */
    icon_edd = efreet_icon_edd();
    fallback_edd = efreet_icon_fallback_edd();
    theme_edd = efreet_icon_theme_edd(EINA_TRUE);

    icon_themes = eina_hash_string_superfast_new(EINA_FREE_CB(icon_theme_free));

    INF("opening theme cache");
    /* open theme file */
    theme_ef = eet_open(efreet_icon_theme_cache_file(), EET_FILE_MODE_READ_WRITE);
    if (!theme_ef) goto on_error_efreet;
    theme_version = eet_data_read(theme_ef, efreet_version_edd(), EFREET_CACHE_VERSION);
    if (theme_version &&
        ((theme_version->major != EFREET_ICON_CACHE_MAJOR) ||
         (theme_version->minor != EFREET_ICON_CACHE_MINOR)))
    {
        // delete old cache
        eet_close(theme_ef);
        if (unlink(efreet_icon_theme_cache_file()) < 0)
        {
            if (errno != ENOENT) goto on_error_efreet;
        }
        theme_ef = eet_open(efreet_icon_theme_cache_file(), EET_FILE_MODE_READ_WRITE);
        if (!theme_ef) goto on_error_efreet;
    }
    if (!theme_version)
        theme_version = NEW(Efreet_Cache_Version, 1);

    theme_version->major = EFREET_ICON_CACHE_MAJOR;
    theme_version->minor = EFREET_ICON_CACHE_MINOR;

    if (add_data(theme_ef, exts, EFREET_CACHE_ICON_EXTENSIONS))
        flush = EINA_TRUE;
    if (add_data(theme_ef, extra_dirs, EFREET_CACHE_ICON_EXTRA_DIRS))
        flush = EINA_TRUE;
    if (flush)
        changed = EINA_TRUE;

    if (exts->count == 0)
    {
        ERR("Need to pass extensions to icon cache create process");
        goto on_error_efreet;
    }

    keys = eet_list(theme_ef, "*", &num);
    if (keys)
    {
        for (i = 0; i < num; i++)
        {
            if (!strncmp(keys[i], "__efreet", 8)) continue;
            theme = eet_data_read(theme_ef, theme_edd, keys[i]);
            if (theme) eina_hash_direct_add(icon_themes, theme->theme.name.internal, theme);
        }
        free(keys);
    }

    INF("scan for themes");
    /* scan themes */
    cache_theme_scan(efreet_icon_deprecated_user_dir_get());
    cache_theme_scan(efreet_icon_user_dir_get());

    xdg_dirs = efreet_data_dirs_get();
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(file, sizeof(file), "%s/icons", dir);
        cache_theme_scan(file);
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(file, sizeof(file), "%s/pixmaps", dir);
        cache_theme_scan(file);
    }
#endif

    cache_theme_scan("/usr/share/pixmaps");

    /* scan icons */
    it = eina_hash_iterator_data_new(icon_themes);
    EINA_ITERATOR_FOREACH(it, theme)
    {
        if (!theme->valid) continue;
#ifndef STRICT_SPEC
        if (!theme->theme.name.name) continue;
#endif
        INF("scan theme %s", theme->theme.name.name);

        theme->changed = check_changed(theme);
        if (flush)
            theme->changed = EINA_TRUE;

        INF("open icon file");
        /* open icon file */
        icon_ef = eet_open(efreet_icon_cache_file(theme->theme.name.internal), EET_FILE_MODE_READ_WRITE);
        if (!icon_ef) goto on_error_efreet;
        icon_version = eet_data_read(icon_ef, efreet_version_edd(), EFREET_CACHE_VERSION);
        if (theme->changed || (icon_version &&
            ((icon_version->major != EFREET_ICON_CACHE_MAJOR) ||
             (icon_version->minor != EFREET_ICON_CACHE_MINOR))))
        {
            // delete old cache
            eet_close(icon_ef);
            if (unlink(efreet_icon_cache_file(theme->theme.name.internal)) < 0)
            {
                if (errno != ENOENT) goto on_error_efreet;
            }
            icon_ef = eet_open(efreet_icon_cache_file(theme->theme.name.internal), EET_FILE_MODE_READ_WRITE);
            if (!icon_ef) goto on_error_efreet;
            theme->changed = EINA_TRUE;
        }

        if (theme->changed)
            changed = EINA_TRUE;

        if (theme->changed)
        {
            Eina_Hash *themes;
            Eina_Hash *icons;

            if (!icon_version)
                icon_version = NEW(Efreet_Cache_Version, 1);

            icon_version->major = EFREET_ICON_CACHE_MAJOR;
            icon_version->minor = EFREET_ICON_CACHE_MINOR;

            themes = eina_hash_string_superfast_new(NULL);
            icons = eina_hash_string_superfast_new(NULL);

            INF("scan icons\n");
            if (cache_scan(&(theme->theme), themes, icons))
            {
                Eina_Iterator *icons_it;
                Eina_Hash_Tuple *tuple;

                INF("generated: '%s' %i (%i)",
                    theme->theme.name.internal,
                    changed,
                    eina_hash_population(icons));

                icons_it = eina_hash_iterator_tuple_new(icons);
                EINA_ITERATOR_FOREACH(icons_it, tuple)
                    eet_data_write(icon_ef, icon_edd, tuple->key, tuple->data, 1);
                eina_iterator_free(icons_it);

                INF("theme change: %s %lld", theme->theme.name.internal, theme->last_cache_check);
                eet_data_write(theme_ef, theme_edd, theme->theme.name.internal, theme, 1);
            }
            eina_hash_free(themes);
            eina_hash_free(icons);
        }

        eet_data_write(icon_ef, efreet_version_edd(), EFREET_CACHE_VERSION, icon_version, 1);
        eet_close(icon_ef);
        efreet_setowner(efreet_icon_cache_file(theme->theme.name.internal));
        free(icon_version);
    }
    eina_iterator_free(it);

    INF("scan fallback icons");
    theme = eet_data_read(theme_ef, theme_edd, EFREET_CACHE_ICON_FALLBACK);
    if (!theme)
    {
        theme = NEW(Efreet_Cache_Icon_Theme, 1);
        theme->changed = EINA_TRUE;
    }
    if (flush)
        theme->changed = EINA_TRUE;

    INF("open fallback file");
    /* open icon file */
    icon_ef = eet_open(efreet_icon_cache_file(EFREET_CACHE_ICON_FALLBACK), EET_FILE_MODE_READ_WRITE);
    if (!icon_ef) goto on_error_efreet;
    icon_version = eet_data_read(icon_ef, efreet_version_edd(), EFREET_CACHE_VERSION);
    if (theme->changed || (icon_version &&
        ((icon_version->major != EFREET_ICON_CACHE_MAJOR) ||
         (icon_version->minor != EFREET_ICON_CACHE_MINOR))))
    {
        // delete old cache
        eet_close(icon_ef);
        if (unlink(efreet_icon_cache_file(EFREET_CACHE_ICON_FALLBACK)) < 0)
        {
            if (errno != ENOENT) goto on_error_efreet;
        }
        icon_ef = eet_open(efreet_icon_cache_file(EFREET_CACHE_ICON_FALLBACK), EET_FILE_MODE_READ_WRITE);
        if (!icon_ef) goto on_error_efreet;
        theme->changed = EINA_TRUE;
    }
    if (!theme->changed)
        theme->changed = check_fallback_changed(theme);
    if (theme->changed && theme->dirs)
    {
        efreet_hash_free(theme->dirs, free);
        theme->dirs = NULL;
    }
    if (!theme->dirs)
        theme->dirs = eina_hash_string_superfast_new(NULL);

    if (theme->changed)
        changed = EINA_TRUE;

    if (theme->changed)
    {
        Eina_Hash *icons;

        if (!icon_version)
            icon_version = NEW(Efreet_Cache_Version, 1);

        icon_version->major = EFREET_ICON_CACHE_MAJOR;
        icon_version->minor = EFREET_ICON_CACHE_MINOR;

        icons = eina_hash_string_superfast_new(NULL);

        INF("scan fallback icons");
        /* Save fallback in the right part */
        if (cache_fallback_scan(icons, theme->dirs))
        {
            Eina_Iterator *icons_it;
            Eina_Hash_Tuple *tuple;

            INF("generated: fallback %i (%i)", theme->changed, eina_hash_population(icons));

            icons_it = eina_hash_iterator_tuple_new(icons);
            EINA_ITERATOR_FOREACH(icons_it, tuple)
                eet_data_write(icon_ef, fallback_edd, tuple->key, tuple->data, 1);
            eina_iterator_free(icons_it);
        }
        eina_hash_free(icons);

        eet_data_write(theme_ef, theme_edd, EFREET_CACHE_ICON_FALLBACK, theme, 1);
    }

    icon_theme_free(theme);

    eet_data_write(icon_ef, efreet_version_edd(), EFREET_CACHE_VERSION, icon_version, 1);
    eet_close(icon_ef);
    efreet_setowner(efreet_icon_cache_file(EFREET_CACHE_ICON_FALLBACK));
    free(icon_version);

    eina_hash_free(icon_themes);

    /* save data */
    eet_data_write(theme_ef, efreet_version_edd(), EFREET_CACHE_VERSION, theme_version, 1);
    save_data(theme_ef, exts, EFREET_CACHE_ICON_EXTENSIONS);
    save_data(theme_ef, extra_dirs, EFREET_CACHE_ICON_EXTRA_DIRS);

    eet_close(theme_ef);
    efreet_setowner(efreet_icon_theme_cache_file());
    free(theme_version);

    /* touch update file */
    snprintf(file, sizeof(file), "%s/efreet/icon_data.update", efreet_cache_home_get());
    tmpfd = open(file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (tmpfd >= 0)
    {
        char c = 'n';

        efreet_fsetowner(tmpfd);
        if (changed) c = 'c';
        if (write(tmpfd, &c, 1) != 1) perror("write");
        close(tmpfd);
    }

    INF("done");
on_error_efreet:
    efreet_shutdown();

on_error:
    if (lockfd >= 0) close(lockfd);

    while ((path = eina_array_pop(strs)))
        eina_stringshare_del(path);
    eina_array_free(strs);
    eina_array_free(exts);
    eina_array_free(extra_dirs);

    ecore_shutdown();
    eet_shutdown();
    eina_log_domain_unregister(_efreet_icon_cache_log_dom);
    eina_shutdown();

    return 0;
}
