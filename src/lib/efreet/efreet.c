#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "eina_internal.h"

/* define macros and variable for using the eina logging system  */
#define EFREET_MODULE_LOG_DOM /* no logging in this file */

#include "Efreet.h"
#include "efreet_private.h"
#include "efreet_xml.h"

/*
 * Needs EAPI because of helper binaries
 */
EAPI int efreet_cache_update = 1;

static int _efreet_init_count = 0;
static int efreet_parsed_locale = 0;
static const char *efreet_lang = NULL;
static const char *efreet_lang_country = NULL;
static const char *efreet_lang_modifier = NULL;
static const char *efreet_language = NULL;
static void efreet_parse_locale(void);
static int efreet_parse_locale_setting(const char *env);

#ifndef _WIN32
static uid_t ruid;
static uid_t rgid;
#endif

static void
_efreet_efreet_app_interface_set(void)
{
   char buf[PATH_MAX];
   char *s;
   const char *home;
   Eina_Vpath_Interface_User user;

   home = eina_vpath_resolve("(:home:)/");

   memset(&user, 0, sizeof(Eina_Vpath_Interface_User));

# if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
#  define ENV_HOME_SET(_env, _dir, _meta) \
   if ((getuid() != geteuid()) || (!(s = getenv(_env)))) { \
      snprintf(buf, sizeof(buf), "%s/"_dir, home); \
      s = buf; \
   } \
   (&user)->_meta = s;
#else
#  define ENV_HOME_SET(_env, _dir, _meta) \
   if (!(s = getenv(_env))) { \
      snprintf(buf, sizeof(buf), "%s/"_dir, home); \
      s = buf; \
   } \
   (&user)->_meta = s;
#endif
   // $XDG_DESKTOP_DIR="$HOME/Desktop"
   ENV_HOME_SET("XDG_DESKTOP_DIR", "Desktop", desktop);
   // $XDG_DOCUMENTS_DIR="$HOME/Documents"
   ENV_HOME_SET("XDG_DOCUMENTS_DIR", "Documents", documents);
   // $XDG_DOWNLOAD_DIR="$HOME/Downloads"
   ENV_HOME_SET("XDG_DOWNLOAD_DIR", "Downloads", downloads);
   // $XDG_MUSIC_DIR="$HOME/Music"
   ENV_HOME_SET("XDG_MUSIC_DIR", "Music", music);
   // $XDG_PICTURES_DIR="$HOME/Pictures"
   ENV_HOME_SET("XDG_PICTURES_DIR", "Pictures", pictures);
   // $XDG_PUBLICSHARE_DIR="$HOME/Public"
   ENV_HOME_SET("XDG_PUBLIC_DIR", "Public", pub);
   // $XDG_TEMPLATES_DIR="$HOME/.Templates"
   ENV_HOME_SET("XDG_TEMPLATES_DIR", ".Templates", templates);
   // $XDG_VIDEOS_DIR="$HOME/Videos"
   ENV_HOME_SET("XDG_VIDEOS_DIR", "Videos", videos);
   // $XDG_DATA_HOME defines the base directory relative to which user
   //   specific data files should be stored. If $XDG_DATA_HOME is either
   //   not set or empty, a default equal to $HOME/.local/share should be
   //   used.
   ENV_HOME_SET("XDG_DATA_HOME", ".local/share", data);
   ENV_HOME_SET("XDG_TMP_HOME", ".local/tmp", tmp);
   // $XDG_CONFIG_HOME defines the base directory relative to which user
   //   specific configuration files should be stored. If $XDG_CONFIG_HOME
   //   is either not set or empty, a default equal to $HOME/.config should
   //   be used.
   ENV_HOME_SET("XDG_CONFIG_HOME", ".config", config);
   // $XDG_CACHE_HOME defines the base directory relative to which
   //   user specific non-essential data files should be stored. If
   //   $XDG_CACHE_HOME is either not set or empty, a default equal to
   //   $HOME/.cache should be used.
   ENV_HOME_SET("XDG_CACHE_HOME", ".cache", cache);

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if ((getuid() != geteuid()) || (!(s = getenv("XDG_RUNTIME_DIR"))))
#else
   if (!(s = getenv("XDG_RUNTIME_DIR")))
#endif
     user.run = NULL;
   else
     user.run = s;

   eina_vpath_interface_user_set(&user);
}


