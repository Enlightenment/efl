#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_CANVAS_VG_OBJECT_CLASS

/* private magic number for vector objects */
static const char o_type[] = "vectors";

const char *o_vg_type = o_type;


static void _efl_canvas_vg_object_render(Evas_Object *eo_obj,
                                         Evas_Object_Protected_Data *obj,
                                         void *type_private_data,
                                         void *engine, void *output, void *context, void *surface,
                                         int x, int y, Eina_Bool do_async);
static void _efl_canvas_vg_object_render_pre(Evas_Object *eo_obj,
                                             Evas_Object_Protected_Data *obj,
                                             void *type_private_data);
static void _efl_canvas_vg_object_render_post(Evas_Object *eo_obj,
                                              Evas_Object_Protected_Data *obj,
                                              void *type_private_data);
static int _efl_canvas_vg_object_is_opaque(Evas_Object *eo_obj,
                                           Evas_Object_Protected_Data *obj,
                                           void *type_private_data);
static int _efl_canvas_vg_object_was_opaque(Evas_Object *eo_obj,
                                            Evas_Object_Protected_Data *obj,
                                            void *type_private_data);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
   _efl_canvas_vg_object_render,
   _efl_canvas_vg_object_render_pre,
   _efl_canvas_vg_object_render_post,
   NULL,
   /* these are optional. NULL = nothing */
   NULL,
   NULL,
   _efl_canvas_vg_object_is_opaque,
   _efl_canvas_vg_object_was_opaque,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL // render_prepare
};

static void
_update_vgtree_viewport(Eo *obj, Efl_Canvas_Vg_Object_Data *pd)
{
   double vb_w, vb_h, vp_w, vp_h, scale_w, scale_h, scale;
   Eina_Size2D sz = efl_gfx_entity_size_get(obj);
   Eina_Matrix3 m;

   eina_matrix3_identity(&m);

   vb_w = pd->viewbox.w;
   vb_h = pd->viewbox.h;
   vp_w = sz.w;
   vp_h = sz.h;

   scale_w = vp_w / vb_w;
   scale_h = vp_h / vb_h;

   if (pd->fill_mode == EFL_CANVAS_VG_FILL_MODE_STRETCH)
     { // Fill the viewport and ignore the aspect ratio
        eina_matrix3_scale(&m, scale_w, scale_h);
        eina_matrix3_translate(&m, -pd->viewbox.x, -pd->viewbox.y);
     }
   else
     {
        if (pd->fill_mode == EFL_CANVAS_VG_FILL_MODE_MEET)
          scale = scale_w < scale_h ? scale_w : scale_h;
        else // slice
          scale = scale_w > scale_h ? scale_w : scale_h;
        eina_matrix3_translate(&m, (vp_w - vb_w * scale) * pd->align_x, (vp_h - vb_h * scale) * pd->align_y);
        eina_matrix3_scale(&m, scale, scale);
        eina_matrix3_translate(&m, -pd->viewbox.x, -pd->viewbox.y);
     }

   efl_canvas_vg_node_transformation_set(pd->root, &m);

   pd->changed = EINA_TRUE;
   evas_object_change(obj, efl_data_scope_get(obj, EFL_CANVAS_OBJECT_CLASS));
}

static void
_evas_vg_resize(void *data, const Efl_Event *ev)
{
   Efl_Canvas_Vg_Object_Data *pd = data;

   if (eina_rectangle_is_empty(&pd->viewbox.rect))
     return;
   _update_vgtree_viewport(ev->object, pd);
}

EOLIAN static Efl_VG *
_efl_canvas_vg_object_root_node_get(const Eo *obj, Efl_Canvas_Vg_Object_Data *pd)
{
   Efl_VG *root;

   if (pd->vg_entry)
     {
        Evas_Coord w, h;
        evas_object_geometry_get(obj, NULL, NULL, &w, &h);

        //Update vg data with current size.
        if ((pd->vg_entry->w != w) || (pd->vg_entry->h != h))
          {
             Vg_Cache_Entry *vg_entry = evas_cache_vg_entry_resize(pd->vg_entry, w, h);
             evas_cache_vg_entry_del(pd->vg_entry);
             pd->vg_entry = vg_entry;
          }
        root = evas_cache_vg_tree_get(pd->vg_entry, pd->frame_idx);
     }
   else if (pd->user_entry) root = pd->user_entry->root;
   else root = pd->root;

   return root;
}

