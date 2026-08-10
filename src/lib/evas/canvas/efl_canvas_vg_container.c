#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_CANVAS_VG_CONTAINER_CLASS

/**
 * Return EINA_TRUE if the composite method requires a mask texture
 * (i.e., it belongs to the matte/mask family handled by the GL span path).
 */
static inline Eina_Bool
_comp_method_needs_mask(Efl_Gfx_Vg_Composite_Method m)
{
   return (m == EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA ||
           m == EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA_INVERSE ||
           m == EFL_GFX_VG_COMPOSITE_METHOD_MASK_INTERSECT ||
           m == EFL_GFX_VG_COMPOSITE_METHOD_MASK_SUBSTRACT);
}

static void
_invalidate_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Canvas_Vg_Container_Data *pd;
   Eina_List *l;
   Efl_VG* child;

   pd = efl_data_scope_get(event->object, MY_CLASS);

   /* Clean up all the references by copying.
      If the container is copied, it copies its children as well.
      Since evas_vg_node is duplicated using efl_add_ref(),
      we definitely call efl_unref() after that. */
   EINA_LIST_FOREACH(pd->children, l, child)
      efl_unref(child);
}

static void
_draw_comp(Evas_Object_Protected_Data *obj, Efl_VG *node,
           Ector_Surface *ector, void *engine, void *output,
           void *context)
{
   if (!efl_gfx_entity_visible_get(node)) return;

   if (efl_isa(node, EFL_CANVAS_VG_CONTAINER_CLASS))
     {
        Efl_Canvas_Vg_Container_Data *cd =
           efl_data_scope_get(node, EFL_CANVAS_VG_CONTAINER_CLASS);

        //Draw Composite Image.
        Efl_VG *child;
        Eina_List *l;
        EINA_LIST_FOREACH(cd->children, l, child)
          _draw_comp(obj, child, ector, engine, output, context);
     }
   else
     {
        Efl_Canvas_Vg_Node_Data *nd = efl_data_scope_get(node, EFL_CANVAS_VG_NODE_CLASS);
        ENFN->ector_renderer_draw(engine, output, context, nd->renderer, NULL, EINA_FALSE);
     }
}

