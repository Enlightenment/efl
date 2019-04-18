#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <pwd.h>

#include <Eina.h>

#include "eina_internal.h"
#include "eina_private.h"

void
eina_xdg_env_init(void)
{
   char home[PATH_MAX], *s;
   Eina_Vpath_Interface_User user;

   eina_vpath_resolve_snprintf(home, sizeof(home), "(:home:)/");
   memset(&user, 0, sizeof(Eina_Vpath_Interface_User));

#define FATAL_SNPRINTF(_buf, _err, _fmt, ...) \
   do { \
      if ((size_t)snprintf(_buf, sizeof(_buf), _fmt, ## __VA_ARGS__) >= (sizeof(_buf) - 1)) { \
         fprintf(stderr, _err"\n", _buf); \
         abort(); \
      } \
   } while (0)

#ifdef _WIN32
# define ENV_SET(_env, _dir, _meta) \
   char _meta[PATH_MAX + 128]; \
   if (_env) { \
      s = getenv(_env); \
      if (!s) s = home; \
   } else s = home; \
   if (_dir) FATAL_SNPRINTF(_meta, "vpath string '%s' truncated - fatal", "%s\\%s", s, (char *)_dir); \
   else FATAL_SNPRINTF(_meta, "vpath string '%s' truncated - fatal", "%s\\", s); \
   (&user)->_meta = _meta;

   ENV_SET(NULL, "Desktop", desktop);
   ENV_SET(NULL, "Documents", documents);
   ENV_SET(NULL, "Downloads", downloads);
   ENV_SET(NULL, "Music", music);
   ENV_SET(NULL, "Pictures", pictures);
   ENV_SET("CommonProgramFiles", NULL, pub);
   ENV_SET("APPDATA", "Microsoft\\Windows\\Templates", templates);
   ENV_SET(NULL, "Videos", videos);
   ENV_SET("LOCALAPPDATA", NULL, data);
   ENV_SET("LOCALAPPDATA", "Temp", tmp);
   ENV_SET("APPDATA", NULL, config);
   ENV_SET("LOCALAPPDATA", NULL, cache);
   if (!(s = getenv("APPDATA")))
     user.run = NULL;
   else
     user.run = s;
#else /* _WIN32 */
# if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
#  define ENV_HOME_SET(_env, _dir, _meta) \
   char _meta [PATH_MAX + 128]; \
   if ((getuid() != geteuid()) || (!(s = getenv(_env)))) { \
      FATAL_SNPRINTF(_meta, "vpath string '%s' truncated - fatal", "%s/"_dir, home); \
      s = _meta; \
   } \
   (&user)->_meta = s;
# else
#  define ENV_HOME_SET(_env, _dir, _meta) \
   char _meta [PATH_MAX + 128]; \
   if (!(s = getenv(_env))) { \
      FATAL_SNPRINTF(_meta, "vpath string '%s' truncated - fatal", "%s/"_dir, home); \
      s = _meta; \
   } \
   (&user)->_meta = s;
# endif
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
   ENV_HOME_SET("XDG_PUBLICSHARE_DIR", "Public", pub);
   // $XDG_TEMPLATES_DIR="$HOME/.Templates"
   ENV_HOME_SET("XDG_TEMPLATES_DIR", "Templates", templates);
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

# if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if ((getuid() != geteuid()) || (!(s = getenv("XDG_RUNTIME_DIR"))))
# else
   if (!(s = getenv("XDG_RUNTIME_DIR")))
# endif
     user.run = NULL;
   else
     user.run = s;
#endif /* _WIN32 */

   eina_vpath_interface_user_set(&user);
}