EOLIAN static void
_efl_canvas_vg_object_root_node_set(Eo *eo_obj, Efl_Canvas_Vg_Object_Data *pd, Efl_VG *root_node)
{
   // if the same root is already set
   if (pd->user_entry && pd->user_entry->root == root_node)
     return;

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   // check if a file has been already set
   if (pd->vg_entry)
     {
        evas_cache_vg_entry_del(pd->vg_entry);
        pd->vg_entry = NULL;
     }

   // detach/free the old root_node
   if (pd->user_entry && pd->user_entry->root)
     {
        efl_canvas_vg_node_vg_obj_set(pd->user_entry->root, NULL, NULL);
        efl_parent_set(pd->user_entry->root, NULL);
     }

   if (root_node)
     {
        if (!pd->user_entry)
          {
             pd->user_entry = malloc(sizeof(Vg_User_Entry));
             if (!pd->user_entry)
               {
                  ERR("Failed to alloc user entry data while setting root node");
                  return;
               }
          }
        pd->user_entry->w = pd->user_entry->h = 0;
        pd->user_entry->root = root_node;

        // set the parent so that vg canvas can render it.
        efl_parent_set(pd->user_entry->root, pd->root);
        efl_canvas_vg_node_vg_obj_set(root_node, eo_obj, pd);
     }
   else if (pd->user_entry)
     {
        // drop any surface cache attached to it.
        ENFN->ector_surface_cache_drop(_evas_engine_context(obj->layer->evas), pd->user_entry->root);
        free(pd->user_entry);
        pd->user_entry = NULL;
     }

   // force a redraw
   pd->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_efl_canvas_vg_object_fill_mode_set(Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Object_Data *pd, Efl_Canvas_Vg_Fill_Mode fill_mode)
{
   pd->fill_mode = fill_mode;
}

EOLIAN static Efl_Canvas_Vg_Fill_Mode
_efl_canvas_vg_object_fill_mode_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Object_Data *pd)
{
   return pd->fill_mode;
}

EOLIAN static void
_efl_canvas_vg_object_viewbox_set(Eo *obj, Efl_Canvas_Vg_Object_Data *pd, Eina_Rect viewbox)
{
   // viewbox should be a valid rectangle
   if (eina_rectangle_is_empty(&viewbox.rect))
     {
        // reset the old viewbox if any
        if (!eina_rectangle_is_empty(&pd->viewbox.rect))
          {
             Eina_Matrix3 m;

             pd->viewbox = EINA_RECT_EMPTY();
             eina_matrix3_identity(&m);
             efl_canvas_vg_node_transformation_set(pd->root, &m);
             // unregister the resize callback
             efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _evas_vg_resize, pd);
          }
        return;
     }
   // register for resize callback if not done yet
   if (eina_rectangle_is_empty(&pd->viewbox.rect))
     efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _evas_vg_resize, pd);

   pd->viewbox = viewbox;
   _update_vgtree_viewport(obj, pd);
}

EOLIAN static Eina_Rect
_efl_canvas_vg_object_viewbox_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Object_Data *pd)
{
   return pd->viewbox;
}

EOLIAN static void
_efl_canvas_vg_object_viewbox_align_set(Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Object_Data *pd, double align_x, double align_y)
{
   align_x = align_x < 0 ? 0 : align_x;
   align_x = align_x > 1 ? 1 : align_x;

   align_y = align_y < 0 ? 0 : align_y;
   align_y = align_y > 1 ? 1 : align_y;

   pd->align_x = align_x;
   pd->align_y = align_y;
}