static Ector_Buffer *
_prepare_comp(Evas_Object_Protected_Data *obj,     //vector object
              Efl_Canvas_Vg_Node* comp_target,
              void *engine, void *output, void *context,
              Ector_Surface *surface,
              Eina_Matrix3 *ptransform,
              Eina_Matrix3 *ctransform,
              int p_opacity,
              int c_opacity,
              Ector_Buffer *comp,
              Efl_Gfx_Vg_Composite_Method comp_method)
{
   Efl_Canvas_Vg_Container_Data *pd = efl_data_scope_get(comp_target, MY_CLASS);
   Efl_Canvas_Vg_Node_Data *nd =
         efl_data_scope_get(comp_target, EFL_CANVAS_VG_NODE_CLASS);
   if (nd->flags == EFL_GFX_CHANGE_FLAG_NONE) return pd->comp.buffer;
   uint32_t init_buffer = 0x0;

   //1. Composite Size
   Eina_Size2D size;
   size.w = obj->cur->geometry.w;
   size.h = obj->cur->geometry.h;

   //FIXME: If composite method is SUBSTRACT or INTERSECT, buffer fills in white color(Full alpha color).
   if (pd->comp.method == EFL_GFX_VG_COMPOSITE_METHOD_MASK_SUBSTRACT ||
       pd->comp.method == EFL_GFX_VG_COMPOSITE_METHOD_MASK_INTERSECT)
     init_buffer = 0xFFFFFFFF;

   /* GL span-buffer path: render the mask shapes into a dedicated FBO surface
    * through the same span-buffer pipeline.  This runs during render_pre,
    * which is BEFORE the main VG object's eng_ector_begin(), so the shared
    * ector surface is free and the nested begin/end does not conflict with
    * any outer begin/end for the main VG render.
    *
    * The mask FBO surface is kept alive on pd->comp.gl_surface across frames
    * and reused when the size matches.  eng_ector_end() of the main VG object
    * reads espd->gl_comp_surface (set by render_pre after we return) to fill
    * the mask texture params on each shape's Span_Pipe_Params. */
   if (ENFN->gl_surface_read_pixels)  /* GL engine only — SW engine sets this to NULL */
     {
        int err = 0;
        void *mask_surface = pd->comp.gl_surface;

        /* Create or resize mask FBO. */
        if (mask_surface)
          {
             int mw = 0, mh = 0;
             ENFN->image_size_get(ENC, mask_surface, &mw, &mh);
             if (mw != size.w || mh != size.h)
               {
                  ENFN->ector_surface_destroy(ENC, mask_surface);
                  mask_surface = NULL;
               }
          }
        if (!mask_surface)
          {
             /* Use ector_mask_surface_create when available so the mask FBO
              * gets a dedicated GL texture that cannot be atlas-shared with the
              * main VG FBO, preventing a read/write feedback loop in the shader. */
             if (ENFN->ector_mask_surface_create)
               mask_surface = ENFN->ector_mask_surface_create(ENC, size.w, size.h, &err);
             else
               mask_surface = ENFN->ector_surface_create(ENC, size.w, size.h, &err);
             if (err || !mask_surface) goto gl_mask_fallback;
          }
        pd->comp.gl_surface = mask_surface;
        pd->comp.size.w = size.w;
        pd->comp.size.h = size.h;
        pd->comp.vg_pd = obj;

        /* Prepare mask shapes via render_pre.
         * The nested composite chain (MASK_ADD) is handled first, exactly as
         * in the CPU path below, but using NULL as the comp buffer since the
         * GL path does not create a CPU comp buffer here. */
        if (pd->comp.method >= EFL_GFX_VG_COMPOSITE_METHOD_MASK_ADD)
          {
             Efl_Canvas_Vg_Container_Data *target_pd = pd;
             for (Efl_VG *ct = pd->comp_target; ct; ct = target_pd->comp_target)
               {
                  Efl_Canvas_Vg_Container_Data *src_pd = NULL;
                  target_pd = efl_data_scope_get(ct, MY_CLASS);
                  src_pd = efl_data_scope_get(eina_list_nth(target_pd->comp.src, 0), MY_CLASS);
                  _evas_vg_render_pre(obj, ct,
                                      engine, output, context, surface,
                                      ctransform, c_opacity, NULL, src_pd->comp.method);
               }
          }

        _evas_vg_render_pre(obj, comp_target,
                            engine, output, context, surface,
                            ptransform, p_opacity, NULL, comp_method);

        /* Render mask shapes into the FBO via span-buffer pipeline.
         * eng_ector_begin installs span collectors on the shared ector surface;
         * _draw_comp drives ector_renderer_draw to populate them;
         * eng_ector_end uploads and draws them to mask_surface's FBO. */
        {
           RGBA_Draw_Context *dc = evas_common_draw_context_new();
           evas_common_draw_context_set_render_op(dc, _EVAS_RENDER_COPY);
           evas_common_draw_context_set_color(dc, 255, 255, 255, 255);

           if (ENFN->ector_begin(ENC, mask_surface, dc, surface, 0, 0, EINA_FALSE))
             {
                _draw_comp(obj, comp_target, surface, engine, output, context);
                ENFN->image_dirty_region(ENC, mask_surface, 0, 0, size.w, size.h);
                ENFN->ector_end(ENC, mask_surface, dc, surface, EINA_FALSE);
             }

           evas_common_draw_context_free(dc);
        }

        /* Signal to the caller that there is no CPU comp buffer.
         * The mask FBO reference lives on pd->comp.gl_surface; render_pre
         * propagates it to the ector surface data for eng_ector_end to pick up. */
        return NULL;
     }

gl_mask_fallback:
   /* Fallback: original CPU pixel buffer path (software backend or GL surface
    * creation failure). */

   //2. Reusable ector buffer?
   if (pd->comp.buffer &&
       ((pd->comp.size.w != size.w) ||
       (pd->comp.size.h != size.h)))

     {
        if (pd->comp.pixels)
          ector_buffer_unmap(pd->comp.buffer, pd->comp.pixels, pd->comp.length);
        efl_unref(pd->comp.buffer);
        pd->comp.buffer = NULL;
     }

   if (!pd->comp.buffer)
     {
        pd->comp.buffer = ENFN->ector_buffer_new(ENC, obj->layer->evas->evas,
                                                 size.w, size.h,
                                                 EFL_GFX_COLORSPACE_ARGB8888,
                                                 ECTOR_BUFFER_FLAG_DRAWABLE |
                                                 ECTOR_BUFFER_FLAG_CPU_READABLE |
                                                 ECTOR_BUFFER_FLAG_CPU_WRITABLE);
        pd->comp.size.w = size.w;
        pd->comp.size.h = size.h;
        pd->comp.vg_pd = obj;

        //Map
        pd->comp.pixels = ector_buffer_map(pd->comp.buffer, &pd->comp.length,
                                           (ECTOR_BUFFER_FLAG_DRAWABLE |
                                            ECTOR_BUFFER_FLAG_CPU_READABLE |
                                            ECTOR_BUFFER_FLAG_CPU_WRITABLE),
                                           0, 0, size.w, size.h,
                                           EFL_GFX_COLORSPACE_ARGB8888,
                                           &pd->comp.stride);
        if (!pd->comp.pixels) ERR("Failed to map VG composite buffer");

        //If composite method is SUBSTRACT or INTERSECT, Buffer needs initialize.
        if (pd->comp.pixels &&
            (pd->comp.method == EFL_GFX_VG_COMPOSITE_METHOD_MASK_SUBSTRACT ||
             pd->comp.method == EFL_GFX_VG_COMPOSITE_METHOD_MASK_INTERSECT))
          memset(pd->comp.pixels, init_buffer, pd->comp.length);
     }
   else
     {
        if (pd->comp.pixels)
          memset(pd->comp.pixels, init_buffer, pd->comp.length);
     }

   if (!pd->comp.buffer) ERR("Composite Buffer is invalid");

   //FIXME: This code means that there is another composite container.
   if (pd->comp.method >= EFL_GFX_VG_COMPOSITE_METHOD_MASK_ADD)
     {
        Efl_Canvas_Vg_Container_Data *target_pd = pd;
        comp = pd->comp.buffer;
        for (Efl_VG *comp_target = pd->comp_target; comp_target; comp_target = target_pd->comp_target)
          {
             Efl_Canvas_Vg_Container_Data *src_pd = NULL;
             target_pd = efl_data_scope_get(comp_target, MY_CLASS);
             src_pd = efl_data_scope_get(eina_list_nth(target_pd->comp.src, 0), MY_CLASS);
             _evas_vg_render_pre(obj, comp_target,
                                 engine, output, context, surface,
                                 ctransform, c_opacity, comp, src_pd->comp.method);
          }
     }

   //3. Prepare Drawing shapes.
   _evas_vg_render_pre(obj, comp_target,
                       engine, output, context,
                       surface,
                       ptransform, p_opacity, comp, comp_method);

   //4. Generating Composite Image.
   ector_buffer_pixels_set(surface, pd->comp.pixels, size.w, size.h, pd->comp.stride,
                           EFL_GFX_COLORSPACE_ARGB8888, EINA_TRUE);
   ector_surface_reference_point_set(surface, 0, 0);
   _draw_comp(obj, comp_target, surface, engine, output, context);

   return pd->comp.buffer;
}

