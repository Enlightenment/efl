#include "gl_engine_filter.h"

static Eina_Bool
_gl_filter_fill(Render_Output_GL_Generic *re, Evas_Filter_Command *cmd)
{
   Evas_Filter_Buffer *fb = cmd->output;
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *surface;
   RGBA_Draw_Context *dc_save;
   int x = MAX(0, cmd->draw.clip.x);
   int y = MAX(0, cmd->draw.clip.y);
   int w, h;

   DEBUG_TIME_BEGIN();

   if (!cmd->draw.clip_mode_lrtb)
     {
        if (cmd->draw.clip.w)
          w = MIN(cmd->draw.clip.w, fb->w - x);
        else
          w = fb->w - x;
        if (cmd->draw.clip.h)
          h = MIN(cmd->draw.clip.h, fb->h - y);
        else
          h = fb->h - y;
     }
   else
     {
        x = MAX(0, cmd->draw.clip.l);
        y = MAX(0, cmd->draw.clip.t);
        w = CLAMP(0, fb->w - x - cmd->draw.clip.r, fb->w - x);
        h = CLAMP(0, fb->h - y - cmd->draw.clip.b, fb->h - y);
     }

   surface = evas_ector_buffer_render_image_get(fb->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, EINA_FALSE);

   DBG("fill rgba(%d,%d,%d,%d) %d,%d %dx%d) -> %d @%p",
       cmd->draw.R, cmd->draw.G, cmd->draw.B, cmd->draw.A, x, y, w, h,
       fb->id, fb->buffer);

   re->window_use(re->software.ob);
   gc = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_target_surface_set(gc, surface);

   dc_save = gc->dc;
   gc->dc = evas_common_draw_context_new();
   evas_common_draw_context_clip_clip(gc->dc, x, y, w, h);
   evas_common_draw_context_set_render_op(gc->dc, _gfx_to_evas_render_op(cmd->draw.rop));

   evas_gl_common_context_rectangle_push(gc, x, y, w, h,
                                         cmd->draw.R, cmd->draw.G, cmd->draw.B, cmd->draw.A,
                                         NULL, 0, 0, 0, 0, EINA_FALSE, EINA_FALSE);

   evas_common_draw_context_free(gc->dc);
   gc->dc = dc_save;

   evas_ector_buffer_engine_image_release(fb->buffer, surface);

   DEBUG_TIME_END();

   return EINA_TRUE;
}

GL_Filter_Apply_Func
gl_filter_fill_func_get(Render_Output_GL_Generic *re EINA_UNUSED, Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);

   return _gl_filter_fill;
}