EOLIAN static void
_efl_canvas_vg_object_viewbox_align_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Object_Data *pd, double *align_x, double *align_y)
{
   if (align_x) *align_x = pd->align_x;
   if (align_y) *align_y = pd->align_y;
}

EOLIAN static Eina_Error
_efl_canvas_vg_object_efl_file_file_set(Eo *eo_obj, Efl_Canvas_Vg_Object_Data *pd EINA_UNUSED, const char *file)
{
   /* Careful: delete previous vg entry.
      When a new efl file is set, ex-file will be invalid.
      Since vg cache hashes all file entries,
      we must remove it from vg cache before we lost file handle. */
   if (efl_file_loaded_get(eo_obj))
     {
        const char *pname = efl_file_get(eo_obj);
        int pl = pname ? strlen(pname) : 0;
        int cl = file ? strlen(file) : 0;

        if ((pl != cl) || strcmp(pname, file))
          {
             Evas_Object_Protected_Data *obj;
             obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
             evas_cache_vg_entry_del(pd->vg_entry);
             evas_object_change(eo_obj, obj);
             pd->vg_entry = NULL;
             evas_object_change(eo_obj, obj);
          }
     }

   Eina_Error err;
   err = efl_file_set(efl_super(eo_obj, MY_CLASS), file);
   if (err) return err;

   return 0;
}

EOLIAN static Eina_Error
_efl_canvas_vg_object_efl_file_load(Eo *eo_obj, Efl_Canvas_Vg_Object_Data *pd)
{
   Eina_Error err;
   if (efl_file_loaded_get(eo_obj)) return 0;

   err = efl_file_load(efl_super(eo_obj, MY_CLASS));
   if (err) return err;

   const Eina_File *file = efl_file_mmap_get(eo_obj);
   const char *key = efl_file_key_get(eo_obj);
   Evas_Object_Protected_Data *obj;

   obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   pd->vg_entry = evas_cache_vg_entry_create(file, key,
                                             obj->cur->geometry.w,
                                             obj->cur->geometry.h);
   evas_object_change(eo_obj, obj);

   return 0;
}

EOLIAN static void
_efl_canvas_vg_object_efl_file_unload(Eo *eo_obj, Efl_Canvas_Vg_Object_Data *pd)
{
   if (!efl_file_loaded_get(eo_obj)) return;

   Evas_Object_Protected_Data *obj;
   obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_cache_vg_entry_del(pd->vg_entry);
   evas_object_change(eo_obj, obj);
   pd->vg_entry = NULL;
}

EOLIAN static Eina_Bool
_efl_canvas_vg_object_efl_file_save_save(const Eo *obj, Efl_Canvas_Vg_Object_Data *pd, const char *file, const char *key, const Efl_File_Save_Info *info)
{
   if (pd->vg_entry)
     return evas_cache_vg_entry_file_save(pd->vg_entry, file, key, info);

   Evas_Coord w, h;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   return evas_cache_vg_file_save(pd->root, w, h, file, key, info);
}

static void
_cleanup_reference(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Canvas_Vg_Object_Data *pd = data;
   Eo *renderer;

   /* unref all renderer and may also destroy them async */
   while ((renderer = eina_array_pop(&pd->cleanup)))
     efl_unref(renderer);
}

EOLIAN static void
_efl_canvas_vg_object_efl_object_destructor(Eo *eo_obj, Efl_Canvas_Vg_Object_Data *pd)
{
   Evas *e = evas_object_evas_get(eo_obj);

   efl_event_callback_del(e, EFL_CANVAS_SCENE_EVENT_RENDER_POST, _cleanup_reference, pd);
   eina_array_flush(&pd->cleanup);

   efl_unref(pd->root);
   pd->root = NULL;

   if (pd->user_entry) free(pd->user_entry);
   pd->user_entry = NULL;
   evas_cache_vg_entry_del(pd->vg_entry);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_canvas_vg_object_efl_object_constructor(Eo *eo_obj, Efl_Canvas_Vg_Object_Data *pd)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = efl_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;

   /* default root node */
   pd->obj = obj;
   pd->root = efl_add_ref(EFL_CANVAS_VG_CONTAINER_CLASS, NULL);

   eina_array_step_set(&pd->cleanup, sizeof(pd->cleanup), 8);

   return eo_obj;
}