EAPI int
efreet_init(void)
{
#ifndef _WIN32
   char *tmp;
#endif

   if (++_efreet_init_count != 1)
     return _efreet_init_count;

#ifndef _WIN32
   /* Find users real uid and gid */
   tmp = getenv("SUDO_UID");
   if (tmp)
     ruid = strtoul(tmp, NULL, 10);
   else
     ruid = getuid();

   tmp = getenv("SUDO_GID");
   if (tmp)
     rgid = strtoul(tmp, NULL, 10);
   else
     rgid = getgid();
#endif

   if (!eina_init())
     return --_efreet_init_count;
   if (!eet_init())
     goto shutdown_eina;
   if (!ecore_init())
     goto shutdown_eet;
   if (!ecore_file_init())
     goto shutdown_ecore;

   //this needs to be early initializied efreet itself is using vpaths
   _efreet_efreet_app_interface_set();

   if (!efreet_base_init())
     goto shutdown_ecore_file;

   if (!efreet_cache_init())
     goto shutdown_efreet_base;

   if (!efreet_xml_init())
     goto shutdown_efreet_cache;

   if (!efreet_icon_init())
     goto shutdown_efreet_xml;

   if (!efreet_ini_init())
     goto shutdown_efreet_icon;

   if (!efreet_desktop_init())
     goto shutdown_efreet_ini;

   if (!efreet_menu_init())
     goto shutdown_efreet_desktop;

   if (!efreet_util_init())
     goto shutdown_efreet_menu;

#ifdef ENABLE_NLS
   bindtextdomain(PACKAGE, LOCALE_DIR);
   bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif

   return _efreet_init_count;

shutdown_efreet_menu:
   efreet_menu_shutdown();
shutdown_efreet_desktop:
   efreet_desktop_shutdown();
shutdown_efreet_ini:
   efreet_ini_shutdown();
shutdown_efreet_icon:
   efreet_icon_shutdown();
shutdown_efreet_xml:
   efreet_xml_shutdown();
shutdown_efreet_cache:
   efreet_cache_shutdown();
shutdown_efreet_base:
   efreet_base_shutdown();
shutdown_ecore_file:
   ecore_file_shutdown();
shutdown_ecore:
   ecore_shutdown();
shutdown_eet:
   eet_shutdown();
shutdown_eina:
   eina_shutdown();

   return --_efreet_init_count;
}

