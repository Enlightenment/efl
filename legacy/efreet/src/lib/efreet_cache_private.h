#ifndef EFREET_CACHE_PRIVATE_H
#define EFREET_CACHE_PRIVATE_H

#define EFREET_DESKTOP_CACHE_MAJOR 1
#define EFREET_DESKTOP_CACHE_MINOR 0
#define EFREET_DESKTOP_UTILS_CACHE_MAJOR 1
#define EFREET_DESKTOP_UTILS_CACHE_MINOR 0

#define EFREET_ICON_CACHE_MAJOR 1
#define EFREET_ICON_CACHE_MINOR 0

#define EFREET_CACHE_VERSION "__efreet//version"
#define EFREET_CACHE_ICON_FALLBACK "__efreet_fallback"
#define EFREET_CACHE_ICON_EXTENSIONS "__efreet//icon_extensions"
#define EFREET_CACHE_ICON_EXTRA_DIRS "__efreet//icon_extra_dirs"
#define EFREET_CACHE_DESKTOP_DIRS "__efreet//desktop_dirs"

EAPI const char *efreet_desktop_util_cache_file(void);
EAPI const char *efreet_desktop_cache_file(void);
EAPI const char *efreet_icon_cache_file(const char *theme);
EAPI const char *efreet_icon_theme_cache_file(void);

EAPI Eet_Data_Descriptor *efreet_version_edd(void);
EAPI Eet_Data_Descriptor *efreet_desktop_edd(void);
EAPI Eet_Data_Descriptor *efreet_hash_array_string_edd(void);
EAPI Eet_Data_Descriptor *efreet_hash_string_edd(void);
EAPI Eet_Data_Descriptor *efreet_array_string_edd(void);
EAPI Eet_Data_Descriptor *efreet_icon_theme_edd(Eina_Bool cache);
EAPI Eet_Data_Descriptor *efreet_icon_edd(void);
EAPI Eet_Data_Descriptor *efreet_icon_fallback_edd(void);

typedef struct _Efreet_Cache_Icon_Theme Efreet_Cache_Icon_Theme;
typedef struct _Efreet_Cache_Directory Efreet_Cache_Directory;
typedef struct _Efreet_Cache_Desktop Efreet_Cache_Desktop;

struct _Efreet_Cache_Icon_Theme
{
    Efreet_Icon_Theme theme;

    long long last_cache_check; /**< Last time the cache was checked */

    Eina_Hash *dirs;            /**< All possible icon paths for this theme */

    const char *path;           /**< path to index.theme */

    Eina_Bool hidden:1;         /**< Should this theme be hidden from users */
    Eina_Bool valid:1;          /**< Have we seen an index for this theme */
    Eina_Bool changed:1;        /**< Changed since last seen */
};

struct _Efreet_Cache_Directory
{
    long long modified_time;
};

struct _Efreet_Cache_Desktop
{
    Efreet_Desktop desktop;

    double check_time; /**< Last time we check for disk modification */
};

#endif