static Efl_Object *
_efl_canvas_vg_object_efl_object_finalize(Eo *obj, Efl_Canvas_Vg_Object_Data *pd)
{
   Evas *e = evas_object_evas_get(obj);
   efl_parent_set(pd->root, obj);

   // TODO: If we start to have to many Evas_Object_VG per canvas, it may be nice
   // to actually have one event per canvas and one array per canvas to.
   efl_event_callback_add(e, EFL_CANVAS_SCENE_EVENT_RENDER_POST, _cleanup_reference, pd);

   return obj;
}

static void
_evas_vg_render(Evas_Object_Protected_Data *obj, Efl_Canvas_Vg_Object_Data *pd,
                void *engine, void *output, void *context, Efl_VG *node,
                Eina_Array *clips, Eina_Bool do_async)
{
   if (!efl_gfx_entity_visible_get(node)) return;

   if (efl_isa(node, EFL_CANVAS_VG_CONTAINER_CLASS))
     {
        Efl_Canvas_Vg_Container_Data *cd =
           efl_data_scope_get(node, EFL_CANVAS_VG_CONTAINER_CLASS);

        if (cd->mask.target) return;   //Don't draw mask itself.

        Efl_VG *child;
        Eina_List *l;

        EINA_LIST_FOREACH(cd->children, l, child)
          _evas_vg_render(obj, pd, engine, output, context, child, clips, do_async);
     }
   else
     {
        Efl_Canvas_Vg_Node_Data *nd = efl_data_scope_get(node, EFL_CANVAS_VG_NODE_CLASS);
        ENFN->ector_renderer_draw(engine, output, context, nd->renderer, clips, do_async);
        if (do_async) eina_array_push(&pd->cleanup, efl_ref(nd->renderer));
     }
}

//renders a vg_tree to an offscreen buffer and push it to the cache.
static void *
_render_to_buffer(Evas_Object_Protected_Data *obj, Efl_Canvas_Vg_Object_Data *pd,
                  void *engine, Efl_VG *root, int w, int h, void *key,
                  void *buffer, Eina_Bool do_async, Eina_Bool cacheable)
{
   Ector_Surface *ector;
   RGBA_Draw_Context *context;
   int error = 0;
   Eina_Bool buffer_created = EINA_FALSE;

   ector = evas_ector_get(obj->layer->evas);
   if (!ector) return NULL;

   //create a buffer
   if (!buffer)
     {
        buffer = ENFN->ector_surface_create(engine, w, h, &error);
        if (error) return NULL;
        buffer_created = EINA_TRUE;
     }

   //initialize buffer
   context = evas_common_draw_context_new();
   evas_common_draw_context_set_render_op(context, _EVAS_RENDER_COPY);
   evas_common_draw_context_set_color(context, 255, 255, 255, 255);

   //ector begin - end for drawing mask images.
   //ENFN->ector_begin(engine, buffer, context, ector, 0, 0, EINA_FALSE, EINA_FALSE);
   _evas_vg_render_pre(obj, root, engine, buffer, context, ector, NULL, NULL, 0);
   //ENFN->ector_end(engine, buffer, context, ector, EINA_FALSE);

   //Actual content drawing
   ENFN->ector_begin(engine, buffer, context, ector, 0, 0, EINA_TRUE, do_async);

   //draw on buffer
   _evas_vg_render(obj, pd,
                   engine, buffer,
                   context, root,
                   NULL,
                   do_async);

   ENFN->image_dirty_region(engine, buffer, 0, 0, w, h);
   ENFN->ector_end(engine, buffer, context, ector, do_async);
   evas_common_draw_context_free(context);

   if (buffer_created && cacheable)
     {
        ENFN->ector_surface_cache_set(engine, key, buffer);
        pd->cached_frame_idx = pd->frame_idx;
     }

   return buffer;
}

