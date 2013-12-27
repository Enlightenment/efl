#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "evas_common_private.h"
#include "evas_private.h"

static inline void
_texture_proxy_set(Evas_3D_Texture *texture, Evas_Object *eo_src, Evas_Object_Protected_Data *src)
{
   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_src)
     {
        proxy_src->proxy_textures = eina_list_append(proxy_src->proxy_textures, texture);
        proxy_src->redraw = EINA_TRUE;
     }
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_src);

   texture->source = eo_src;
}

static inline void
_texture_proxy_unset(Evas_3D_Texture *texture)
{
   Evas_Object_Protected_Data *src = eo_data_scope_get(texture->source, EVAS_OBJ_CLASS);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_src);
     {
        proxy_src->proxy_textures = eina_list_remove(proxy_src->proxy_textures, texture);

        if (eina_list_count(proxy_src->proxy_textures) == 0)
          {
             if (eina_list_count(proxy_src->proxies) == 0)
               {
                  Evas_Public_Data *e = src->layer->evas;
                  e->engine.func->image_map_surface_free(e->engine.data.output,
                                                         proxy_src->surface);
                  proxy_src->surface = NULL;
               }
          }

        if (proxy_src->src_invisible)
          {
             proxy_src->src_invisible = EINA_FALSE;
             src->changed_src_visible = EINA_TRUE;
             evas_object_change(texture->source, src);
             evas_object_smart_member_cache_invalidate(texture->source,
                                                       EINA_FALSE, EINA_FALSE, EINA_TRUE);
          }
     }
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_src);

   texture->source = NULL;
}

static inline void
_texture_proxy_subrender(Evas_3D_Texture *texture)
{
   /* Code taken from _proxy_subrender() in file evas_object_image.c */

   Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);
   Evas_Object_Protected_Data *source;
   void *ctx, *image;
   int w, h;
   Eina_Bool is_image;

   if (!texture->source)
     return;

   source = eo_data_scope_get(texture->source, EVAS_OBJ_CLASS);

   is_image = eo_isa(texture->source, EVAS_OBJ_IMAGE_CLASS);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, source->proxy, Evas_Object_Proxy_Data, proxy_write)
     {
        proxy_write->redraw = EINA_FALSE;

        if (is_image)
          {
             image = source->func->engine_data_get(texture->source);
             e->engine.func->image_size_get(e->engine.data.output, image, &w, &h);
          }
        else
          {
             w = source->cur->geometry.w;
             h = source->cur->geometry.h;
          }

        /* We need to redraw surface then */
        if ((proxy_write->surface) &&
            ((proxy_write->w != w) || (proxy_write->h != h)))
          {
             e->engine.func->image_map_surface_free(e->engine.data.output,
                                                    proxy_write->surface);
             proxy_write->surface = NULL;
          }

        /* FIXME: Hardcoded alpha 'on' */
        /* FIXME (cont): Should see if the object has alpha */
        if (!proxy_write->surface)
          {
             proxy_write->surface = e->engine.func->image_map_surface_new
               (e->engine.data.output, w, h, 1);
             if (!proxy_write->surface) goto end;
             proxy_write->w = w;
             proxy_write->h = h;
          }

        ctx = e->engine.func->context_new(e->engine.data.output);
        e->engine.func->context_color_set(e->engine.data.output, ctx, 0, 0,
                                          0, 0);
        e->engine.func->context_render_op_set(e->engine.data.output, ctx,
                                              EVAS_RENDER_COPY);
        e->engine.func->rectangle_draw(e->engine.data.output, ctx,
                                       proxy_write->surface, 0, 0, w, h,
                                       EINA_FALSE);
        e->engine.func->context_free(e->engine.data.output, ctx);

        ctx = e->engine.func->context_new(e->engine.data.output);

        if (is_image)
          {
             void *image = source->func->engine_data_get(texture->source);

             if (image)
               {
                  int imagew, imageh;
                  e->engine.func->image_size_get(e->engine.data.output, image,
                                                 &imagew, &imageh);
                  e->engine.func->image_draw(e->engine.data.output, ctx,
                                             proxy_write->surface, image,
                                             0, 0, imagew, imageh, 0, 0, w, h, 0, EINA_FALSE);
               }
          }
        else
          {
             evas_render_mapped(e, texture->source, source, ctx, proxy_write->surface,
                                -source->cur->geometry.x,
                                -source->cur->geometry.y,
                                1, 0, 0, e->output.w, e->output.h,
                                NULL
#ifdef REND_DBG
                                , 1
#endif
                                , EINA_FALSE);
          }

        e->engine.func->context_free(e->engine.data.output, ctx);
        proxy_write->surface = e->engine.func->image_dirty_region
           (e->engine.data.output, proxy_write->surface, 0, 0, w, h);
     }
 end:
   EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, proxy_write);
}

