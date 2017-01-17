#include "gl_engine_filter.h"

static Eina_Bool
_gl_filter_blend(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd)
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *image;
   Evas_Canvas *eo_evas;
   void *im = NULL;

   re->window_use(re->software.ob);
   gc = re->window_gl_context_get(re->software.ob);

   evas_ector_buffer_engine_image_get(cmd->input->buffer, &eo_evas, &im);
   image = im;

   evas_gl_common_image_draw(gc, image, 0, 0, image->w, image->h, 0, 0, image->w, image->h, EINA_TRUE);

   return EINA_TRUE;
}

GL_Filter_Apply_Func
gl_filter_blend_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);

   return NULL; // _gl_filter_blend;
}
