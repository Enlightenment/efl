#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#undef alloca
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "Efreet.h"
#include "efreet_private.h"

#ifdef EFREET_MODULE_LOG_DOM
#undef EFREET_MODULE_LOG_DOM
#endif
#define EFREET_MODULE_LOG_DOM _efreet_icon_log_dom

static int _efreet_icon_log_dom = -1;

/* TODO: Scan efreet_extra_icon_dirs for themes */

static const char *efreet_icon_deprecated_user_dir = NULL;
static const char *efreet_icon_user_dir = NULL;
static Eina_Hash *efreet_icon_themes = NULL;
static Eina_List *efreet_icon_extensions = NULL;
static Eina_List *efreet_extra_icon_dirs = NULL;
#ifndef ICON_CACHE
static Eina_Hash *efreet_icon_cache = NULL;
#endif

static Eina_Hash *change_monitors = NULL;

typedef struct Efreet_Icon_Cache Efreet_Icon_Cache;
struct Efreet_Icon_Cache
{
    const char *key;
    const char *path;
    time_t lasttime;
};

static char *efreet_icon_remove_extension(const char *icon);

#ifndef ICON_CACHE
static Efreet_Icon_Theme *efreet_icon_find_theme_check(const char *theme_name);

static const char *efreet_icon_find_fallback(Efreet_Icon_Theme *theme,
                                             const char *icon,
                                             unsigned int size);
static const char *efreet_icon_list_find_fallback(Efreet_Icon_Theme *theme,
                                                  Eina_List *icons,
                                                  unsigned int size);
static const char *efreet_icon_find_helper(Efreet_Icon_Theme *theme,
                                           const char *icon, unsigned int size);
static const char *efreet_icon_list_find_helper(Efreet_Icon_Theme *theme,
                                                Eina_List *icons, unsigned int size);
static const char *efreet_icon_lookup_icon(Efreet_Icon_Theme *theme,
                                           const char *icon_name, unsigned int size);
static const char *efreet_icon_fallback_icon(const char *icon_name);
static const char *efreet_icon_fallback_dir_scan(const char *dir,
                                                 const char *icon_name);

static const char *efreet_icon_lookup_directory(Efreet_Icon_Theme *theme,
                                                Efreet_Icon_Theme_Directory *dir,
                                                const char *icon_name);
static double efreet_icon_directory_size_distance(Efreet_Icon_Theme_Directory *dir,
                                                  unsigned int size);
static int efreet_icon_directory_size_match(Efreet_Icon_Theme_Directory *dir,
                                            unsigned int size);
static const char *efreet_icon_lookup_directory_helper(Efreet_Icon_Theme_Directory *dir,
                                                       const char *path, const char *icon_name);

#endif

static Efreet_Icon *efreet_icon_new(const char *path);
static void efreet_icon_populate(Efreet_Icon *icon, const char *file);

#ifndef ICON_CACHE
static Efreet_Icon_Theme *efreet_icon_theme_new(void);
static void efreet_icon_theme_free(Efreet_Icon_Theme *theme);
static void efreet_icon_theme_dir_scan_all(const char *theme_name);
static void efreet_icon_theme_dir_scan(const char *dir,
                                       const char *theme_name);
static void efreet_icon_theme_path_add(Efreet_Icon_Theme *theme,
                                       const char *path);
static void efreet_icon_theme_index_read(Efreet_Icon_Theme *theme,
                                         const char *path);

static Efreet_Icon_Theme_Directory *efreet_icon_theme_directory_new(Efreet_Ini *ini,
                                                                    const char *name);
static void efreet_icon_theme_directory_free(Efreet_Icon_Theme_Directory *dir);

static void efreet_icon_theme_cache_check(Efreet_Icon_Theme *theme);
static int efreet_icon_theme_cache_check_dir(Efreet_Icon_Theme *theme,
                                             const char *dir);

static void efreet_icon_cache_free(Efreet_Icon_Cache *value);
static const char *efreet_icon_cache_check(Efreet_Icon_Theme *theme, const char *icon, unsigned int size);
static void efreet_icon_cache_add(Efreet_Icon_Theme *theme, const char *icon, unsigned int size, const char *value);
#endif

#ifdef ICON_CACHE
static const char *efreet_cache_icon_lookup_icon(Efreet_Cache_Icon *icon, unsigned int size);
static const char *efreet_cache_icon_list_lookup_icon(Efreet_Icon_Theme *theme, Eina_List *icons, unsigned int size);
static int efreet_cache_icon_size_match(Efreet_Cache_Icon_Element *elem, unsigned int size);
static double efreet_cache_icon_size_distance(Efreet_Cache_Icon_Element *elem, unsigned int size);
static const char *efreet_cache_icon_lookup_path(Efreet_Cache_Icon_Element *elem);
static const char *efreet_cache_icon_lookup_path_path(Efreet_Cache_Icon_Element *elem, const char *path);
static const char *efreet_cache_icon_fallback_lookup_path(Efreet_Cache_Fallback_Icon *icon);
static const char *efreet_cache_icon_fallback_lookup_path_path(Efreet_Cache_Fallback_Icon *icon,
                                                               const char *path);

static void efreet_icon_changes_listen(void);
static void efreet_icon_changes_monitor_add(const char *path);
static void efreet_icon_changes_cb(void *data, Ecore_File_Monitor *em,
                                   Ecore_File_Event event, const char *path);

#endif

/**
 * @internal
 * @return Returns 1 on success or 0 on failure
 * @brief Initializes the icon system
 */
int
efreet_icon_init(void)
{
    const char *default_exts[] = {".png", ".xpm", NULL};
    int i;

    _efreet_icon_log_dom = eina_log_domain_register
        ("efreet_icon", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_icon_log_dom < 0)
        return 0;

    /* setup the default extension list */
    for (i = 0; default_exts[i]; i++)
        efreet_icon_extensions = eina_list_append(efreet_icon_extensions, eina_stringshare_add(default_exts[i]));

#ifndef ICON_CACHE
    efreet_icon_themes = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_icon_theme_free));
    efreet_icon_cache = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_icon_cache_free));
#else
    efreet_icon_themes = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_icon_theme_free));
    efreet_icon_changes_listen();
#endif

    efreet_extra_icon_dirs = NULL;

    return 1;
}

/**
 * @internal
 * @return Returns no value
 * @brief Shuts down the icon system
 */
void
efreet_icon_shutdown(void)
{
    IF_RELEASE(efreet_icon_user_dir);
    IF_RELEASE(efreet_icon_deprecated_user_dir);

    IF_FREE_LIST(efreet_icon_extensions, eina_stringshare_del);
    IF_FREE_HASH(efreet_icon_themes);
    efreet_extra_icon_dirs = eina_list_free(efreet_extra_icon_dirs);

#ifndef ICON_CACHE
    IF_FREE_HASH(efreet_icon_cache);
#endif

    eina_log_domain_unregister(_efreet_icon_log_dom);
    IF_FREE_HASH(change_monitors);
}

/**
 * @return Returns the user icon directory
 * @brief Returns the user icon directory
 */
