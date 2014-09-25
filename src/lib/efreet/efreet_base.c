/* TODO:
 * XDG_CURRENT_DESKTOP
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>

#ifdef _WIN32
# include <winsock2.h>
#endif

#include <Ecore_File.h>

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
static const char *xdg_download_dir = NULL;
static const char *xdg_templates_dir = NULL;
static const char *xdg_publicshare_dir = NULL;
static const char *xdg_documents_dir = NULL;
static const char *xdg_music_dir = NULL;
static const char *xdg_pictures_dir = NULL;
static const char *xdg_videos_dir = NULL;
static const char *hostname = NULL;

static void efreet_dirs_init(void);
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
    efreet_dirs_init();
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
    IF_RELEASE(xdg_download_dir);
    IF_RELEASE(xdg_templates_dir);
    IF_RELEASE(xdg_publicshare_dir);
    IF_RELEASE(xdg_documents_dir);
    IF_RELEASE(xdg_music_dir);
    IF_RELEASE(xdg_pictures_dir);
    IF_RELEASE(xdg_videos_dir);

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
efreet_download_dir_get(void)
{
    if (xdg_download_dir) return xdg_download_dir;
    xdg_download_dir = efreet_user_dir_get("XDG_DOWNLOAD_DIR", _("Downloads"));
    return xdg_download_dir;
}

EAPI const char *
efreet_templates_dir_get(void)
{
    if (xdg_templates_dir) return xdg_templates_dir;
    xdg_templates_dir = efreet_user_dir_get("XDG_TEMPLATES_DIR",
                                            _("Templates"));
    return xdg_templates_dir;
}

EAPI const char *
efreet_public_share_dir_get(void)
{
    if (xdg_publicshare_dir) return xdg_publicshare_dir;
    xdg_publicshare_dir = efreet_user_dir_get("XDG_PUBLICSHARE_DIR",
                                              _("Public"));
    return xdg_publicshare_dir;
}

EAPI const char *
efreet_documents_dir_get(void)
{
    if (xdg_documents_dir) return xdg_documents_dir;
    xdg_documents_dir = efreet_user_dir_get("XDG_DOCUMENTS_DIR",
                                            _("Documents"));
    return xdg_documents_dir;
}

EAPI const char *
efreet_music_dir_get(void)
{
    if (xdg_music_dir) return xdg_music_dir;
    xdg_music_dir = efreet_user_dir_get("XDG_MUSIC_DIR", _("Music"));
    return xdg_music_dir;
}

EAPI const char *
efreet_pictures_dir_get(void)
{
    if (xdg_pictures_dir) return xdg_pictures_dir;
    xdg_pictures_dir = efreet_user_dir_get("XDG_PICTURES_DIR", _("Pictures"));
    return xdg_pictures_dir;
}

EAPI const char *
efreet_videos_dir_get(void)
{
    if (xdg_videos_dir) return xdg_videos_dir;
    xdg_videos_dir = efreet_user_dir_get("XDG_VIDEOS_DIR", _("Videos"));
    return xdg_videos_dir;
}

EAPI const char *
efreet_data_home_get(void)
{
    return xdg_data_home;
}

EAPI Eina_List *
efreet_data_dirs_get(void)
{
    return xdg_data_dirs;
}

EAPI const char *
efreet_config_home_get(void)
{
    return xdg_config_home;
}

EAPI Eina_List *
efreet_config_dirs_get(void)
{
    return xdg_config_dirs;
}

EAPI const char *
efreet_cache_home_get(void)
{
    return xdg_cache_home;
}

EAPI const char *
efreet_runtime_dir_get(void)
{
    return xdg_runtime_dir;
}

EAPI const char *
efreet_hostname_get(void)
{
    return hostname;
}

/**
 * @internal
 * @param user_dir The user directory to work with
 * @param system_dirs The system directories to work with
 * @param suffix The path suffix to add
 * @return Returns the list of directories
 * @brief Creates the list of directories based on the user
 * dir, system dirs and given suffix.
 *
 * Needs EAPI because of helper binaries
 */
