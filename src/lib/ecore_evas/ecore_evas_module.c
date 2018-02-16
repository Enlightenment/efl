#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"
#include <unistd.h>

#include "../../static_libs/buildsystem/buildsystem.h"

static Eina_Hash *_registered_engines = NULL;
static Eina_List *_engines_paths = NULL;
static Eina_List *_engines_available = NULL;
static Eina_Module *_ecore_evas_vnc = NULL;

#ifdef _WIN32
# define ECORE_EVAS_ENGINE_NAME "module.dll"
#else
# define ECORE_EVAS_ENGINE_NAME "module.so"
#endif

static inline Eina_Bool
_file_exists(const char *file)
{
   if (!file) return EINA_FALSE;

   if (access(file, F_OK) == -1) return EINA_FALSE;
   return EINA_TRUE;
}


static Eina_Module *
_ecore_evas_vnc_server_module_try_load(const char *prefix,
                                       Eina_Bool use_prefix_only)
{
   Eina_Module *m;

   if (use_prefix_only)
     m = eina_module_new(prefix);
   else
     {
        char path[PATH_MAX];

        snprintf(path, sizeof(path), "%s/vnc_server/%s/%s", prefix,
                 MODULE_ARCH, ECORE_EVAS_ENGINE_NAME);
        m = eina_module_new(path);
     }

   if (!m)
     return NULL;
   if (!eina_module_load(m))
     {
        eina_module_free(m);
        _ecore_evas_vnc = NULL;
        return NULL;
     }

   return m;
}

Eina_Module *
_ecore_evas_vnc_server_module_load(void)
{
   char *prefix;
   char buf[PATH_MAX];

   if (_ecore_evas_vnc)
     return _ecore_evas_vnc;

   if (bs_mod_get(buf, sizeof(buf), "ecore_evas", "vnc_server"))
     {
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
        if (getuid() == geteuid())
#endif
          {
             _ecore_evas_vnc = _ecore_evas_vnc_server_module_try_load(buf,
                                                                      EINA_TRUE);
             if (_ecore_evas_vnc)
               return _ecore_evas_vnc;
          }
     }

   prefix = eina_module_symbol_path_get(_ecore_evas_vnc_server_module_load,
                                        "/ecore_evas");
   _ecore_evas_vnc = _ecore_evas_vnc_server_module_try_load(prefix, EINA_FALSE);
   free(prefix);
   //Last try...
   if (!_ecore_evas_vnc)
     {
        _ecore_evas_vnc = _ecore_evas_vnc_server_module_try_load(PACKAGE_LIB_DIR"/ecore_evas",
                                                                 EINA_FALSE);
        if (!_ecore_evas_vnc)
          ERR("Could not find a valid VNC module to load!");
     }
   return _ecore_evas_vnc;
}

Eina_Module *
_ecore_evas_engine_load(const char *engine)
{
   const char *path;
   Eina_List *l;
   Eina_Module *em = NULL;
   char tmp[PATH_MAX] = "";

   EINA_SAFETY_ON_NULL_RETURN_VAL(engine, NULL);

   em =  (Eina_Module *)eina_hash_find(_registered_engines, engine);
   if (em) return em;

   if (bs_mod_get(tmp, sizeof(tmp), "ecore_evas/engines", engine))
     {
        em = eina_module_new(tmp);
        if (!em) return NULL;

        if (!eina_module_load(em))
          {
             eina_module_free(em);
             return NULL;
          }
        if (eina_hash_add(_registered_engines, engine, em))
          return em;
     }

   EINA_LIST_FOREACH(_engines_paths, l, path)
     {
        snprintf(tmp, sizeof(tmp), "%s/%s/%s/%s",
                 path, engine, MODULE_ARCH, ECORE_EVAS_ENGINE_NAME);

        em = eina_module_new(tmp);
        if (!em) continue;

        if (!eina_module_load(em))
          {
             eina_module_free(em);
             continue;
          }
        if (eina_hash_add(_registered_engines, engine, em))
          return em;
     }

   return NULL;
}

