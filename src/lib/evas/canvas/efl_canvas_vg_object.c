#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_CANVAS_VG_OBJECT_CLASS

/* private magic number for rectangle objects */
static const char o_type[] = "vectors";

const char *o_vg_type = o_type;


static void _efl_canvas_vg_render(Evas_Object *eo_obj,
                                  Evas_Object_Protected_Data *obj,
                                  void *type_private_data,
                                  void *engine, void *output, void *context, void *surface,
                                  int x, int y, Eina_Bool do_async);
static void _efl_canvas_vg_render_pre(Evas_Object *eo_obj,
                                      Evas_Object_Protected_Data *obj,
                                      void *type_private_data);
static void _efl_canvas_vg_render_post(Evas_Object *eo_obj,
                                       Evas_Object_Protected_Data *obj,
                                       void *type_private_data);
static unsigned int _efl_canvas_vg_id_get(Evas_Object *eo_obj);
static unsigned int _efl_canvas_vg_visual_id_get(Evas_Object *eo_obj);
static void *_efl_canvas_vg_engine_data_get(Evas_Object *eo_obj);
static int _efl_canvas_vg_is_opaque(Evas_Object *eo_obj,
                                    Evas_Object_Protected_Data *obj,
                                    void *type_private_data);
static int _efl_canvas_vg_was_opaque(Evas_Object *eo_obj,
                                     Evas_Object_Protected_Data *obj,
                                     void *type_private_data);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
     _efl_canvas_vg_render,
     _efl_canvas_vg_render_pre,
     _efl_canvas_vg_render_post,
     _efl_canvas_vg_id_get,
     _efl_canvas_vg_visual_id_get,
     _efl_canvas_vg_engine_data_get,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     _efl_canvas_vg_is_opaque,
     _efl_canvas_vg_was_opaque,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL // render_prepare
};

static void
_evas_vg_tree_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = data;
   Efl_Canvas_Vg_Object_Data *pd = efl_data_scope_get(obj->object, MY_CLASS);

   if (pd->changed) return;

   pd->changed = EINA_TRUE;

   evas_object_change(obj->object, obj);
}

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
}

static void
_evas_vg_resize(void *data, const Efl_Event *ev)
{
   Efl_Canvas_Vg_Object_Data *pd = data;

   if (eina_rectangle_is_empty(&pd->viewbox.rect))
     return;
   _update_vgtree_viewport(ev->object, pd);
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

EOLIAN static Efl_VG *
_efl_canvas_vg_object_root_node_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Object_Data *pd)
{
   Efl_VG *root = NULL;

   if (pd->vg_entry)
     root = evas_cache_vg_tree_get(pd->vg_entry);
   else if (pd->user_entry)
     root = pd->user_entry->root;

   return root;
}

EOLIAN static void
_efl_canvas_vg_object_root_node_set(Eo *obj, Efl_Canvas_Vg_Object_Data *pd, Efl_VG *root_node)
{
   // if the same root is already set
   if (pd->user_entry && pd->user_entry->root == root_node)
     return;

   // check if a file has been already set
   if (pd->vg_entry)
     {
        evas_cache_vg_entry_del(pd->vg_entry);
        pd->vg_entry = NULL;
     }

   // detach/free the old root_node
   if (pd->user_entry && pd->user_entry->root)
     efl_parent_set(pd->user_entry->root, NULL);

   if (root_node)
     {
        if (!pd->user_entry)
          pd->user_entry = malloc(sizeof(User_Vg_Entry));

        pd->user_entry->w = pd->user_entry->h = 0;
        pd->user_entry->root = root_node;

        // set the parent so that vg canvas can render it.
        efl_parent_set(pd->user_entry->root, pd->root);

        efl_canvas_vg_node_root_set(root_node, obj);
     }
   else if (pd->user_entry)
     {
        // drop any surface cache attached to it.
        Evas_Object_Protected_Data *eobj = efl_data_scope_get(obj, EFL_CANVAS_OBJECT_CLASS);
        eobj->layer->evas->engine.func->ector_surface_cache_drop(_evas_engine_context(eobj->layer->evas),
                                                                 pd->user_entry);
        free(pd->user_entry);
        pd->user_entry = NULL;
     }

   // force a redraw
   pd->changed = EINA_TRUE;

   evas_object_change(obj, efl_data_scope_get(obj, EFL_CANVAS_OBJECT_CLASS));
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
             // un register the resize callback
             efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_RESIZE, _evas_vg_resize, pd);
          }
        return;
     }
   // register for resize callback if not done yet
   if (eina_rectangle_is_empty(&pd->viewbox.rect))
     efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_RESIZE, _evas_vg_resize, pd);

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