static void
_efl_canvas_vg_container_render_pre(Evas_Object_Protected_Data *vg_pd,
                                    Efl_VG *obj EINA_UNUSED,
                                    Efl_Canvas_Vg_Node_Data *nd,
                                    void *engine, void *output, void *context,
                                    Ector_Surface *surface,
                                    Eina_Matrix3 *ptransform,
                                    int p_opacity,
                                    Ector_Buffer *comp,
                                    Efl_Gfx_Vg_Composite_Method comp_method,
                                    void *data)
{
   Efl_Canvas_Vg_Container_Data *pd = data;
   Eina_List *l;
   Efl_VG *child;
   Efl_Gfx_Change_Flag flag;

   /* Even when the container's flags are NONE (nothing changed), we must
    * still propagate the GL composite mask reference to the ector surface
    * every frame — eng_ector_end clears it after use. */
   if (pd->comp_target && _comp_method_needs_mask(pd->comp.method))
     {
        Efl_Canvas_Vg_Container_Data *cpd =
           efl_data_scope_get(pd->comp_target, MY_CLASS);
        _maybe_set_gl_comp(surface, cpd, pd->comp.method);
     }

   if (nd->flags == EFL_GFX_CHANGE_FLAG_NONE) return;

   flag = nd->flags;
   nd->flags = EFL_GFX_CHANGE_FLAG_NONE;

   EFL_CANVAS_VG_COMPUTE_MATRIX(ctransform, ptransform, nd);
   EFL_CANVAS_VG_COMPUTE_ALPHA(c_r, c_g, c_b, c_a, p_opacity, nd);

   //Container may have composite target.
   //FIXME : _prepare_comp() should only work in cases with matte or masking.
   // This condition is valid because the masking use same type as matte.
   if (pd->comp_target && _comp_method_needs_mask(pd->comp.method))
     {
        comp_method = pd->comp.method;
        comp = _prepare_comp(vg_pd, pd->comp_target,
                             engine, output, context, surface,
                             ptransform, ctransform, p_opacity, c_a, comp, comp_method);

        /* From here on comp_method is handed to this container's own children,
         * i.e. to the *source* shapes, for which the mask is already built and
         * only has to be applied.  MASK_INTERSECT and MASK_SUBSTRACT describe
         * how a mask is combined while it is being built, and the software
         * rasterizer maps them onto _comp_mask_ins()/_comp_mask_sub(), which
         * only ever mutate the mask buffer and never write the raster buffer.
         * Letting the source shapes reach those would draw nothing at all.
         * Applying an intersect mask is exactly MATTE_ALPHA, and applying a
         * subtract mask is exactly MATTE_ALPHA_INVERSE, which is also what the
         * GL span shader does for these two methods.  Note that _prepare_comp()
         * above still gets the original method, since the mask-building pass
         * does need the combining behaviour. */
        if (comp_method == EFL_GFX_VG_COMPOSITE_METHOD_MASK_INTERSECT)
          comp_method = EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA;
        else if (comp_method == EFL_GFX_VG_COMPOSITE_METHOD_MASK_SUBSTRACT)
          comp_method = EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA_INVERSE;

        /* GL span-buffer path: _prepare_comp returns NULL but stores the
         * mask FBO on pd->comp.gl_surface.  Propagate it to the shared ector
         * surface so eng_ector_end() can find it when building pipe params. */
        if (!comp && pd->comp_target)
          {
             Efl_Canvas_Vg_Container_Data *cpd =
                efl_data_scope_get(pd->comp_target, MY_CLASS);
             _maybe_set_gl_comp(surface, cpd, pd->comp.method);
          }
     }

   //If the container has transparency, it internally alpha blends with ector buffer.
   //So ector buffer must be created synchronously.
   if (c_a < 255 && vg_pd && vg_pd->object)
     {
        Efl_Canvas_Vg_Object_Data *od = efl_data_scope_get(vg_pd->object, EFL_CANVAS_VG_OBJECT_CLASS);
        od->sync_render = EINA_TRUE;
     }

   EINA_LIST_FOREACH(pd->children, l, child)
     {
        //Don't need to update composite nodes.
        if (efl_isa(child, MY_CLASS))
          {
             Efl_Canvas_Vg_Container_Data *child_cd =
                efl_data_scope_get(child, MY_CLASS);
             if (child_cd->comp.src) continue;
          }

        //Skip Gradients. they will be updated by Shape.
        if (efl_isa(child, EFL_CANVAS_VG_GRADIENT_CLASS))
          continue;

        Efl_Canvas_Vg_Node_Data *child_nd =
           efl_data_scope_get(child, EFL_CANVAS_VG_NODE_CLASS);

        if (flag & EFL_GFX_CHANGE_FLAG_MATRIX)
          child_nd->flags |= EFL_GFX_CHANGE_FLAG_MATRIX;

        // Note: If Vg.Container has transparency, this is calculated by _evas_vg_render in Efl.Canvas.Vg.Object.
        //       Therefore, there is no need to propagate the transparency of the container to the child.
        //       However, if there is a composition target, the child must refer to the parent's opacity.
        //       Because _evas_vg_render does not support opacity calculation for containers that need to be composited.
        //       These things need to be refactored in a better way later.
        /* If there is a composite (CPU buffer or GL FBO mask), children must
         * inherit the container's opacity.  In the GL span-buffer path,
         * comp is NULL even when a mask FBO was prepared, so also check for
         * gl_surface on the comp_target's pd. */
        {
           Eina_Bool has_comp = comp != NULL;
           if (!has_comp && pd->comp_target)
             {
                Efl_Canvas_Vg_Container_Data *cpd =
                   efl_data_scope_get(pd->comp_target, MY_CLASS);
                if (cpd && cpd->comp.gl_surface) has_comp = EINA_TRUE;
             }
           c_a = !has_comp ? 255 : c_a;
        }

        _evas_vg_render_pre(vg_pd, child,
                            engine, output, context, surface,
                            ctransform, c_a, comp, comp_method);
     }
}