void
_ecore_evas_engine_init(void)
{
   char *paths[2] = { NULL, NULL };
   unsigned int i;
   unsigned int j;

/* avoid freeing modules ever to avoid deferred cb symbol problems */
//   _registered_engines = eina_hash_string_small_new(EINA_FREE_CB(eina_module_free));
   _registered_engines = eina_hash_string_small_new(NULL);

   /* 1. libecore_evas.so/../ecore_evas/engines/ */
   paths[0] = eina_module_symbol_path_get(_ecore_evas_engine_init, "/ecore_evas/engines");
#ifndef _WIN32
   /* 3. PREFIX/ecore_evas/engines/ */
   paths[1] = strdup(PACKAGE_LIB_DIR "/ecore_evas/engines");
#else
   paths[1] = eina_module_symbol_path_get(_ecore_evas_engine_init, "/../lib/ecore_evas/engines");
#endif

   for (j = 0; j < ((sizeof (paths) / sizeof (char*)) - 1); ++j)
     for (i = j + 1; i < sizeof (paths) / sizeof (char*); ++i)
       if (paths[i] && paths[j] && !strcmp(paths[i], paths[j]))
         {
            free(paths[i]);
            paths[i] = NULL;
         }

   for (i = 0; i < sizeof (paths) / sizeof (char*); ++i)
     if (paths[i])
       _engines_paths = eina_list_append(_engines_paths, paths[i]);
}

void
_ecore_evas_engine_shutdown(void)
{
   char *path;

/* don't free modules to avoid fn callback deferred symbol problems
   if (_registered_engines)
     {
       eina_hash_free(_registered_engines);
       _registered_engines = NULL;
     }
 */

   EINA_LIST_FREE(_engines_paths, path)
     free(path);

   EINA_LIST_FREE(_engines_available, path)
     eina_stringshare_del(path);
}

const Eina_List *
_ecore_evas_available_engines_get(void)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *it;
   Eina_List *l = NULL, *result = NULL;
   Eina_Strbuf *buf;
   const char *path;

   if (_engines_available) return _engines_available;

   buf = eina_strbuf_new();
   EINA_LIST_FOREACH(_engines_paths, l, path)
     {
        it = eina_file_direct_ls(path);

        EINA_ITERATOR_FOREACH(it, info)
          {
             eina_strbuf_append_printf(buf, "%s/%s/" ECORE_EVAS_ENGINE_NAME,
                                       info->path, MODULE_ARCH);

             if (_file_exists(eina_strbuf_string_get(buf)))
               {
                  const char *name;

#ifdef _WIN32
                  EVIL_PATH_SEP_WIN32_TO_UNIX(info->path);
#endif
                  name = strrchr(info->path, '/');
                  if (name) name++;
                  else name = info->path;
#define ADDENG(x) result = eina_list_append(result, eina_stringshare_add(x))
                  if (!strcmp(name, "fb"))
                    {
#ifdef BUILD_ECORE_EVAS_FB
                       ADDENG("fb");
#endif
                    }
                  else if (!strcmp(name, "x"))
                    {
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
                       ADDENG("opengl_x11");
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_XLIB
                       ADDENG("software_x11");
#endif
                    }
                  else if (!strcmp(name, "buffer"))
                    {
#ifdef BUILD_ECORE_EVAS_BUFFER
                       ADDENG("buffer");
#endif
#ifdef BUILD_ECORE_EVAS_EWS
                       ADDENG("ews");
#endif
                    }
                  else if (!strcmp(name, "cocoa"))
                    {
#ifdef BUILD_ECORE_EVAS_OPENGL_COCOA
                       ADDENG("opengl_cocoa");
#endif
                    }
                  else if (!strcmp(name, "sdl"))
                    {
#ifdef BUILD_ECORE_EVAS_OPENGL_SDL
                       ADDENG("opengl_sdl");
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_SDL
                       ADDENG("sdl");
#endif
                    }
                  else if (!strcmp(name, "wayland"))
                    {
#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
                       ADDENG("wayland_shm");
#endif
#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
                       ADDENG("wayland_egl");
#endif
                    }
                  else if (!strcmp(name, "win32"))
                    {
#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
                       ADDENG("software_gdi");
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
                       ADDENG("software_ddraw");
#endif
#ifdef BUILD_ECORE_EVAS_DIRECT3D
                       ADDENG("direct3d");
#endif
#ifdef BUILD_ECORE_EVAS_OPENGL_GLEW
                       ADDENG("opengl_glew");
#endif
                    }
                  else if (!strcmp(name, "drm"))
                    {
#ifdef BUILD_ECORE_EVAS_DRM
                       ADDENG("drm");
#endif
#ifdef BUILD_ECORE_EVAS_GL_DRM
                       ADDENG("gl_drm");
#endif
                    }
               }
             eina_strbuf_reset(buf);
          }
        eina_iterator_free(it);
     }
   eina_strbuf_free(buf);

   _engines_available = result;
   return result;
}
