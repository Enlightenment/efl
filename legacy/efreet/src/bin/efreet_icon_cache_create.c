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

static const char *exts[] = { ".png", ".xpm", ".svg", ".edj", NULL };
static Eina_Array *strs = NULL;
static Eina_Hash *icon_themes = NULL;
static int verbose = 0;

static Eina_Bool
cache_directory_find(Eina_Hash *dirs, const char *dir)
{
    Efreet_Cache_Directory *dcache;
    struct stat st;

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
_cache_extention_lookup(const char *ext)
{
    unsigned int i;

    for (i = 0; exts[i]; ++i)
        if (!strcmp(exts[i], ext))
            return EINA_TRUE;
    return EINA_FALSE;
}

static Eina_Bool
cache_fallback_scan_dir(Eina_Hash *icons, Eina_Hash *dirs, const char *dir, Eina_Bool *changed)
{
    Eina_Iterator *it;
    Eina_File_Direct_Info *entry;

    if (!cache_directory_find(dirs, dir)) return EINA_TRUE;

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
        if (!ext || !_cache_extention_lookup(ext))
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
            continue ;

        /* we don't really track path deat here, so we will leak... */
        icon->icons = realloc(icon->icons, sizeof (char *) * (icon->icons_count + 1));
        icon->icons[icon->icons_count] = eina_stringshare_add(entry->path);
        eina_array_push(strs, icon->icons[icon->icons_count++]);

        *changed = EINA_TRUE;
    }

    eina_iterator_free(it);

    return EINA_TRUE;
}

static Eina_Bool
cache_fallback_scan(Eina_Hash *icons, Eina_Hash *dirs, Eina_Bool *changed)
{
    Eina_List *xdg_dirs, *l;
    const char *dir;
    char path[PATH_MAX];

    cache_fallback_scan_dir(icons, dirs, efreet_icon_deprecated_user_dir_get(), changed);
    cache_fallback_scan_dir(icons, dirs, efreet_icon_user_dir_get(), changed);

    xdg_dirs = efreet_data_dirs_get();
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/icons", dir);
        cache_fallback_scan_dir(icons, dirs, path, changed);
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/pixmaps", dir);
        cache_fallback_scan_dir(icons, dirs, path, changed);
    }
#endif

    cache_fallback_scan_dir(icons, dirs, "/usr/share/pixmaps", changed);

    return EINA_TRUE;
}

static Eina_Bool
cache_scan_path_dir(Efreet_Icon_Theme *theme,
                    const char *path,
                    Efreet_Icon_Theme_Directory *dir,
                    Eina_Hash *icons,
                    Eina_Hash *dirs,
                    Eina_Bool *changed)
{
    Eina_Iterator *it;
    char buf[PATH_MAX];
    Eina_File_Direct_Info *entry;

    snprintf(buf, sizeof(buf), "%s/%s", path, dir->name);

    if (!cache_directory_find(dirs, buf))
        return EINA_TRUE;

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
        if (!ext || !_cache_extention_lookup(ext))
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
                    break ;

            if (j != icon->icons[i]->paths_count)
                continue ;
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

        *changed = EINA_TRUE;
    }

    eina_iterator_free(it);

    return EINA_TRUE;
}

static Eina_Bool
cache_scan_path(Efreet_Icon_Theme *theme, Eina_Hash *icons, Eina_Hash *dirs, const char *path, Eina_Bool *changed)
{
    Eina_List *l;
    Efreet_Icon_Theme_Directory *dir;

    EINA_LIST_FOREACH(theme->directories, l, dir)
        if (!cache_scan_path_dir(theme, path, dir, icons, dirs, changed)) return EINA_FALSE;

    return EINA_TRUE;
}

