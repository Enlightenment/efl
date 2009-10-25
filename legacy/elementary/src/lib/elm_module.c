#include <Elementary.h>
#include "elm_priv.h"

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <dlfcn.h>      /* dlopen,dlclose,etc */

static Eina_Hash *modules = NULL;

void
_elm_module_init(void)
{
   modules = eina_hash_string_small_new(NULL);
}

void
_elm_module_shutdown(void)
{
   // FIXME: unload all modules
   eina_hash_free(modules);
   modules = NULL;
}

Elm_Module *
_elm_module_add(const char *name)
{
   Elm_Module *m;
   char buf[PATH_MAX];

   m = eina_hash_find(modules, name);
   if (m)
     {
        m->references++;
        return m;
     }
   m = calloc(1, sizeof(Elm_Module));
   if (!m) return NULL;
   m->version = 1;
   if (name[0] != '/')
     {
        const char *home = getenv("HOME");
        
        if (home)
          {
             snprintf(buf, sizeof(buf), "%s/.elementary/modules/%s/%s/module" EFL_SHARED_EXTENSION, home, name, MODULE_ARCH);
             m->handle = dlopen(buf, RTLD_NOW | RTLD_GLOBAL);
             if (m->handle)
               {
                  m->init_func = dlsym(m->handle, "elm_modapi_init");
                  if (m->init_func)
                    {
                       m->shutdown_func = dlsym(m->handle, "elm_modapi_shutdown");
                       m->so_path = eina_stringshare_add(buf);
                       m->name = eina_stringshare_add(name);
                       snprintf(buf, sizeof(buf), "%s/.elementary/modules/%s/%s", home, name, MODULE_ARCH);
                       m->bin_dir = eina_stringshare_add(buf);
                       snprintf(buf, sizeof(buf), "%s/.elementary/modules/%s", home, name);
                       m->data_dir = eina_stringshare_add(buf);
                    }
                  else
                    {
                       dlclose(m->handle);
                       free(m);
                       return NULL;
                    }
               }
          }
        if (!m->handle)
          {
             snprintf(buf, sizeof(buf), "%s/elementary/modules/%s/%s/module" EFL_SHARED_EXTENSION, _elm_lib_dir, name, MODULE_ARCH);
             m->handle = dlopen(buf, RTLD_NOW | RTLD_GLOBAL);
             if (m->handle)
               {
                  m->init_func = dlsym(m->handle, "elm_modapi_init");
                  if (m->init_func)
                    {
                       m->shutdown_func = dlsym(m->handle, "elm_modapi_shutdown");
                       m->so_path = eina_stringshare_add(buf);
                       m->name = eina_stringshare_add(name);
                       snprintf(buf, sizeof(buf), "%s/elementary/modules/%s/%s", _elm_lib_dir, name, MODULE_ARCH);
                       m->bin_dir = eina_stringshare_add(buf);
                       snprintf(buf, sizeof(buf), "%s/elementary/modules/%s", _elm_lib_dir, name, MODULE_ARCH);
                       m->data_dir = eina_stringshare_add(buf);
                    }
                  else
                    {
                       dlclose(m->handle);
                       free(m);
                       return NULL;
                    }
               }
          }
     }
   if (!m->handle)
     {
        free(m);
        return NULL;
     }
   if (!m->init_func(m))
     {
        dlclose(m->handle);
        eina_stringshare_del(m->name);
        eina_stringshare_del(m->so_path);
        eina_stringshare_del(m->data_dir);
        eina_stringshare_del(m->bin_dir);
        free(m);
        return NULL;
     }
   m->references = 1;
   eina_hash_direct_add(modules, m->name, m);
   return m;
}

void
_elm_module_del(Elm_Module *m)
{
   m->references--;
   if (m->references > 0) return;
   if (m->shutdown_func) m->shutdown_func(m);
   eina_hash_del(modules, m->name, m);
   dlclose(m->handle);
   eina_stringshare_del(m->name);
   eina_stringshare_del(m->so_path);
   eina_stringshare_del(m->data_dir);
   eina_stringshare_del(m->bin_dir);
   free(m);
}

const void *
_elm_module_symbol_get(Elm_Module *m, const char *name)
{
   return dlsym(m->handle, name);
}