EAPI const char *
efreet_icon_deprecated_user_dir_get(void)
{
    const char *user;
    char *tmp;
    int len;

    if (efreet_icon_deprecated_user_dir) return efreet_icon_deprecated_user_dir;

    user = efreet_home_dir_get();
    len = strlen(user) + strlen("/.icons") + 1;
    tmp = alloca(len);
    snprintf(tmp, len, "%s/.icons", user);

    efreet_icon_deprecated_user_dir = eina_stringshare_add_length(tmp, len - 1);

    return efreet_icon_deprecated_user_dir;
}

EAPI const char *
efreet_icon_user_dir_get(void)
{
    const char *user;
    char *tmp;
    int len;

    if (efreet_icon_user_dir) return efreet_icon_user_dir;

    user = efreet_data_home_get();
    len = strlen(user) + strlen("/icons") + 1;
    tmp = alloca(len);
    snprintf(tmp, len, "%s/icons", user);

    efreet_icon_user_dir = eina_stringshare_add_length(tmp, len - 1);

    return efreet_icon_user_dir;
}

/**
 * @param ext: The extension to add to the list of checked extensions
 * @return Returns no value.
 * @brief Adds the given extension to the list of possible icon extensions
 */
EAPI void
efreet_icon_extension_add(const char *ext)
{
    Eina_List *l;

    ext = eina_stringshare_add(ext);

    if ((l = eina_list_data_find_list(efreet_icon_extensions, ext)))
    {
        efreet_icon_extensions = eina_list_promote_list(efreet_icon_extensions, l);
        eina_stringshare_del(ext);
    }
    else
        efreet_icon_extensions = eina_list_prepend(efreet_icon_extensions, ext);
}

/**
 * @return Returns a list of strings that are paths to other icon directories
 * @brief Gets the list of all extra directories to look for icons. These
 * directories are used to look for icons after looking in the user icon dir
 * and before looking in standard system directories. The order of search is
 * from first to last directory in this list. the strings in the list should
 * be created with eina_stringshare_add().
 */
EAPI Eina_List **
efreet_icon_extra_list_get(void)
{
    return &efreet_extra_icon_dirs;
}

/**
 * @return Returns a list of Efreet_Icon structs for all the non-hidden icon
 * themes
 * @brief Retrieves all of the non-hidden icon themes available on the system.
 * The returned list must be freed. Do not free the list data.
 */
EAPI Eina_List *
efreet_icon_theme_list_get(void)
{
    Eina_List *list = NULL;
#ifndef ICON_CACHE
    Eina_Iterator *it;
    Efreet_Icon_Theme *theme;

    /* update the list to include all icon themes */
    efreet_icon_theme_dir_scan_all(NULL);

    /* create the list for the user */
    it = eina_hash_iterator_data_new(efreet_icon_themes);
    EINA_ITERATOR_FOREACH(it, theme)
    {
        if (theme->hidden || !theme->valid) continue;
#ifndef STRICT_SPEC
        if (!theme->name.name) continue;
#endif

        list = eina_list_append(list, theme);
    }
    eina_iterator_free(it);
#else
    char **keys;
    int num, i;

    keys = efreet_cache_icon_theme_name_list(&num);
    if (keys)
    {
        for (i = 0; i < num; i++)
        {
            Efreet_Icon_Theme *theme;

            theme = efreet_icon_theme_find(keys[i]);
            list = eina_list_append(list, theme);
        }
        free(keys);
    }
#endif

    return list;
}

/**
 * @param theme_name: The theme to look for
 * @return Returns the icon theme related to the given theme name or NULL if
 * none exists.
 * @brief Tries to get the icon theme structure for the given theme name
 */
EAPI Efreet_Icon_Theme *
efreet_icon_theme_find(const char *theme_name)
{
    Efreet_Icon_Theme *theme;

    if (!theme_name) return NULL;

    theme = eina_hash_find(efreet_icon_themes, theme_name);
    if (!theme)
    {
#ifndef ICON_CACHE
        efreet_icon_theme_dir_scan_all(theme_name);
        theme = eina_hash_find(efreet_icon_themes, theme_name);
#else
        theme = efreet_cache_icon_theme_find(theme_name);
        if (theme) eina_hash_direct_add(efreet_icon_themes, theme->name.internal, theme);
#endif
    }

    return theme;
}

#ifdef ICON_CACHE
/**
 * @internal
 * @brief Clears icon theme cache
 */
void
efreet_icon_themes_flush(void)
{
    IF_FREE_HASH(efreet_icon_themes);
    efreet_icon_themes = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_icon_theme_free));
}
#endif

/**
 * @internal
 * @param icon: The icon name to strip extension
 * @return Extension removed if in list of extensions, else untouched.
 * @brief Removes extension from icon name if in list of extensions.
 */
static char *
efreet_icon_remove_extension(const char *icon)
{
    Eina_List *l;
    char *tmp = NULL, *ext = NULL;

    if (!icon) return NULL;

    tmp = strdup(icon);
    ext = strrchr(tmp, '.');
    if (ext)
    {
        const char *ext2;
        EINA_LIST_FOREACH(efreet_icon_extensions, l, ext2)
        {
            if (!strcmp(ext, ext2))
            {
#ifdef STRICT_SPEC
                WRN("[Efreet]: Requesting an icon with an extension: %s",
                    icon);
#endif
                *ext = '\0';
                break;
            }
        }
    }

    return tmp;
}

#ifndef ICON_CACHE
/**
 * @internal
 * @param theme_name: The icon theme to look for
 * @return Returns the Efreet_Icon_Theme structure representing this theme
 * or a new blank theme if not found
 * @brief Retrieves a theme, or creates a blank one if not found.
 */
static Efreet_Icon_Theme *
efreet_icon_find_theme_check(const char *theme_name)
{
    Efreet_Icon_Theme *theme = NULL;
    if (!theme_name) return NULL;
    theme = efreet_icon_theme_find(theme_name);
    if (!theme)
    {
        theme = efreet_icon_theme_new();
        if (!theme) return NULL;
        theme->name.internal = eina_stringshare_add(theme_name);
        eina_hash_direct_add(efreet_icon_themes, (void *)theme->name.internal, theme);
    }

    return theme;
}
#endif

/**
 * @param theme_name: The icon theme to look for
 * @param icon: The icon to look for
 * @param size; The icon size to look for
 * @return Returns the path to the given icon or NULL if none found
 * @brief Retrives the path to the given icon.
 */
