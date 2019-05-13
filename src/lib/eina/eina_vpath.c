#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <pwd.h>

#include <Eina.h>

#include "eina_internal.h"
#include "eina_private.h"

static Eina_Hash *vpath_data = NULL;

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_eina_vpath_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_vpath_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_vpath_log_dom, __VA_ARGS__)

static int _eina_vpath_log_dom = -1;

static inline void
_eina_vpath_data_add(const char *key, const char *value)
{
   eina_hash_add(vpath_data, key, eina_stringshare_add(value));
}

static inline Eina_Stringshare*
_eina_vpath_data_get(const char *key)
{
   return eina_hash_find(vpath_data, key);
}


static char *
_fallback_runtime_dir(const char *home)
{
   char buf[PATH_MAX];
#if defined(HAVE_GETUID)
   uid_t uid = getuid();
#endif
   struct stat st;

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (setuid(geteuid()) != 0)
      {
         fprintf(stderr,
                 "FATAL: Cannot setuid - errno=%i\n",
                 errno);
         abort();
      }
#endif
   // fallback - make ~/.run
   snprintf(buf, sizeof(buf), "%s/.run", home);
   if (!!mkdir(buf,  S_IRUSR | S_IWUSR | S_IXUSR))
     {
        if (errno == EEXIST)
          {
             if (stat(buf, &st) == 0)
               {
                  // some sanity checks - but not for security
                  if (!(st.st_mode & S_IFDIR))
                    {
                       // fatal - exists but is not a dir
                       fprintf(stderr,
                               "FATAL: run dir '%s' exists but not a dir\n",
                               buf);
                       abort();
                    }
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
                  if (st.st_uid != geteuid())
                    {
                       // fatal - run dir doesn't belong to user
                       fprintf(stderr,
                               "FATAL: run dir '%s' not owned by uid %i\n",
                               buf, (int)geteuid());
                       abort();
                    }
#endif
               }
             else
               {
                  // fatal - we cant create our run dir in ~/
                  fprintf(stderr,
                          "FATAL: Cannot verify run dir '%s' errno=%i\n",
                          buf, errno);
                  abort();
               }
          }
        else
          {
             // fatal - we cant create our run dir in ~/
             fprintf(stderr,
                     "FATAL: Cannot create run dir '%s' - errno=%i\n",
                     buf, errno);
             abort();
          }
     }
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
  if (setreuid(uid, geteuid()) != 0)
     {
        fprintf(stderr,
                "FATAL: Cannot setreuid - errno=%i\n",
                errno);
        abort();
     }
#endif

   return strdup(buf);
}

static char *
_fallback_home_dir()
{
   char buf[PATH_MAX];
   /* Windows does not have getuid(), but home can't be NULL */
#ifdef HAVE_GETEUID
   uid_t uid = geteuid();
   struct stat st;

   snprintf(buf, sizeof(buf), "/tmp/%i", (int)uid);
   if (mkdir(buf,  S_IRUSR | S_IWUSR | S_IXUSR) < 0)
     {
        if (errno != EEXIST)
          {
             if (stat("/tmp", &st) == 0)
               snprintf(buf, sizeof(buf), "/tmp");
             else
               snprintf(buf, sizeof(buf), "/");
          }
     }
   if (stat(buf, &st) != 0)
     {
        if (stat("/tmp", &st) == 0)
          snprintf(buf, sizeof(buf), "/tmp");
        else
          snprintf(buf, sizeof(buf), "/");
     }
#else
   snprintf(buf, sizeof(buf), "/");
#endif
   return strdup(buf);
}

static void
_eina_vpath_interface_sys_init(void)
{
   const char *home, *tmp;

   // $HOME / ~/ etc.
   home = eina_environment_home_get();
   if (!home)
     {
        char *home2 = _fallback_home_dir();
        _eina_vpath_data_add("home", home2);
        free(home2);
     }
   else
     _eina_vpath_data_add("home", home);

   // tmp dir - system wide
   tmp = eina_environment_tmp_get();
   _eina_vpath_data_add("tmp", tmp);
}

Eina_Bool
eina_vpath_init(void)
{
   vpath_data = eina_hash_string_superfast_new((Eina_Free_Cb)eina_stringshare_del);

   _eina_vpath_interface_sys_init();
   eina_xdg_env_init();

   _eina_vpath_log_dom = eina_log_domain_register("vpath", "cyan");
   return EINA_TRUE;
}

Eina_Bool
eina_vpath_shutdown(void)
{
   eina_hash_free(vpath_data);
   vpath_data = NULL;
   eina_log_domain_unregister(_eina_vpath_log_dom);
   _eina_vpath_log_dom = -1;
   return EINA_TRUE;
}

static Eina_Bool
_fetch_user_homedir(char **str, const char *name, const char *error)
{
  *str = NULL;
#ifdef HAVE_GETPWENT
  struct passwd *pwent;

  pwent = getpwnam(name);
  if (!pwent)
    {
       ERR("User %s not found\nThe string was: %s", name, error);
       return EINA_FALSE;
    }
  *str = pwent->pw_dir;

  return EINA_TRUE;
#else
  ERR("User fetching is disabled on this system\nThe string was: %s", error);
  return EINA_FALSE;
  (void) name;
#endif
}

