#include "config.h"
#include "Efl.h"

#ifdef HAVE_GETPWENT
# include <sys/types.h>
# include <pwd.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define MY_CLASS EFL_VPATH_CORE_CLASS

typedef struct _Efl_Vpath_Core_Data Efl_Vpath_Core_Data;

struct _Efl_Vpath_Core_Data
{
   Eina_Spinlock lock;
   Eina_Hash *meta;
};

static Efl_Vpath_Core *vpath_core = NULL;

EOLIAN static Efl_Object *
_efl_vpath_core_efl_object_constructor(Eo *obj, Efl_Vpath_Core_Data *pd)
{
   char buf[PATH_MAX], bufhome[PATH_MAX];
   const char *home, *s;

   if (vpath_core) return NULL;
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->meta = eina_hash_string_superfast_new
     ((Eina_Free_Cb)eina_stringshare_del);
   eina_spinlock_new(&(pd->lock));

   vpath_core = obj;

   // $HOME / ~/ etc.
   home = eina_environment_home_get();
   if (!home)
     {
        /* Windows does not have getuid(), but home can't be NULL */
#ifdef HAVE_GETUID
        uid_t uid = getuid();
        struct stat st;

        snprintf(bufhome, sizeof(bufhome), "/tmp/%i", (int)uid);
        if (mkdir(bufhome,  S_IRUSR | S_IWUSR | S_IXUSR) < 0)
          {
             if (errno != EEXIST)
               {
                  if (stat("/tmp", &st) == 0) home = "/tmp";
                  else home = "/";
               }
          }
        if (stat(bufhome, &st) == 0) home = bufhome;
        else
          {
             if (stat("/tmp", &st) == 0) home = "/tmp";
             else home = "/";
          }
#else
        home = "/";
#endif
     }
   efl_vpath_core_meta_set(obj, "home", home);
   // tmp dir - system wide
   s = eina_environment_tmp_get();
   efl_vpath_core_meta_set(obj, "tmp", s);

#define ENV_HOME_SET(_env, _dir, _meta) \
   if (!(s = getenv(_env))) { \
        snprintf(buf, sizeof(buf), "%s/"_dir, home); s = buf; \
     } efl_vpath_core_meta_set(obj, _meta, s);
   // $XDG_DATA_HOME defines the base directory relative to which user
   //   specific data files should be stored. If $XDG_DATA_HOME is either
   //   not set or empty, a default equal to $HOME/.local/share should be
   //   used.
   ENV_HOME_SET("XDG_DATA_HOME", ".local/share", "data");
   // $XDG_CONFIG_HOME defines the base directory relative to which user
   //   specific configuration files should be stored. If $XDG_CONFIG_HOME
   //   is either not set or empty, a default equal to $HOME/.config should
   //   be used.
   ENV_HOME_SET("XDG_CONFIG_HOME", ".config", "config");
   // $XDG_CACHE_HOME defines the base directory relative to which
   //   user specific non-essential data files should be stored. If
   //   $XDG_CACHE_HOME is either not set or empty, a default equal to
   //   $HOME/.cache should be used.
   ENV_HOME_SET("XDG_CACHE_HOME", ".cache", "cache");
   // $XDG_RUNTIME_DIR defines the base directory relative to which
   //   user-specific non-essential runtime files and other file objects
   //   (such as sockets, named pipes, ...) should be stored. The
   //   directory MUST be owned by the user, and he MUST be the only one
   //   having read and write access to it. Its Unix access mode MUST
   //   be 0700.
   if (!(s = getenv("XDG_RUNTIME_DIR")))
     {
#ifdef HAVE_GETUID
        struct stat st;

        // fallback - make ~/.run
        snprintf(buf, sizeof(buf), "%s/.run", home);
        mkdir(buf,  S_IRUSR | S_IWUSR | S_IXUSR);
        // if mkdir worked - use, otherwse use /tmp
        if (stat(buf, &st) == 0) s = buf;
        else
          {
             uid_t uid;

             // use /tmp/.run-UID if ~/ dir cant be made
             s = (char *)efl_vpath_core_meta_get(obj, "tmp");
             uid = getuid();
             snprintf(buf, sizeof(buf), "%s/.run-%i", s, (int)uid);
             mkdir(buf,  S_IRUSR | S_IWUSR | S_IXUSR);
             // if ok - use it or fall back to /tmp
             if (stat(buf, &st) == 0) s = buf;
             else s = (char *)efl_vpath_core_meta_get(obj, "tmp");
          }
#else
	s = (char *)efl_vpath_core_meta_get(obj, "tmp");
#endif
     }
   efl_vpath_core_meta_set(obj, "run", s);
   // https://www.freedesktop.org/wiki/Software/xdg-user-dirs/
   // https://wiki.archlinux.org/index.php/Xdg_user_directories
   // ^^^^ we don't handle:
   //   /etc/xdg/user-dirs.conf
   //   /etc/xdg/user-dirs.defaults
   //   (:config:)/user-dirs.conf
   //   (:config:)/user-dirs.defaults

   // $XDG_DESKTOP_DIR="$HOME/Desktop"
   ENV_HOME_SET("XDG_DESKTOP_DIR", "Desktop", "desktop");
   // $XDG_DOCUMENTS_DIR="$HOME/Documents"
   ENV_HOME_SET("XDG_DOCUMENTS_DIR", "Documents", "documents");
   // $XDG_DOWNLOAD_DIR="$HOME/Downloads"
   ENV_HOME_SET("XDG_DOWNLOAD_DIR", "Downloads", "downloads");
   // $XDG_MUSIC_DIR="$HOME/Music"
   ENV_HOME_SET("XDG_MUSIC_DIR", "Music", "music");
   // $XDG_PICTURES_DIR="$HOME/Pictures"
   ENV_HOME_SET("XDG_PICTURES_DIR", "Pictures", "pictures");
   // $XDG_PUBLICSHARE_DIR="$HOME/Public"
   ENV_HOME_SET("XDG_PUBLIC_DIR", "Public", "public");
   // $XDG_TEMPLATES_DIR="$HOME/.Templates"
   ENV_HOME_SET("XDG_TEMPLATES_DIR", ".Templates", "templates");
   // $XDG_VIDEOS_DIR="$HOME/Videos"
   ENV_HOME_SET("XDG_VIDEOS_DIR", "Videos", "videos");

   // Add ~/Applications for user-installed apps
   ENV_HOME_SET("E_APPS_DIR", "Applications", "apps");

   // XXX: do the below ... later
   //
   // FHS FOR APP:
   // app.dir    = PREFIX
   // app.bin    = PREFIX/bin
   // app.lib    = PREFIX/lib
   // app.data   = PREFIX/share/APPNAME
   // app.locale = PREFIX/share/locale 
   //
   // XXX: figure out how to merge these with XDG/FHS?
   // Tizen:
   // App Dir Structure:
   // bin           Executable binary pathOwner: Read
   // lib            Library pathOwner: Read
   // data           Used to store private data of an application.
   // res            Used to read resource files that are delivered with the application package.
   // shared         Parent directory of the data, res, and trusted sub-directories. Files in this directory cannot be delivered with the application package.Owner: Read
   // shared/data    Used to share data with other applications.
   // shared/res     Used to share resources with other applications. The resource files are delivered with the application package.
   // shared/trusted Used to share data with family of trusted applications. The family applications signed with the same certificate can access data in the shared/trusted directory.
   //
   // XXX: figure out how to merge these with XDG?
   // Media/...vvv
   // Images           Used for Image data.Read and Write
   // Sounds           Used for Sound data.
   // Videos           Used for Video data.
   // Cameras          Used for Camera pictures.
   // Downloads        Used for Downloaded data.
   // Music            Used for Music data.
   // Documents        Used for Documents.
   // Others           Used for other types.
   // System Ringtones Used for System default ringtones.Read
   //
   // $TZ_SYS_HOME=/home 
   // $TZ_SYS_DB=/var/db
   // $TZ_SYS_CONFIG=/var/kdb
   // $TZ_SYS_CONFIG_VOLATILE=/run/kdb
   // $TZ_SYS_APP=/usr/apps
   // $TZ_SYS_DESKTOP_APP=/usr/share/applications
   //
   // $TS_USER_DB=<user_homeid>/.tizen/db 
   // $TZ_USER_CONFIG=<user_home_dir>/.tizen/kdb
   // $TZ_USER_APP=<user_home_dir>/.tizen/apps
   // $TZ_USER_DESKTOP_APP=<user_home_dir>/.tizen/desktop
   // $TZ_USER_DOCUMENTS=<user_home_dir>/Documents
   // $TZ_USER_PICTURES=<user_home_dir>/Pictures
   // $TZ_USER_VIDEOS=<user_home_dir>/Videos
   // $TZ_USER_MUSIC=<user_home_dir>/Music
   // $TZ_USER_DOWNLOADS=<user_home_dir>/Downloads
   // $TZ_USER_PUBLIC=<user_home_dir>/Public
   return obj;
}