EAPI const char *
efreet_icon_path_find(const char *theme_name, const char *icon, unsigned int size)
{
    const char *value = NULL;
    Efreet_Icon_Theme *theme;

#ifndef ICON_CACHE
    theme = efreet_icon_find_theme_check(theme_name);
#else
    theme = efreet_icon_theme_find(theme_name);
#endif

    if (theme)
    {
        char *tmp;
#ifdef ICON_CACHE
        Efreet_Cache_Icon *cache;
#endif

#ifdef SLOPPY_SPEC
        tmp = efreet_icon_remove_extension(icon);
        if (!tmp) return NULL;
#else
        tmp = icon;
#endif

#ifdef ICON_CACHE
        cache = efreet_cache_icon_find(theme, tmp);
        value = efreet_cache_icon_lookup_icon(cache, size);
        if (!value) INFO("lookup for `%s` failed in theme `%s` with %p.", icon, theme_name, cache);
#else
        value = efreet_icon_find_helper(theme, tmp, size);
#endif

#ifdef SLOPPY_SPEC
        FREE(tmp);
#endif
    }

    /* we didn't find the icon in the theme or in the inherited directories
     * then just look for a non theme icon
     */
    if (!value || (value == NON_EXISTING))
#ifdef ICON_CACHE
    {
        Efreet_Cache_Fallback_Icon *cache;

        cache = efreet_cache_icon_fallback_find(icon);
        value = efreet_cache_icon_fallback_lookup_path(cache);
        if (!value) INFO("lookup for `%s` failed in fallback too with %p.", icon, cache);
    }
#else
    value = efreet_icon_fallback_icon(icon);
#endif

    if (value == NON_EXISTING) value = NULL;
    return value;
}

/**
 * @param theme_name: The icon theme to look for
 * @param icons: List of icons to look for
 * @param size; The icon size to look for
 * @return Returns the path representing first found icon or
 * NULL if none of the icons are found
 * @brief Retrieves all of the information about the first found icon in
 * the list.
 * @note This function will search the given theme for all icons before falling
 * back. This is useful when searching for mimetype icons.
 */
EAPI const char *
efreet_icon_list_find(const char *theme_name, Eina_List *icons,
                      unsigned int size)
{
    Eina_List *l;
    const char *icon = NULL;
    const char *value = NULL;
    char *data;
    Efreet_Icon_Theme *theme;

#ifndef ICON_CACHE
    theme = efreet_icon_find_theme_check(theme_name);
#else
    theme = efreet_icon_theme_find(theme_name);
#endif

    if (theme)
    {
        Eina_List *tmps = NULL;
#ifdef ICON_CACHE
        Eina_List *tmps2 = NULL;
        Efreet_Cache_Icon *cache;
#endif

#ifdef SLOPPY_SPEC
        EINA_LIST_FOREACH(icons, l, icon)
        {
            data = efreet_icon_remove_extension(icon);
            if (!data) return NULL;
            tmps = eina_list_append(tmps, data);
        }
#else
        tmps = icons;
#endif

#ifdef ICON_CACHE
        EINA_LIST_FOREACH(tmps, l, icon)
        {
            cache = efreet_cache_icon_find(theme, icon);
            if (cache)
            {
                /* If the icon is in the asked for theme, return it */
                if (!strcmp(cache->theme, theme->name.internal))
                {
                    value = efreet_cache_icon_lookup_icon(cache, size);
                    break;
                }
                else
                    tmps2 = eina_list_append(tmps2, cache);
            }
        }
        if (tmps2)
        {
            if (!value)
                value = efreet_cache_icon_list_lookup_icon(theme, tmps2, size);
            EINA_LIST_FREE(tmps2, cache)
                efreet_cache_icon_free(cache);
        }
#else
        value = efreet_icon_list_find_helper(theme, tmps, size);
#endif

#ifdef SLOPPY_SPEC
        EINA_LIST_FREE(tmps, data)
            free(data);
#endif
    }

    /* we didn't find the icons in the theme or in the inherited directories
     * then just look for a non theme icon
     */
    if (!value || (value == NON_EXISTING))
    {
#ifdef ICON_CACHE
        Efreet_Cache_Fallback_Icon *cache;
#endif
        EINA_LIST_FOREACH(icons, l, icon)
        {
#ifdef ICON_CACHE
            cache = efreet_cache_icon_fallback_find(icon);
            value = efreet_cache_icon_fallback_lookup_path(cache);
            efreet_cache_icon_fallback_free(cache);
#else
            value = efreet_icon_fallback_icon(icon);
#endif
            if (value && (value != NON_EXISTING))
                break;
        }
    }

    if (value == NON_EXISTING) value = NULL;
    return value;
}

/**
 * @param theme: The icon theme to look for
 * @param icon: The icon to look for
 * @param size: The icon size to look for
 * @return Returns the Efreet_Icon structure representing this icon or NULL
 * if the icon is not found
 * @brief Retrieves all of the information about the given icon.
 */
EAPI Efreet_Icon *
efreet_icon_find(const char *theme_name, const char *icon, unsigned int size)
{
    const char *path;

    path = efreet_icon_path_find(theme_name, icon, size);
    if (path)
    {
        Efreet_Icon *ic;

        ic = efreet_icon_new(path);
        return ic;
    }

    return NULL;
}

#ifndef ICON_CACHE
/**
 * @internal
 * @param theme: The theme to search in
 * @param icon: The icon to search for
 * @param size: The size to search for
 * @return Returns the icon matching the given information or NULL if no
 * icon found
 * @brief Scans inheriting themes for the given icon
 */
static const char *
efreet_icon_find_fallback(Efreet_Icon_Theme *theme,
                          const char *icon, unsigned int size)
{
    Eina_List *l;
    const char *parent = NULL;
    const char *value = NULL;

    if (!theme->valid) return NULL;

    if (theme->inherits)
    {
        EINA_LIST_FOREACH(theme->inherits, l, parent)
        {
            Efreet_Icon_Theme *parent_theme;

            parent_theme = efreet_icon_theme_find(parent);
            if ((!parent_theme) || (parent_theme == theme)) continue;

            value = efreet_icon_find_helper(parent_theme, icon, size);
            if (value && (value != NON_EXISTING)) break;
        }
    }
    /* if this isn't the hicolor theme, and we have no other fallbacks
     * check hicolor */
    else if (strcmp(theme->name.internal, "hicolor"))
    {
        Efreet_Icon_Theme *parent_theme;

        parent_theme = efreet_icon_theme_find("hicolor");
        if (parent_theme)
            value = efreet_icon_find_helper(parent_theme, icon, size);
    }

    return value;
}

/**
 * @internal
 * @param theme: The theme to search in
 * @param icon: The icon to search for
 * @param size: The size to search for
 * @return Returns the icon matching the given information or NULL if no
 * icon found
 * @brief Scans the theme and any inheriting themes for the given icon
 */
static const char *
efreet_icon_find_helper(Efreet_Icon_Theme *theme,
                        const char *icon, unsigned int size)
{
    const char *value;
    static int recurse = 0;

    efreet_icon_theme_cache_check(theme);


    /* limit recursion in finding themes and inherited themes to 256 levels */
    if (recurse > 256) return NULL;
    recurse++;

    /* go no further if this theme is not valid */
    if (!theme->valid)
        value = NULL;
    else
        value = efreet_icon_lookup_icon(theme, icon, size);

    /* we didin't find the image check the inherited themes */
    if (!value || (value == NON_EXISTING))
        value = efreet_icon_find_fallback(theme, icon, size);

    recurse--;
    return value;
}

/**
 * @internal
 * @param theme: The theme to search in
 * @param icons: The icons to search for
 * @param size: The size to search for
 * @return Returns the icon matching the given information or NULL if no
 * icon found
 * @brief Scans inheriting themes for the given icons
 */