static void
_render_buffer_to_screen(Evas_Object_Protected_Data *obj,
                         void *engine, void *output, void *context, void *surface,
                         void *buffer,
                         int x, int y, int w, int h,
                         Eina_Bool do_async, Eina_Bool cacheable)
{
   Eina_Bool async_unref;

   //Draw the buffer as image to canvas
   async_unref = ENFN->image_draw(engine, output, context, surface,
                                                           buffer,
                                                           0, 0, w, h,
                                                           x, y, w, h,
                                                           EINA_TRUE, do_async);
   if (do_async && async_unref)
     {
        //Free buffer after drawing.
        evas_cache_image_ref((Image_Entry *)buffer);
        evas_unref_queue_image_put(obj->layer->evas, buffer);
     }

   //TODO: Reuse buffer if size is same?
   if (!cacheable) ENFN->ector_surface_destroy(engine, buffer);
}

static void
_cache_vg_entry_render(Evas_Object_Protected_Data *obj,
                       Efl_Canvas_Vg_Object_Data *pd,
                       void *engine, void *output, void *context, void *surface,
                       int x, int y, int w, int h, Eina_Bool do_async,
                       Eina_Bool cacheable)
{
   Vg_Cache_Entry *vg_entry = pd->vg_entry;
   Efl_VG *root;
   Eina_Position2D offset = {0, 0};  //Offset after keeping aspect ratio.
   Eina_Bool drop_cache = EINA_FALSE;
   void *buffer = NULL;

   // if the size changed in between path set and the draw call;
   if ((vg_entry->w != w) ||
       (vg_entry->h != h))
     {
        Eina_Size2D size = evas_cache_vg_entry_default_size_get(pd->vg_entry);

        //adjust size for aspect ratio.
        if (size.w > 0 && size.h > 0)
          {
             float rw = (float) w / (float) size.w;
             float rh = (float) h / (float) size.h;

             if (rw < rh)
               {
                  size.w = w;
                  size.h = (int) ((float) size.h * rw);
               }
             else
               {
                  size.w = (int) ((float) size.w * rh);
                  size.h = h;
               }
          }
        else
          {
              size.w = w;
              size.h = h;
          }

        //Size is changed, cached data is invalid.
        if ((size.w != vg_entry->w) || (size.h != vg_entry->h))
          {
             drop_cache = EINA_TRUE;
             vg_entry = evas_cache_vg_entry_resize(vg_entry, size.w, size.h);
             evas_cache_vg_entry_del(pd->vg_entry);
             pd->vg_entry = vg_entry;
          }

        //update for adjusted pos and size.
        offset.x = w - size.w;
        if (offset.x > 0) offset.x /= 2;
        offset.y = h - size.h;
        if (offset.y > 0) offset.y /= 2;
        w = size.w;
        h = size.h;

     }
   root = evas_cache_vg_tree_get(vg_entry, pd->frame_idx);
   if (!root) return;

   if (cacheable)
     {
        //if the size doesn't match, drop previous cache surface.
        if (drop_cache)
          ENFN->ector_surface_cache_drop(engine, (void *) root);
        //Cache Hit!
        else if (pd->frame_idx == pd->cached_frame_idx)
          buffer = ENFN->ector_surface_cache_get(engine, (void *) root);
        //Drop invalid one.
        else
          ENFN->ector_surface_cache_drop(engine, (void *) root);
     }

   if (!buffer)
     buffer = _render_to_buffer(obj, pd, engine, root, w, h, root, NULL,
                                do_async, cacheable);
   else
     //cache reference was increased when we get the cache.
     ENFN->ector_surface_cache_drop(engine, (void *) root);

   _render_buffer_to_screen(obj,
                            engine, output, context, surface,
                            buffer,
                            x + offset.x, y + offset.y, w, h,
                            do_async, cacheable);
}