EOLIAN static void
_efl_vpath_core_efl_object_destructor(Eo *obj, Efl_Vpath_Core_Data *pd)
{
   eina_hash_free(pd->meta);
   pd->meta = NULL;
   eina_spinlock_free(&(pd->lock));
   if (vpath_core == obj) vpath_core = NULL;
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Vpath_Core *
_efl_vpath_core_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   // no locks here as we expect efl to init this early in main "thread"
   if (!vpath_core) vpath_core = efl_add(EFL_VPATH_CORE_CLASS, NULL);
   return vpath_core;
}

EOLIAN static const char *
_efl_vpath_core_meta_get(Eo *obj EINA_UNUSED, Efl_Vpath_Core_Data *pd, const char *key)
{
   const char *meta;

   if (!key) return NULL;
   eina_spinlock_take(&(pd->lock));
   meta = eina_hash_find(pd->meta, key);
   eina_spinlock_release(&(pd->lock));
   return meta;
}

EOLIAN static void
_efl_vpath_core_meta_set(Eo *obj EINA_UNUSED, Efl_Vpath_Core_Data *pd, const char *key, const char *path)
{
   if (!key) return;
   eina_spinlock_take(&(pd->lock));
   if (path) eina_hash_add(pd->meta, key, eina_stringshare_add(path));
   else eina_hash_del(pd->meta, key, NULL);
   eina_spinlock_release(&(pd->lock));
}