static const char *
efreet_icon_list_find_fallback(Efreet_Icon_Theme *theme,
                               Eina_List *icons, unsigned int size)
{
    Eina_List *l;
    const char *parent = NULL;
    const char *value = NULL;

    if (theme->inherits)
    {
        EINA_LIST_FOREACH(theme->inherits, l, parent)
        {
            Efreet_Icon_Theme *parent_theme;

            parent_theme = efreet_icon_theme_find(parent);
            if ((!parent_theme) || (parent_theme == theme)) continue;

            value = efreet_icon_list_find_helper(parent_theme,
                                                 icons, size);
            if (value && (value != NON_EXISTING)) break;
        }
    }

    /* if this isn't the hicolor theme, and we have no other fallbacks
     * check hicolor
     */
    else if (strcmp(theme->name.internal, "hicolor"))
    {
        Efreet_Icon_Theme *parent_theme;

        parent_theme = efreet_icon_theme_find("hicolor");
        if (parent_theme)
            value = efreet_icon_list_find_helper(parent_theme,
                                                 icons, size);
    }

    return value;
}

/**
 * @internal
 * @param theme: The theme to search in
 * @param icons: The icons to search for
 * @param size: The size to search for
 * @return Returns the icon matching the given information or NULL if no
 * icon found
 * @brief Scans the theme and any inheriting themes for the given icons
 */
static const char *
efreet_icon_list_find_helper(Efreet_Icon_Theme *theme,
                             Eina_List *icons, unsigned int size)
{
    Eina_List *l;
    const char *value = NULL;
    const char *icon = NULL;
    static int recurse = 0;

    efreet_icon_theme_cache_check(theme);

    /* go no further if this theme is not valid */
    if (!theme->valid) return NULL;

    /* limit recursion in finding themes and inherited themes to 256 levels */
    if (recurse > 256) return NULL;
    recurse++;

    EINA_LIST_FOREACH(icons, l, icon)
    {
        value = efreet_icon_lookup_icon(theme, icon, size);
        if (value && (value != NON_EXISTING))
            break;
    }

    /* we didn't find the image check the inherited themes */
    if (!value || (value == NON_EXISTING))
        value = efreet_icon_list_find_fallback(theme, icons, size);

    recurse--;
    return value;
}

/**
 * @internal
 * @param theme: The icon theme to look in
 * @param icon_name: The icon name to look for
 * @param size: The icon size to look for
 * @return Returns the path for the theme/icon/size combo or NULL if
 * none found
 * @brief Looks for the @a icon in the @a theme for the @a size given.
 */
static const char *
efreet_icon_lookup_icon(Efreet_Icon_Theme *theme, const char *icon_name,
                        unsigned int size)
{
    Eina_List *l;
    const char *icon = NULL, *tmp = NULL;
    Efreet_Icon_Theme_Directory *dir;
    double minimal_distance = INT_MAX;
    unsigned int ret_size = 0;

    if (!theme || (!theme->paths) || !icon_name || !size)
        return NULL;

    icon = efreet_icon_cache_check(theme, icon_name, size);
    if (icon) return icon;

    /* search for allowed size == requested size */
    EINA_LIST_FOREACH(theme->directories, l, dir)
    {
        if (!efreet_icon_directory_size_match(dir, size)) continue;
        icon = efreet_icon_lookup_directory(theme, dir,
                                            icon_name);
        if (icon)
        {
            efreet_icon_cache_add(theme, icon_name, size, icon);
            return icon;
        }
    }

    /* search for any icon that matches */
    EINA_LIST_FOREACH(theme->directories, l, dir)
    {
        double distance;

        distance = efreet_icon_directory_size_distance(dir, size);
        if (distance > minimal_distance) continue;
        // prefer downsizing
        if ((distance == minimal_distance) && (size < ret_size)) continue;

        tmp = efreet_icon_lookup_directory(theme, dir,
                                           icon_name);
        if (tmp)
        {
            icon = tmp;
            minimal_distance = distance;
            ret_size = size;
        }
    }

    efreet_icon_cache_add(theme, icon_name, size, icon);
    return icon;
}


/**
 * @internal
 * @param theme: The theme to use
 * @param dir: The theme directory to look in
 * @param icon_name: The icon name to look for
 * @return Returns the icon cloest matching the given information or NULL if
 * none found
 * @brief Tries to find the file closest matching the given icon
 */
static const char *
efreet_icon_lookup_directory(Efreet_Icon_Theme *theme,
                             Efreet_Icon_Theme_Directory *dir,
                             const char *icon_name)
{
    Eina_List *l;
    const char *icon = NULL;
    const char *path;
    const char *tmp;

    tmp = eina_stringshare_add(icon_name);

    EINA_LIST_FOREACH(theme->paths, l, path)
    {
        icon = efreet_icon_lookup_directory_helper(dir, path, tmp);
        if (icon) break;
    }

    eina_stringshare_del(tmp);
    return icon;
}

/**
 * @internal
 * @param dir: The theme directory to work with
 * @param size: The size to check
 * @return Returns true if the size matches for the given directory, 0
 * otherwise
 * @brief Checks if the size matches for the given directory or not
 */
static int
efreet_icon_directory_size_match(Efreet_Icon_Theme_Directory *dir,
                                 unsigned int size)
{
    if (dir->type == EFREET_ICON_SIZE_TYPE_FIXED)
        return (dir->size.normal == size);

    if ((dir->type == EFREET_ICON_SIZE_TYPE_SCALABLE) ||
        (dir->type == EFREET_ICON_SIZE_TYPE_THRESHOLD))
        return ((dir->size.min < size) && (size < dir->size.max));

    return 0;
}

/**
 * @internal
 * @param dir: The directory to work with
 * @param size: The size to check for
 * @return Returns the distance this size is away from the desired size
 * @brief Returns the distance the given size is away from the desired size
 */
static double
efreet_icon_directory_size_distance(Efreet_Icon_Theme_Directory *dir,
                                    unsigned int size)
{
    if (dir->type == EFREET_ICON_SIZE_TYPE_FIXED)
        return (abs(dir->size.normal - size));

    if ((dir->type == EFREET_ICON_SIZE_TYPE_SCALABLE) ||
        (dir->type == EFREET_ICON_SIZE_TYPE_THRESHOLD))
    {
#ifdef STRICT_SPEC
        if (size < dir->size.min)
            return (dir->size.min - size);
        if (dir->size.max < size)
            return (size - dir->size.max);
#else
        if (size < dir->size.min)
            return (dir->size.min / (double)size);
        if (dir->size.max < size)
            return (size / (double)dir->size.max);
#endif
        return 0;
    }

    return 0;
}

/**
 * @internal
 * @param icon_name: The icon name to look for
 * @return Returns the Efreet_Icon for the given name or NULL if none found
 * @brief Looks for the un-themed icon in the base directories
 */