static Eo *
_efl_canvas_vg_container_efl_object_constructor(Eo *obj,
                                      Efl_Canvas_Vg_Container_Data *pd)
{
   Efl_Canvas_Vg_Node_Data *nd;

   pd->names = eina_hash_stringshared_new(NULL);

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   nd = efl_data_scope_get(obj, EFL_CANVAS_VG_NODE_CLASS);
   nd->render_pre = _efl_canvas_vg_container_render_pre;
   nd->data = pd;
   nd->flags = EFL_GFX_CHANGE_FLAG_ALL;

   efl_gfx_color_set(obj, 255, 255, 255, 255);

   return obj;
}

static void
_efl_canvas_vg_container_efl_object_destructor(Eo *obj,
                                               Efl_Canvas_Vg_Container_Data *pd)
{
   efl_canvas_vg_container_blend_buffer_clear(obj, pd);

   //Destroy comp surface
   if (pd->comp.buffer)
     {
        if (pd->comp.pixels)
          ector_buffer_unmap(pd->comp.buffer, pd->comp.pixels, pd->comp.length);
        efl_unref(pd->comp.buffer);
     }

   /* Destroy GL FBO mask surface (GL span-buffer path).
    * pd->comp.gl_surface is an Evas_GL_Image* allocated via
    * ENFN->ector_surface_create.  We need the engine handle to free it.
    * Retrieve the engine and output from the VG object's Evas layer if
    * available.  If the engine context has already been torn down, skip
    * the free to avoid use-after-free (minor leak on shutdown). */
   if (pd->comp.gl_surface && pd->comp.vg_pd)
     {
        Evas_Object_Protected_Data *vg_pd = pd->comp.vg_pd;
        if (vg_pd->layer && vg_pd->layer->evas)
          {
             Evas_Public_Data *evas = vg_pd->layer->evas;
             if (evas->engine.func && evas->engine.func->ector_surface_destroy)
               evas->engine.func->ector_surface_destroy(
                  _evas_engine_context(evas), pd->comp.gl_surface);
          }
        pd->comp.gl_surface = NULL;
     }

   efl_unref(pd->comp_target);
   eina_list_free(pd->comp.src);
   eina_hash_free(pd->names);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
_efl_canvas_vg_container_efl_gfx_path_bounds_get(const Eo *obj EINA_UNUSED,
                                                 Efl_Canvas_Vg_Container_Data *pd,
                                                 Eina_Rect *r)
{
   Eina_Rect s;
   Eina_Bool first = EINA_TRUE;
   Eina_List *l;
   Eo *child;

   EINA_RECT_SET(s, -1, -1, 0, 0);

   EINA_LIST_FOREACH(pd->children, l, child)
     {
        Eina_Position2D pos = efl_gfx_entity_position_get(child);
        double miterlimit = 0.0, stroke_gap = 0.0;
        int stroke_gap_int;

        if (efl_isa(child, EFL_CANVAS_VG_SHAPE_CLASS))
          {
             miterlimit = efl_gfx_shape_stroke_miterlimit_get(child);
             stroke_gap = efl_gfx_shape_stroke_width_get(child) * (miterlimit <= 0 ? 1 : miterlimit);
          }
        // convert stroke_gap to an int and round it up to the 
        stroke_gap_int = 2 * ((stroke_gap + 1.0000) / 2);
        if (first)
          {
             efl_gfx_path_bounds_get(child, r);
             r->pos.x += pos.x;
             r->pos.y += pos.y;
             if (stroke_gap_int > 0)
               {
                  r->pos.x -= stroke_gap_int / 2;
                  r->pos.y -= stroke_gap_int / 2;
                  r->size.w += stroke_gap_int;
                  r->size.h += stroke_gap_int;
               }
             first = EINA_FALSE;
          }
        else
          {
             efl_gfx_path_bounds_get(child, &s);
             s.pos.x += pos.x;
             s.pos.y += pos.y;
             if (stroke_gap_int > 0)
               {
                  s.pos.x -= stroke_gap_int / 2;
                  s.pos.y -= stroke_gap_int / 2;
                  s.size.w += stroke_gap_int;
                  s.size.h += stroke_gap_int;
               }
             eina_rectangle_union(&r->rect, &s.rect);
          }
     }
}

static Efl_VG *
_efl_canvas_vg_container_child_get(Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Container_Data *pd, const char *name)
{
   const char *tmp = eina_stringshare_add(name);
   Efl_VG *r;

   r = eina_hash_find(pd->names, tmp);
   eina_stringshare_del(tmp);

   return r;
}

static Eina_Iterator *
_efl_canvas_vg_container_children_get(Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Container_Data *pd)
{
   return eina_list_iterator_new(pd->children);
}

static const Eina_List *
_efl_canvas_vg_container_children_direct_get(Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Container_Data *pd)
{
   return pd->children;
}

static Eina_Bool
_efl_canvas_vg_container_efl_gfx_path_interpolate(Eo *obj, Efl_Canvas_Vg_Container_Data *pd, const Efl_VG *from, const Efl_VG *to, double pos_map)
{
   Eina_Iterator *from_it, *to_it;
   Eina_List *l;
   Eina_Bool r, res = EINA_TRUE;
   Eo *from_child, *to_child, *child;

   //Check if both objects have same type
   if (!(efl_isa(from, MY_CLASS) && efl_isa(to, MY_CLASS)))
     return EINA_FALSE;

   //Interpolates this nodes
   r = efl_gfx_path_interpolate(efl_super(obj, MY_CLASS), from, to, pos_map);
   if (!r) return EINA_FALSE;

   from_it = efl_canvas_vg_container_children_get((Efl_VG *)from);
   to_it = efl_canvas_vg_container_children_get((Efl_VG *)to);

   //Interpolates children
   EINA_LIST_FOREACH (pd->children, l, child)
     {
        res &= eina_iterator_next(from_it, (void **)&from_child);
        res &= eina_iterator_next(to_it, (void **)&to_child);
        if (!res && (efl_class_get(from_child) != efl_class_get(to_child) ||
            (efl_class_get(child) != efl_class_get(from_child))))
          {
             r = EINA_FALSE;
             break;
          }
        r = efl_gfx_path_interpolate(child, from_child, to_child, pos_map);
        if (!r) break;
     }

   //Interpolates Composite
   Efl_Canvas_Vg_Container_Data *fromd = efl_data_scope_get(from, MY_CLASS);
   Efl_Canvas_Vg_Container_Data *tod = efl_data_scope_get(to, MY_CLASS);

   if (fromd->comp_target && tod->comp_target && pd->comp_target)
     {
        if (!efl_gfx_path_interpolate(pd->comp_target,
                                      fromd->comp_target, tod->comp_target, pos_map))
          return EINA_FALSE;
     }

   eina_iterator_free(from_it);
   eina_iterator_free(to_it);

   return r;
}

static void
_efl_canvas_vg_container_efl_canvas_vg_node_comp_method_set(Eo *obj,
                                                            Efl_Canvas_Vg_Container_Data *pd,
                                                            Efl_Canvas_Vg_Node *target,
                                                            Efl_Gfx_Vg_Composite_Method method)
{
   if (pd->comp_target == target) return;

   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(target, MY_CLASS));

   if (pd->comp_target)
     {
        Efl_Canvas_Vg_Container_Data *pd2 =
              efl_data_scope_get(pd->comp_target, MY_CLASS);
        pd2->comp.src = eina_list_remove(pd2->comp.src, obj);
     }

   if (target)
     {
        Efl_Canvas_Vg_Container_Data *pd2 = efl_data_scope_get(target, MY_CLASS);
        pd2->comp.src = eina_list_append(pd2->comp.src, obj);
     }

   pd->comp.method = method;
   efl_replace(&pd->comp_target, target);
   efl_canvas_vg_node_change(obj);
}

