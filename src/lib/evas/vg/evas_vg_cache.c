#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_vg_private.h"

static Vg_Cache* vg_cache = NULL;

struct ext_loader_s
{
   unsigned int length;
   const char *extension;
   const char *loader;
};

struct ext_saver_s
{
   unsigned int length;
   const char *extension;
   const char *saver;
};

#define MATCHING(Ext, Module) { sizeof(Ext)-1, Ext, Module }

static const struct ext_loader_s loaders[] =
{ /* map extensions to loaders to use for good first-guess tries */
   MATCHING(".eet", "eet"),
   MATCHING(".edj", "eet"),
   MATCHING(".svg", "svg"),
   MATCHING(".svgz", "svg"),
   MATCHING(".svg.gz", "svg")
};

static const char *loaders_name[] =
{ /* in order of most likely needed */
  "eet", "svg"
};

static const struct ext_saver_s savers[] =
{ /* map extensions to savers to use for good first-guess tries */
   MATCHING(".eet", "eet"),
   MATCHING(".edj", "eet"),
   MATCHING(".svg", "svg")
};

static Evas_Module *
_find_loader_module(const char *file)
{
   const char           *loader = NULL, *end;
   Evas_Module          *em = NULL;
   unsigned int          i;
   int                   len, len2;
   len = strlen(file);
   end = file + len;
   for (i = 0; i < (sizeof (loaders) / sizeof(struct ext_loader_s)); i++)
     {
        len2 = loaders[i].length;
        if (len2 > len) continue;
        if (!strcasecmp(end - len2, loaders[i].extension))
          {
             loader = loaders[i].loader;
             break;
          }
     }
   if (loader)
     em = evas_module_find_type(EVAS_MODULE_TYPE_VG_LOADER, loader);
   return em;
}

static Vg_File_Data *
_vg_load_from_file(const char *file, const char *key)
{
   Evas_Module       *em;
   Evas_Vg_Load_Func *loader;
   int                error = EVAS_LOAD_ERROR_GENERIC;
   Vg_File_Data      *vfd;
   unsigned int i;

   em = _find_loader_module(file);
   if (em)
     {
        loader = em->functions;
        vfd = loader->file_open(file, key, &error);
        if (vfd)
          {
             vfd->loader = loader;
             return vfd;
          }
     }
   else
     {
        for (i = 0; i < sizeof (loaders_name) / sizeof (char *); i++)
          {
             em = evas_module_find_type(EVAS_MODULE_TYPE_VG_LOADER, loaders_name[i]);
             if (em)
               {
                  loader = em->functions;
                  vfd = loader->file_open(file, key, &error);
                    {
                       vfd->loader = loader;
                       return vfd;
                    }
               }
          }
     }
   WRN("Exhausted all means to load vector file = %s", file);
   return NULL;
}

static Evas_Module *
_find_saver_module(const char *file)
{
   const char           *saver = NULL, *end;
   Evas_Module          *em = NULL;
   unsigned int          i;
   int                   len, len2;
   len = strlen(file);
   end = file + len;
   for (i = 0; i < (sizeof (savers) / sizeof(struct ext_saver_s)); i++)
     {
        len2 = savers[i].length;
        if (len2 > len) continue;
        if (!strcasecmp(end - len2, savers[i].extension))
          {
             saver = savers[i].saver;
             break;
          }
     }
   if (saver)
     em = evas_module_find_type(EVAS_MODULE_TYPE_VG_SAVER, saver);
   return em;
}

static void
_evas_cache_vg_data_free_cb(void *data)
{
   Vg_File_Data *vfd = data;
   vfd->loader->file_close(vfd);
}

static void
_evas_cache_vg_entry_free_cb(void *data)
{
   Vg_Cache_Entry *vg_entry = data;

   if (vg_entry->vfd)
     {
        vg_entry->vfd->ref--;
        if (vg_entry->vfd->ref <= 0)
          {
             Eina_Strbuf *hash_key = eina_strbuf_new();
             eina_strbuf_append_printf(hash_key, "%s/%s",
                                       vg_entry->file,
                                       vg_entry->key);
             if (!eina_hash_del(vg_cache->vfd_hash, eina_strbuf_string_get(hash_key), vg_entry->vfd))
               ERR("Failed to delete vfd = (%p) from hash", vg_entry->vfd);
             eina_strbuf_free(hash_key);
          }
     }

   eina_stringshare_del(vg_entry->key);
   free(vg_entry->hash_key);
   efl_unref(vg_entry->root);
   free(vg_entry);
}