static const char *
efreet_icon_fallback_icon(const char *icon_name)
{
    const char *icon;

    if (!icon_name) return NULL;
    icon = efreet_icon_cache_check(NULL, icon_name, 0);
    if (icon) return icon;

    icon = efreet_icon_fallback_dir_scan(efreet_icon_deprecated_user_dir_get(), icon_name);
    if (!icon)
        icon = efreet_icon_fallback_dir_scan(efreet_icon_user_dir_get(), icon_name);
    if (!icon)
    {
        Eina_List *xdg_dirs, *l;
        const char *dir;
        char path[PATH_MAX];

        EINA_LIST_FOREACH(efreet_extra_icon_dirs, l, dir)
        {
            icon = efreet_icon_fallback_dir_scan(dir, icon_name);
            if (icon)
            {
                efreet_icon_cache_add(NULL, icon_name, 0, icon);
                return icon;
            }
        }

        xdg_dirs = efreet_data_dirs_get();

        EINA_LIST_FOREACH(xdg_dirs, l, dir)
        {
            snprintf(path, sizeof(path), "%s/icons", dir);
            icon = efreet_icon_fallback_dir_scan(path, icon_name);
            if (icon)
            {
                efreet_icon_cache_add(NULL, icon_name, 0, icon);
                return icon;
            }
        }

#ifndef STRICT_SPEC
        EINA_LIST_FOREACH(xdg_dirs, l, dir)
        {
            snprintf(path, sizeof(path), "%s/pixmaps", dir);
            icon = efreet_icon_fallback_dir_scan(path, icon_name);
            if (icon)
            {
                efreet_icon_cache_add(NULL, icon_name, 0, icon);
                return icon;
            }
        }
#endif

        icon = efreet_icon_fallback_dir_scan("/usr/share/pixmaps", icon_name);
    }

    efreet_icon_cache_add(NULL, icon_name, 0, icon);
    return icon;
}

/**
 * @internal
 * @param dir: The directory to scan
 * @param icon_name: The icon to look for
 * @return Returns the icon for the given name or NULL on failure
 * @brief Scans the given @a dir for the given @a icon_name returning the
 * Efreet_icon if found, NULL otherwise.
 */
static const char *
efreet_icon_fallback_dir_scan(const char *dir, const char *icon_name)
{
    Eina_List *l;
    const char *icon = NULL;
    char path[PATH_MAX], *ext;
    const char *icon_path[] = { dir, "/", icon_name, NULL };
    size_t size;

    if (!dir || !icon_name) return NULL;

    size = efreet_array_cat(path, sizeof(path), icon_path);
    EINA_LIST_FOREACH(efreet_icon_extensions, l, ext)
    {
        eina_strlcpy(path + size, ext, sizeof(path) - size);

        if (ecore_file_exists(path))
        {
            icon = eina_stringshare_add(path);
            if (icon) break;
        }
        *(path + size) = '\0';
    }
    /* This is to catch non-conforming .desktop files */
#ifdef SLOPPY_SPEC
    if (!icon)
    {
        if ((ecore_file_exists(path))  && (!ecore_file_is_dir(path)))
        {
            icon = eina_stringshare_add(path);
#ifdef STRICT_SPEC
            if (icon)
                WRN("[Efreet]: Found an icon that already has an extension: %s", path);
#endif
        }
    }
#endif

    return icon;
}

/**
 * @internal
 * @param theme: The theme to work with
 * @param dir: The theme directory to work with
 * @param path: The partial path to use
 * @return Returns no value
 * @brief Caches the icons in the given theme directory path at the given
 * size
 */
static const char *
efreet_icon_lookup_directory_helper(Efreet_Icon_Theme_Directory *dir,
                                    const char *path, const char *icon_name)
{
    Eina_List *l;
    const char *icon = NULL;
    char file_path[PATH_MAX];
    const char *ext;
    size_t len;

    /* build "$(path)/$(dir->name)/$(icon_name) */
    len = eina_stringshare_strlen(path);
    memcpy(file_path, path, len);
    file_path[len++] = '/';
    memcpy(file_path + len, dir->name, eina_stringshare_strlen(dir->name));
    len += eina_stringshare_strlen(dir->name);
    file_path[len++] = '/';
    memcpy(file_path + len, icon_name, eina_stringshare_strlen(icon_name));
    len += eina_stringshare_strlen(icon_name);

    EINA_LIST_FOREACH(efreet_icon_extensions, l, ext)
    {
        memcpy(file_path + len, ext, eina_stringshare_strlen(ext) + 1);

        if (ecore_file_exists(file_path))
        {
            icon = eina_stringshare_add(file_path);
            break;
        }
    }
    return icon;
}
#endif

/**
 * @internal
 * @return Returns a new Efreet_Icon struct on success or NULL on failure
 * @brief Creates a new Efreet_Icon struct
 */
static Efreet_Icon *
efreet_icon_new(const char *path)
{
    Efreet_Icon *icon;
    char *p;

    icon = NEW(Efreet_Icon, 1);
    if (!icon) return NULL;
    icon->path = eina_stringshare_add(path);

    /* load the .icon file if it's available */
    p = strrchr(icon->path, '.');
    if (p)
    {
        char ico_path[PATH_MAX];

        *p = '\0';

        snprintf(ico_path, sizeof(ico_path), "%s.icon", icon->path);
        *p = '.';

        if (ecore_file_exists(ico_path))
            efreet_icon_populate(icon, ico_path);
    }

    if (!icon->name)
    {
        const char *file;

        file = ecore_file_file_get(icon->path);
        p = strrchr(icon->path, '.');
        if (p) *p = '\0';
        icon->name = eina_stringshare_add(file);
        if (p) *p = '.';
    }

    return icon;
}

/**
 * @param icon: The Efreet_Icon to cleanup
 * @return Returns no value.
 * @brief Free's the given icon and all its internal data.
 */
EAPI void
efreet_icon_free(Efreet_Icon *icon)
{
    if (!icon) return;

    icon->ref_count --;
    if (icon->ref_count > 0) return;

    IF_RELEASE(icon->path);
    IF_RELEASE(icon->name);
    IF_FREE_LIST(icon->attach_points, free);

    FREE(icon);
}

/**
 * @internal
 * @param icon: The icon to populate
 * @param file: The file to populate from
 * @return Returns no value
 * @brief Tries to populate the icon information from the given file
 */
static void
efreet_icon_populate(Efreet_Icon *icon, const char *file)
{
    Efreet_Ini *ini;
    const char *tmp;

    ini = efreet_ini_new(file);
    if (!ini) return;
    if (!ini->data)
    {
        efreet_ini_free(ini);
        return;
    }

    efreet_ini_section_set(ini, "Icon Data");
    tmp = efreet_ini_localestring_get(ini, "DisplayName");
    if (tmp) icon->name = eina_stringshare_add(tmp);

    tmp = efreet_ini_string_get(ini, "EmbeddedTextRectangle");
    if (tmp)
    {
        int points[4];
        char *t, *s, *p;
        int i;
        size_t len;

        len = strlen(tmp) + 1;
        t = alloca(len);
        memcpy(t, tmp, len);
        s = t;
        for (i = 0; i < 4; i++)
        {
            if (s)
            {
                p = strchr(s, ',');

                if (p) *p = '\0';
                points[i] = atoi(s);

                if (p) s = ++p;
                else s = NULL;
            }
            else
            {
                points[i] = 0;
            }
        }

        icon->has_embedded_text_rectangle = 1;
        icon->embedded_text_rectangle.x0 = points[0];
        icon->embedded_text_rectangle.y0 = points[1];
        icon->embedded_text_rectangle.x1 = points[2];
        icon->embedded_text_rectangle.y1 = points[3];
    }

    tmp = efreet_ini_string_get(ini, "AttachPoints");
    if (tmp)
    {
        char *t, *s, *p;
        size_t len;

        len = strlen(tmp) + 1;
        t = alloca(len);
        memcpy(t, tmp, len);
        s = t;
        while (s)
        {
            Efreet_Icon_Point *point;

            p = strchr(s, ',');
            /* If this happens there is something wrong with the .icon file */
            if (!p) break;

            point = NEW(Efreet_Icon_Point, 1);
            if (!point) goto error;

            *p = '\0';
            point->x = atoi(s);

            s = ++p;
            p = strchr(s, '|');
            if (p) *p = '\0';

            point->y = atoi(s);

            icon->attach_points = eina_list_append(icon->attach_points, point);

            if (p) s = ++p;
            else s = NULL;
        }
    }

error:
    efreet_ini_free(ini);
}