static Eina_Bool
cache_scan(Efreet_Icon_Theme *theme, Eina_Hash *themes, Eina_Hash *icons, Eina_Hash *dirs, Eina_Bool *changed)
{
    Eina_List *l;
    const char *path;
    const char *name;

    if (!theme) return EINA_TRUE;
    if (eina_hash_find(themes, theme->name.internal)) return EINA_TRUE;
    eina_hash_direct_add(themes, theme->name.internal, theme);

    /* scan theme */
    EINA_LIST_FOREACH(theme->paths, l, path)
        if (!cache_scan_path(theme, icons, dirs, path, changed)) return EINA_FALSE;

    /* scan inherits */
    if (theme->inherits)
    {
        EINA_LIST_FOREACH(theme->inherits, l, name)
        {
            Efreet_Icon_Theme *inherit;

            inherit = eina_hash_find(icon_themes, name);
            if (!inherit) fprintf(stderr, "Theme `%s` not found for `%s`.\n",
                                  name, theme->name.internal);
            if (!cache_scan(inherit, themes, icons, dirs, changed)) return EINA_FALSE;
        }
    }
    else if (strcmp(theme->name.internal, "hicolor"))
    {
        theme = eina_hash_find(icon_themes, "hicolor");
        if (!cache_scan(theme, themes, icons, dirs, changed)) return EINA_FALSE;
    }

    return EINA_TRUE;
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

static void
icon_theme_index_read(Efreet_Icon_Theme *theme, const char *path)
{
    /* TODO: return error value */
    Efreet_Ini *ini;
    Efreet_Icon_Theme_Directory *dir;
    const char *tmp;

    if (!theme || !path) return;

    ini = efreet_ini_new(path);
    if (!ini) return;
    if (!ini->data)
    {
        efreet_ini_free(ini);
        return;
    }

    efreet_ini_section_set(ini, "Icon Theme");
    tmp = efreet_ini_localestring_get(ini, "Name");
    if (tmp) theme->name.name = eina_stringshare_add(tmp);

    tmp = efreet_ini_localestring_get(ini, "Comment");
    if (tmp) theme->comment = eina_stringshare_add(tmp);

    tmp = efreet_ini_string_get(ini, "Example");
    if (tmp) theme->example_icon = eina_stringshare_add(tmp);

    theme->hidden = efreet_ini_boolean_get(ini, "Hidden");

    theme->valid = 1;

    /* Check the inheritance. If there is none we inherit from the hicolor theme */
    tmp = efreet_ini_string_get(ini, "Inherits");
    if (tmp)
    {
        char *t, *s, *p;
        size_t len;

        len = strlen(tmp) + 1;
        t = alloca(len);
        memcpy(t, tmp, len);
        s = t;
        p = strchr(s, ',');

        while (p)
        {
            *p = '\0';

            theme->inherits = eina_list_append(theme->inherits, eina_stringshare_add(s));
            s = ++p;
            p = strchr(s, ',');
        }
        theme->inherits = eina_list_append(theme->inherits, eina_stringshare_add(s));
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
            theme->directories = eina_list_append(theme->directories, dir);

            if (p) s = ++p;
        }
    }

error:
    efreet_ini_free(ini);
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
        Efreet_Icon_Theme *theme;
        const char *name;
        const char *path;
        char buf[PATH_MAX];

        if ((entry->type != EINA_FILE_DIR) &&
            (entry->type != EINA_FILE_LNK))
            continue;

        name = entry->path + entry->name_start;
        theme = eina_hash_find(icon_themes, name);

        if (!theme)
        {
            theme = NEW(Efreet_Icon_Theme, 1);
            theme->name.internal = eina_stringshare_add(name);
            eina_hash_direct_add(icon_themes,
                          (void *)theme->name.internal, theme);
        }

        path = eina_stringshare_add(entry->path);
        if (!eina_list_data_find(theme->paths, path))
            theme->paths = eina_list_append(theme->paths, path);
        else
            eina_stringshare_del(path);

        theme->last_cache_check = ecore_time_unix_get();
        /* we're already valid so no reason to check for an index.theme file */
        if (theme->valid) continue;

        /* if the index.theme file exists we parse it into the theme */
        memcpy(buf, entry->path, entry->path_length);
        memcpy(buf + entry->path_length, "/index.theme", sizeof("/index.theme"));
        if (ecore_file_exists(buf))
            icon_theme_index_read(theme, buf);
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

    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(lockfd, F_SETLK, &fl) < 0)
    {
        if (verbose) printf("LOCKED! You may want to delete %s if this persists\n", file);
        close(lockfd);
        return -1;
    }

    return lockfd;
}

