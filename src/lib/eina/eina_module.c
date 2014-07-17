/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga, Cedric BAIL
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

#ifdef HAVE_DLOPEN
# include <dlfcn.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef HAVE_ESCAPE
# include <Escape.h>
#endif

#ifdef HAVE_EXOTIC
# include <Exotic.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_alloca.h"
#include "eina_file.h"
#include "eina_log.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_module.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

static int EINA_MODULE_LOG_DOM = -1;
#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(EINA_MODULE_LOG_DOM, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(EINA_MODULE_LOG_DOM, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(EINA_MODULE_LOG_DOM, __VA_ARGS__)

#ifdef _WIN32
# define SEP_C '\\'
# define SEP_S "\\"
#else
# define SEP_C '/'
# define SEP_S "/"
#endif

#define EINA_MODULE_SYMBOL_INIT "__eina_module_init"
#define EINA_MODULE_SYMBOL_SHUTDOWN "__eina_module_shutdown"

struct _Eina_Module
{
   void *handle;
   int ref;

   Eina_Bool global;

   const char file[1];
};

typedef struct _Dir_List_Get_Cb_Data
{
   Eina_Module_Cb cb;
   void *data;
   Eina_Array *array;
} Dir_List_Get_Cb_Data;

typedef struct _Dir_List_Cb_Data
{
   Eina_Module_Cb cb;
   void *data;
} Dir_List_Cb_Data;

static Eina_Bool _dir_list_get_cb(Eina_Module *m, void *data)
{
   Dir_List_Get_Cb_Data *cb_data = data;
   Eina_Bool ret = EINA_TRUE;

   if (cb_data->cb)
      ret = cb_data->cb(m, cb_data->data);

   if (ret)
      eina_array_push(cb_data->array, m);

   return ret;
}

static void _dir_list_cb(const char *name, const char *path, void *data)
{
   Dir_List_Cb_Data *cb_data = data;
   size_t length;

   length = strlen(name);
   if (length < sizeof(SHARED_LIB_SUFFIX)) /* x.so */
      return;

   if (!strcmp(name + length - sizeof(SHARED_LIB_SUFFIX) + 1,
               SHARED_LIB_SUFFIX))
     {
        char *file;
        Eina_Module *m;

        length = strlen(path) + strlen(name) + 2;

        file = alloca(sizeof (char) * length);

        snprintf(file, length, "%s" SEP_S "%s", path, name);
        m = eina_module_new(file);
        if (!m)
          {
             return; /* call the user provided cb on this module */

          }

        if (!cb_data->cb(m, cb_data->data))
           eina_module_free(m);
     }
}

static void _dir_arch_list_cb(const char *name, const char *path, void *data)
{
   Dir_List_Get_Cb_Data *cb_data = data;
   Eina_Module *m;
   char *file = NULL;
   size_t length;

   length = strlen(path) + 1 + strlen(name) + 1 +
      strlen((char *)(cb_data->data)) + 1 + sizeof("module") +
      sizeof(SHARED_LIB_SUFFIX) + 1;

   file = alloca(length);
   snprintf(file, length, "%s" SEP_S "%s" SEP_S "%s" SEP_S "module" SHARED_LIB_SUFFIX,
            path, name, (char *)(cb_data->data));
   m = eina_module_new(file);
   if (!m)
      return;

   eina_array_push(cb_data->array, m);
}

/**
 * @endcond
 */


/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

EAPI Eina_Error EINA_ERROR_WRONG_MODULE = 0;
EAPI Eina_Error EINA_ERROR_MODULE_INIT_FAILED = 0;

/**
 * @endcond
 */

/**
 * @internal
 * @brief Initialize the module loader module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the module loader module of Eina. It is
 * called by eina_init().
 *
 * This function sets up the module module of Eina.
 *
 * @see eina_init()
 */