// file set and save api implementation

EOLIAN static Eina_Bool
_efl_canvas_vg_object_efl_file_file_set(Eo *obj, Efl_Canvas_Vg_Object_Data *pd, const char *file, const char *key)
{
   Evas_Cache_Vg_Entry *old_entry;
   int w, h;

   if (!file) return EINA_FALSE;

   old_entry = pd->vg_entry;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   pd->vg_entry = evas_cache_vg_entry_find(file, key, w, h);
   if (pd->vg_entry != old_entry)
     evas_object_change(obj, efl_data_scope_get(obj, EFL_CANVAS_OBJECT_CLASS));
   evas_cache_vg_entry_del(old_entry);

   return EINA_TRUE;
}

EOLIAN static void
_efl_canvas_vg_object_efl_file_file_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Object_Data *pd, const char **file, const char **key)
{
   if (pd->vg_entry)
     {
        if (file) *file = pd->vg_entry->file;
        if (key)  *key = pd->vg_entry->key;
     }
}

EOLIAN static Eina_Bool
_efl_canvas_vg_object_efl_file_save(const Eo *obj, Efl_Canvas_Vg_Object_Data *pd, const char *file, const char *key, const char *flags)
{
   Vg_File_Data tmp = {};
   Vg_File_Data *info = &tmp;

   if (pd->vg_entry && pd->vg_entry->file)
     {
        info = evas_cache_vg_file_info(pd->vg_entry->file, pd->vg_entry->key);
     }
   else
     {
        info->view_box.x = 0;
        info->view_box.y = 0;
        evas_object_geometry_get(obj, NULL, NULL, &info->view_box.w, &info->view_box.h);
        info->root = pd->root;
        info->preserve_aspect = EINA_FALSE;
     }
   return evas_vg_save_to_file(info, file, key, flags);
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

   efl_unref(pd->root);
   pd->root = NULL;

   if (pd->user_entry)
     free(pd->user_entry);
   pd->user_entry = NULL;

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

   /* root node */
   //FIXME: Well. I don't think this is necessary if user set a new root node...
   pd->root = efl_add_ref(EFL_CANVAS_VG_CONTAINER_CLASS, NULL);

   eina_array_step_set(&pd->cleanup, sizeof(pd->cleanup), 8);

   efl_event_callback_add(pd->root, EFL_GFX_PATH_EVENT_CHANGED, _evas_vg_tree_changed, obj);

   return eo_obj;
}

static Efl_Object *
_efl_canvas_vg_object_efl_object_finalize(Eo *obj, Efl_Canvas_Vg_Object_Data *pd)
{
   Evas *e = evas_object_evas_get(obj);

   // TODO: If we start to have to many Evas_Object_VG per canvas, it may be nice
   // to actually have one event per canvas and one array per canvas to.
   efl_event_callback_add(e, EFL_CANVAS_SCENE_EVENT_RENDER_POST, _cleanup_reference, pd);

   return obj;
}

static void
_evas_vg_render(Evas_Object_Protected_Data *obj, Efl_Canvas_Vg_Object_Data *vd,
                void *engine, void *output, void *context, void *surface, Efl_VG *n,
                Eina_Array *clips, Eina_Bool do_async)
{
   if (efl_isa(n, EFL_CANVAS_VG_CONTAINER_CLASS))
     {
        Efl_Canvas_Vg_Container_Data *vc;
        Efl_VG *child;
        Eina_List *l;

        vc = efl_data_scope_get(n, EFL_CANVAS_VG_CONTAINER_CLASS);

        EINA_LIST_FOREACH(vc->children, l, child)
          _evas_vg_render(obj, vd,
                          engine, output, context, surface, child,
                          clips, do_async);
     }
   else
     {
        Efl_Canvas_Vg_Node_Data *nd;
        nd = efl_data_scope_get(n, EFL_CANVAS_VG_NODE_CLASS);
        obj->layer->evas->engine.func->ector_renderer_draw(engine, output, context, surface, nd->renderer, clips, do_async);
        if (do_async)
          eina_array_push(&vd->cleanup, efl_ref(nd->renderer));
     }
}