static void
_efl_canvas_vg_container_efl_object_parent_set(Eo *obj,
                                               Efl_Canvas_Vg_Container_Data *cd EINA_UNUSED,
                                               Eo *parent)
{
   efl_parent_set(efl_super(obj, MY_CLASS), parent);

   Efl_Canvas_Vg_Node_Data *nd = efl_data_scope_get(obj, EFL_CANVAS_VG_NODE_CLASS);
   efl_canvas_vg_container_vg_obj_update(obj, nd);
}

EOLIAN static Efl_VG *
_efl_canvas_vg_container_efl_duplicate_duplicate(const Eo *obj,
                                                 Efl_Canvas_Vg_Container_Data *pd)
{
   Eina_List *l;
   Efl_VG *child;
   Efl_VG *container;

   container = efl_duplicate(efl_super(obj, MY_CLASS));
   efl_event_callback_add(container, EFL_EVENT_INVALIDATE, _invalidate_cb, NULL);

   //Copy Composite
   if (pd->comp_target)
     {
        /* OPTIMIZE: How to skip copying the target
           if this target is linked to multiple sources?? */
        Eo * comp_target = efl_duplicate(pd->comp_target);

        //FIXME: comp_target parent should be its own parent(duplicated obj this case)
        efl_parent_set(comp_target, container);
        efl_canvas_vg_node_comp_method_set(container, comp_target, pd->comp.method);
     }

   //Copy Children
   EINA_LIST_FOREACH(pd->children, l, child)
     {
        //Skip, We already copied composite target before.
        if (efl_isa(child, MY_CLASS))
          {
             Efl_Canvas_Vg_Container_Data *pd2 =
                efl_data_scope_get(child, MY_CLASS);
             if (pd2->comp.src) continue;
          }

        Efl_VG *eo = efl_duplicate(child);
        efl_parent_set(eo, container);
     }

   return container;
}