#ifndef ICON_CACHE
/**
 * @internal
 * @return Returns a new Efreet_Icon_Theme on success or NULL on failure
 * @brief Creates a new Efreet_Icon_Theme structure
 *
 * Needs EAPI because of helper binaries
 */
static Efreet_Icon_Theme *
efreet_icon_theme_new(void)
{
    Efreet_Icon_Theme *theme;

    theme = NEW(Efreet_Icon_Theme, 1);
    if (!theme) return NULL;

    return theme;
}

/**
 * @internal
 * @param theme: The theme to free
 * @return Returns no value
 * @brief Frees up the @a theme structure.
 */
static void
efreet_icon_theme_free(Efreet_Icon_Theme *theme)
{
    if (!theme) return;

    IF_RELEASE(theme->name.internal);
    IF_RELEASE(theme->name.name);

    IF_RELEASE(theme->comment);
    IF_RELEASE(theme->example_icon);

    IF_FREE_LIST(theme->paths, eina_stringshare_del);
    IF_FREE_LIST(theme->inherits, eina_stringshare_del);
    IF_FREE_LIST(theme->directories, efreet_icon_theme_directory_free);

    FREE(theme);
}

/**
 * @internal
 * @param theme: The theme to work with
 * @param path: The path to add
 * @return Returns no value
 * @brief This will correctly add the given path to the list of theme paths.
 * @Note Assumes you've already verified that @a path is a valid directory.
 */
static void
efreet_icon_theme_path_add(Efreet_Icon_Theme *theme, const char *path)
{
    if (!theme || !path) return;

    if (!eina_list_search_unsorted(theme->paths, EINA_COMPARE_CB(strcmp), path))
        theme->paths = eina_list_append(theme->paths, eina_stringshare_add(path));
}

/**
 * @internal
 * @return Returns no value
 * @brief This validates that our cache is still valid.
 *
 * This is checked by the following algorithm:
 *   - if we've check less then 5 seconds ago we're good
 *   - if the mtime on the dir is less then our last check time we're good
 *   - otherwise, invalidate the caches
 */
static void
efreet_icon_theme_cache_check(Efreet_Icon_Theme *theme)
{
    Eina_List *l;
    double new_check;

    new_check = ecore_time_get();

    /* we're within 5 seconds of the last time we checked the cache */
    if ((new_check - 5) <= theme->last_cache_check) return;

    if (!theme->valid)
        efreet_icon_theme_dir_scan_all(theme->name.internal);

    else
    {
        char *path;

        EINA_LIST_FOREACH(theme->paths, l, path)
        {
            if (!efreet_icon_theme_cache_check_dir(theme, path))
                break;
        }
    }
    theme->last_cache_check = new_check;
}

/**
 * @internal
 * @param theme: The icon theme to check
 * @param dir: The directory to check
 * @return Returns 1 if the cache is still valid, 0 otherwise
 * @brief This will check if the theme cache is still valid. If it isn't the
 * cache will be invalided and 0 returned.
 */
static int
efreet_icon_theme_cache_check_dir(Efreet_Icon_Theme *theme, const char *dir)
{
    struct stat buf;

    /* have we modified this directory since our last cache check? */
    if (stat(dir, &buf) || (buf.st_mtime > theme->last_cache_check))
    {
        char key[4096];
        char *elem;
        Eina_Iterator *it;
        Eina_List *keys = NULL;
        size_t len;

        snprintf(key, sizeof(key), "%s::", theme->name.internal);
        len = strlen(key);

        it = eina_hash_iterator_key_new(efreet_icon_cache);
        EINA_ITERATOR_FOREACH(it, elem)
            if (!strncmp(elem, key, len))
                keys = eina_list_append(keys, elem);
        eina_iterator_free(it);

        EINA_LIST_FREE(keys, elem)
            eina_hash_del_by_key(efreet_icon_cache, elem);
        return 0;
    }

    return 1;
}

/**
 * @internal
 * @param theme_name: The theme to scan for
 * @return Returns no value
 * @brief Scans the theme directories. If @a theme_name is NULL it will load
 * up all theme data. If @a theme_name is not NULL it will look for that
 * specific theme data
 */
static void
efreet_icon_theme_dir_scan_all(const char *theme_name)
{
    Eina_List *xdg_dirs, *l;
    char path[PATH_MAX], *dir;

    efreet_icon_theme_dir_scan(efreet_icon_deprecated_user_dir_get(), theme_name);
    efreet_icon_theme_dir_scan(efreet_icon_user_dir_get(), theme_name);

    xdg_dirs = efreet_data_dirs_get();
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/icons", dir);
        efreet_icon_theme_dir_scan(path, theme_name);
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/pixmaps", dir);
        efreet_icon_theme_dir_scan(path, theme_name);
    }
#endif

    efreet_icon_theme_dir_scan("/usr/share/pixmaps", theme_name);
}

/**
 * @internal
 * @param search_dir: The directory to scan
 * @param theme_name: Scan for this specific theme, set to NULL to find all
 * themes.
 * @return Returns no value
 * @brief Scans the given directory and adds non-hidden icon themes to the
 * given list. If the theme isn't' in our cache then load the index.theme and
 * add to the cache.
 */
static void
efreet_icon_theme_dir_scan(const char *search_dir, const char *theme_name)
{
    DIR *dirs;
    struct dirent *dir;

    if (!search_dir) return;

    dirs = opendir(search_dir);
    if (!dirs) return;

    while ((dir = readdir(dirs)))
    {
        Efreet_Icon_Theme *theme;
        char path[PATH_MAX];
        const char *key;

        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) continue;

        /* only care if this is a directory or the theme name matches the
         * given name */
        snprintf(path, sizeof(path), "%s/%s", search_dir, dir->d_name);
        if (((theme_name) && (strcmp(theme_name, dir->d_name)))
            || !ecore_file_is_dir(path))
            continue;

        key = eina_stringshare_add(dir->d_name);
        theme = eina_hash_find(efreet_icon_themes, key);

        if (!theme)
        {
            theme = efreet_icon_theme_new();
            if (!theme) goto error;
            theme->name.internal = key;
            eina_hash_direct_add(efreet_icon_themes,
                          (void *)theme->name.internal, theme);
        }
        else
            eina_stringshare_del(key);

        efreet_icon_theme_path_add(theme, path);

        /* we're already valid so no reason to check for an index.theme file */
        if (theme->valid) continue;

        /* if the index.theme file exists we parse it into the theme */
        strncat(path, "/index.theme", sizeof(path));
        if (ecore_file_exists(path))
            efreet_icon_theme_index_read(theme, path);
    }
