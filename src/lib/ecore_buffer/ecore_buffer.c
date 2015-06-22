#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Eina.h"
#include "Ecore.h"

#include "Ecore_Buffer.h"
#include "ecore_buffer_private.h"

typedef struct _Ecore_Buffer_Module Ecore_Buffer_Module;
typedef struct _Ecore_Buffer_Cb_Data Ecore_Buffer_Cb_Data;

struct _Ecore_Buffer_Module
{
     Ecore_Buffer_Backend *be;
     Ecore_Buffer_Module_Data data;
};

struct _Ecore_Buffer
{
   unsigned int width;
   unsigned int height;
   int format;
   unsigned int flags;

   Ecore_Buffer_Data buffer_data;
   Ecore_Buffer_Module *bm;

   Eina_Hash *data;
   Eina_Inlist *free_callbacks;
};

struct _Ecore_Buffer_Cb_Data
{
   EINA_INLIST;
   Ecore_Buffer_Cb cb;
   void *data;
};

static Eina_Hash *_backends;
static Eina_Array *_modules;
static int _ecore_buffer_init_count = 0;
static int _ecore_buffer_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_buffer_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_buffer_log_dom, __VA_ARGS__)

#ifndef PACKAGE_LIB_DIR
#define PACKAGE_LIB_DIR ""
#endif
#ifndef MODULE_ARCH
#define MODULE_ARCH ""
#endif

static Ecore_Buffer_Module *
_ecore_buffer_get_backend(const char *name)
{
   Ecore_Buffer_Module *bm = NULL;
   Eina_Iterator *backend_name_itr;
   const char *backend_name = NULL;

   backend_name = name;

   if (backend_name == NULL)
     {
        backend_name = (const char*)getenv("ECORE_BUFFER_ENGINE");
        if (!backend_name)
          {
             backend_name_itr = eina_hash_iterator_data_new(_backends);
             while((!bm) &&
                   (eina_iterator_next(backend_name_itr, (void **)&bm)));
             eina_iterator_free(backend_name_itr);
          }
     }
   else
     bm = eina_hash_find(_backends, backend_name);

   if ((!bm) || (!bm->be))
     return NULL;

   if (bm->be->init)
     bm->data = bm->be->init(NULL, NULL);

   return bm;
}

