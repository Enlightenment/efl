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

#ifdef _WIN32
# include <winsock2.h>
#endif

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
static const char *xdg_runtime_dir = NULL;
static Eina_List  *xdg_data_dirs = NULL;
static Eina_List  *xdg_config_dirs = NULL;
static const char *xdg_desktop_dir = NULL;
static const char *hostname = NULL;

static const char *efreet_dir_get(const char *key, const char *fallback);
static Eina_List  *efreet_dirs_get(const char *key,
                                        const char *fallback);
static const char *efreet_user_dir_get(const char *key, const char *fallback);

/**
 * @internal
 * @return Returns @c 1 on success or @c 0 on failure
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
    IF_RELEASE(xdg_desktop_dir);
    IF_RELEASE(xdg_data_home);
    IF_RELEASE(xdg_config_home);
    IF_RELEASE(xdg_cache_home);
    IF_RELEASE(xdg_runtime_dir);

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
efreet_desktop_dir_get(void)
{
    if (xdg_desktop_dir) return xdg_desktop_dir;
    xdg_desktop_dir = efreet_user_dir_get("XDG_DESKTOP_DIR", _("Desktop"));
    return xdg_desktop_dir;
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
efreet_runtime_dir_get(void)
{
    if (xdg_runtime_dir) return xdg_runtime_dir;
    xdg_runtime_dir = efreet_dir_get("XDG_RUNTIME_DIR", "/tmp");
    return xdg_runtime_dir;
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

void
efreet_dirs_reset(void)
{
    eina_stringshare_replace(&xdg_desktop_dir, NULL);
}

/**
 * @internal
 * @param key The environment key to lookup
 * @param fallback The fallback value to use
 * @return Returns the directory related to the given key or the fallback
 * @brief This tries to determine the correct directory name given the
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
        dir = alloca(len);
        snprintf(dir, len, "%s%s", user, fallback);

        t = eina_stringshare_add(dir);
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

static const char *
efreet_env_expand(const char *in)
{
   Eina_Strbuf *sb;
   const char *ret, *p, *e1 = NULL, *e2 = NULL, *val;
   char *env;

   if (!in) return NULL;
   sb = eina_strbuf_new();
   if (!sb) return NULL;
   
   /* maximum length of any env var is the input string */
   env = alloca(strlen(in) + 1);
   for (p = in; *p; p++)
     {
        if (!e1)
          {
             if (*p == '$') e1 = p + 1;
             else eina_strbuf_append_char(sb, *p);
          }
        else if (!(((*p >= 'a') && (*p <= 'z')) ||
                   ((*p >= 'A') && (*p <= 'Z')) ||
                   ((*p >= '0') && (*p <= '9')) ||
                   (*p == '_')))
          {
             size_t len;
             
             e2 = p;
             len = (size_t)(e2 - e1);
             if (len > 0)
               {
                  memcpy(env, e1, len);
                  env[len] = 0;
                  val = getenv(env);
                  if (val) eina_strbuf_append(sb, val);
               }
             e1 = NULL;
             eina_strbuf_append_char(sb, *p);
          }
     }
   ret = eina_stringshare_add(eina_strbuf_string_get(sb));
   eina_strbuf_free(sb);
   return ret;
}

/**
 * @internal
 * @param key The user-dirs key to lookup
 * @param fallback The fallback value to use
 * @return Returns the directory related to the given key or the fallback
 * @brief This tries to determine the correct directory name given the
 * user-dirs key @a key and fallbacks @a fallback.
 */
static const char *
efreet_user_dir_get(const char *key, const char *fallback)
{
   Eina_File *file = NULL;
   Eina_File_Line *line;
   Eina_Iterator *it = NULL;
   const char *config_home;
   char path[PATH_MAX];
   char *ret = NULL;

   config_home = efreet_config_home_get();
   snprintf(path, sizeof(path), "%s/user-dirs.dirs", config_home);

   file = eina_file_open(path, EINA_FALSE);
   if (!file) goto fallback;
   it = eina_file_map_lines(file);
   if (!it) goto fallback;
   EINA_ITERATOR_FOREACH(it, line)
     {
        const char *eq, *end;

        if (line->length < 3) continue;
        if (line->start[0] == '#') continue;
        if (strncmp(line->start, "XDG", 3)) continue;
        eq = memchr(line->start, '=', line->length);
        if (!eq) continue;
        if (strncmp(key, line->start, eq - line->start)) continue;
        if (++eq >= line->end) continue;
        if (*eq != '"') continue;
        if (++eq >= line->end) continue;
        end = memchr(eq, '"', line->end - eq);
        if (!end) continue;
        ret = alloca(end - eq + 1);
        memcpy(ret, eq, end - eq);
        ret[end - eq] = '\0';
        break;
     }
fallback:
   if (it) eina_iterator_free(it);
   if (file) eina_file_close(file);
   if (!ret)
     {
        const char *home;
        home = efreet_home_dir_get();
        ret = alloca(strlen(home) + strlen(fallback) + 2);
        sprintf(ret, "%s/%s", home, fallback);
     }
   return efreet_env_expand(ret);
}