static void
_user_vg_entry_render(Evas_Object_Protected_Data *obj,
                      Efl_Canvas_Vg_Object_Data *pd,
                      void *engine, void *output, void *context, void *surface,
                      int x, int y, int w, int h, Eina_Bool do_async,
                      Eina_Bool cacheable)
{
   Vg_User_Entry *user_entry = pd->user_entry;

   //if the size doesn't match, drop previous cache surface.
   if ((user_entry->w != w ) ||
       (user_entry->h != h))
     {
         ENFN->ector_surface_cache_drop(engine, user_entry->root);
         user_entry->w = w;
         user_entry->h = h;
     }

   //if the buffer is not created yet
   void *buffer = NULL;

   if (cacheable)
     buffer = ENFN->ector_surface_cache_get(engine, user_entry->root);

   if (!buffer)
     {
        // render to the buffer
        buffer = _render_to_buffer(obj, pd, engine, user_entry->root,
                                   w, h, user_entry, buffer,
                                   do_async, cacheable);
     }
   else
     {
        // render to the buffer
        if (pd->changed)
          buffer = _render_to_buffer(obj, pd, engine,
                                     user_entry->root,
                                     w, h,
                                     user_entry,
                                     buffer,
                                     do_async, EINA_FALSE);
        //cache reference was increased when we get the cache.
        ENFN->ector_surface_cache_drop(engine, user_entry->root);
     }

   _render_buffer_to_screen(obj,
                            engine, output, context, surface,
                            buffer,
                            x, y, w, h,
                            do_async, cacheable);
}

static void
_efl_canvas_vg_object_render(Evas_Object *eo_obj EINA_UNUSED,
                             Evas_Object_Protected_Data *obj,
                             void *type_private_data,
                             void *engine, void *output, void *context, void *surface,
                             int x, int y, Eina_Bool do_async)
{
   Efl_Canvas_Vg_Object_Data *pd = type_private_data;

   /* render object to surface with context, and offxet by x,y */
   ENFN->context_color_set(engine, context, 255, 255, 255, 255);
   ENFN->context_multiplier_set(engine, context,
                                obj->cur->cache.clip.r,
                                obj->cur->cache.clip.g,
                                obj->cur->cache.clip.b,
                                obj->cur->cache.clip.a);
   ENFN->context_anti_alias_set(engine, context, obj->cur->anti_alias);
   ENFN->context_render_op_set(engine, context, obj->cur->render_op);

   //Cache surface?
   Eina_Bool cacheable = EINA_FALSE;

   /* Try caching buffer only for first and last frames
      because it's an overhead task if it caches all frame images.
      We assume the first and last frame images are the most resusable
      in generic scenarios. */
   if (pd->frame_idx == 0 ||
       (pd->frame_idx == (int) (evas_cache_vg_anim_frame_count_get(pd->vg_entry) - 1)))
     cacheable = EINA_TRUE;

   if (pd->vg_entry)
     {
        _cache_vg_entry_render(obj, pd,
                               engine, output, context, surface,
                               obj->cur->geometry.x + x, obj->cur->geometry.y + y,
                               obj->cur->geometry.w, obj->cur->geometry.h, do_async, cacheable);
     }
   if (pd->user_entry)
     {
        _user_vg_entry_render(obj, pd,
                              engine, output, context, surface,
                              obj->cur->geometry.x + x, obj->cur->geometry.y + y,
                              obj->cur->geometry.w, obj->cur->geometry.h, do_async, cacheable);
     }
   pd->changed = EINA_FALSE;
}