static Eina_Bool
_ecore_buffer_backends_free(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   Ecore_Buffer_Module *bm = data;

   if (!bm)
     return EINA_FALSE;

   if (bm->data)
     bm->be->shutdown(bm->data);

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_buffer_register(Ecore_Buffer_Backend *be)
{
   Ecore_Buffer_Module *bm;

   EINA_SAFETY_ON_NULL_RETURN_VAL(be, 0);

   bm = calloc(1, sizeof(Ecore_Buffer_Module));
   if (!bm)
     return EINA_FALSE;

   bm->be = be;
   bm->data = NULL;

   return eina_hash_add(_backends, be->name, bm);
}

EAPI void
ecore_buffer_unregister(Ecore_Buffer_Backend *be)
{
   Ecore_Buffer_Module *bm;

   EINA_SAFETY_ON_NULL_RETURN(be);

   bm = eina_hash_find(_backends, be->name);
   if (!bm)
     return;

   eina_hash_del(_backends, be->name, bm);
   free(bm);
}

EAPI Eina_Bool
ecore_buffer_init(void)
{
   char *path;

   if (++_ecore_buffer_init_count > 1)
     return EINA_TRUE;

   _ecore_buffer_log_dom = eina_log_domain_register("ecore_buffer", EINA_COLOR_BLUE);
   if (_ecore_buffer_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: ecore_buffer");
        goto err;
     }

   _backends = eina_hash_string_superfast_new(NULL);

   /* dynamic backends */
   _modules = eina_module_arch_list_get(NULL,
                                        PACKAGE_LIB_DIR "/ecore_buffer/modules",
                                        MODULE_ARCH);

   path = eina_module_symbol_path_get((const void *)ecore_buffer_init,
                                      "/ecore_buffer/modules");

   _modules = eina_module_arch_list_get(_modules, path, MODULE_ARCH);
   if (path)
     free(path);

   /* fallback using module where in build directory */
   if ((!_modules) ||
       (eina_array_count(_modules) == 0))
     {
        ERR("No available module in library directy: %s",
            PACKAGE_LIB_DIR "/ecore_buffer/modules");
        ERR("Fallback to load module where in build directory :%s",
            PACKAGE_BUILD_DIR "/src/modules/");
        _modules = eina_module_list_get(NULL,
                                        PACKAGE_BUILD_DIR "/src/modules/",
                                        EINA_TRUE, NULL, NULL);
     }

   if ((!_modules) ||
       (eina_array_count(_modules) == 0))
     {
        ERR("no ecore_buffer modules able to be loaded.");
        eina_hash_free(_backends);
        eina_log_domain_unregister(_ecore_buffer_log_dom);
        _ecore_buffer_log_dom = -1;
        goto err;
     }

   // XXX: MODFIX: do not list ALL modules and load them ALL! this is
   // wrong. load the module we need WHEN we need it (by name etc. etc.
   // from api).
   eina_module_list_load(_modules);

   return EINA_TRUE;

err:
   _ecore_buffer_init_count--;
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_buffer_shutdown(void)
{
   if (_ecore_buffer_init_count < 1)
     {
        WARN("Ecore_Buffer shut down called without init");
        return EINA_FALSE;
     }

   if (--_ecore_buffer_init_count != 0)
     return EINA_FALSE;

   /* dynamic backends */
   eina_hash_foreach(_backends, _ecore_buffer_backends_free, NULL);

   eina_module_list_free(_modules);
   if (_modules)
     eina_array_free(_modules);

   if (_backends)
     eina_hash_free(_backends);

   eina_log_domain_unregister(_ecore_buffer_log_dom);
   _ecore_buffer_log_dom = -1;

   return EINA_TRUE;
}

EAPI Ecore_Buffer*
ecore_buffer_new(const char *engine, unsigned int width, unsigned int height, Ecore_Buffer_Format format, unsigned int flags)
{
   Ecore_Buffer_Module *bm;
   Ecore_Buffer *bo;
   void *bo_data;

   bm = _ecore_buffer_get_backend(engine);
   if (!bm)
     {
        ERR("Failed to get backend: %s", engine);
        return NULL;
     }

   EINA_SAFETY_ON_NULL_RETURN_VAL(bm->be, NULL);

   if (!bm->be->buffer_alloc)
     {
        ERR("Not supported create buffer");
        return NULL;
     }

   bo = calloc(1, sizeof(Ecore_Buffer));
   if (!bo)
     return NULL;

   bo_data = bm->be->buffer_alloc(bm->data, width, height, format, flags);
   if (!bo_data)
     {
        free(bo);
        return NULL;
     }

   bo->bm = bm;
   bo->width = width;
   bo->height = height;
   bo->format = format;
   bo->flags = flags;
   bo->buffer_data = bo_data;

   return bo;
}

EAPI void
ecore_buffer_free(Ecore_Buffer *buf)
{
   Ecore_Buffer_Cb_Data *free_cb;

   EINA_SAFETY_ON_NULL_RETURN(buf);

   //Call free_cb
   while (buf->free_callbacks)
     {
        free_cb = EINA_INLIST_CONTAINER_GET(buf->free_callbacks, Ecore_Buffer_Cb_Data);
        buf->free_callbacks = eina_inlist_remove(buf->free_callbacks, buf->free_callbacks);

        free_cb->cb(buf, free_cb->data);
        free(free_cb);
     }

   EINA_SAFETY_ON_NULL_RETURN(buf->bm);
   EINA_SAFETY_ON_NULL_RETURN(buf->bm->be);
   EINA_SAFETY_ON_NULL_RETURN(buf->bm->be->buffer_free);

   buf->bm->be->buffer_free(buf->bm->data, buf->buffer_data);

   //Free User Data
   if (buf->data)
     eina_hash_free(buf->data);

   free(buf);
}

EAPI void *
ecore_buffer_data_get(Ecore_Buffer *buf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm->be, NULL);

   if (!buf->bm->be->data_get)
     return NULL;

   return buf->bm->be->data_get(buf->bm->data, buf->buffer_data);
}

EAPI Ecore_Pixmap
ecore_buffer_pixmap_get(Ecore_Buffer *buf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm->be, 0);

   if (!buf->bm->be->pixmap_get)
     return 0;

   return buf->bm->be->pixmap_get(buf->bm->data, buf->buffer_data);
}

