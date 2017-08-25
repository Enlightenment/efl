#include "gl_engine_filter.h"

static Eina_Bool
_gl_filter_displace(Render_Output_GL_Generic *re, Evas_Filter_Command *cmd)
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *image, *surface, *orig_map, *use_map;
   RGBA_Draw_Context *dc_save;
   int x, y, w, h, map_w, map_h;
   Eina_Bool nearest = EINA_FALSE;
   double dx, dy;

   DEBUG_TIME_BEGIN();

   w = cmd->input->w;
   h = cmd->input->h;
   dx = dy = cmd->displacement.intensity;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(w == cmd->output->w, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(h == cmd->output->h, EINA_FALSE);

   image = evas_ector_buffer_drawable_image_get(cmd->input->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);

   orig_map = evas_ector_buffer_drawable_image_get(cmd->mask->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(orig_map, EINA_FALSE);

   surface = evas_ector_buffer_render_image_get(cmd->output->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, EINA_FALSE);

   DBG("displace %d @%p map %d %p -> %d @%p", cmd->input->id, cmd->input->buffer,
       cmd->mask->id, cmd->mask->buffer, cmd->output->id, cmd->output->buffer);

   re->window_use(re->software.ob);
   gc = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_target_surface_set(gc, surface);

   dc_save = gc->dc;
   gc->dc = evas_common_draw_context_new();
   evas_common_draw_context_set_multiplier(gc->dc, cmd->draw.R, cmd->draw.G, cmd->draw.B, cmd->draw.A);
   evas_common_draw_context_clip_clip(gc->dc, 0, 0, w, h);

   map_w = (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X) ? w : cmd->mask->w;
   map_h = (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y) ? h : cmd->mask->h;
   use_map = evas_gl_common_image_virtual_scaled_get(NULL, orig_map, map_w, map_h, EINA_TRUE);

   // FIXME: Flags not handled:
   // EVAS_FILTER_DISPLACE_BLACK vs. EVAS_FILTER_DISPLACE_STRETCH

   if ((cmd->displacement.flags & EVAS_FILTER_DISPLACE_LINEAR) == 0)
     nearest = EINA_TRUE;

   for (y = 0; y < h; y += map_h)
     for (x = 0; x < w; x += map_w)
       {
          int sw, sh;

          sw = MIN(map_w, w - x);
          sh = MIN(map_h, h - y);

          evas_gl_common_filter_displace_push(gc, image->tex, use_map->tex,
                                              x, y, sw, sh, dx, dy, nearest);
       }

   evas_gl_common_image_free(use_map);
   evas_common_draw_context_free(gc->dc);
   gc->dc = dc_save;

   evas_ector_buffer_engine_image_release(cmd->input->buffer, image);
   evas_ector_buffer_engine_image_release(cmd->mask->buffer, orig_map);
   evas_ector_buffer_engine_image_release(cmd->output->buffer, surface);

   DEBUG_TIME_END();

   return EINA_TRUE;
}

GL_Filter_Apply_Func
gl_filter_displace_func_get(Render_Output_GL_Generic *re EINA_UNUSED, Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->mask, NULL);

   return _gl_filter_displace;
}
