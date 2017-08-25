#include "gl_engine_filter.h"

static Eina_Bool
_gl_filter_mask(Render_Output_GL_Generic *re, Evas_Filter_Command *cmd)
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *image, *surface, *orig_mask, *use_mask = NULL;
   RGBA_Draw_Context *dc_save;
   int x, y, w, h, mask_w, mask_h;

   DEBUG_TIME_BEGIN();

   w = cmd->input->w;
   h = cmd->input->h;

   image = evas_ector_buffer_drawable_image_get(cmd->input->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);

   orig_mask = evas_ector_buffer_drawable_image_get(cmd->mask->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(orig_mask, EINA_FALSE);

   surface = evas_ector_buffer_render_image_get(cmd->output->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, EINA_FALSE);

   DBG("mask %d @%p + %d %p -> %d @%p", cmd->input->id, cmd->input->buffer,
       cmd->mask->id, cmd->mask->buffer, cmd->output->id, cmd->output->buffer);

   re->window_use(re->software.ob);
   gc = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_target_surface_set(gc, surface);

   dc_save = gc->dc;
   gc->dc = evas_common_draw_context_new();
   evas_common_draw_context_set_multiplier(gc->dc, cmd->draw.R, cmd->draw.G, cmd->draw.B, cmd->draw.A);
   evas_common_draw_context_clip_clip(gc->dc, 0, 0, w, h);

   mask_w = (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X) ? w : cmd->mask->w;
   mask_h = (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y) ? h : cmd->mask->h;
   use_mask = evas_gl_common_image_virtual_scaled_get(NULL, orig_mask, mask_w, mask_h, EINA_TRUE);

   gc->dc->clip.mask = use_mask;
   gc->dc->clip.mask_color = !cmd->mask->alpha_only;

   for (y = 0; y < h; y += mask_h)
     for (x = 0; x < w; x += mask_w)
       {
          int sw, sh;

          sw = MIN(mask_w, w - x);
          sh = MIN(mask_h, h - y);
          gc->dc->clip.mask_x = x;
          gc->dc->clip.mask_y = y;

          evas_gl_common_image_draw(gc, image, x, y, sw, sh, x, y, sw, sh, EINA_TRUE);
       }

   evas_gl_common_image_free(use_mask);
   evas_common_draw_context_free(gc->dc);
   gc->dc = dc_save;

   evas_ector_buffer_engine_image_release(cmd->input->buffer, image);
   evas_ector_buffer_engine_image_release(cmd->mask->buffer, orig_mask);
   evas_ector_buffer_engine_image_release(cmd->output->buffer, surface);

   DEBUG_TIME_END();

   return EINA_TRUE;
}

GL_Filter_Apply_Func
gl_filter_mask_func_get(Render_Output_GL_Generic *re EINA_UNUSED, Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->mask, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((cmd->input->w > 0) && (cmd->input->h > 0), NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((cmd->mask->w > 0) && (cmd->mask->h > 0), NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->input->w == cmd->output->w, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->input->h == cmd->output->h, NULL);

   return _gl_filter_mask;
}