static void
_texture_fini(Evas_3D_Texture *texture)
{
   if (texture->engine_data)
     {
        Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);
        e->engine.func->texture_free(e->engine.data.output, texture->engine_data);
        texture->engine_data = NULL;
     }

   if (texture->materials)
     {
        eina_hash_free(texture->materials);
        texture->materials = NULL;
     }

   if (texture->source)
     {
        _texture_proxy_unset(texture);
     }
}

static void
_texture_free(Evas_3D_Object *obj)
{
   Evas_3D_Texture *texture = (Evas_3D_Texture *)obj;

   _texture_fini(texture);
   free(texture);
}

static Eina_Bool
_texture_material_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                                  void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Material *m = *(Evas_3D_Material **)key;
   evas_3d_object_change(&m->base, EVAS_3D_STATE_MATERIAL_TEXTURE, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static void
_texture_change(Evas_3D_Object *obj, Evas_3D_State state EINA_UNUSED,
                Evas_3D_Object *ref EINA_UNUSED)
{
   Evas_3D_Texture  *texture = (Evas_3D_Texture *)obj;

   if (texture->materials)
     eina_hash_foreach(texture->materials, _texture_material_change_notify, obj);
}

static void
_texture_update(Evas_3D_Object *obj)
{
   Evas_3D_Texture *texture = (Evas_3D_Texture *)obj;

   if (texture->source)
     {
        Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);
        Evas_Object_Protected_Data *src = eo_data_scope_get(texture->source, EVAS_OBJ_CLASS);

        if (texture->engine_data == NULL)
          {
             texture->engine_data = e->engine.func->texture_new(e->engine.data.output);

             if (texture->engine_data == NULL)
               {
                  ERR("Failed to create engine-side texture object.");
                  return;
               }
          }

        if (src->proxy->surface && !src->proxy->redraw)
          {
             e->engine.func->texture_image_set(e->engine.data.output, texture->engine_data,
                                               src->proxy->surface);
             return;

          }

        texture->proxy_rendering = EINA_TRUE;
        _texture_proxy_subrender(texture);

        e->engine.func->texture_image_set(e->engine.data.output, texture->engine_data,
                                          src->proxy->surface);
        texture->proxy_rendering = EINA_FALSE;
     }
}

static const Evas_3D_Object_Func texture_func =
{
   _texture_free,
   _texture_change,
   _texture_update,
};

