#include "gl_engine_filter.h"

static Eina_Bool
_gl_filter_curve(Render_Output_GL_Generic *re, Evas_Filter_Command *cmd)
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *image, *surface;
   RGBA_Draw_Context *dc_save;
   const uint8_t *points;
   int channel;
   int w, h;

   DEBUG_TIME_BEGIN();

   w = cmd->input->w;
   h = cmd->input->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(w == cmd->output->w, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(h == cmd->output->h, EINA_FALSE);

   image = evas_ector_buffer_drawable_image_get(cmd->input->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);

   surface = evas_ector_buffer_render_image_get(cmd->output->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, EINA_FALSE);

   DBG("curve %d @%p -> %d @%p", cmd->input->id, cmd->input->buffer,
       cmd->output->id, cmd->output->buffer);

   re->window_use(re->software.ob);
   gc = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_target_surface_set(gc, surface);

   dc_save = gc->dc;
   gc->dc = evas_common_draw_context_new();
   evas_common_draw_context_set_multiplier(gc->dc, cmd->draw.R, cmd->draw.G, cmd->draw.B, cmd->draw.A);
   evas_common_draw_context_clip_clip(gc->dc, 0, 0, w, h);
   if (cmd->input == cmd->output)
     gc->dc->render_op = EVAS_RENDER_COPY;

   points = cmd->curve.data;
   channel = (int) cmd->curve.channel;
   if (cmd->input->alpha_only)
     channel = 5;

   evas_gl_common_filter_curve_push(gc, image->tex, 0, 0, w, h, points, channel);

   evas_common_draw_context_free(gc->dc);
   gc->dc = dc_save;

   evas_ector_buffer_engine_image_release(cmd->input->buffer, image);
   evas_ector_buffer_engine_image_release(cmd->output->buffer, surface);

   DEBUG_TIME_END();

   return EINA_TRUE;
}

GL_Filter_Apply_Func
gl_filter_curve_func_get(Render_Output_GL_Generic *re EINA_UNUSED, Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->curve.data, NULL);

   return _gl_filter_curve;
}