static void
icon_theme_free(Efreet_Icon_Theme *theme)
{
    void *data;
    Efreet_Icon_Theme_Directory *dir;

    EINA_LIST_FREE(theme->paths, data)
        eina_stringshare_del(data);
    EINA_LIST_FREE(theme->inherits, data)
        eina_stringshare_del(data);
    EINA_LIST_FREE(theme->directories, dir)
    {
        eina_stringshare_del(dir->name);
        free(dir);
    }
    free(theme);
}

int
main(int argc, char **argv)
{
    /* TODO:
     * - Add file monitor on files, so that we catch changes on files
     *   during whilst this program runs.
     * - Maybe linger for a while to reduce number of cache re-creates.
     * - pass extra dirs to binary, and read them
     * - make sure programs with different extra dirs all work together
     */
    Eina_Iterator *it;
    Efreet_Cache_Version *icon_version;
    Efreet_Cache_Version *theme_version;
    Efreet_Cache_Icons *cache;
    Efreet_Icon_Theme *theme;
    Eet_Data_Descriptor *edd;
    Eet_File *icon_ef;
    Eet_File *theme_ef;
    Eina_List *xdg_dirs = NULL;
    Eina_List *l = NULL;
    char file[PATH_MAX];
    const char *path;
    char *dir = NULL;
    Eina_Bool changed = EINA_FALSE;
    int lockfd = -1;
    int tmpfd = -1;
    int i;

    for (i = 1; i < argc; i++)
    {
        if      (!strcmp(argv[i], "-v")) verbose = 1;
        else if ((!strcmp(argv[i], "-h")) ||
                 (!strcmp(argv[i], "-help")) ||
                 (!strcmp(argv[i], "--h")) ||
                 (!strcmp(argv[i], "--help")))
        {
            printf("Options:\n");
            printf("  -v     Verbose mode\n");
            exit(0);
        }
    }

    /* init external subsystems */
    if (!eina_init()) return -1;
    if (!eet_init()) return -1;
    if (!ecore_init()) return -1;

    efreet_cache_update = 0;
    strs = eina_array_new(32);

    /* FIXME: should be in cache dir maybe */
    /* create homedir */
    snprintf(file, sizeof(file), "%s/efreet", efreet_cache_home_get());
    if (!ecore_file_mkpath(file)) return -1;

    /* lock process, so that we only run one copy of this program */
    lockfd = cache_lock_file();
    if (lockfd == -1) return -1;

    /* create dir for icon cache */
    dir = ecore_file_dir_get(efreet_icon_cache_file());
    if (!ecore_file_mkpath(dir)) goto on_error;
    free(dir);

    /* finish efreet init */
    if (!efreet_init()) goto on_error;

    icon_themes = eina_hash_string_superfast_new(EINA_FREE_CB(icon_theme_free));

    /* open icon file */
    icon_ef = eet_open(efreet_icon_cache_file(), EET_FILE_MODE_READ_WRITE);
    if (!icon_ef) goto on_error_efreet;
    icon_version = eet_data_read(icon_ef, efreet_version_edd(), EFREET_CACHE_VERSION);
    if (icon_version &&
        ((icon_version->major != EFREET_ICON_CACHE_MAJOR) ||
         (icon_version->minor != EFREET_ICON_CACHE_MINOR)))
    {
        // delete old cache
        eet_close(icon_ef);
        if (unlink(efreet_icon_cache_file()) < 0)
        {
            if (errno != ENOENT) goto on_error_efreet;
        }
        icon_ef = eet_open(efreet_icon_cache_file(), EET_FILE_MODE_READ_WRITE);
        if (!icon_ef) goto on_error_efreet;
    }
    if (!icon_version)
        icon_version = NEW(Efreet_Cache_Version, 1);

    icon_version->major = EFREET_ICON_CACHE_MAJOR;
    icon_version->minor = EFREET_ICON_CACHE_MINOR;

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
    edd = efreet_icons_edd(EINA_TRUE);
    it = eina_hash_iterator_data_new(icon_themes);
    EINA_ITERATOR_FOREACH(it, theme)
    {
        Eina_Hash *themes;

        if (!theme->valid || theme->hidden) continue;
#ifndef STRICT_SPEC
        if (!theme->name.name) continue;
#endif

        themes = eina_hash_string_superfast_new(NULL);

        /* read icons from the eet file */
        cache = eet_data_read(icon_ef, edd, theme->name.internal);

        /* No existing cache before, so create it */
        if (!cache)
        {
            cache = NEW(Efreet_Cache_Icons, 1);
            if (!cache) goto on_error_efreet;

            changed = EINA_TRUE;
        }

        if (!cache->icons)
            cache->icons = eina_hash_string_superfast_new(NULL);

        if (!cache->dirs)
            cache->dirs = eina_hash_string_superfast_new(NULL);

        if (cache_scan(theme, themes, cache->icons, cache->dirs, &changed))
        {
            fprintf(stderr, "generated: '%s' %i (%i)\n",
                    theme->name.internal,
                    changed,
                    eina_hash_population(cache->icons));
            if (changed)
                eet_data_write(icon_ef, edd, theme->name.internal, cache, 1);
            changed = EINA_FALSE;
        }

        eina_hash_free(themes);
        efreet_hash_free(cache->icons, EINA_FREE_CB(efreet_cache_icon_free));
        efreet_hash_free(cache->dirs, free);
        free(cache);

        /* TODO: Only write if changed */
        eet_data_write(theme_ef, efreet_icon_theme_edd(), theme->name.internal, theme, 1);
    }
    eina_iterator_free(it);

    edd = efreet_icons_fallback_edd(EINA_TRUE);

    /* read fallback icons from the eet file */
    cache = eet_data_read(icon_ef, edd, EFREET_CACHE_ICON_FALLBACK);

    /* No existing fallback, create it */
    if (!cache)
    {
        cache = NEW(Efreet_Cache_Icons, 1);
        if (!cache) goto on_error_efreet;

        changed = EINA_TRUE;
    }

    if (!cache->icons)
        cache->icons = eina_hash_string_superfast_new(NULL);

    if (!cache->dirs)
        cache->dirs = eina_hash_string_superfast_new(NULL);

    /* Save fallback in the right part */
    if (cache_fallback_scan(cache->icons, cache->dirs, &changed))
    {
        fprintf(stderr, "generated: fallback %i (%i)\n", changed, eina_hash_population(cache->icons));
        if (changed)
            eet_data_write(icon_ef, edd, EFREET_CACHE_ICON_FALLBACK, cache, 1);
    }

    efreet_hash_free(cache->icons, EINA_FREE_CB(efreet_cache_icon_fallback_free));
    efreet_hash_free(cache->dirs, free);
    free(cache);

    eina_hash_free(icon_themes);

    /* save data */
    eet_data_write(icon_ef, efreet_version_edd(), EFREET_CACHE_VERSION, icon_version, 1);
    eet_close(icon_ef);
    free(icon_version);
    eet_data_write(theme_ef, efreet_version_edd(), EFREET_CACHE_VERSION, theme_version, 1);
    eet_close(theme_ef);
    free(theme_version);

    /* touch update file */
    snprintf(file, sizeof(file), "%s/efreet/icon_data.update", efreet_cache_home_get());
    tmpfd = open(file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (tmpfd >= 0)
    {
        write(tmpfd, "a", 1);
        close(tmpfd);
    }

on_error_efreet:
    efreet_shutdown();

on_error:
    close(lockfd);

    while ((path = eina_array_pop(strs)))
        eina_stringshare_del(path);
    eina_array_free(strs);

    ecore_shutdown();
    eet_shutdown();
    eina_shutdown();

    return 0;
}