Eina_Bool
eina_module_init(void)
{
   EINA_MODULE_LOG_DOM = eina_log_domain_register
         ("eina_module", EINA_LOG_COLOR_DEFAULT);
   if (EINA_MODULE_LOG_DOM < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_module");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the module loader module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the module loader module set up by
 * eina_module_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_module_shutdown(void)
{
   /* TODO should we store every module when "new" is called and
    * delete the list of modules here
    */

   eina_log_domain_unregister(EINA_MODULE_LOG_DOM);
   EINA_MODULE_LOG_DOM = -1;
   return EINA_TRUE;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI Eina_Module *eina_module_new(const char *file)
{
   Eina_Module *m;
   size_t len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   /* TODO check that the file exists. Update doc too */

   len = strlen(file);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(len > 0, NULL);

   m = malloc(sizeof(Eina_Module) + len + 1);
   if (!m)
     {
        ERR("could not malloc(%lu)",
            (unsigned long)(sizeof(Eina_Module) + len + 1));
        return NULL;
     }

   memcpy((char *)m->file, file, len + 1);
   m->ref = 0;
   m->handle = NULL;
   m->global = EINA_FALSE;
   DBG("m=%p, file=%s", m, file);

   return m;
}

EAPI Eina_Bool eina_module_free(Eina_Module *m)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, EINA_FALSE);

   DBG("m=%p, handle=%p, file=%s, refs=%d", m, m->handle, m->file, m->ref);

   if (m->handle)
      if (eina_module_unload(m) == EINA_FALSE)
         return EINA_FALSE;

   free(m);
   return EINA_TRUE;
}

EAPI Eina_Bool eina_module_load(Eina_Module *m)
{
#ifdef HAVE_DLOPEN
   void *dl_handle;
   Eina_Module_Init *initcall;
   int flag = RTLD_NOW;

   EINA_SAFETY_ON_NULL_RETURN_VAL(m, EINA_FALSE);

   DBG("m=%p, handle=%p, file=%s, refs=%d", m, m->handle, m->file, m->ref);

   if (m->handle)
      goto loaded;

   if (getenv("EINA_MODULE_LAZY_LOAD")) flag = RTLD_LAZY;

   if (m->global) flag |= RTLD_GLOBAL;
   dl_handle = dlopen(m->file, flag);
   if (m->global) flag &= ~RTLD_GLOBAL;

   if (!dl_handle)
     {
        WRN("could not dlopen(\"%s\", %s): %s", m->file, dlerror(), 
            (flag == RTLD_NOW) ? "RTLD_NOW" : "RTLD_LAZY");
        return EINA_FALSE;
     }

   initcall = dlsym(dl_handle, EINA_MODULE_SYMBOL_INIT);
   if ((!initcall) || (!(*initcall)))
      goto ok;

   if ((*initcall)() == EINA_TRUE)
      goto ok;

   WRN("could not find eina's entry symbol %s inside module %s, or the init function failed",
       EINA_MODULE_SYMBOL_INIT, m->file);
   dlclose(dl_handle);
   return EINA_FALSE;
ok:
   DBG("successfully loaded %s", m->file);
   m->handle = dl_handle;
loaded:
   m->ref++;
   DBG("ref %d", m->ref);
   return EINA_TRUE;
#else
   (void) m;
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool eina_module_unload(Eina_Module *m)
{
#ifdef HAVE_DLOPEN
   Eina_Module_Shutdown *shut;
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, EINA_FALSE);

   DBG("m=%p, handle=%p, file=%s, refs=%d", m, m->handle, m->file, m->ref);

   m->ref--;
   if (!m->ref)
     {
        shut = dlsym(m->handle, EINA_MODULE_SYMBOL_SHUTDOWN);
        if ((shut) && (*shut))
           (*shut)();

        dlclose(m->handle);
        m->handle = NULL;
        DBG("unloaded module %s", m->file);
        return EINA_TRUE;
     }

   return EINA_FALSE;
#else
   (void) m;
   return EINA_FALSE;
#endif
}

EAPI void *eina_module_symbol_get(const Eina_Module *m, const char *symbol)
{
#ifdef HAVE_DLOPEN
   EINA_SAFETY_ON_NULL_RETURN_VAL(m,         NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(m->handle, NULL);
   return dlsym(m->handle, symbol);
#else
   (void) m;
   (void) symbol;
   return NULL;
#endif
}

EAPI const char *eina_module_file_get(const Eina_Module *m)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);
   return m->file;
}

EAPI void eina_module_global_set(Eina_Module *module, Eina_Bool global)
{
   module->global = !!global;
}

