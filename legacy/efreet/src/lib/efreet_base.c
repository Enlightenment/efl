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

#include <unistd.h>

/* define macros and variable for using the eina logging system  */
#define EFREET_MODULE_LOG_DOM _efreet_base_log_dom
static int _efreet_base_log_dom = -1;

#include "Efreet.h"
#include "efreet_private.h"

static Efreet_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Efreet_Version *efreet_version = &_version;

#ifdef _WIN32
# define EFREET_PATH_SEP ';'
#else
# define EFREET_PATH_SEP ':'
#endif

static const char *efreet_home_dir = NULL;
static const char *xdg_data_home = NULL;
static const char *xdg_config_home = NULL;
static const char *xdg_cache_home = NULL;
static Eina_List  *xdg_data_dirs = NULL;
static Eina_List  *xdg_config_dirs = NULL;
static const char *hostname = NULL;

static const char *efreet_dir_get(const char *key, const char *fallback);
static Eina_List  *efreet_dirs_get(const char *key,
                                        const char *fallback);

/**
 * @internal
 * @return Returns 1 on success or 0 on failure
 * @brief Initializes the efreet base settings
 */
int
efreet_base_init(void)
{
    _efreet_base_log_dom = eina_log_domain_register
      ("efreet_base", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_base_log_dom < 0)
    {
        EINA_LOG_ERR("Efreet: Could not create a log domain for efreet_base.\n");
        return 0;
    }
    return 1;
}

/**
 * @internal
 * @return Returns no value
 * @brief Cleans up the efreet base settings system
 */
void
efreet_base_shutdown(void)
{
    IF_RELEASE(efreet_home_dir);
    IF_RELEASE(xdg_data_home);
    IF_RELEASE(xdg_config_home);
    IF_RELEASE(xdg_cache_home);

    IF_FREE_LIST(xdg_data_dirs, eina_stringshare_del);
    IF_FREE_LIST(xdg_config_dirs, eina_stringshare_del);

    IF_RELEASE(hostname);

    eina_log_domain_unregister(_efreet_base_log_dom);
    _efreet_base_log_dom = -1;
}

/**
 * @internal
 * @return Returns the users home directory
 * @brief Gets the users home directory and returns it.
 */
const char *
efreet_home_dir_get(void)
{
    if (efreet_home_dir) return efreet_home_dir;

    efreet_home_dir = getenv("HOME");
#ifdef _WIN32
    if (!efreet_home_dir || efreet_home_dir[0] == '\0')
        efreet_home_dir = getenv("USERPROFILE");
#endif
    if (!efreet_home_dir || efreet_home_dir[0] == '\0')
        efreet_home_dir = "/tmp";

    efreet_home_dir = eina_stringshare_add(efreet_home_dir);

    return efreet_home_dir;
}

EAPI const char *
efreet_data_home_get(void)
{
    if (xdg_data_home) return xdg_data_home;
    xdg_data_home = efreet_dir_get("XDG_DATA_HOME", "/.local/share");
    return xdg_data_home;
}

EAPI Eina_List *
efreet_data_dirs_get(void)
{
#ifdef _WIN32
    char buf[4096];
#endif

    if (xdg_data_dirs) return xdg_data_dirs;

#ifdef _WIN32
    snprintf(buf, 4096, "%s\\Efl;" PACKAGE_DATA_DIR ";/usr/share;/usr/local/share", getenv("APPDATA"));
    xdg_data_dirs = efreet_dirs_get("XDG_DATA_DIRS", buf);
#else
    xdg_data_dirs = efreet_dirs_get("XDG_DATA_DIRS",
                            PACKAGE_DATA_DIR ":/usr/share:/usr/local/share");
#endif
    return xdg_data_dirs;
}

EAPI const char *
efreet_config_home_get(void)
{
    if (xdg_config_home) return xdg_config_home;
    xdg_config_home = efreet_dir_get("XDG_CONFIG_HOME", "/.config");
    return xdg_config_home;
}

EAPI Eina_List *
efreet_config_dirs_get(void)
{
    if (xdg_config_dirs) return xdg_config_dirs;
    xdg_config_dirs = efreet_dirs_get("XDG_CONFIG_DIRS", "/etc/xdg");
    return xdg_config_dirs;
}

EAPI const char *
efreet_cache_home_get(void)
{
    if (xdg_cache_home) return xdg_cache_home;
    xdg_cache_home = efreet_dir_get("XDG_CACHE_HOME", "/.cache");
    return xdg_cache_home;
}

EAPI const char *
efreet_hostname_get(void)
{
    char buf[256];

    if (hostname) return hostname;
    if (gethostname(buf, sizeof(buf)) < 0)
        hostname = eina_stringshare_add("");
    else
        hostname = eina_stringshare_add(buf);
    return hostname;
}

/**
 * @internal
 * @param key The environemnt key to lookup
 * @param fallback The fallback value to use
 * @return Returns the directory related to the given key or the fallback
 * @brief This trys to determine the correct directory name given the
 * environment key @a key and fallbacks @a fallback.
 */
static const char *
efreet_dir_get(const char *key, const char *fallback)
{
    char *dir;
    const char *t;

    dir = getenv(key);
    if (!dir || dir[0] == '\0')
    {
        int len;
        const char *user;

        user = efreet_home_dir_get();
        len = strlen(user) + strlen(fallback) + 1;
        dir = malloc(len);
        if (!dir) return NULL;
        snprintf(dir, len, "%s%s", user, fallback);

        t = eina_stringshare_add(dir);
        FREE(dir);
    }
    else t = eina_stringshare_add(dir);

    return t;
}

/**
 * @internal
 * @param key The environment key to lookup
 * @param fallback The fallback value to use
 * @return Returns a list of directories specified by the given key @a key
 * or from the list of fallbacks in @a fallback.
 * @brief Creates a list of directories as given in the environment key @a
 * key or from the fallbacks in @a fallback
 */
static Eina_List *
efreet_dirs_get(const char *key, const char *fallback)
{
    Eina_List *dirs = NULL;
    const char *path;
    char *tmp, *s, *p;
    char ts[PATH_MAX];
    size_t len;

    path = getenv(key);
    if (!path || (path[0] == '\0')) path = fallback;

    if (!path) return dirs;

    len = strlen(path) + 1;
    tmp = alloca(len);
    memcpy(tmp, path, len);
    s = tmp;
    p = strchr(s, EFREET_PATH_SEP);
    while (p)
    {
        *p = '\0';
        if (!eina_list_search_unsorted(dirs, EINA_COMPARE_CB(strcmp), s))
        {
            // resolve path properly/fully to remove path//path2 to
            // path/path2, path/./path2 to path/path2 etc.
            if (realpath(s, ts))
                dirs = eina_list_append(dirs, (void *)eina_stringshare_add(ts));
        }

        s = ++p;
        p = strchr(s, EFREET_PATH_SEP);
    }
    if (!eina_list_search_unsorted(dirs, EINA_COMPARE_CB(strcmp), s))
    {
        // resolve path properly/fully to remove path//path2 to
        // path/path2, path/./path2 to path/path2 etc.
        if (realpath(s, ts))
            dirs = eina_list_append(dirs, (void *)eina_stringshare_add(ts));
    }

    return dirs;
}