// renders a vg_tree to an offscreen buffer
// and push it to the cache.
static void *
_render_to_buffer(Evas_Object_Protected_Data *obj, Efl_Canvas_Vg_Object_Data *vd,
                  void *engine, void *surface,
                  Efl_VG *root, int w, int h, void *key,
                  void *buffer, Eina_Bool do_async)
{
   Ector_Surface *ector;
   RGBA_Draw_Context *context;
   int error = 0;
   Eina_Bool buffer_created = EINA_FALSE;

   ector = evas_ector_get(obj->layer->evas);
   if (!ector) return NULL;

   if (!buffer)
     {
        // 2. create a buffer
        buffer = obj->layer->evas->engine.func->ector_surface_create(engine,
                                                                     w, h,
                                                                     &error);
        if (error)
          return NULL; // surface creation error
        buffer_created = EINA_TRUE;
     }


   //1. render pre
   _evas_vg_render_pre(root, ector, NULL);

   //3. draw into the buffer
   context = evas_common_draw_context_new();
   evas_common_draw_context_set_render_op(context, _EVAS_RENDER_COPY);
   evas_common_draw_context_set_color(context, 255, 255, 255, 255);
   obj->layer->evas->engine.func->ector_begin(engine, buffer,
                                              context, surface,
                                              ector,
                                              0, 0,
                                              do_async);
   _evas_vg_render(obj, vd,
                   engine, buffer,
                   context, surface,
                   root, NULL,
                   do_async);

   obj->layer->evas->engine.func->image_dirty_region(engine, buffer, 0, 0, 0, 0);
   obj->layer->evas->engine.func->ector_end(engine, buffer,
                                            context, surface,
                                            ector,do_async);

   evas_common_draw_context_free(context);

   if (buffer_created)
     obj->layer->evas->engine.func->ector_surface_cache_set(engine, key, buffer);

   return buffer;
}

static void
_render_buffer_to_screen(Evas_Object_Protected_Data *obj,
                         void *engine, void *output, void *context, void *surface,
                         void *buffer,
                         int x, int y, int w, int h,
                         Eina_Bool do_async)
{
   Eina_Bool async_unref;

   // draw the buffer as image to canvas
   async_unref = obj->layer->evas->engine.func->image_draw(engine, output, context, surface,
                                                           buffer,
                                                           0, 0, w, h,
                                                           x, y, w, h,
                                                           EINA_TRUE, do_async);
   if (do_async && async_unref)
     {
        evas_cache_image_ref((Image_Entry *)buffer);
        evas_unref_queue_image_put(obj->layer->evas, buffer);
     }
}

static void
_cache_vg_entry_render(Evas_Object_Protected_Data *obj,
                       Efl_Canvas_Vg_Object_Data *vd,
                       void *engine, void *output, void *context, void *surface,
                       int x, int y, int w, int h, Eina_Bool do_async)
{
   Evas_Cache_Vg_Entry *vg_entry = vd->vg_entry;
   Efl_VG *root, *dupe_root;
   void *buffer;

   // if the size changed in between path set and the draw call;
   if ((vg_entry->w != w) ||
       (vg_entry->h != h))
     {
         evas_cache_vg_entry_del(vg_entry);
         vg_entry = evas_cache_vg_entry_find(vg_entry->file, vg_entry->key,
                                             w, h);
         vd->vg_entry = vg_entry;
     }
   root = evas_cache_vg_tree_get(vg_entry);
   if (!root) return;
   buffer = obj->layer->evas->engine.func->ector_surface_cache_get(engine, root);

   // if the buffer is not created yet
   if (!buffer)
     {
        dupe_root = efl_duplicate(root);
        // render to the buffer
        buffer = _render_to_buffer(obj, vd,
                                   engine, surface,
                                   dupe_root,
                                   w, h,
                                   root,
                                   buffer,
                                   do_async);
        efl_unref(dupe_root);
     }
   else
     {
        obj->layer->evas->engine.func->ector_surface_cache_drop(engine, root);
     }

   _render_buffer_to_screen(obj,
                            engine, output, context, surface,
                            buffer,
                            x, y, w, h,
                            do_async);
}

