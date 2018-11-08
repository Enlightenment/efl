#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_File.h>

/* define macros and variable for using the eina logging system  */
#define EFREET_MODULE_LOG_DOM _efreet_icon_log_dom
static int _efreet_icon_log_dom = -1;

#include "Efreet.h"
#include "efreet_private.h"

static const char *efreet_icon_deprecated_user_dir = NULL;
static const char *efreet_icon_user_dir = NULL;
static Eina_List *efreet_icon_extensions = NULL;
static Eina_List *efreet_extra_icon_dirs = NULL;

typedef struct Efreet_Icon_Cache Efreet_Icon_Cache;
struct Efreet_Icon_Cache
{
    const char *key;
    const char *path;
    time_t lasttime;
};

static char *efreet_icon_remove_extension(const char *icon);

static Efreet_Icon *efreet_icon_new(const char *path);
static void efreet_icon_populate(Efreet_Icon *icon, const char *file);

static const char *efreet_icon_lookup_icon(Efreet_Cache_Icon *icon, unsigned int size);
static const char *efreet_icon_list_lookup_icon(Efreet_Icon_Theme *theme, Eina_List *icons, unsigned int size);
static int efreet_icon_size_match(Efreet_Cache_Icon_Element *elem, unsigned int size);
static double efreet_icon_size_distance(Efreet_Cache_Icon_Element *elem, unsigned int size);
static const char *efreet_icon_lookup_path(Efreet_Cache_Icon_Element *elem);
static const char *efreet_icon_lookup_path_path(Efreet_Cache_Icon_Element *elem, const char *path);
static const char *efreet_icon_fallback_lookup_path(Efreet_Cache_Fallback_Icon *icon);
static const char *efreet_icon_fallback_lookup_path_path(Efreet_Cache_Fallback_Icon *icon,
                                                               const char *path);
static void efreet_cache_icon_dirs_add_cb(void *data);

/**
 * @internal
 * @return Returns 1 on success or 0 on failure
 * @brief Initializes the icon system
 */
int
efreet_icon_init(void)
{
    const char *default_exts[] = {".png", ".xpm", ".svg", NULL};
    int i;

    _efreet_icon_log_dom = eina_log_domain_register
        ("efreet_icon", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_icon_log_dom < 0)
        return 0;

    /* setup the default extension list */
    for (i = 0; default_exts[i]; i++)
        efreet_icon_extensions = eina_list_append(efreet_icon_extensions, eina_stringshare_add(default_exts[i]));

    efreet_extra_icon_dirs = NULL;
    efreet_icon_extensions_refresh();

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
    efreet_extra_icon_dirs = eina_list_free(efreet_extra_icon_dirs);

    eina_log_domain_unregister(_efreet_icon_log_dom);
    _efreet_icon_log_dom = -1;
}

void
efreet_icon_extensions_refresh(void)
{
   efreet_cache_icon_exts_add(efreet_icon_extensions);
}

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

    tmp = eina_file_path_sanitize(tmp);
    efreet_icon_deprecated_user_dir = eina_stringshare_add_length(tmp, len - 1);
    free(tmp);

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

    tmp = eina_file_path_sanitize(tmp);
    efreet_icon_user_dir = eina_stringshare_add_length(tmp, len - 1);
    free(tmp);

    return efreet_icon_user_dir;
}

EAPI void
efreet_icon_extension_add(const char *ext)
{
    Eina_List *l;

    EINA_SAFETY_ON_NULL_RETURN(ext);

    ext = eina_stringshare_add(ext);

    if ((l = eina_list_data_find_list(efreet_icon_extensions, ext)))
    {
        efreet_icon_extensions = eina_list_promote_list(efreet_icon_extensions, l);
        eina_stringshare_del(ext);
    }
    else
        efreet_icon_extensions = eina_list_prepend(efreet_icon_extensions, ext);
    efreet_icon_extensions_refresh();
}

EAPI Eina_List **
efreet_icon_extra_list_get(void)
{
    ecore_job_add(efreet_cache_icon_dirs_add_cb, NULL);
    return &efreet_extra_icon_dirs;
}

EAPI Eina_List *
efreet_icon_extensions_list_get(void)
{
    return efreet_icon_extensions;
}

EAPI Eina_List *
efreet_icon_theme_list_get(void)
{
    return efreet_cache_icon_theme_list();
}

EAPI Efreet_Icon_Theme *
efreet_icon_theme_find(const char *theme_name)
{
    if (!theme_name) return NULL;

    return efreet_cache_icon_theme_find(theme_name);
}

