#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_vg_private.h"

static Evas_Cache_Vg* vg_cache = NULL;

struct ext_loader_s
{
   unsigned int length;
   const char *extension;
   const char *loader;
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

Vg_File_Data *
_vg_load_from_file(const char *file, const char *key)
{
   Evas_Module       *em;
   Evas_Vg_Load_Func *loader;
   int                error = EVAS_LOAD_ERROR_GENERIC;
   Vg_File_Data          *evg_data = NULL;
   unsigned int i;

   em = _find_loader_module(file);
   if (em)
     {
        loader = em->functions;
        evg_data = loader->file_data(file, key, &error);
     }
   else
     {
        for (i = 0; i < sizeof (loaders_name) / sizeof (char *); i++)
          {
             em = evas_module_find_type(EVAS_MODULE_TYPE_VG_LOADER, loaders_name[i]);
             if (em)
               {
                  loader = em->functions;
                  evg_data = loader->file_data(file, key, &error);
                  if (evg_data)
                    return evg_data;
               }
             else
               DBG("could not find module '%s'", loaders_name[i]);
          }
        INF("exhausted all means to load image '%s'", file);
     }
   return evg_data;
}


// evg file save
struct ext_saver_s
{
   unsigned int length;
   const char *extension;
   const char *saver;
};

static const struct ext_saver_s savers[] =
{ /* map extensions to savers to use for good first-guess tries */
   MATCHING(".eet", "eet"),
   MATCHING(".edj", "eet"),
   MATCHING(".svg", "svg")
};

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

Eina_Bool
evas_vg_save_to_file(Vg_File_Data *evg_data, const char *file, const char *key, const char *flags)
{
   Evas_Module       *em;
   Evas_Vg_Save_Func *saver;
   int                error = EVAS_LOAD_ERROR_GENERIC;
   int                compress = 9;

   if (!file) return EINA_FALSE;

   if (flags)
     {
        char *p, *pp;
        char *tflags;

        tflags = alloca(strlen(flags) + 1);
        strcpy(tflags, flags);
        p = tflags;
        while (p)
          {
             pp = strchr(p, ' ');
             if (pp) *pp = 0;
             sscanf(p, "compress=%i", &compress);
             if (pp) p = pp + 1;
             else break;
          }
     }

   em = _find_saver_module(file);
   if (em)
     {
        saver = em->functions;
        error = saver->vg_save(evg_data, file, key, compress);
     }

   if (error)
     return EINA_FALSE;

   return EINA_TRUE;
}



static void
_evas_cache_vg_data_free_cb(void *data)
{
   Vg_File_Data *val = data;
   efl_unref(val->root);
   free(val);
}

static void
_evas_cache_svg_entry_free_cb(void *data)
{
   Evas_Cache_Vg_Entry *entry = data;

   eina_stringshare_del(entry->file);
   eina_stringshare_del(entry->key);
   free(entry->hash_key);
   efl_unref(entry->root);
   free(entry);
}

void
evas_cache_vg_init(void)
{
   if (vg_cache)
     {
        vg_cache->ref++;
        return;
     }
   vg_cache =  calloc(1, sizeof(Evas_Cache_Vg));
   vg_cache->vg_hash = eina_hash_string_superfast_new(_evas_cache_vg_data_free_cb);
   vg_cache->active = eina_hash_string_superfast_new(_evas_cache_svg_entry_free_cb);
   vg_cache->ref++;
}

void
evas_cache_vg_shutdown(void)
{
   if (!vg_cache) return;
   vg_cache->ref--;
   if (vg_cache->ref) return;
   eina_hash_free(vg_cache->vg_hash);
   eina_hash_free(vg_cache->active);
   free(vg_cache);
   vg_cache = NULL;
}

Vg_File_Data *
evas_cache_vg_file_info(const char *file, const char *key)
{
   Vg_File_Data *vd;
   Eina_Strbuf *hash_key;

   hash_key = eina_strbuf_new();
   eina_strbuf_append_printf(hash_key, "%s/%s", file, key);
   vd = eina_hash_find(vg_cache->vg_hash, eina_strbuf_string_get(hash_key));
   if (!vd)
     {
        vd = _vg_load_from_file(file, key);
        eina_hash_add(vg_cache->vg_hash, eina_strbuf_string_get(hash_key), vd);
     }
   eina_strbuf_free(hash_key);
   return vd;
}

static void
_apply_transformation(Efl_VG *root, double w, double h, Vg_File_Data *vg_data)
{
   double sx = 0, sy= 0, scale;
   Eina_Matrix3 m;

   if (vg_data->view_box.w)
     sx = w/vg_data->view_box.w;
   if (vg_data->view_box.h)
     sy = h/vg_data->view_box.h;
   scale = sx < sy ? sx: sy;
   eina_matrix3_identity(&m);
   // allign hcenter and vcenter
   if (vg_data->preserve_aspect)
     {
        eina_matrix3_translate(&m, (w - vg_data->view_box.w * scale)/2.0, (h - vg_data->view_box.h * scale)/2.0);
        eina_matrix3_scale(&m, scale, scale);
        eina_matrix3_translate(&m, -vg_data->view_box.x, -vg_data->view_box.y);
     }
   else
     {
        eina_matrix3_scale(&m, sx, sy);
        eina_matrix3_translate(&m, -vg_data->view_box.x, -vg_data->view_box.y);
     }
   efl_canvas_vg_node_transformation_set(root, &m);
}

static Efl_VG *
_evas_vg_dup_vg_tree(Vg_File_Data *fd, double w, double h)
{
   Efl_VG *root;

   if (!fd) return NULL;
   if (w < 1 || h < 1) return NULL;

   root = efl_duplicate(fd->root);
   _apply_transformation(root, w, h, fd);

   return root;
}

static void
_evas_cache_svg_vg_tree_update(Evas_Cache_Vg_Entry *entry)
{
   Vg_File_Data *src_vg = NULL;
   if(!entry) return;

   if (!entry->file)
     {
        entry->root = NULL;
        return;
     }

   src_vg = evas_cache_vg_file_info(entry->file, entry->key);

   entry->root = _evas_vg_dup_vg_tree(src_vg, entry->w, entry->h);
   eina_stringshare_del(entry->file);
   eina_stringshare_del(entry->key);
   entry->file = NULL;
   entry->key = NULL;
}

Evas_Cache_Vg_Entry*
evas_cache_vg_entry_find(const char *file, const char *key,
                         int w, int h)
{
   Evas_Cache_Vg_Entry* se;
   Eina_Strbuf *hash_key;

   if (!vg_cache) return NULL;

   hash_key = eina_strbuf_new();
   eina_strbuf_append_printf(hash_key, "%s/%s/%d/%d",
                             file, key, w, h);
   se = eina_hash_find(vg_cache->active, eina_strbuf_string_get(hash_key));
   if (!se)
     {
        se = calloc(1, sizeof(Evas_Cache_Vg_Entry));
        se->file = eina_stringshare_add(file);
        se->key = eina_stringshare_add(key);
        se->w = w;
        se->h = h;
        se->hash_key = eina_strbuf_string_steal(hash_key);
        eina_hash_direct_add(vg_cache->active, se->hash_key, se);
     }
   eina_strbuf_free(hash_key);
   se->ref++;
   return se;
}

Efl_VG*
evas_cache_vg_tree_get(Evas_Cache_Vg_Entry *entry)
{
   if (entry->root) return entry->root;

   if (entry->file)
     _evas_cache_svg_vg_tree_update(entry);

   return entry->root;
}

void
evas_cache_vg_entry_del(Evas_Cache_Vg_Entry *svg_entry)
{
   if (!svg_entry) return;

   svg_entry->ref--;
   // FIXME implement delete logic (LRU)
}

