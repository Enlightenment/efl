#include "evas_gl_private.h"

#ifdef _WIN32
# include <evil_private.h> /* mkdir */
#endif

static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

static Eina_Bool
evas_gl_common_file_cache_is_dir(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return EINA_FALSE;
   if (S_ISDIR(st.st_mode)) return EINA_TRUE;
   return EINA_FALSE;
}

Eina_Bool
evas_gl_common_file_cache_file_exists(const char *file)
{
   struct stat st;
   if (!file) return EINA_FALSE;
   if (stat(file, &st) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
evas_gl_common_file_cache_mkpath_if_not_exists(const char *path)
{
   struct stat st;

   if (stat(path, &st) < 0)
     return mkdir(path, default_mode) == 0;
   else
     return S_ISDIR(st.st_mode);
}

Eina_Bool
evas_gl_common_file_cache_mkpath(const char *path)
{
   char ss[PATH_MAX];
   unsigned int i;
   Eina_Bool found = EINA_FALSE;

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() != geteuid()) return EINA_FALSE;
#endif
   if (evas_gl_common_file_cache_is_dir(path)) return EINA_TRUE;

   for (i = 0; path[i]; ss[i] = path[i], i++)
     {
        if (i == sizeof(ss) - 1) return EINA_FALSE;
        if ((path[i] == '/')
#ifdef _WIN32
            || (path[i] == '\\')
#endif
            )
          {
             if (found)
               {
                  ss[i] = 0;
                  if (!evas_gl_common_file_cache_mkpath_if_not_exists(ss))
                    return EINA_FALSE;
               }
             found = EINA_TRUE;
          }
     }
   ss[i] = 0;
   return evas_gl_common_file_cache_mkpath_if_not_exists(ss);
}

int
evas_gl_common_file_cache_dir_check(char *cache_dir, int num)
{
   const char *home;
   const char *subdir = ".cache/evas_gl_common_caches";

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() != geteuid()) return EINA_FALSE;
#endif
   home = eina_environment_home_get();
   if (!home) return EINA_FALSE;

   snprintf(cache_dir, num, "%s/%s", home, subdir);
   return evas_gl_common_file_cache_file_exists(cache_dir);
}

int
evas_gl_common_file_cache_file_check(const char *cache_dir, const char *cache_name, char *cache_file, int dir_num)
{
   char before_name[PATH_MAX];
   char after_name[PATH_MAX];
   int new_path_len = 0;
   int i = 0, j = 0;

   char *vendor = NULL;
   char *driver = NULL;
   char *version = NULL;

#ifdef _WIN32
   vendor = "Google";
   version = "OpenGL-ES-3_0_0";
   driver = "Angle-D3D11-vs_5_0-ps_5_0";
   /* no colon character in file names on Windows */
#define SEP_GL "__"
#else
   vendor = (char *)glGetString(GL_VENDOR);
   driver = (char *)glGetString(GL_RENDERER);
   version = (char *)glGetString(GL_VERSION);

   if (!vendor)  vendor  = "-UNKNOWN-";
   if (!driver)  driver  = "-UNKNOWN-";
   if (!version) version = "-UNKNOWN-";
#define SEP_GL "::"
#endif

   new_path_len = snprintf(before_name, sizeof(before_name),
                           "%s" SEP_GL "%s" SEP_GL "%s" SEP_GL "%s.%d" SEP_GL "%s.eet",
                           vendor, version, driver, MODULE_ARCH, evas_version->micro, cache_name);

   /* remove '/' from file name */
   for (i = 0; i < new_path_len; i++)
     {
        if (before_name[i] != '/')
          {
             after_name[j] = before_name[i];
             j++;
          }
     }
   after_name[j] = 0;

   snprintf(cache_file, dir_num, "%s/%s", cache_dir, after_name);

   return evas_gl_common_file_cache_file_exists(cache_file);
}