/**
 * @internal
 * @param icon The icon name to strip extension
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
                WRN("Requesting an icon with an extension: %s",
                    icon);
#endif
                *ext = '\0';
                break;
            }
        }
    }

    return tmp;
}

EAPI const char *
efreet_icon_path_find(const char *theme_name, const char *icon, unsigned int size)
{
#ifdef SLOPPY_SPEC
    char *tmp;
#else
    const char *tmp;
#endif
    const char *value = NULL;
    Efreet_Icon_Theme *theme;

    EINA_SAFETY_ON_NULL_RETURN_VAL(icon, NULL);

    theme = efreet_icon_theme_find(theme_name);

#ifdef SLOPPY_SPEC
    tmp = efreet_icon_remove_extension(icon);
    if (!tmp) return NULL;
#else
    tmp = icon;
#endif

    if (theme)
    {
        Efreet_Cache_Icon *cache;
        cache = efreet_cache_icon_find(theme, tmp);
        value = efreet_icon_lookup_icon(cache, size);
        if (!value) INF("lookup for '%s' failed in theme '%s' with %p.", icon, theme_name, cache);
    }

    /* we didn't find the icon in the theme or in the inherited directories
     * then just look for a non theme icon
     */
    if (!value)
    {
        Efreet_Cache_Fallback_Icon *cache;

        cache = efreet_cache_icon_fallback_find(tmp);
        value = efreet_icon_fallback_lookup_path(cache);
        if (!value) INF("lookup for '%s' failed in fallback too with %p.", icon, cache);
    }

#ifdef SLOPPY_SPEC
    FREE(tmp);
#endif
    return value;
}

EAPI const char *
efreet_icon_list_find(const char *theme_name, Eina_List *icons,
                      unsigned int size)
{
    Eina_List *l;
    Eina_List *tmps = NULL;
    const char *icon = NULL;
    const char *value = NULL;
#ifdef SLOPPY_SPEC
    char *data;
#endif
    Efreet_Icon_Theme *theme;

    EINA_SAFETY_ON_NULL_RETURN_VAL(icons, NULL);

    theme = efreet_icon_theme_find(theme_name);

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

    if (theme)
    {
        Eina_List *tmps2 = NULL;
        Efreet_Cache_Icon *cache;

        EINA_LIST_FOREACH(tmps, l, icon)
        {
            cache = efreet_cache_icon_find(theme, icon);
            if (cache)
            {
                /* If the icon is in the asked for theme, return it */
                if (!strcmp(cache->theme, theme->name.internal))
                {
                    value = efreet_icon_lookup_icon(cache, size);
                    break;
                }
                else
                    tmps2 = eina_list_append(tmps2, cache);
            }
        }
        if (tmps2)
        {
            if (!value)
                value = efreet_icon_list_lookup_icon(theme, tmps2, size);
            eina_list_free(tmps2);
        }
    }

    /* we didn't find the icons in the theme or in the inherited directories
     * then just look for a non theme icon
     */
    if (!value)
    {
        Efreet_Cache_Fallback_Icon *cache;
        EINA_LIST_FOREACH(tmps, l, icon)
        {
            cache = efreet_cache_icon_fallback_find(icon);
            value = efreet_icon_fallback_lookup_path(cache);
            if (value)
                break;
        }
    }

#ifdef SLOPPY_SPEC
    EINA_LIST_FREE(tmps, data)
        free(data);
#endif

    return value;
}

EAPI Efreet_Icon *
efreet_icon_find(const char *theme_name, const char *icon, unsigned int size)
{
    const char *path;

    EINA_SAFETY_ON_NULL_RETURN_VAL(icon, NULL);

    path = efreet_icon_path_find(theme_name, icon, size);
    if (path)
    {
        Efreet_Icon *ic;

        ic = efreet_icon_new(path);
        return ic;
    }

    return NULL;
}

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
 * @param icon The icon to populate
 * @param file The file to populate from
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

static const char *
efreet_icon_lookup_icon(Efreet_Cache_Icon *icon, unsigned int size)
{
    const char *path = NULL;
    double minimal_distance = INT_MAX;
    unsigned int ret_size = 0;
    unsigned int i;

    if (!icon) return NULL;

    /* search for allowed size == requested size */
    for (i = 0; i < icon->icons_count; ++i)
    {
        if (!efreet_icon_size_match(icon->icons[i], size)) continue;
        path = efreet_icon_lookup_path(icon->icons[i]);
        if (path) return path;
    }

    /* search for any icon that matches */
    for (i = 0; i < icon->icons_count; ++i)
    {
        const char *tmp = NULL;
        double distance;

        distance = efreet_icon_size_distance(icon->icons[i], size);
        if (distance > minimal_distance) continue;
        // prefer downsizing
        if ((EINA_DBL_EQ(distance, minimal_distance)) &&
            (icon->icons[i]->normal < ret_size))
         continue;

        tmp = efreet_icon_lookup_path(icon->icons[i]);

        if (tmp)
        {
            path = tmp;
            minimal_distance = distance;
            ret_size = icon->icons[i]->normal;
        }
    }

    return path;
}