static void
_efl_canvas_vg_object_render_pre(Evas_Object *eo_obj,
                                 Evas_Object_Protected_Data *obj,
                                 void *type_private_data)
{
   Efl_Canvas_Vg_Object_Data *pd = type_private_data;
   int is_v, was_v;
   Ector_Surface *s;

   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;

   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur->clipper)
     {
        if (obj->cur->cache.clip.dirty)
          evas_object_clip_recalc(obj->cur->clipper);
        obj->cur->clipper->func->render_pre(obj->cur->clipper->object,
                                            obj->cur->clipper,
                                            obj->cur->clipper->private_data);
     }

   // FIXME: handle damage only on changed renderer.
   // FIXME: Move this render_pre to efl_canvas_vg_render()
   s = evas_ector_get(obj->layer->evas);
   if (pd->root && s)
     _evas_vg_render_pre(obj, pd->root, NULL, NULL, NULL, s, NULL, NULL, 0);

   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(eo_obj, obj);
   was_v = evas_object_was_visible(eo_obj,obj);
   if (!(is_v | was_v)) goto done;

   if (pd->changed)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
        goto done;
     }

   if (is_v != was_v)
     {
        evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes, eo_obj, is_v, was_v);
        goto done;
     }
   if (obj->changed_map || obj->changed_src_visible)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, eo_obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if ((obj->restack) && (!obj->clip.clipees))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
        goto done;
     }
   /* if it changed render op */
   if (obj->cur->render_op != obj->prev->render_op)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
        goto done;
     }
   /* if it changed color */
   if ((obj->cur->color.r != obj->prev->color.r) ||
       (obj->cur->color.g != obj->prev->color.g) ||
       (obj->cur->color.b != obj->prev->color.b) ||
       (obj->cur->color.a != obj->prev->color.a))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
        goto done;
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur->geometry.x != obj->prev->geometry.x) ||
       (obj->cur->geometry.y != obj->prev->geometry.y) ||
       (obj->cur->geometry.w != obj->prev->geometry.w) ||
       (obj->cur->geometry.h != obj->prev->geometry.h))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
        goto done;
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* arent fully opaque and we are visible */
   if (evas_object_is_visible(eo_obj, obj) &&
       evas_object_is_opaque(eo_obj, obj) &&
       (!obj->clip.clipees))
     {
        Evas_Coord x, y, w, h;

        x = obj->cur->cache.clip.x;
        y = obj->cur->cache.clip.y;
        w = obj->cur->cache.clip.w;
        h = obj->cur->cache.clip.h;
        if (obj->cur->clipper)
          {
             RECTS_CLIP_TO_RECT(x, y, w, h,
                                obj->cur->clipper->cur->cache.clip.x,
                                obj->cur->clipper->cur->cache.clip.y,
                                obj->cur->clipper->cur->cache.clip.w,
                                obj->cur->clipper->cur->cache.clip.h);
          }
        evas_render_update_del(obj->layer->evas,
                               x + obj->layer->evas->framespace.x,
                               y + obj->layer->evas->framespace.y,
                               w, h);
     }

done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes, eo_obj, is_v, was_v);
}

static void
_efl_canvas_vg_object_render_post(Evas_Object *eo_obj EINA_UNUSED,
                                  Evas_Object_Protected_Data *obj,
                                  void *type_private_data EINA_UNUSED)
{
   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(obj);
}

static int
_efl_canvas_vg_object_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
                                Evas_Object_Protected_Data *obj EINA_UNUSED,
                                void *type_private_data EINA_UNUSED)
{
   return 0;
}

static int
_efl_canvas_vg_object_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
                                 Evas_Object_Protected_Data *obj EINA_UNUSED,
                                 void *type_private_data EINA_UNUSED)
{
   return 0;
}

/* animated feature */
EOLIAN static Eina_Bool
_efl_canvas_vg_object_efl_gfx_frame_controller_animated_get(const Eo *eo_obj EINA_UNUSED,
                                                                      Efl_Canvas_Vg_Object_Data *pd EINA_UNUSED EINA_UNUSED)
{
   //TODO:
   return EINA_TRUE;
}

EOLIAN static int
_efl_canvas_vg_object_efl_gfx_frame_controller_frame_count_get(const Eo *eo_obj EINA_UNUSED,
                                                                                  Efl_Canvas_Vg_Object_Data *pd EINA_UNUSED)
{
   if (!pd->vg_entry) return 0;
   return evas_cache_vg_anim_frame_count_get(pd->vg_entry);
}