error:
    closedir(dirs);
}

/**
 * @internal
 * @param theme: The theme to set the values into
 * @param path: The path to the index.theme file for this theme
 * @return Returns no value
 * @brief This will load up the theme with the data in the index.theme file
 */
static void
efreet_icon_theme_index_read(Efreet_Icon_Theme *theme, const char *path)
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

            dir = efreet_icon_theme_directory_new(ini, s);
            if (!dir) goto error;
            theme->directories = eina_list_append(theme->directories, dir);

            if (p) s = ++p;
        }
    }

error:
    efreet_ini_free(ini);
}

/**
 * @internal
 * @param ini: The ini file with information on this directory
 * @param name: The name of the directory
 * @return Returns a new Efreet_Icon_Theme_Directory based on the
 * information in @a ini.
 * @brief Creates and initialises an icon theme directory from the given ini
 * information
 *
 * Needs EAPI because of helper binaries
 */
static Efreet_Icon_Theme_Directory *
efreet_icon_theme_directory_new(Efreet_Ini *ini, const char *name)
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

/**
 * @internal
 * @param dir: The Efreet_Icon_Theme_Directory to free
 * @return Returns no value
 * @brief Frees the given directory @a dir
 */
static void
efreet_icon_theme_directory_free(Efreet_Icon_Theme_Directory *dir)
{
    if (!dir) return;

    IF_RELEASE(dir->name);
    FREE(dir);
}

static void
efreet_icon_cache_free(Efreet_Icon_Cache *value)
{
    if (!value) return;

    IF_RELEASE(value->key);
    IF_RELEASE(value->path);
    free(value);
}

static const char *
efreet_icon_cache_check(Efreet_Icon_Theme *theme, const char *icon, unsigned int size)
{
    Efreet_Icon_Cache *cache;
    char key[4096];
    struct stat st;

    if (theme)
        snprintf(key, sizeof(key), "%s::%s::%d", theme->name.internal, icon, size);
    else
        snprintf(key, sizeof(key), "(null)::%s::%d", icon, size);

    cache = eina_hash_find(efreet_icon_cache, key);
    if (cache)
    {
        if (!cache->path)
            return NON_EXISTING;
        else if (!stat(cache->path, &st) && st.st_mtime == cache->lasttime)
            return cache->path;
        eina_hash_del_by_key(efreet_icon_cache, key);
    }
    return NULL;
}

static void
efreet_icon_cache_add(Efreet_Icon_Theme *theme, const char *icon, unsigned int size, const char *value)
{
    Efreet_Icon_Cache *cache;
    char key[4096];
    struct stat st;

    cache = NEW(Efreet_Icon_Cache, 1);
    if (!cache) return;
    if (theme)
        snprintf(key, sizeof(key), "%s::%s::%d", theme->name.internal, icon, size);
    else
        snprintf(key, sizeof(key), "(null)::%s::%d", icon, size);

    if ((value) && !stat(value, &st))
    {
        cache->path = value;
        cache->lasttime = st.st_mtime;
    }
    else
        cache->lasttime = ecore_time_get();

    eina_hash_set(efreet_icon_cache, key, cache);
}
#endif

#ifdef ICON_CACHE
static const char *
efreet_cache_icon_lookup_icon(Efreet_Cache_Icon *icon, unsigned int size)
{
    const char *path = NULL;
    double minimal_distance = INT_MAX;
    unsigned int ret_size = 0;
    unsigned int i;

    if (!icon) return NULL;

    /* search for allowed size == requested size */
    for (i = 0; i < icon->icons_count; ++i)
    {
        if (!efreet_cache_icon_size_match(icon->icons[i], size)) continue;
        path = efreet_cache_icon_lookup_path(icon->icons[i]);
        if (path) return path;
    }

    /* search for any icon that matches */
    for (i = 0; i < icon->icons_count; ++i)
    {
        const char *tmp = NULL;
        double distance;

        distance = efreet_cache_icon_size_distance(icon->icons[i], size);
        if (distance > minimal_distance) continue;
        // prefer downsizing
        if ((distance == minimal_distance) && (size < ret_size)) continue;

        tmp = efreet_cache_icon_lookup_path(icon->icons[i]);
        if (tmp)
        {
            path = tmp;
            minimal_distance = distance;
            ret_size = size;
        }
    }

    return path;
}

static const char *
efreet_cache_icon_list_lookup_icon(Efreet_Icon_Theme *theme, Eina_List *icons, unsigned int size)
{
    const char *value = NULL;
    Efreet_Cache_Icon *cache;
    Eina_List *l;

    EINA_LIST_FOREACH(icons, l, cache)
    {
        if (!strcmp(cache->theme, theme->name.internal))
        {
            value = efreet_cache_icon_lookup_icon(cache, size);
            if (value) break;
        }
    }
    if (value) return value;
    if (theme->inherits)
    {
        const char *parent;
        EINA_LIST_FOREACH(theme->inherits, l, parent)
        {
            Efreet_Icon_Theme *parent_theme;

            parent_theme = efreet_icon_theme_find(parent);
            if ((!parent_theme) || (parent_theme == theme)) continue;

            value = efreet_cache_icon_list_lookup_icon(parent_theme, icons, size);
            if (value) break;
        }
    }
    /* if this isn't the hicolor theme, and we have no other fallbacks
     * check hicolor */
    else if (strcmp(theme->name.internal, "hicolor"))
    {
        Efreet_Icon_Theme *parent_theme;

        parent_theme = efreet_icon_theme_find("hicolor");
        if (parent_theme)
            value = efreet_cache_icon_list_lookup_icon(parent_theme, icons, size);
    }
    return value;
}

static int
efreet_cache_icon_size_match(Efreet_Cache_Icon_Element *elem, unsigned int size)
{
    if (elem->type == EFREET_ICON_SIZE_TYPE_FIXED)
        return (elem->normal == size);

    if ((elem->type == EFREET_ICON_SIZE_TYPE_SCALABLE) ||
        (elem->type == EFREET_ICON_SIZE_TYPE_THRESHOLD))
        return ((elem->min < size) && (size < elem->max));

    return 0;
}

static double
efreet_cache_icon_size_distance(Efreet_Cache_Icon_Element *elem, unsigned int size)
{
    if (elem->type == EFREET_ICON_SIZE_TYPE_FIXED)
        return (abs(elem->normal - size));

    if ((elem->type == EFREET_ICON_SIZE_TYPE_SCALABLE) ||
        (elem->type == EFREET_ICON_SIZE_TYPE_THRESHOLD))
    {
#ifdef STRICT_SPEC
        if (size < elem->min)
            return (elem->min - size);
        if (elem->max < size)
            return (size - elem->max);
#else
        if (size < elem->min)
            return (elem->min / (double)size);
        if (elem->max < size)
            return (size / (double)elem->max);
#endif
    }

    return 0;
}