EAPI void *
ecore_buffer_tbm_surface_get(Ecore_Buffer *buf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm->be, NULL);

   if (!buf->bm->be->tbm_surface_get)
     {
        ERR("TBM is not supported\n");
        return NULL;
     }

   return buf->bm->be->tbm_surface_get(buf->bm->data, buf->buffer_data);
}

EAPI Eina_Bool
ecore_buffer_size_get(Ecore_Buffer *buf, unsigned int *width, unsigned int *height)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, EINA_FALSE);

   if (width) *width = buf->width;
   if (height) *height = buf->height;

   return EINA_TRUE;
}

EAPI unsigned int
ecore_buffer_format_get(Ecore_Buffer *buf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, 0);

   return buf->format;
}

EAPI unsigned int
ecore_buffer_flags_get(Ecore_Buffer *buf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, 0);

   return buf->flags;
}

EAPI void
ecore_buffer_free_callback_add(Ecore_Buffer *buf, Ecore_Buffer_Cb func, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(buf);
   EINA_SAFETY_ON_NULL_RETURN(func);

   Ecore_Buffer_Cb_Data *free_cb;

   free_cb = calloc(1, sizeof(Ecore_Buffer_Cb_Data));
   if (!free_cb)
     return;

   free_cb->cb = func;
   free_cb->data = data;
   buf->free_callbacks = eina_inlist_append(buf->free_callbacks, EINA_INLIST_GET(free_cb));
}

EAPI void
ecore_buffer_free_callback_remove(Ecore_Buffer *buf, Ecore_Buffer_Cb func, void *data)
{
   Ecore_Buffer_Cb_Data *free_cb;

   EINA_SAFETY_ON_NULL_RETURN(buf);
   EINA_SAFETY_ON_NULL_RETURN(func);

   if (buf->free_callbacks)
     {
        Eina_Inlist *itrn;
        EINA_INLIST_FOREACH_SAFE(buf->free_callbacks, itrn, free_cb)
          {
             if (free_cb->cb == func && free_cb->data == data)
               {
                  buf->free_callbacks =
                     eina_inlist_remove(buf->free_callbacks,
                                        EINA_INLIST_GET(free_cb));
                  free(free_cb);
               }
          }
     }
}

const char *
_ecore_buffer_engine_name_get(Ecore_Buffer *buf)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm->be, 0);

   return buf->bm->be->name;
}

Ecore_Export_Type
_ecore_buffer_export(Ecore_Buffer *buf, int *id)
{
   Ecore_Export_Type type = EXPORT_TYPE_INVALID;
   int ret_id;

   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, type);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm, type);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf->bm->be, type);

   if (!buf->bm->be->buffer_export)
     return type;

   type = buf->bm->be->buffer_export(buf->bm->data, buf->buffer_data, &ret_id);

   if (id) *id = ret_id;

   return type;
}

Ecore_Buffer *
_ecore_buffer_import(const char *engine, int width, int height, Ecore_Buffer_Format format, Ecore_Export_Type type, int export_id, unsigned int flags)
{
   Ecore_Buffer_Module *bm;
   Ecore_Buffer *bo;
   void *bo_data;

   bm = _ecore_buffer_get_backend(engine);
   if (!bm)
     {
        ERR("Filed to get Backend: %s", engine);
        return NULL;
     }

   EINA_SAFETY_ON_NULL_RETURN_VAL(bm->be, NULL);

   if (!bm->be->buffer_import)
     {
        ERR("Not supported import buffer");
        return NULL;
     }

   bo = calloc(1, sizeof(Ecore_Buffer));
   if (!bo)
     return NULL;

   bo_data = bm->be->buffer_import(bm->data, width, height, format, type, export_id, flags);
   if (!bo_data)
     {
        free(bo);
        return NULL;
     }

   bo->bm = bm;
   bo->width = width;
   bo->height = height;
   bo->format = format;
   bo->flags = flags;
   bo->buffer_data = bo_data;

   return bo;
}