static Eina_Bool
_vg_file_save(Vg_File_Data *vfd, const char *file, const char *key, const Efl_File_Save_Info *info)
{
   Evas_Module       *em;
   Evas_Vg_Save_Func *saver;
   int                error = EVAS_LOAD_ERROR_GENERIC;
   int                compress = 9;

   if (!file) return EINA_FALSE;

   if (info) compress = info->compression;

   em = _find_saver_module(file);
   if (em)
     {
        saver = em->functions;
        error = saver->file_save(vfd, file, key, compress);
     }

   if (error)
     return EINA_FALSE;

   return EINA_TRUE;
}

static Efl_VG*
_cached_root_get(Vg_Cache_Entry *vg_entry)
{
   return vg_entry->root;
}

static void
_caching_root_update(Vg_Cache_Entry *vg_entry)
{
   Vg_File_Data *vfd = vg_entry->vfd;

   /* Optimization: static viewbox may have same root data regardless of size.
      So we can't use the root data directly, but copy it for each vg_entries.
      In the meantime, non-static viewbox root data may have difference instance for each
      size. So it's affordable to share the root data for each vg_entries. */
   if (vfd->static_viewbox)
     {
        /* TODO: Yet trivial but still we may have a better solution to
           avoid this unnecessary copy. If the ector surface key is not
           to this root pointer. */
        vg_entry->root = efl_duplicate(vfd->root);
     }
   else if (vg_entry->root != vfd->root)
     {
        if (vg_entry->root) efl_unref(vg_entry->root);
        vg_entry->root = efl_ref(vfd->root);
     }
}

static void
_local_transform(Efl_VG *root, double w, double h, Vg_File_Data *vfd)
{
   double sx = 0, sy= 0, scale;
   Eina_Matrix3 m;

   if (!vfd->static_viewbox) return;
   if (vfd->view_box.w == w && vfd->view_box.h == h) return;

   sx = w / vfd->view_box.w;
   sy = h / vfd->view_box.h;

   scale = sx < sy ? sx : sy;
   eina_matrix3_identity(&m);

   // align hcenter and vcenter
   if (vfd->preserve_aspect)
     {
        eina_matrix3_translate(&m, (w - vfd->view_box.w * scale)/2.0, (h - vfd->view_box.h * scale)/2.0);
        eina_matrix3_scale(&m, scale, scale);
        eina_matrix3_translate(&m, -vfd->view_box.x, -vfd->view_box.y);
     }
   else
     {
        eina_matrix3_scale(&m, sx, sy);
        eina_matrix3_translate(&m, -vfd->view_box.x, -vfd->view_box.y);
     }
   efl_canvas_vg_node_transformation_set(root, &m);
}

void
evas_cache_vg_init(void)
{
   if (vg_cache)
     {
        vg_cache->ref++;
        return;
     }
   vg_cache = calloc(1, sizeof(Vg_Cache));
   if (!vg_cache)
     {
        CRI("Failed to alloc Vg_Cache");
        return;
     }

   vg_cache->vfd_hash = eina_hash_string_superfast_new(_evas_cache_vg_data_free_cb);
   vg_cache->vg_entry_hash = eina_hash_string_superfast_new(_evas_cache_vg_entry_free_cb);
   vg_cache->ref++;
}

void
evas_cache_vg_shutdown(void)
{
   if (!vg_cache) return;
   vg_cache->ref--;
   if (vg_cache->ref > 0) return;
   eina_hash_free(vg_cache->vfd_hash);
   eina_hash_free(vg_cache->vg_entry_hash);
   free(vg_cache);
   vg_cache = NULL;
}

Vg_File_Data *
evas_cache_vg_file_open(const char *file, const char *key)
{
   Vg_File_Data *vfd;
   Eina_Strbuf *hash_key;

   hash_key = eina_strbuf_new();
   eina_strbuf_append_printf(hash_key, "%s/%s", file, key);
   vfd = eina_hash_find(vg_cache->vfd_hash, eina_strbuf_string_get(hash_key));
   if (!vfd)
     {
        vfd = _vg_load_from_file(file, key);
        //File exists.
        if (vfd) eina_hash_add(vg_cache->vfd_hash, eina_strbuf_string_get(hash_key), vfd);
     }
   eina_strbuf_free(hash_key);
   return vfd;
}