EOLIAN static Efl_Vpath_File *
_efl_vpath_core_efl_vpath_fetch(Eo *obj, Efl_Vpath_Core_Data *pd EINA_UNUSED, const char *path)
{
   Efl_Vpath_File_Core *file;

   file = efl_add(EFL_VPATH_FILE_CORE_CLASS, obj);
   efl_vpath_file_path_set(file, path);
   // XXX: implement parse of path then look up in hash if not just create
   // object where path and result are the same and return that with
   // path set and result set to resolved path - return obj handler calls
   // "do" on object to get the result inside fetched or failed callback.
   // if it's a url then we need a new classs that overrides the do and
   // begins a fetch and on finish calls the event cb or when wait is called
   if (path)
     {
        /* FIXME: not working for WIndows */
        // /* <- full path
        if (path[0] == '/')
          {
             efl_vpath_file_result_set(file, path);
             return file;
          }
        // .*
        if (path[0] == '.')
          {
             // .[/]* <- current dir relative
             if ((path[1] == '/') || (path[1] == 0))
               {
                  efl_vpath_file_result_set(file, path);
                  return file;
               }
             // ..[/]* <- parent dir relative
             if ((path[1] == '.') && ((path[2] == '/') || (path[2] == 0)))
               {
                  efl_vpath_file_result_set(file, path);
                  return file;
               }
          }
        // ~* ...
        if (path[0] == '~')
          {
             // ~/ <- home directory
             if (path[1] == '/')
               {
                  char buf[PATH_MAX];
                  const char *home = efl_vpath_core_meta_get(obj, "home");

                  if (home)
                    {
                       snprintf(buf, sizeof(buf), "%s%s", home, path + 1);
                       efl_vpath_file_result_set(file, buf);
                       return file;
                    }
               }
#ifdef HAVE_GETPWENT
             // ~username/ <- homedir of user "username"
             else
               {
                  const char *p;
                  struct passwd pwent, *pwent2 = NULL;
                  char *name, buf[PATH_MAX], pwbuf[8129];

                  for (p = path + 1; *p; p++)
                    {
                       if (*p =='/') break;
                    }
                  name = alloca(p - path);
                  strncpy(name, path + 1, p - path - 1);
                  name[p - path - 1] = 0;
                  if (!getpwnam_r(name, &pwent, pwbuf, sizeof(pwbuf), &pwent2))
                    {
                       if ((pwent2) && (pwent.pw_dir))
                         {
                            snprintf(buf, sizeof(buf), "%s%s", pwent.pw_dir, p);
                            efl_vpath_file_result_set(file, buf);
                            return file;
                         }
                    }
                }
#endif /* HAVE_GETPWENT */
          }
        // (:xxx/* ... <- meta has table
        if ((path[0] == '(') && (path[1] == ':'))
          {
             const char *p, *meta;
             char *name, buf[PATH_MAX];
             Eina_Bool found = EINA_FALSE;

             for (p = path + 2; *p; p++)
               {
                  if ((p[0] ==':') && (p[1] == ')') && (p[2] == '/'))
                    {
                       found = EINA_TRUE;
                       break;
                    }
               }
             if (found)
               {
                  name = alloca(p - path);
                  strncpy(name, path + 2, p - path - 2);
                  name[p - path - 2] = 0;
                  eina_spinlock_take(&(pd->lock));
                  meta = eina_hash_find(pd->meta, name);
                  eina_spinlock_release(&(pd->lock));
                  if (meta)
                    {
                       snprintf(buf, sizeof(buf), "%s%s", meta, p + 2);
                       efl_vpath_file_result_set(file, buf);
                       return file;
                    }
               }
          }
        // file:/// <- local file path uri
        // file://localhost/ <- local file path uri
        // file://hostname/ <- remove file path uri
        // XXX: %c4,%17,%fe etc. are bytes escaped
        // http://www.ietf.org/rfc/rfc2396.txt
        // http://www.ietf.org/rfc/rfc1738.txt
        // http://equinox-project.org/spec/file-uri-spec.txt
        // http://en.wikipedia.org/wiki/File_URI_scheme
     }
   return file;
}

#include "interfaces/efl_vpath_core.eo.c"