static int
_eina_vpath_resolve(const char *path, char *str, size_t size)
{
   if (path[0] == '~')
     {
        char *home = NULL;
        // ~/ <- home directory
        if (path[1] == '/')
          {
             home = eina_hash_find(vpath_data, "home");
             path ++;
          }
        // ~username/ <- homedir of user "username"
        else
          {
             const char *p;
             char *name;

             for (p = path + 1; *p; p++)
               {
                  if (*p =='/') break;
               }
             name = alloca(p - path);
             strncpy(name, path + 1, p - path - 1);
             name[p - path - 1] = 0;

             if (!_fetch_user_homedir(&home, name, path))
               return 0;
             path = p;
           }
         if (home)
           {
              return snprintf(str, size, "%s%s", home, path);
           }
     }
   // (:xxx:)/* ... <- meta hash table
   else if (((path[0] == '(') && (path[1] == ':')) ||
            ((path[0] == '$') && (path[1] == '{')))
     {
        const char *p, *end, *meta;
        const char *msg_start, *msg_end;
        char *name;
        int offset;
        Eina_Bool found = EINA_FALSE;

        if (path[0] == '(')
          {
             end = p = strstr(path + 2, ":)");
             offset = 2;
             msg_start = "(:";
             msg_end = ":)";
          }
        else
          {
             end = p = strchr(path + 2, '}');
             offset = 1;
             msg_start = "${";
             msg_end = "}";
          }
        if (p) found = EINA_TRUE;
        p += offset;

        if (!found)
          {
             ERR("'%s' Needs to have a matching '%s'\nThe string was: %s", msg_start, msg_end, path);
             return 0;
          }

        if (*p != '/')
          {
             ERR("A / is expected after '%s'\nThe string was: %s", msg_end, path);
             return 0;
          }

        if (found)
          {
             name = alloca(end - path);
             strncpy(name, path + 2, end - path - offset);
             name[end - path - 2] = 0;
             meta = _eina_vpath_data_get(name);
             if (meta)
               {
                  return snprintf(str, size, "%s%s", meta, end + offset);
               }
             else
               {
                  ERR("Meta key '%s' was not registered!\nThe string was: %s", name, path);
                  return 0;
               }
          }
     }
   //just return the path, since we assume that this is a normal path
   else
     {
        return snprintf(str, size, "%s", path);
     }
   str[0] = '\0';
   return 0;
}

EAPI char *
eina_vpath_resolve(const char* path)
{
   char buf[PATH_MAX];

   if (_eina_vpath_resolve(path, buf, sizeof(buf)) > 0)
     return strdup(buf);
   return NULL;
}

EAPI int
eina_vpath_resolve_snprintf(char *str, size_t size, const char *format, ...)
{
   va_list args;
   char *path;
   int r;

   // XXX: implement parse of path then look up in hash if not just create
   // object where path and result are the same and return that with
   // path set and result set to resolved path - return obj handler calls
   // "do" on object to get the result inside fetched or failed callback.
   // if it's a url then we need a new classs that overrides the do and
   // begins a fetch and on finish calls the event cb or when wait is called
   /* FIXME: not working for WIndows */
   // /* <- full path

   path = alloca(size + 1);

   va_start(args, format);
   vsnprintf(path, size, format, args);
   va_end(args);

   r = _eina_vpath_resolve(path, str, size);
   if (r > 0) return r;

   ERR("The path has to start with either '~/' or '(:NAME:)/' or be a normal path \nThe string was: %s", path);

   return 0;
}

EAPI void
eina_vpath_interface_app_set(const char *app_domain, Eina_Prefix *app_pfx)
{
   char buf[PATH_MAX];

   EINA_SAFETY_ON_NULL_RETURN(app_domain);
   EINA_SAFETY_ON_NULL_RETURN(app_pfx);

   _eina_vpath_data_add("app.dir", eina_prefix_get(app_pfx));
   _eina_vpath_data_add("app.bin", eina_prefix_bin_get(app_pfx));
   _eina_vpath_data_add("app.lib", eina_prefix_lib_get(app_pfx));
   _eina_vpath_data_add("app.data", eina_prefix_data_get(app_pfx));
   _eina_vpath_data_add("app.locale", eina_prefix_locale_get(app_pfx));
   snprintf(buf, sizeof(buf), "%s/%s",
            _eina_vpath_data_get("usr.config"), app_domain);
   _eina_vpath_data_add("app.config", buf);
   snprintf(buf, sizeof(buf), "%s/%s",
            _eina_vpath_data_get("usr.cache"), app_domain);
   _eina_vpath_data_add("app.cache", buf);
   snprintf(buf, sizeof(buf), "%s/%s",
            _eina_vpath_data_get("usr.data"), app_domain);
   _eina_vpath_data_add("app.local", buf);
   snprintf(buf, sizeof(buf), "%s/%s",
            _eina_vpath_data_get("usr.tmp"), app_domain);
   _eina_vpath_data_add("app.tmp", buf);
}

EAPI void
eina_vpath_interface_user_set(Eina_Vpath_Interface_User *user)
{
   Eina_Bool free_run = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN(user);

   if (!user->run)
     {
        user->run = _fallback_runtime_dir(_eina_vpath_data_get("home"));
        free_run = EINA_TRUE;
     }

#define ADD(a) _eina_vpath_data_add("usr." #a , user->a)
   ADD(desktop);
   ADD(documents);
   ADD(downloads);
   ADD(music);
   ADD(pictures);
   ADD(pub);
   ADD(templates);
   ADD(videos);
   ADD(data);
   ADD(config);
   ADD(cache);
   ADD(run);
   ADD(tmp);
#undef ADD

   if (free_run)
     free((char *)user->run);
}
