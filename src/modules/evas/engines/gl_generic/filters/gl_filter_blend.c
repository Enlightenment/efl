#include "gl_engine_filter.h"

// Copied logic from SW engine
static Eina_Bool
_mapped_blend(Evas_Engine_GL_Context *gc,
              Evas_GL_Image *image,
              Evas_Filter_Fill_Mode fillmode,
              int sx, int sy, int sw, int sh,
              int dx, int dy, int dw, int dh)
{
   int right = 0, bottom = 0, left = 0, top = 0;
   int row, col, rows, cols;
   Eina_Bool ret = EINA_TRUE;

   if (fillmode == EVAS_FILTER_FILL_MODE_NONE)
     {
        DBG("blend: %d,%d,%d,%d --> %d,%d,%d,%d", sx, sy, sw, sh, dx, dy, sw, sh);
        evas_gl_common_image_draw(gc, image, sx, sy, sw, sh, dx, dy, sw, sh, EINA_TRUE);
        return EINA_TRUE;
     }

   if (fillmode & EVAS_FILTER_FILL_MODE_REPEAT_X)
     {
        if (dx > 0) left = dx % sw;
        else if (dx < 0) left = sw + (dx % sw);
        cols = (dw  /*- left*/) / sw;
        if (left > 0)
          right = dw - (sw * (cols - 1)) - left;
        else
          right = dw - (sw * cols);
        dx = 0;
     }
   else if (fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
     {
        cols = 0;
     }
   else
     {
        // FIXME: Probably wrong if dx != 0
        cols = 0;
        dw -= dx;
     }

   if (fillmode & EVAS_FILTER_FILL_MODE_REPEAT_Y)
     {
        if (dy > 0) top = dy % sh;
        else if (dy < 0) top = sh + (dy % sh);
        rows = (dh /*- top*/) / sh;
        if (top > 0)
          bottom = dh - (sh * (rows - 1)) - top;
        else
          bottom = dh - (sh * rows);
        dy = 0;
     }
   else if (fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
     {
        rows = 0;
     }
   else
     {
        // FIXME: Probably wrong if dy != 0
        rows = 0;
        dh -= dy;
     }

   if (top > 0) row = -1;
   else row = 0;
   for (; row <= rows; row++)
     {
        int src_x, src_y, src_w, src_h;
        int dst_x, dst_y, dst_w, dst_h;

        if (row == -1 && top > 0)
          {
             // repeat only
             src_h = top;
             src_y = sh - top;
             dst_y = dy;
             dst_h = src_h;
          }
        else if (row == rows && bottom > 0)
          {
             // repeat only
             src_h = bottom;
             src_y = 0;
             dst_y = top + dy + row * sh;
             dst_h = src_h;
          }
        else
          {
             src_y = 0;
             if (fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
               {
                  src_y = sy;
                  src_h = sh;
                  dst_y = dy;
                  dst_h = dh;
               }
             else
               {
                  dst_y = top + dy + row * sh;
                  src_h = MIN(dh - dst_y, sh);
                  dst_h = src_h;
               }
          }
        if (src_h <= 0 || dst_h <= 0) break;

        if (left > 0) col = -1;
        else col = 0;
        for (; col <= cols; col++)
          {
             if (col == -1 && left > 0)
               {
                  // repeat only
                  src_w = left;
                  src_x = sw - left;
                  dst_x = dx;
                  dst_w = src_w;
               }
             else if (col == cols && right > 0)
               {
                  // repeat only
                  src_w = right;
                  src_x = 0;
                  dst_x = left + dx + col * sw;
                  dst_w = src_w;
               }
             else
               {
                  src_x = 0;
                  if (fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
                    {
                       src_x = sx;
                       src_w = sw;
                       dst_x = dx;
                       dst_w = dw;
                    }
                  else
                    {
                       dst_x = left + dx + col * sw;
                       src_w = MIN(dw - dst_x, sw);
                       dst_w = src_w;
                    }
               }
             if (src_w <= 0 || dst_w <= 0) break;

             DBG("blend: [%d,%d] %d,%d,%dx%d --> %d,%d,%dx%d "
                 "(src %dx%d, dst %dx%d)",
                 col, row, src_x, src_y, src_w, src_h,
                 dst_x, dst_y, dst_w, dst_h, sw, sh, dw, dh);
             evas_gl_common_image_draw(gc, image, src_x, src_y, src_w, src_h,
                                       dst_x, dst_y, dst_w, dst_h, EINA_TRUE);
          }
     }
   return ret;
}

static Eina_Bool
_gl_filter_blend(Render_Output_GL_Generic *re, Evas_Filter_Command *cmd)
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *image, *surface;
   RGBA_Draw_Context *dc_save;
   int src_w, src_h, dst_w, dst_h, src_x, src_y, dst_x, dst_y;

   DEBUG_TIME_BEGIN();

   re->window_use(re->software.ob);
   gc = re->window_gl_context_get(re->software.ob);

   image = evas_ector_buffer_drawable_image_get(cmd->input->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);

   surface = evas_ector_buffer_render_image_get(cmd->output->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, EINA_FALSE);

   evas_gl_common_context_target_surface_set(gc, surface);

   dc_save = gc->dc;
   gc->dc = evas_common_draw_context_new();
   evas_common_draw_context_set_multiplier(gc->dc, cmd->draw.R, cmd->draw.G, cmd->draw.B, cmd->draw.A);
   gc->dc->render_op = _gfx_to_evas_render_op(cmd->draw.rop);

   // FIXME: Maybe need to clear buffer in case of COPY mode with an offset?

   if ((cmd->draw.fillmode == EVAS_FILTER_FILL_MODE_STRETCH_XY) &&
       ((image->w != surface->w) || (image->h != surface->h)))
     {
        double scale_w, scale_h;
        int pad_x, pad_y;

        pad_x = cmd->draw.scale.pad_x;
        pad_y = cmd->draw.scale.pad_y;
        scale_w = MAX(cmd->draw.scale.factor_x, 1);
        scale_h = MAX(cmd->draw.scale.factor_y, 1);

        if (cmd->draw.scale.down)
          {
             src_x = -pad_x;
             src_y = -pad_y;
             src_w = (ceil(image->w / scale_w) + 1) * scale_w;
             src_h = (ceil(image->h / scale_h) + 1) * scale_h;
             dst_x = 0;
             dst_y = 0;
             dst_w = src_w / scale_w;
             dst_h = src_h / scale_h;
          }
        else
          {
             src_x = 0;
             src_y = 0;
             src_w = image->w;
             src_h = image->h;
             dst_x = cmd->draw.ox - pad_x;
             dst_y = cmd->draw.oy - pad_y;
             dst_w = (ceil(surface->w / scale_w) + 1) * scale_w;
             dst_h = (ceil(surface->h / scale_h) + 1) * scale_h;
          }
     }
   else
     {
        src_x = 0;
        src_y = 0;
        src_w = image->w;
        src_h = image->h;
        dst_x = cmd->draw.ox;
        dst_y = cmd->draw.oy;
        dst_w = surface->w;
        dst_h = surface->h;
     }

   DBG("blend %d @%p -> %d @%p", cmd->input->id, cmd->input->buffer,
       cmd->output->id, cmd->output->buffer);
   _mapped_blend(gc, image, cmd->draw.fillmode,
                 src_x, src_y, src_w, src_h,
                 dst_x, dst_y, dst_w, dst_h);

   evas_common_draw_context_free(gc->dc);
   gc->dc = dc_save;

   evas_ector_buffer_engine_image_release(cmd->input->buffer, image);
   evas_ector_buffer_engine_image_release(cmd->output->buffer, surface);

   DEBUG_TIME_END();

   return EINA_TRUE;
}

GL_Filter_Apply_Func
gl_filter_blend_func_get(Render_Output_GL_Generic *re EINA_UNUSED, Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);

   return _gl_filter_blend;
}
