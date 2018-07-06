#include "evas_image_private.h"
#include "efl_canvas_scene3d.eo.h"

#define MY_CLASS EFL_CANVAS_SCENE3D_CLASS

EOLIAN static void
_efl_canvas_scene3d_scene3d_set(Eo *eo_obj, void *pd EINA_UNUSED, Evas_Canvas3D_Scene *scene)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   Evas_Image_Load_Opts lo;

   if (o->cur->scene == scene) return;

   evas_object_async_block(obj);
   _evas_image_init_set(NULL, NULL, eo_obj, obj, o, &lo);
   o->engine_data = ENFN->image_mmap(ENC, o->cur->f, o->cur->key, &o->load_error, &lo);
   _evas_image_done_set(eo_obj, obj, o);

   if (scene) _evas_image_3d_set(eo_obj, scene);
   else _evas_image_3d_unset(eo_obj, obj, o);
}

EOLIAN static Evas_Canvas3D_Scene *
_efl_canvas_scene3d_scene3d_get(const Eo *eo_obj, void *pd EINA_UNUSED)
{
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   return o->cur->scene;
}

void
_evas_image_3d_render(Evas *eo_e, Evas_Object *eo_obj,
                      Evas_Object_Protected_Data *obj, Evas_Image_Data *o EINA_UNUSED,
                      Evas_Canvas3D_Scene *scene, void *engine, void *output)
{
   Evas_Public_Data *e;
   Eina_Bool need_native_set = EINA_FALSE;
   Evas_Canvas3D_Scene_Public_Data scene_data;
   Evas_Canvas3D_Scene_Data *pd_scene = NULL;

   pd_scene = efl_data_scope_get(scene, EVAS_CANVAS3D_SCENE_CLASS);

   if ((pd_scene->w == 0) || (pd_scene->h == 0)) return;
   if (!pd_scene->camera_node)
     {
        WRN("Camera has not been set to scene(%p)", scene);
        return;
     }

   e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if (pd_scene->surface)
     {
        int w = 0;
        int h = 0;

        if (e->engine.func->drawable_size_get)
          {
             e->engine.func->drawable_size_get(engine,
                                               pd_scene->surface, &w, &h);
          }
        if ((w != pd_scene->w) || (h != pd_scene->h))
          {
             if (e->engine.func->drawable_free)
               {
                  e->engine.func->drawable_free(engine,
                                                pd_scene->surface);
               }
             pd_scene->surface = NULL;
             need_native_set = EINA_TRUE;
          }
     }
   else
     {
        /* TODO: Hard-coded alpha on. */
        if (e->engine.func->drawable_new)
          {
             pd_scene->surface =
               e->engine.func->drawable_new(engine,
                                            pd_scene->w, pd_scene->h, 1);
          }
        need_native_set = EINA_TRUE;
     }

   EINA_COW_WRITE_BEGIN(evas_object_3d_cow, obj->data_3d, Evas_Object_3D_Data,
                        data)
     {
        if (need_native_set)
          {
             if (e->engine.func->image_drawable_set)
               {
                  data->surface =
                    e->engine.func->image_drawable_set(engine,
                                                       data->surface,
                                                       pd_scene->surface);
               }
          }
        data->w = pd_scene->w;
        data->h = pd_scene->h;
     }
   EINA_COW_WRITE_END(evas_object_3d_cow, obj->data_3d, data);

   evas_canvas3d_scene_data_init(&scene_data);

   scene_data.bg_color = pd_scene->bg_color;
   scene_data.shadows_enabled = pd_scene->shadows_enabled;
   scene_data.camera_node = pd_scene->camera_node;
   scene_data.depth_offset = pd_scene->depth_offset;
   scene_data.depth_constant = pd_scene->depth_constant;
   if (evas_object_anti_alias_get(eo_obj))
     {
        /*Use post processing render*/
        scene_data.post_processing = EINA_TRUE;
        scene_data.color_pick_enabled = EINA_FALSE;
        scene_data.render_to_texture = EINA_TRUE;
        scene_data.post_processing_type = EVAS_CANVAS3D_SHADER_MODE_POST_PROCESSING_FXAA;
     }
   /* Phase 1 - Update scene graph tree. */
   evas_canvas3d_object_update(scene);

   /* Phase 2 - Do frustum culling and get visible model nodes. */
   evas_canvas3d_node_tree_traverse(pd_scene->root_node,
                              EVAS_CANVAS3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_TRUE,
                              evas_canvas3d_node_mesh_collect, &scene_data);

   /* Phase 3 - Collect active light nodes in the scene graph tree. */
   evas_canvas3d_node_tree_traverse(pd_scene->root_node,
                              EVAS_CANVAS3D_TREE_TRAVERSE_ANY_ORDER, EINA_FALSE,
                              evas_canvas3d_node_light_collect, &scene_data);

   /* Phase 5 - Draw the scene. */
   if (e->engine.func->drawable_scene_render)
     {
        e->engine.func->drawable_scene_render(engine, output,
                                              pd_scene->surface, &scene_data);
     }
   /* Clean up temporary resources. */
   evas_canvas3d_scene_data_fini(&scene_data);
}

