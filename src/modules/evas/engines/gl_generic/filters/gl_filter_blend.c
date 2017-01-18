#include "gl_engine_filter.h"

static Eina_Bool
_gl_filter_blend(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd)
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *image, *surface;
   RGBA_Draw_Context *dc_save;

   re->window_use(re->software.ob);
   gc = re->window_gl_context_get(re->software.ob);

   image = evas_ector_buffer_drawable_image_get(cmd->input->buffer, EINA_TRUE);

   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(image->tex, EINA_FALSE);

   surface = evas_ector_buffer_render_image_get(cmd->output->buffer, EINA_FALSE);

   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface->tex, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface->tex->pt, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(surface->tex->pt->fb != 0, EINA_FALSE);
   evas_gl_common_context_target_surface_set(gc, surface);

   // TODO: mapped render iteration

   dc_save = gc->dc;
   gc->dc = evas_common_draw_context_new();
   evas_common_draw_context_set_multiplier(gc->dc, cmd->draw.R, cmd->draw.G, cmd->draw.B, cmd->draw.A);

   evas_gl_common_image_draw(gc, image, 0, 0, image->w, image->h,
                             cmd->draw.ox, cmd->draw.oy, image->w, image->h,
                             EINA_TRUE);

   evas_common_draw_context_free(gc->dc);
   gc->dc = dc_save;

   evas_ector_buffer_engine_image_release(cmd->input->buffer, image);
   evas_ector_buffer_engine_image_release(cmd->output->buffer, surface);

   return EINA_TRUE;
}

GL_Filter_Apply_Func
gl_filter_blend_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);

   return _gl_filter_blend;
}