static void
_user_vg_entry_render(Evas_Object_Protected_Data *obj,
                      Efl_Canvas_Vg_Object_Data *vd,
                      void *engine, void *output, void *context, void *surface,
                      int x, int y, int w, int h, Eina_Bool do_async)
{
   User_Vg_Entry *user_entry = vd->user_entry;
   void *buffer;

   // if the size dosen't match
   if ((user_entry->w != w ) ||
       (user_entry->h != h))
     {
         obj->layer->evas->engine.func->ector_surface_cache_drop(engine, user_entry);
         user_entry->w = w;
         user_entry->h = h;
         vd->user_entry = user_entry;
     }
   // if the buffer is not created yet
   buffer = obj->layer->evas->engine.func->ector_surface_cache_get(engine, user_entry);
   if (!buffer)
     {
        // render to the buffer
        buffer = _render_to_buffer(obj, vd,
                                   engine, surface,
                                   user_entry->root,
                                   w, h,
                                   user_entry,
                                   buffer,
                                   do_async);
     }
   else
     {
        // render to the buffer
        if (vd->changed)
          buffer = _render_to_buffer(obj, vd,
                                     engine, surface,
                                     user_entry->root,
                                     w, h,
                                     user_entry,
                                     buffer,
                                     do_async);
        obj->layer->evas->engine.func->ector_surface_cache_drop(engine, user_entry);
     }

   _render_buffer_to_screen(obj,
                            engine, output, context, surface,
                            buffer,
                            x, y, w, h,
                            do_async);
}

static void
_efl_canvas_vg_render(Evas_Object *eo_obj EINA_UNUSED,
                      Evas_Object_Protected_Data *obj,
                      void *type_private_data,
                      void *engine, void *output, void *context, void *surface,
                      int x, int y, Eina_Bool do_async)
{
   Efl_Canvas_Vg_Object_Data *vd = type_private_data;

   /* render object to surface with context, and offxet by x,y */
   obj->layer->evas->engine.func->context_color_set(engine,
                                                    context,
                                                    255,
                                                    255,
                                                    255,
                                                    255);
   obj->layer->evas->engine.func->context_multiplier_set(engine,
                                                         context,
                                                         obj->cur->cache.clip.r,
                                                         obj->cur->cache.clip.g,
                                                         obj->cur->cache.clip.b,
                                                         obj->cur->cache.clip.a);
   obj->layer->evas->engine.func->context_anti_alias_set(engine, context,
                                                         obj->cur->anti_alias);
   obj->layer->evas->engine.func->context_render_op_set(engine, context,
                                                        obj->cur->render_op);
   if (vd->vg_entry)
     {
        _cache_vg_entry_render(obj, vd,
                               engine, output, context, surface,
                               obj->cur->geometry.x + x, obj->cur->geometry.y + y,
                               obj->cur->geometry.w, obj->cur->geometry.h, do_async);
     }
   if (vd->user_entry)
     {
        _user_vg_entry_render(obj, vd,
                              engine, output, context, surface,
                              obj->cur->geometry.x + x, obj->cur->geometry.y + y,
                              obj->cur->geometry.w, obj->cur->geometry.h, do_async);
     }
   vd->changed = EINA_FALSE;
}

static void
_efl_canvas_vg_render_pre(Evas_Object *eo_obj,
                          Evas_Object_Protected_Data *obj,
                          void *type_private_data)
{
   Efl_Canvas_Vg_Object_Data *vd = type_private_data;
   int is_v, was_v;
   Ector_Surface *s;

   /* dont pre-render the obj twice! */
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
   s = evas_ector_get(obj->layer->evas);
   if (vd->root && s)
     _evas_vg_render_pre(vd->root, s, NULL);

   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(eo_obj, obj);
   was_v = evas_object_was_visible(eo_obj,obj);
   if (!(is_v | was_v)) goto done;

   if (vd->changed)
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
_efl_canvas_vg_render_post(Evas_Object *eo_obj EINA_UNUSED,
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

static unsigned int
_efl_canvas_vg_id_get(Evas_Object *eo_obj)
{
   Efl_Canvas_Vg_Object_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_VG;
}

static unsigned int
_efl_canvas_vg_visual_id_get(Evas_Object *eo_obj)
{
   Efl_Canvas_Vg_Object_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_SHAPE;
}

static void *
_efl_canvas_vg_engine_data_get(Evas_Object *eo_obj)
{
   Efl_Canvas_Vg_Object_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   return o->engine_data;
}

static int
_efl_canvas_vg_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
                         Evas_Object_Protected_Data *obj EINA_UNUSED,
                         void *type_private_data EINA_UNUSED)
{
   return 0;
}

static int
_efl_canvas_vg_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
                          Evas_Object_Protected_Data *obj EINA_UNUSED,
                          void *type_private_data EINA_UNUSED)
{
   return 0;
}

#include "efl_canvas_vg_object.eo.c"