static const char *
efreet_icon_list_lookup_icon(Efreet_Icon_Theme *theme, Eina_List *icons, unsigned int size)
{
    const char *value = NULL;
    Efreet_Cache_Icon *cache;
    Eina_List *l;

    EINA_LIST_FOREACH(icons, l, cache)
    {
        if (!strcmp(cache->theme, theme->name.internal))
        {
            value = efreet_icon_lookup_icon(cache, size);
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

            value = efreet_icon_list_lookup_icon(parent_theme, icons, size);
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
            value = efreet_icon_list_lookup_icon(parent_theme, icons, size);
    }
    return value;
}

static int
efreet_icon_size_match(Efreet_Cache_Icon_Element *elem, unsigned int size)
{
    if (elem->type == EFREET_ICON_SIZE_TYPE_FIXED)
        return (elem->normal == size);

    if ((elem->type == EFREET_ICON_SIZE_TYPE_SCALABLE) ||
        (elem->type == EFREET_ICON_SIZE_TYPE_THRESHOLD))
        return ((elem->min < size) && (size < elem->max));

    return 0;
}

static double
efreet_icon_size_distance(Efreet_Cache_Icon_Element *elem, unsigned int size)
{
    if (elem->type == EFREET_ICON_SIZE_TYPE_FIXED)
    {
        if (elem->normal > size) return elem->normal - size;
        else return size - elem->normal;
    }

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
efreet_icon_lookup_path(Efreet_Cache_Icon_Element *elem)
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

    path = efreet_icon_lookup_path_path(elem, efreet_icon_deprecated_user_dir_get());
    if (path) return path;

    path = efreet_icon_lookup_path_path(elem, efreet_icon_user_dir_get());
    if (path) return path;

#if 0
    EINA_LIST_FOREACH(efreet_extra_icon_dirs, l, dir)
    {
        path = efreet_icon_lookup_path_path(elem, dir);
        if (path) return path;
    }
#endif

    xdg_dirs = efreet_data_dirs_get();

    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(buf, sizeof(buf), "%s/icons", dir);

        path = efreet_icon_lookup_path_path(elem, buf);
        if (path) return path;
    }

    return NULL;
}

static const char *
efreet_icon_lookup_path_path(Efreet_Cache_Icon_Element *elem, const char *path)
{
    Eina_List *ll;
    const char *ext, *pp;
    const char *r = NULL;
    unsigned int i;
    int len;

    path = eina_file_path_sanitize(path);
    len = strlen(path);

    for (i = 0; i < elem->paths_count; ++i)
    {
        if (strncmp(path, elem->paths[i], len)) continue;
        pp = strrchr(elem->paths[i], '.');
        if (!pp) continue;

        EINA_LIST_FOREACH(efreet_icon_extensions, ll, ext)
            if (!strcmp(pp, ext))
            {
                r = elem->paths[i];
                break;
            }
        if (r)
          break;
    }

    free((void*) path);

    return r;
}

static const char *
efreet_icon_fallback_lookup_path(Efreet_Cache_Fallback_Icon *icon)
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

    path = efreet_icon_fallback_lookup_path_path(icon, efreet_icon_deprecated_user_dir_get());
    if (path) return path;

    path = efreet_icon_fallback_lookup_path_path(icon, efreet_icon_user_dir_get());
    if (path) return path;

    EINA_LIST_FOREACH(efreet_extra_icon_dirs, l, dir)
    {
        path = efreet_icon_fallback_lookup_path_path(icon, dir);
        if (path) return path;
    }

    xdg_dirs = efreet_data_dirs_get();

    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(buf, sizeof(buf), "%s/icons", dir);

        path = efreet_icon_fallback_lookup_path_path(icon, buf);
        if (path) return path;
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(buf, sizeof(buf), "%s/pixmaps", dir);

        path = efreet_icon_fallback_lookup_path_path(icon, buf);
        if (path) return path;
    }
#endif

    path = efreet_icon_fallback_lookup_path_path(icon, "/usr/share/pixmaps");
    if (path) return path;

    return NULL;
}

static const char *
efreet_icon_fallback_lookup_path_path(Efreet_Cache_Fallback_Icon *icon, const char *path)
{
    Eina_List *ll;
    const char *ext, *pp;
    const char *r = NULL;
    unsigned int i;
    int len;

    path = eina_file_path_sanitize(path);
    len = strlen(path);

    for (i = 0; i < icon->icons_count; ++i)
    {
        if (strncmp(path, icon->icons[i], len)) continue;

        pp = strrchr(icon->icons[i], '.');
        if (!pp) continue;

        EINA_LIST_FOREACH(efreet_icon_extensions, ll, ext)
            if (!strcmp(pp, ext))
            {
                r = icon->icons[i];
                break;
            }
    }

    free((void*) path);

    return r;
}

static void
efreet_cache_icon_dirs_add_cb(void *data EINA_UNUSED)
{
    efreet_cache_icon_dirs_add(efreet_extra_icon_dirs);
}