void
evas_3d_texture_material_add(Evas_3D_Texture *texture, Evas_3D_Material *material)
{
   int count = 0;

   if (texture->materials == NULL)
     {
        texture->materials = eina_hash_pointer_new(NULL);

        if (texture->materials == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)eina_hash_find(texture->materials, &material);

   /* Increase reference count or add new one if not exist. */
   eina_hash_set(texture->materials, &material, (const void *)(count + 1));
}

void
evas_3d_texture_material_del(Evas_3D_Texture *texture, Evas_3D_Material *material)
{
   int count = 0;

   if (texture->materials == NULL)
     {
        ERR("No material to delete.");
        return;
     }

   count = (int)eina_hash_find(texture->materials, &material);

   if (count == 1)
     eina_hash_del(texture->materials, &material, NULL);
   else
     eina_hash_set(texture->materials, &material, (const void *)(count - 1));
}

Evas_3D_Texture *
evas_3d_texture_new(Evas *e)
{
   Evas_3D_Texture *texture = NULL;

   texture = (Evas_3D_Texture *)calloc(1, sizeof(Evas_3D_Texture));

   if (texture == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   evas_3d_object_init(&texture->base, e, EVAS_3D_OBJECT_TYPE_TEXTURE, &texture_func);
   return texture;
}

EAPI Evas_3D_Texture *
evas_3d_texture_add(Evas *e)
{
   return evas_3d_texture_new(e);
}

EAPI void
evas_3d_texture_del(Evas_3D_Texture *texture)
{
   evas_3d_object_unreference(&texture->base);
}

EAPI Evas *
evas_3d_texture_evas_get(const Evas_3D_Texture *texture)
{
   return texture->base.evas;
}

EAPI void
evas_3d_texture_data_set(Evas_3D_Texture *texture, Evas_3D_Color_Format color_format,
                         Evas_3D_Pixel_Format pixel_format, int w, int h, const void *data)
{
   Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);

   if (texture->engine_data == NULL)
     texture->engine_data = e->engine.func->texture_new(e->engine.data.output);

   e->engine.func->texture_data_set(e->engine.data.output, texture->engine_data,
                                    color_format, pixel_format, w, h, data);

   evas_3d_object_change(&texture->base, EVAS_3D_STATE_TEXTURE_DATA, NULL);
}

EAPI void
evas_3d_texture_file_set(Evas_3D_Texture *texture, const char *file, const char *key)
{
   Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);

   if (texture->engine_data == NULL)
     texture->engine_data = e->engine.func->texture_new(e->engine.data.output);

   e->engine.func->texture_file_set(e->engine.data.output, texture->engine_data, file, key);
   evas_3d_object_change(&texture->base, EVAS_3D_STATE_TEXTURE_DATA, NULL);
}

EAPI void
evas_3d_texture_source_set(Evas_3D_Texture *texture, Evas_Object *source)
{
   Evas_Object_Protected_Data *src;

   if (source == texture->source)
     return;

   _texture_fini(texture);

   if (source == NULL)
     return;

   if (evas_object_evas_get(source) != texture->base.evas)
     {
        ERR("Not matching canvas.");
        return;
     }

   src = eo_data_scope_get(source, EVAS_OBJ_CLASS);

   if (src->delete_me)
     {
        ERR("Source object is deleted.");
        return;
     }

   if (src->layer == NULL)
     {
        ERR("No evas surface associated with the source object.");
        return;
     }

   _texture_proxy_set(texture, source, src);
   evas_3d_object_change(&texture->base, EVAS_3D_STATE_TEXTURE_DATA, NULL);
}

EAPI Evas_3D_Color_Format
evas_3d_texture_color_format_get(const Evas_3D_Texture *texture)
{
   Evas_Public_Data    *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);
   Evas_3D_Color_Format format;

   e->engine.func->texture_color_format_get(e->engine.data.output, texture->engine_data, &format);
   return format;
}

EAPI void
evas_3d_texture_size_get(const Evas_3D_Texture *texture, int *w, int *h)
{
   Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);
   e->engine.func->texture_size_get(e->engine.data.output, texture->engine_data, w, h);
}

EAPI void
evas_3d_texture_wrap_set(Evas_3D_Texture *texture,
                         Evas_3D_Wrap_Mode s, Evas_3D_Wrap_Mode t)
{
   Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);
   e->engine.func->texture_wrap_set(e->engine.data.output, texture->engine_data, s, t);
   evas_3d_object_change(&texture->base, EVAS_3D_STATE_TEXTURE_WRAP, NULL);
}

EAPI void
evas_3d_texture_wrap_get(const Evas_3D_Texture *texture,
                         Evas_3D_Wrap_Mode *s, Evas_3D_Wrap_Mode *t)
{
   Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);
   e->engine.func->texture_wrap_get(e->engine.data.output, texture->engine_data, s, t);
}

EAPI void
evas_3d_texture_filter_set(Evas_3D_Texture *texture,
                           Evas_3D_Texture_Filter min, Evas_3D_Texture_Filter mag)
{
   Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);
   e->engine.func->texture_filter_set(e->engine.data.output, texture->engine_data, min, mag);
   evas_3d_object_change(&texture->base, EVAS_3D_STATE_TEXTURE_FILTER, NULL);
}

EAPI void
evas_3d_texture_filter_get(const Evas_3D_Texture *texture,
                           Evas_3D_Texture_Filter *min, Evas_3D_Texture_Filter *mag)
{
   Evas_Public_Data *e = eo_data_scope_get(texture->base.evas, EVAS_CLASS);
   e->engine.func->texture_filter_get(e->engine.data.output, texture->engine_data, min, mag);
}