void
efl_canvas_vg_container_vg_obj_update(Efl_VG *obj, Efl_Canvas_Vg_Node_Data *nd)
{
   if (!obj) return;

   Efl_Canvas_Vg_Container_Data *cd = efl_data_scope_get(obj, MY_CLASS);
   if (!cd) return;

   Eina_List *l;
   Efl_VG* child;

   EINA_LIST_FOREACH(cd->children, l, child)
     {
        Efl_Canvas_Vg_Node_Data *child_nd =
           efl_data_scope_get(child, EFL_CANVAS_VG_NODE_CLASS);

        if (child_nd->vg_obj == nd->vg_obj) continue;

        child_nd->vg_obj = nd->vg_obj;
        child_nd->vd = nd->vd;

        if (efl_isa(child, MY_CLASS))
          efl_canvas_vg_container_vg_obj_update(child, child_nd);
     }
}

void
efl_canvas_vg_container_blend_buffer_clear(Efl_VG *obj EINA_UNUSED, Efl_Canvas_Vg_Container_Data *cd)
{
   if (!cd->blend.buffer) return;

   if (cd->blend.pixels)
     {
        ector_buffer_unmap(cd->blend.buffer, cd->blend.pixels, cd->blend.length);
        cd->blend.pixels = NULL;
     }
   if (cd->blend.buffer) efl_unref(cd->blend.buffer);
   cd->blend.buffer = NULL;
}

EVAS_API Evas_Vg_Container *
evas_vg_container_add(Evas_Object *parent)
{
   /* Warn it because the usage has been changed.
      We can remove this message after v1.21. */
   if (!parent)
     {
        ERR("Efl_VG Container doesn't allow null parent!");
        return NULL;
     }
   return efl_add(MY_CLASS, parent);
}

#include "efl_canvas_vg_container.eo.c"
#include "efl_canvas_vg_container_eo.legacy.c"
