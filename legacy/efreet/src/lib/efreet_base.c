/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

static const char *efreet_home_dir = NULL;
static const char *xdg_data_home = NULL;
static const char *xdg_config_home = NULL;
static const char *xdg_cache_home = NULL;
static Eina_List  *xdg_data_dirs = NULL;
static Eina_List  *xdg_config_dirs = NULL;

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
    if (!eina_stringshare_init()) return 0;

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

    eina_stringshare_shutdown();
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
    if (!efreet_home_dir || efreet_home_dir[0] == '\0')
        efreet_home_dir = "/tmp";

    efreet_home_dir = eina_stringshare_add(efreet_home_dir);

    return efreet_home_dir;
}

/**
 * @return Returns the XDG Data Home directory
 * @brief Retrieves the XDG Data Home directory
 */
EAPI const char *
efreet_data_home_get(void)
{
    if (xdg_data_home) return xdg_data_home;
    xdg_data_home = efreet_dir_get("XDG_DATA_HOME", "/.local/share");
    return xdg_data_home;
}

/**
 * @return Returns the Eina_List of preference ordered extra data directories
 * @brief Returns the Eina_List of prefernece oredred extra data
 * directories
 *
 * @note The returned list is static inside Efreet. If you add/remove from the
 * list then the next call to efreet_data_dirs_get() will return your
 * modified values. DO NOT free this list.
 */
EAPI Eina_List *
efreet_data_dirs_get(void)
{
    if (xdg_data_dirs) return xdg_data_dirs;
    xdg_data_dirs = efreet_dirs_get("XDG_DATA_DIRS",
                            "/usr/local/share:/usr/share");
    return xdg_data_dirs;
}

/**
 * @return Returns the XDG Config Home directory
 * @brief Retrieves the XDG Config Home directory
 */
EAPI const char *
efreet_config_home_get(void)
{
    if (xdg_config_home) return xdg_config_home;
    xdg_config_home = efreet_dir_get("XDG_CONFIG_HOME", "/.config");
    return xdg_config_home;
}

/**
 * @return Returns the Eina_List of preference ordered extra config directories
 * @brief Returns the Eina_List of prefernece oredred extra config
 * directories
 *
 * @note The returned list is static inside Efreet. If you add/remove from the
 * list then the next call to efreet_config_dirs_get() will return your
 * modified values. DO NOT free this list.
 */
EAPI Eina_List *
efreet_config_dirs_get(void)
{
    if (xdg_config_dirs) return xdg_config_dirs;
    xdg_config_dirs = efreet_dirs_get("XDG_CONFIG_DIRS", "/etc/xdg");
    return xdg_config_dirs;
}

/**
 * @return Returns the XDG Cache Home directory
 * @brief Retrieves the XDG Cache Home directory
 */
EAPI const char *
efreet_cache_home_get(void)
{
    if (xdg_cache_home) return xdg_cache_home;
    xdg_cache_home = efreet_dir_get("XDG_CACHE_HOME", "/.cache");
    return xdg_cache_home;
}

/**
 * @internal
 * @param key: The environemnt key to lookup
 * @param fallback: The fallback value to use
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
        dir = malloc(sizeof(char) * len);
        snprintf(dir, len, "%s%s", user, fallback);

        t = eina_stringshare_add(dir);
        FREE(dir);
    }
    else t = eina_stringshare_add(dir);

    return t;
}

/**
 * @internal
 * @param key: The environment key to lookup
 * @param fallback: The fallback value to use
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

    path = getenv(key);
    if (!path || (path[0] == '\0')) path = fallback;

    if (!path) return dirs;

    tmp = strdup(path);
    s = tmp;
    p = strchr(s, ':');
    while (p)
    {
        *p = '\0';
        if (!eina_list_search_unsorted(dirs, EINA_COMPARE_CB(strcmp), s))
            dirs = eina_list_append(dirs, (void *)eina_stringshare_add(s));

        s = ++p;
        p = strchr(s, ':');
    }
    if (!eina_list_search_unsorted(dirs, EINA_COMPARE_CB(strcmp), s))
      dirs = eina_list_append(dirs, (void *)eina_stringshare_add(s));
    FREE(tmp);

    return dirs;
}