EAPI int
efreet_shutdown(void)
{
   if (_efreet_init_count <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   if (--_efreet_init_count != 0)
     return _efreet_init_count;

   efreet_util_shutdown();
   efreet_menu_shutdown();
   efreet_desktop_shutdown();
   efreet_ini_shutdown();
   efreet_icon_shutdown();
   efreet_xml_shutdown();
   efreet_cache_shutdown();
   efreet_base_shutdown();

   IF_RELEASE(efreet_lang);
   IF_RELEASE(efreet_lang_country);
   IF_RELEASE(efreet_lang_modifier);
   IF_RELEASE(efreet_language);
   efreet_parsed_locale = 0;  /* reset this in case they init efreet again */

   ecore_file_shutdown();
   ecore_shutdown();
   eet_shutdown();
   eina_shutdown();

   return _efreet_init_count;
}

EAPI void
efreet_lang_reset(void)
{
   IF_RELEASE(efreet_lang);
   IF_RELEASE(efreet_lang_country);
   IF_RELEASE(efreet_lang_modifier);
   IF_RELEASE(efreet_language);
   efreet_parsed_locale = 0;  /* reset this in case they init efreet again */

   efreet_dirs_reset();
   efreet_parse_locale();
   efreet_cache_desktop_close();
   efreet_cache_desktop_build();
}

/**
 * @internal
 * @return Returns the current users language setting or NULL if none set
 * @brief Retrieves the current language setting
 */
const char *
efreet_lang_get(void)
{
   if (efreet_parsed_locale) return efreet_lang;

   efreet_parse_locale();
   return efreet_lang;
}

/**
 * @internal
 * @return Returns the current language country setting or NULL if none set
 * @brief Retrieves the current country setting for the current language or
 */
const char *
efreet_lang_country_get(void)
{
   if (efreet_parsed_locale) return efreet_lang_country;

   efreet_parse_locale();
   return efreet_lang_country;
}

/**
 * @internal
 * @return Returns the current language modifier setting or NULL if none
 * set.
 * @brief Retrieves the modifier setting for the language.
 */
const char *
efreet_lang_modifier_get(void)
{
   if (efreet_parsed_locale) return efreet_lang_modifier;

   efreet_parse_locale();
   return efreet_lang_modifier;
}

EAPI const char *
efreet_language_get(void)
{
   if (efreet_parsed_locale) return efreet_language;

   efreet_parse_locale();
   return efreet_language;
}

/**
 * @internal
 * @return Returns no value
 * @brief Parses out the language, country and modifer setting from the
 * LC_MESSAGES environment variable on UNIX. On Windows, retrieve them from
 * the system.
 */
static void
efreet_parse_locale(void)
{
   efreet_parsed_locale = 1;

   if (efreet_parse_locale_setting("LANG"))
     return;

   if (efreet_parse_locale_setting("LC_ALL"))
     return;

   if (efreet_parse_locale_setting("LC_MESSAGES"))
     return;

   efreet_language = eina_stringshare_add("C");
}

/**
 * @internal
 * @param env The environment variable to grab
 * @return Returns 1 if we parsed something of @a env, 0 otherwise
 * @brief Tries to parse the lang settings out of the given environment
 * variable
 *
 * @note @a env is not used on Windows.
 */
static int
efreet_parse_locale_setting(const char *env)
{
#ifdef _WIN32
   char buf_lang[18];
   char buf[9];
   int l1;
   int l2;

   l1 = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME,
                      buf, sizeof(buf));
   if (!l1)
     return 0;

   efreet_lang = eina_stringshare_add(buf);
   memcpy(buf_lang, buf, l1 - 1);
   buf_lang[l1 - 1] = '_';

   l2 = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME,
                      buf, sizeof(buf));
   if (!l2)
     return 0;

   efreet_lang_country = eina_stringshare_add(buf);
   memcpy(buf_lang + l1, buf, l2);

   efreet_language = eina_stringshare_add(buf_lang);

   return 1;

   (void)env;
#else
   int found = 0;
   char *setting;
   char *p;
   size_t len;

   p = getenv(env);
   if (!p) return 0;
   len = strlen(p) + 1;
   setting = alloca(len);
   memcpy(setting, p, len);

   /* pull the modifier off the end */
   p = strrchr(setting, '@');
   if (p)
     {
        *p = '\0';
        efreet_lang_modifier = eina_stringshare_add(p + 1);
        found = 1;
     }

   /* if there is an encoding we ignore it */
   p = strrchr(setting, '.');
   if (p) *p = '\0';

   /* get the country if available */
   p = strrchr(setting, '_');
   if (p)
     {
        *p = '\0';
        efreet_lang_country = eina_stringshare_add(p + 1);
        found = 1;
     }

   if (*setting != '\0')
     {
        efreet_lang = eina_stringshare_add(setting);
        found = 1;
     }

   if (found)
     efreet_language = eina_stringshare_add(getenv(env));
   return found;
#endif
}

/**
 * @internal
 * @param buffer The destination buffer
 * @param size The destination buffer size
 * @param strs The strings to concatenate together
 * @return Returns the size of the string in @a buffer
 * @brief Concatenates the strings in @a strs into the given @a buffer not
 * exceeding the given @a size.
 */
size_t
efreet_array_cat(char *buffer, size_t size, const char *strs[])
{
   int i;
   size_t n;
   for (i = 0, n = 0; n < size && strs[i]; i++)
     {
        n += eina_strlcpy(buffer + n, strs[i], size - n);
     }
   return n;
}

#ifndef _WIN32
EAPI void
efreet_fsetowner(int fd)
{
   struct stat st;

   if (fd < 0) return;
   if (fstat(fd, &st) < 0) return;
   if (st.st_uid == ruid) return;

   if (fchown(fd, ruid, rgid) != 0) return;
}
#else
EAPI void
efreet_fsetowner(int fd EINA_UNUSED)
{
}
#endif

#ifndef _WIN32
EAPI void
efreet_setowner(const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN(path);

   int fd;

   fd = open(path, O_RDONLY);
   if (fd < 0) return;
   efreet_fsetowner(fd);
   close(fd);
}
#else
EAPI void
efreet_setowner(const char *path EINA_UNUSED)
{
}
#endif