void
_evas_image_3d_set(Evas_Object *eo_obj, Evas_Canvas3D_Scene *scene)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   Evas_Canvas3D_Scene_Data *pd_scene = efl_data_scope_get(scene,
                                                    EVAS_CANVAS3D_SCENE_CLASS);
   EINA_COW_WRITE_BEGIN(evas_object_3d_cow, obj->data_3d, Evas_Object_3D_Data,
                        data)
     {
        data->surface = NULL;
        data->w = 0;
        data->h = 0;
        efl_ref(scene);
     }
   EINA_COW_WRITE_END(evas_object_3d_cow, obj->data_3d, data);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     {
        state_write->scene = scene;
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   pd_scene->images = eina_list_append(pd_scene->images, eo_obj);
}

void
_evas_image_3d_unset(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj,
                     Evas_Image_Data *o)
{
   Evas_Public_Data *e;

   if (!o->cur->scene) return;

   Evas_Canvas3D_Scene_Data *pd_scene =
      efl_data_scope_get(o->cur->scene, EVAS_CANVAS3D_SCENE_CLASS);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     {
        pd_scene->images = eina_list_remove(pd_scene->images, eo_obj);
        efl_unref(state_write->scene);
        state_write->scene = NULL;
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   if (o->cur->defmap)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          {
             evas_map_free(state_write->defmap);
             state_write->defmap = NULL;
          }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }

   EINA_COW_WRITE_BEGIN(evas_object_3d_cow, obj->data_3d, Evas_Object_3D_Data,
                        data)
     {
        e = obj->layer->evas;

        if (data->surface)
          e->engine.func->image_free(_evas_engine_context(e), data->surface);

        data->surface = NULL;
        data->w = 0;
        data->h = 0;
     }
   EINA_COW_WRITE_END(evas_object_3d_cow, obj->data_3d, data);
}

EOLIAN static Eina_Rw_Slice
_efl_canvas_scene3d_efl_gfx_buffer_buffer_map(Eo *eo_obj, void *_pd EINA_UNUSED,
                                              Efl_Gfx_Buffer_Access_Mode mode,
                                              const Eina_Rect *region,
                                              Efl_Gfx_Colorspace cspace, int plane,
                                              int *stride)
{
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   Evas_Public_Data *e;
   Evas_Canvas3D_Object_Data *pd_parent;
   Evas_Canvas3D_Scene_Data *pd_scene;
   int width = -1, height = -1, ntex = -1;
   unsigned char *pixels = NULL;
   Eina_Rw_Slice slice = {};
   int x, y, w, h;
   size_t len = 0;

   if (!o->cur->scene)
     {
        ERR("invalid scene data");
        return slice;
     }
   if (mode & EFL_GFX_BUFFER_ACCESS_MODE_WRITE)
     {
        ERR("invalid map access mode");
        return slice;
     }
   if (cspace != EFL_GFX_COLORSPACE_ARGB8888)
     {
        ERR("invalid map colorspace. Only ARGB is supported");
        return slice;
     }

   pd_parent = efl_data_scope_get(o->cur->scene, EVAS_CANVAS3D_OBJECT_CLASS);
   e = efl_data_scope_get(pd_parent->evas, EVAS_CANVAS_CLASS);
   pd_scene = efl_data_scope_get(o->cur->scene, EVAS_CANVAS3D_SCENE_CLASS);

   if (e->engine.func->drawable_size_get)
      {
         e->engine.func->drawable_size_get(_evas_engine_context(e),
                                           pd_scene->surface, &width, &height);
      }

   if (region)
     {
        x = region->x;
        y = region->y;
        w = region->w;
        h = region->h;
     }
   else
     {
        x = y = 0;
        w = width;
        h = height;
     }

   if ((x < 0) || (y < 0) || (w <= 0) || (h <= 0) || ((x + w) > width) || ((y + h) > height))
     {
        ERR("Invalid map dimensions : %dx%d +%d,%d. Image is %dx%d.",
            w, h, x, y, width, height);
        return slice;
     }

   if (e->engine.func->drawable_texture_target_id_get)
     {
        ntex = e->engine.func->drawable_texture_target_id_get(pd_scene->surface);

        if (e->engine.func->drawable_texture_rendered_pixels_get)
          {
             len = w * h * sizeof(DATA32); //four component texture
             pixels = malloc(len + sizeof(slice) + 8);
             e->engine.func->drawable_texture_rendered_pixels_get(ntex, x, y, w, h,
                                                                  pd_scene->surface, pixels);
          }
        else
          return slice;
     }
   else
     return slice;

   if (stride) *stride = w * sizeof(DATA32);
   slice.mem = pixels;
   slice.len = len;
   DBG("map(%p, %d,%d %dx%d plane:%d) -> " EINA_SLICE_FMT,
       eo_obj, x, y, w, h, plane, EINA_SLICE_PRINT(slice));

   return slice;
}
EOLIAN static Eina_Bool
_efl_canvas_scene3d_efl_gfx_buffer_buffer_unmap(Eo *eo_obj EINA_UNUSED, void *_pd EINA_UNUSED,
                                                const Eina_Rw_Slice slice)
{
   free(slice.mem);
   return EINA_TRUE;
}

#include "efl_canvas_scene3d.eo.c"