Vg_Cache_Entry*
evas_cache_vg_entry_resize(Vg_Cache_Entry *vg_entry, int w, int h)
{
   return evas_cache_vg_entry_create(vg_entry->file, vg_entry->key, w, h);
}

Vg_Cache_Entry*
evas_cache_vg_entry_create(const char *file,
                           const char *key,
                           int w, int h)
{
   Vg_Cache_Entry* vg_entry;
   Eina_Strbuf *hash_key;

   if (!vg_cache) return NULL;

   //TODO: zero-sized entry is useless. how to skip it?

   hash_key = eina_strbuf_new();
   eina_strbuf_append_printf(hash_key, "%s/%s/%d/%d", file, key, w, h);
   vg_entry = eina_hash_find(vg_cache->vg_entry_hash, eina_strbuf_string_get(hash_key));
   if (!vg_entry)
     {
        vg_entry = calloc(1, sizeof(Vg_Cache_Entry));
        if (!vg_entry)
          {
             CRI("Failed to alloc Vg_Cache_Entry");
             eina_strbuf_free(hash_key);
             return NULL;
          }
        vg_entry->file = eina_stringshare_add(file);
        vg_entry->key = eina_stringshare_add(key);
        vg_entry->w = w;
        vg_entry->h = h;
        vg_entry->hash_key = eina_strbuf_string_steal(hash_key);
        eina_hash_direct_add(vg_cache->vg_entry_hash, vg_entry->hash_key, vg_entry);
     }
   eina_strbuf_free(hash_key);
   vg_entry->ref++;

   vg_entry->vfd = evas_cache_vg_file_open(file, key);
   //No File??
   if (!vg_entry->vfd)
     {
        evas_cache_vg_entry_del(vg_entry);
        return NULL;
     }
   vg_entry->vfd->ref++;

   return vg_entry;
}

Efl_VG*
evas_cache_vg_tree_get(Vg_Cache_Entry *vg_entry)
{
   if (!vg_entry) return NULL;
   if ((vg_entry->w < 1) || (vg_entry->h < 1)) return NULL;

   Vg_File_Data *vfd = vg_entry->vfd;
   if (!vfd) return NULL;

   Efl_VG *root = _cached_root_get(vg_entry);
   if (root) return root;

   if (!vfd->static_viewbox)
     {
        vfd->view_box.w = vg_entry->w;
        vfd->view_box.h = vg_entry->h;
     }

   if (!vfd->loader->file_data(vfd)) return NULL;

   _caching_root_update(vg_entry);

   _local_transform(vg_entry->root, vg_entry->w, vg_entry->h, vfd);

   return vg_entry->root;
}

void
evas_cache_vg_entry_del(Vg_Cache_Entry *vg_entry)
{
   if (!vg_cache || !vg_entry) return;
   vg_entry->ref--;
   if (vg_entry->ref > 0) return;
   if (!eina_hash_del(vg_cache->vg_entry_hash, vg_entry->hash_key, vg_entry))
     ERR("Failed to delete vg_entry = (%p) from hash", vg_entry);
}

Eina_Bool
evas_cache_vg_entry_file_save(Vg_Cache_Entry *vg_entry, const char *file, const char *key, const Efl_File_Save_Info *info)
{
   Vg_File_Data *vfd =
      evas_cache_vg_file_open(vg_entry->file, vg_entry->key);

   if (!vfd) return EINA_FALSE;

   return _vg_file_save(vfd, file, key, info);
}

Eina_Bool
evas_cache_vg_file_save(Efl_VG *root, int w, int h, const char *file, const char *key, const Efl_File_Save_Info *info)
{
   Vg_File_Data vfd = {};

   if (!root) return EINA_FALSE;

   vfd.view_box.x = w;
   vfd.view_box.y = h;
   vfd.root = root;
   vfd.preserve_aspect = EINA_FALSE;

   return _vg_file_save(&vfd, file, key, info);
}