EAPI Eina_List *
efreet_default_dirs_get(const char *user_dir, Eina_List *system_dirs,
                                                    const char *suffix)
{
    const char *xdg_dir;
    char dir[PATH_MAX];
    Eina_List *list = NULL;
    Eina_List *l;

    EINA_SAFETY_ON_NULL_RETURN_VAL(user_dir, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(suffix, NULL);

    snprintf(dir, sizeof(dir), "%s/%s", user_dir, suffix);
    list = eina_list_append(list, eina_stringshare_add(dir));

    EINA_LIST_FOREACH(system_dirs, l, xdg_dir)
    {
        snprintf(dir, sizeof(dir), "%s/%s", xdg_dir, suffix);
        list = eina_list_append(list, eina_stringshare_add(dir));
    }

    return list;
}

void
efreet_dirs_reset(void)
{
    eina_stringshare_replace(&xdg_desktop_dir, NULL);
    eina_stringshare_replace(&xdg_download_dir, NULL);
    eina_stringshare_replace(&xdg_templates_dir, NULL);
    eina_stringshare_replace(&xdg_publicshare_dir, NULL);
    eina_stringshare_replace(&xdg_documents_dir, NULL);
    eina_stringshare_replace(&xdg_music_dir, NULL);
    eina_stringshare_replace(&xdg_pictures_dir, NULL);
    eina_stringshare_replace(&xdg_videos_dir, NULL);
}

static void
efreet_dirs_init(void)
{
    char buf[4096];

    /* efreet_home_dir */
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
    if (getuid() != geteuid())
      {
         struct passwd *pw = getpwent();

         if ((pw) && (pw->pw_dir)) efreet_home_dir = pw->pw_dir;
      }
    else
#endif
      efreet_home_dir = getenv("HOME");

#ifdef _WIN32
    if (!efreet_home_dir || efreet_home_dir[0] == '\0')
      {
         efreet_home_dir = getenv("USERPROFILE");
      }
#endif
    if (!efreet_home_dir || efreet_home_dir[0] == '\0')
      efreet_home_dir = "/tmp";

    efreet_home_dir = eina_stringshare_add(efreet_home_dir);

    /* xdg_<dir>_home */
    xdg_data_home = efreet_dir_get("XDG_DATA_HOME", "/.local/share");
    xdg_config_home = efreet_dir_get("XDG_CONFIG_HOME", "/.config");
    xdg_cache_home = efreet_dir_get("XDG_CACHE_HOME", "/.cache");

    /* xdg_data_dirs */
#ifdef _WIN32
    snprintf(buf, sizeof(buf), "%s\\Efl;" DATA_DIR ";/usr/share;/usr/local/share", getenv("APPDATA"));
    xdg_data_dirs = efreet_dirs_get("XDG_DATA_DIRS", buf);
#else
    xdg_data_dirs = efreet_dirs_get("XDG_DATA_DIRS",
                                    DATA_DIR ":/usr/share:/usr/local/share");
#endif
    /* xdg_config_dirs */
    xdg_config_dirs = efreet_dirs_get("XDG_CONFIG_DIRS", "/etc/xdg");

    /* xdg_runtime_dir */
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");

    /* hostname */
    if (gethostname(buf, sizeof(buf)) < 0)
      hostname = eina_stringshare_add("");
    else
      hostname = eina_stringshare_add(buf);
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
    char *dir = NULL;
    const char *t;

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
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
    const char *path = NULL;
    char *s, *p;
    size_t len;

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
    if (getuid() == geteuid())
#endif
      path = getenv(key);
    if (!path || (path[0] == '\0')) path = fallback;

    if (!path) return dirs;

    len = strlen(path) + 1;
    s = alloca(len);
    memcpy(s, path, len);
    p = strchr(s, EFREET_PATH_SEP);
    while (p)
    {
        *p = '\0';
        if (!eina_list_search_unsorted(dirs, EINA_COMPARE_CB(strcmp), s))
        {
            char *tmp = eina_file_path_sanitize(s);
            if (tmp)
            {
                dirs = eina_list_append(dirs, eina_stringshare_add(tmp));
                free(tmp);
            }
        }

        s = ++p;
        p = strchr(s, EFREET_PATH_SEP);
    }
    if (!eina_list_search_unsorted(dirs, EINA_COMPARE_CB(strcmp), s))
    {
        char *tmp = eina_file_path_sanitize(s);
        if (tmp)
        {
            dirs = eina_list_append(dirs, eina_stringshare_add(tmp));
            free(tmp);
        }
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
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
               if (getuid() == geteuid())
#endif
                {
                    val = getenv(env);
                    if (val) eina_strbuf_append(sb, val);
                }
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
        if (*eq == '"')
        {
            if (++eq >= line->end) continue;
            end = memchr(eq, '"', line->end - eq);
        }
        else
        {
            end = line->end;
            while (isspace(*end)) end--;
        }
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