EOLIAN static Efl_Gfx_Frame_Controller_Loop_Hint
_efl_canvas_vg_object_efl_gfx_frame_controller_loop_type_get(const Eo *eo_obj EINA_UNUSED,
                                                                                Efl_Canvas_Vg_Object_Data *pd EINA_UNUSED)
{
   //TODO:
   return EFL_GFX_FRAME_CONTROLLER_LOOP_HINT_NONE;
}

EOLIAN static int
_efl_canvas_vg_object_efl_gfx_frame_controller_loop_count_get(const Eo *eo_obj EINA_UNUSED,
                                                                                 Efl_Canvas_Vg_Object_Data *pd EINA_UNUSED)
{
   //TODO:
   return 0;
}

EOLIAN static double
_efl_canvas_vg_object_efl_gfx_frame_controller_frame_duration_get(const Eo *eo_obj EINA_UNUSED,
                                                                                     Efl_Canvas_Vg_Object_Data *pd,
                                                                                     int start_frame EINA_UNUSED,
                                                                                     int frame_num EINA_UNUSED)
{
   if (!pd->vg_entry) return 0;
   return evas_cache_vg_anim_duration_get(pd->vg_entry);
}

EOLIAN static Eina_Bool
_efl_canvas_vg_object_efl_gfx_frame_controller_frame_set(Eo *eo_obj,
                                                                            Efl_Canvas_Vg_Object_Data *pd,
                                                                            int frame_index)
{
   //TODO: Validate frame_index range
   if (pd->frame_idx == frame_index) return EINA_TRUE;

   //Image is changed, drop previous cached image.
   pd->frame_idx = frame_index;
   pd->changed = EINA_TRUE;
   evas_object_change(eo_obj, efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS));

   return EINA_TRUE;
}

EOLIAN static int
_efl_canvas_vg_object_efl_gfx_frame_controller_frame_get(const Eo *eo_obj EINA_UNUSED,
                                                                            Efl_Canvas_Vg_Object_Data *pd EINA_UNUSED)
{
   return pd->frame_idx;
}

EOLIAN static Eina_Size2D
_efl_canvas_vg_object_default_size_get(const Eo *eo_obj EINA_UNUSED,
                                       Efl_Canvas_Vg_Object_Data *pd EINA_UNUSED)
{
   return evas_cache_vg_entry_default_size_get(pd->vg_entry);
}

/* the actual api call to add a vector graphic object */
EAPI Evas_Object *
evas_object_vg_add(Evas *e)
{
   e = evas_find(e);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(e, EVAS_CANVAS_CLASS), NULL);
   // TODO: Ask backend to return the main Ector_Surface
   return efl_add(MY_CLASS, e, efl_canvas_object_legacy_ctor(efl_added));
}

EAPI int
evas_object_vg_animated_frame_get(const Evas_Object *obj)
{
   return efl_gfx_frame_controller_frame_get(obj);
}

EAPI double
evas_object_vg_animated_frame_duration_get(const Evas_Object *obj, int start_frame, int frame_num)
{
   return efl_gfx_frame_controller_frame_duration_get(obj, start_frame, frame_num);
}

EAPI int
evas_object_vg_animated_frame_count_get(const Evas_Object *obj)
{
   return efl_gfx_frame_controller_frame_count_get(obj);
}

EAPI Eina_Bool
evas_object_vg_animated_frame_set(Evas_Object *obj, int frame_index)
{
   return efl_gfx_frame_controller_frame_set(obj, frame_index);
}

EAPI Eina_Bool
evas_object_vg_mmap_set(Evas_Object *obj, const Eina_File *f, const char *key)
{
   return efl_file_simple_mmap_load(obj, f, key);
}

EAPI Eina_Bool
evas_object_vg_file_set(Evas_Object *obj, const char *file, const char *key)
{
   return efl_file_simple_load(obj, file, key);
}

#include "efl_canvas_vg_object.eo.c"