EAPI char *eina_module_symbol_path_get(const void *symbol, const char *sub_dir)
{
#ifdef HAVE_DLADDR
   Dl_info eina_dl;

   EINA_SAFETY_ON_NULL_RETURN_VAL(symbol, NULL);

   if (dladdr(symbol, &eina_dl))
     {
        char *pos = strrchr(eina_dl.dli_fname, SEP_C);
        if (pos)
          {
             char *path;
             int l0;
             int l1;
             int l2 = 0;

             l0 = strlen(eina_dl.dli_fname);
             l1 = strlen(pos);
             if (sub_dir && (*sub_dir != '\0'))
                l2 = strlen(sub_dir);

             path = malloc(l0 - l1 + l2 + 1);
             if (path)
               {
                  memcpy(path, eina_dl.dli_fname, l0 - l1);
                  if (sub_dir && (*sub_dir != '\0'))
                     memcpy(path + l0 - l1, sub_dir, l2);

                  path[l0 - l1 + l2] = '\0';
                  return path;
               }
          }
     }
#else
   (void) symbol;
   (void) sub_dir;
#endif /* ! HAVE_DLADDR */

   return NULL;
}

EAPI char *eina_module_environment_path_get(const char *env,
                                            const char *sub_dir)
{
   const char *env_dir;

   EINA_SAFETY_ON_NULL_RETURN_VAL(env, NULL);
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() != geteuid()) return NULL; // if setuid dont use dangerous env
#endif
   env_dir = getenv(env);
   if (env_dir)
     {
        char *path;
        size_t l1;
        size_t l2 = 0;

        l1 = strlen(env_dir);
        if (sub_dir && (*sub_dir != '\0'))
           l2 = strlen(sub_dir);

        path = (char *)malloc(l1 + l2 + 1);
        if (path)
          {
                memcpy(path,      env_dir, l1);
             if (sub_dir && (*sub_dir != '\0'))
                memcpy(path + l1, sub_dir, l2);

             path[l1 + l2] = '\0';

             return path;
          }
     }

   return NULL;
}

EAPI Eina_Array *eina_module_arch_list_get(Eina_Array *array,
                                           const char *path,
                                           const char *arch)
{
   Dir_List_Get_Cb_Data list_get_cb_data;

   if ((!path) || (!arch))
      return array;

   list_get_cb_data.array = array ? array : eina_array_new(4);
   list_get_cb_data.cb = NULL;
   list_get_cb_data.data = (void *)arch;

   eina_file_dir_list(path, 0, &_dir_arch_list_cb, &list_get_cb_data);

   return list_get_cb_data.array;
}

EAPI Eina_Array *eina_module_list_get(Eina_Array *array,
                                      const char *path,
                                      Eina_Bool recursive,
                                      Eina_Module_Cb cb,
                                      void *data)
{
   Dir_List_Get_Cb_Data list_get_cb_data;
   Dir_List_Cb_Data list_cb_data;

   if (!path)
      return array;

   list_get_cb_data.array = array ? array : eina_array_new(4);
   list_get_cb_data.cb = cb;
   list_get_cb_data.data = data;

   list_cb_data.cb = &_dir_list_get_cb;
   list_cb_data.data = &list_get_cb_data;

   eina_file_dir_list(path, recursive, &_dir_list_cb, &list_cb_data);

   return list_get_cb_data.array;
}

EAPI Eina_Module *
eina_module_find(const Eina_Array *array, const char *module)
{
   unsigned int i;
   Eina_Array_Iterator iterator;
   Eina_Module *m;

   EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
     {
        char *file_m;
        char *tmp;
        ssize_t len;

        /* basename() can modify its argument, so we first get a copie */
        /* do not use strdupa, as opensolaris does not have it */
        len = strlen(eina_module_file_get(m));
        tmp = alloca(len + 1);
        memcpy(tmp, eina_module_file_get(m), len + 1);
        file_m = basename(tmp);
        len = strlen(file_m);
        len -= sizeof(SHARED_LIB_SUFFIX) - 1;
        if (len <= 0)
           continue;

        if (!strncmp(module, file_m, len))
	   return m;
     }

   return NULL;
}

EAPI void eina_module_list_load(Eina_Array *array)
{
   Eina_Array_Iterator iterator;
   Eina_Module *m;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN(array);
   DBG("array %p, count %u", array, array->count);
   EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
     eina_module_load(m);
}

EAPI void eina_module_list_unload(Eina_Array *array)
{
   Eina_Array_Iterator iterator;
   Eina_Module *m;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN(array);
   DBG("array %p, count %u", array, array->count);
   EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
     eina_module_unload(m);
}

EAPI void eina_module_list_free(Eina_Array *array)
{
   Eina_Array_Iterator iterator;
   Eina_Module *m;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN(array);
   DBG("array %p, count %u", array, array->count);
   EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
     eina_module_free(m);

   eina_array_flush(array);
}