static const char *
efreet_cache_icon_lookup_path(Efreet_Cache_Icon_Element *elem)
{
    Eina_List *xdg_dirs, *l;
    const char *path;
    const char *dir;
    char buf[PATH_MAX];

    if (elem->paths_count == 1)
    {
        const char *pp, *ext;

        pp = strrchr(elem->paths[0], '.');
        if (!pp) return NULL;

        EINA_LIST_FOREACH(efreet_icon_extensions, l, ext)
            if (!strcmp(pp, ext))
                return elem->paths[0];
        return NULL;
    }

    path = efreet_cache_icon_lookup_path_path(elem, efreet_icon_deprecated_user_dir_get());
    if (path) return path;

    path = efreet_cache_icon_lookup_path_path(elem, efreet_icon_user_dir_get());
    if (path) return path;

#if 0
    EINA_LIST_FOREACH(efreet_extra_icon_dirs, l, dir)
    {
        path = efreet_cache_icon_lookup_path_path(elem, dir);
        if (path) return path;
    }
#endif

    xdg_dirs = efreet_data_dirs_get();

    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(buf, sizeof(buf), "%s/icons", dir);

        path = efreet_cache_icon_lookup_path_path(elem, buf);
        if (path) return path;
    }

    return NULL;
}

static const char *
efreet_cache_icon_lookup_path_path(Efreet_Cache_Icon_Element *elem, const char *path)
{
    Eina_List *ll;
    const char *ext, *pp;
    unsigned int i;
    int len;

    len = strlen(path);

    for (i = 0; i < elem->paths_count; ++i)
    {
        if (strncmp(path, elem->paths[i], len)) continue;
        pp = strrchr(elem->paths[i], '.');
        if (!pp) continue;

        EINA_LIST_FOREACH(efreet_icon_extensions, ll, ext)
            if (!strcmp(pp, ext))
                return elem->paths[i];
    }

    return NULL;
}

static const char *
efreet_cache_icon_fallback_lookup_path(Efreet_Cache_Fallback_Icon *icon)
{
    const char *path;
    Eina_List *xdg_dirs, *l;
    const char *dir;
    char buf[PATH_MAX];

    if (!icon) return NULL;

    if (icon->icons_count == 1)
    {
        const char *pp, *ext;

        pp = strrchr(icon->icons[0], '.');
        if (!pp) return NULL;

        EINA_LIST_FOREACH(efreet_icon_extensions, l, ext)
            if (!strcmp(pp, ext))
                return icon->icons[0];
        return NULL;
    }

    path = efreet_cache_icon_fallback_lookup_path_path(icon, efreet_icon_deprecated_user_dir_get());
    if (path) return path;

    path = efreet_cache_icon_fallback_lookup_path_path(icon, efreet_icon_user_dir_get());
    if (path) return path;

#if 0
    EINA_LIST_FOREACH(efreet_extra_icon_dirs, l, dir)
    {
        path = efreet_cache_icon_lookup_path_path(elem, dir);
        if (path) return path;
    }
#endif

    xdg_dirs = efreet_data_dirs_get();

    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(buf, sizeof(buf), "%s/icons", dir);

        path = efreet_cache_icon_fallback_lookup_path_path(icon, buf);
        if (path) return path;
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(buf, sizeof(buf), "%s/pixmaps", dir);

        path = efreet_cache_icon_fallback_lookup_path_path(icon, buf);
        if (path) return path;
    }
#endif

    path = efreet_cache_icon_fallback_lookup_path_path(icon, "/usr/share/pixmaps");
    if (path) return path;

    return NULL;
}

static const char *
efreet_cache_icon_fallback_lookup_path_path(Efreet_Cache_Fallback_Icon *icon, const char *path)
{
    Eina_List *ll;
    const char *ext, *pp;
    unsigned int i;
    int len;

    len = strlen(path);

    for (i = 0; i < icon->icons_count; ++i)
    {
        if (strncmp(path, icon->icons[i], len)) continue;

        pp = strrchr(icon->icons[i], '.');
        if (!pp) continue;

        EINA_LIST_FOREACH(efreet_icon_extensions, ll, ext)
            if (!strcmp(pp, ext))
                return icon->icons[i];
    }

    return NULL;
}

static void
efreet_icon_changes_listen(void)
{
    Eina_List *l;
    Eina_List *xdg_dirs;
    char buf[PATH_MAX];
    const char *dir;

    if (!efreet_cache_update) return;

    change_monitors = eina_hash_string_superfast_new(EINA_FREE_CB(ecore_file_monitor_del));
    if (!change_monitors) return;

    efreet_icon_changes_monitor_add(efreet_icon_deprecated_user_dir_get());
    efreet_icon_changes_monitor_add(efreet_icon_user_dir_get());
    EINA_LIST_FOREACH(efreet_extra_icon_dirs, l, dir)
        efreet_icon_changes_monitor_add(dir);

    xdg_dirs = efreet_data_dirs_get();
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(buf, sizeof(buf), "%s/icons", dir);
        efreet_icon_changes_monitor_add(buf);
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(buf, sizeof(buf), "%s/pixmaps", dir);
        efreet_icon_changes_monitor_add(buf);
    }
#endif

    efreet_icon_changes_monitor_add("/usr/share/pixmaps");
}

static void
efreet_icon_changes_monitor_add(const char *path)
{
    char rp[PATH_MAX];

    if (!realpath(path, rp)) return;
    if (!ecore_file_is_dir(rp)) return;
    if (eina_hash_find(change_monitors, rp)) return;
    eina_hash_add(change_monitors, rp,
                  ecore_file_monitor_add(rp,
                                         efreet_icon_changes_cb,
                                         NULL));

    if (ecore_file_is_dir(rp))
    {
        Eina_Iterator *it;
        const char *ent;

        it = eina_file_ls(rp);
        if (!it) return;
        EINA_ITERATOR_FOREACH(it, ent)
        {
            if (!realpath(ent, rp)) continue;
            if (!ecore_file_is_dir(rp)) continue;
            eina_hash_add(change_monitors, rp,
                          ecore_file_monitor_add(rp,
                                                 efreet_icon_changes_cb,
                                                 NULL));
        }
        eina_iterator_free(it);
    }
}

static void
efreet_icon_changes_cb(void *data __UNUSED__, Ecore_File_Monitor *em __UNUSED__,
                       Ecore_File_Event event, const char *path)
{
    switch (event)
    {
        case ECORE_FILE_EVENT_NONE:
            /* noop */
            break;

        case ECORE_FILE_EVENT_CREATED_FILE:
        case ECORE_FILE_EVENT_DELETED_FILE:
        case ECORE_FILE_EVENT_MODIFIED:
        case ECORE_FILE_EVENT_DELETED_DIRECTORY:
        case ECORE_FILE_EVENT_CREATED_DIRECTORY:
            efreet_cache_icon_update();
            break;

        case ECORE_FILE_EVENT_DELETED_SELF:
            eina_hash_del_by_key(change_monitors, path);
            efreet_cache_icon_update();
            break;
    }
}
#endif
