#include "evas_engine_filter.h"

// FIXME: This should all be based on ector renderer

// Use a better formula than R+G+B for rgba to alpha conversion (RGB to YCbCr)
#define RGBA2ALPHA_WEIGHTED 1

typedef Eina_Bool (*draw_func) (void *context, const void *src_map, unsigned int src_stride, void *dst_map, unsigned int dst_stride, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth, Eina_Bool do_async);
static Eina_Bool _mapped_blend(void *drawctx, const void *src_map, unsigned int src_stride, void *dst_map, unsigned int dst_stride, Evas_Filter_Fill_Mode fillmode, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, draw_func image_draw);

typedef struct _Filter_Blend_Draw_Context
{
   Efl_Gfx_Render_Op rop;
   uint32_t color;
   Eina_Bool alphaonly;
} Filter_Blend_Draw_Context;

#define LINELEN(stride, ptr) (stride / (sizeof(*ptr)))

static Eina_Bool
_image_draw_cpu_alpha_alpha(void *context,
                            const void *src_map, unsigned int src_stride,
                            void *dst_map, unsigned int dst_stride,
                            int src_x, int src_y, int src_w, int src_h,
                            int dst_x, int dst_y, int dst_w, int dst_h,
                            int smooth EINA_UNUSED,
                            Eina_Bool do_async EINA_UNUSED)
{
   Filter_Blend_Draw_Context *dc = context;
   const uint8_t *srcdata = src_map;
   uint8_t *dstdata = dst_map;
   Draw_Func_Alpha func;
   int y, sw, dw;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((src_w == dst_w) && (src_h == dst_h), EINA_FALSE);

   func = efl_draw_alpha_func_get(dc->rop, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, EINA_FALSE);

   sw = LINELEN(src_stride, srcdata);
   dw = LINELEN(dst_stride, dstdata);

   srcdata += src_y * sw;
   dstdata += dst_y * dw;
   for (y = src_h; y; y--)
     {
        func(dstdata + dst_x, srcdata + src_x, src_w);
        srcdata += sw;
        dstdata += dw;
     }

   return EINA_TRUE;
}

static Eina_Bool
_image_draw_cpu_alpha_rgba(void *context,
                           const void *src_map, unsigned int src_stride,
                           void *dst_map, unsigned int dst_stride,
                           int src_x, int src_y, int src_w, int src_h,
                           int dst_x, int dst_y, int dst_w, int dst_h,
                           int smooth EINA_UNUSED,
                           Eina_Bool do_async EINA_UNUSED)
{
   Filter_Blend_Draw_Context *dc = context;
   uint8_t *srcdata = (uint8_t *) src_map;
   uint32_t *dstdata = dst_map;
   RGBA_Comp_Func_Mask func;
   int y, sw, dw;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((src_w == dst_w) && (src_h == dst_h), EINA_FALSE);

   func = efl_draw_func_mask_span_get(dc->rop, dc->color);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, EINA_FALSE);

   sw = LINELEN(src_stride, srcdata);
   dw = LINELEN(dst_stride, dstdata);

   srcdata += src_y * sw;
   dstdata += dst_y * dw;
   for (y = src_h; y; y--)
     {
        func(dstdata + dst_x, srcdata + src_x, src_w, dc->color);
        srcdata += sw;
        dstdata += dw;
     }

   return EINA_TRUE;
}

static Eina_Bool
_image_draw_cpu_rgba_rgba(void *context,
                          const void *src_map, unsigned int src_stride,
                          void *dst_map, unsigned int dst_stride,
                          int src_x, int src_y, int src_w, int src_h,
                          int dst_x, int dst_y, int dst_w, int dst_h,
                          int smooth EINA_UNUSED,
                          Eina_Bool do_async EINA_UNUSED)
{
   Filter_Blend_Draw_Context *dc = context;
   uint32_t *srcdata = (uint32_t *) src_map;
   uint32_t *dstdata = dst_map;
   RGBA_Comp_Func func;
   int y, sw, dw;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((src_w == dst_w) && (src_h == dst_h), EINA_FALSE);

   if (!dc->color)
     return EINA_TRUE;
   else
     func = efl_draw_func_span_get(dc->rop, dc->color, EINA_TRUE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, EINA_FALSE);

   sw = LINELEN(src_stride, srcdata);
   dw = LINELEN(dst_stride, dstdata);

   srcdata += src_y * sw;
   dstdata += dst_y * dw;
   for (y = src_h; y; y--)
     {
        func(dstdata + dst_x, srcdata + src_x, src_w, dc->color, 255);
        srcdata += sw;
        dstdata += dw;
     }

   return EINA_TRUE;
}

static Eina_Bool
_image_draw_cpu_rgba_alpha(void *context,
                           const void *src_map, unsigned int src_stride,
                           void *dst_map, unsigned int dst_stride,
                           int src_x, int src_y, int src_w, int src_h,
                           int dst_x, int dst_y, int dst_w, int dst_h,
                           int smooth EINA_UNUSED,
                           Eina_Bool do_async EINA_UNUSED)
{
   Filter_Blend_Draw_Context *dc = context;
   Eina_Bool alphaonly = dc && dc->alphaonly;
   uint32_t *srcdata = (uint32_t *) src_map;
   uint8_t *dstdata = dst_map;
   int x, y, sw, dw;
#if RGBA2ALPHA_WEIGHTED
   const int WR = 299;
   const int WG = 587;
   const int WB = 114;
#else
   const int WR = 1;
   const int WG = 1;
   const int WB = 1;
#endif
   DEFINE_DIVIDER(WR + WG + WB);

   EINA_SAFETY_ON_FALSE_RETURN_VAL((src_w == dst_w) && (src_h == dst_h), EINA_FALSE);

   sw = LINELEN(src_stride, srcdata);
   dw = LINELEN(dst_stride, dstdata);

   /* FIXME: no draw function here? */

   srcdata += src_y * sw;
   dstdata += dst_y * dw;

   if (!alphaonly)
     {
        for (y = src_h; y; y--)
          {
             uint32_t *s = srcdata + src_x;
             uint8_t *d = dstdata + dst_x;
             for (x = src_w; x; x--, d++, s++)
               *d = (uint8_t) DIVIDE((R_VAL(s) * WR) + (G_VAL(s) * WG) + (B_VAL(s) * WB));
             srcdata += sw;
             dstdata += dw;
          }
     }
   else
     {
        for (y = src_h; y; y--)
          {
             uint32_t *s = srcdata + src_x;
             uint8_t *d = dstdata + dst_x;
             for (x = src_w; x; x--, d++, s++)
               *d = A_VAL(s);
             srcdata += sw;
             dstdata += dw;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_filter_blend_cpu_generic_do(Evas_Filter_Command *cmd, draw_func image_draw)
{
   unsigned int src_len, src_stride, dst_len, dst_stride;
   int sw, sh, dx, dy, dw, dh, sx, sy;
   Filter_Blend_Draw_Context dc;
   Eina_Bool ret = EINA_FALSE;
   Evas_Filter_Buffer *src_fb;
   void *src = NULL, *dst = NULL;

   sx = 0;
   sy = 0;
   ector_buffer_size_get(cmd->input->buffer, &sw, &sh);

   dx = cmd->draw.ox;
   dy = cmd->draw.oy;
   ector_buffer_size_get(cmd->output->buffer, &dw, &dh);

   if ((dw <= 0) || (dh <= 0) || (sw <= 0) || (sh <= 0))
     return EINA_TRUE;

   // Stretch if necessary.

   /* NOTE: As of 2014/03/11, this will happen only with RGBA buffers, since
    * only proxy sources may be scaled. So, we don't need an alpha scaling
    * algorithm just now.
    */

   if ((sw != dw || sh != dh) && (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_XY))
     {
        EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->ctx->buffer_scaled_get, EINA_FALSE);

        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
          sw = dw;
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
          sh = dh;

        BUFFERS_LOCK();
        src_fb = cmd->ctx->buffer_scaled_get(cmd->ctx, cmd->input, sw, sh);
        BUFFERS_UNLOCK();

        EINA_SAFETY_ON_NULL_GOTO(src_fb, end);
        src_fb->locked = EINA_FALSE;
     }
   else src_fb = cmd->input;

   src = _buffer_map_all(src_fb->buffer, &src_len, E_READ, src_fb->alpha_only ? E_ALPHA : E_ARGB, &src_stride);
   dst = _buffer_map_all(cmd->output->buffer, &dst_len, E_WRITE, cmd->output->alpha_only ? E_ALPHA : E_ARGB, &dst_stride);
   EINA_SAFETY_ON_FALSE_GOTO(src && dst, end);

   dc.rop = cmd->draw.rop;
   dc.alphaonly = cmd->draw.alphaonly;
   dc.color = ARGB_JOIN(cmd->draw.A, cmd->draw.R, cmd->draw.G, cmd->draw.B);

   ret = _mapped_blend(&dc, src, src_stride, dst, dst_stride, cmd->draw.fillmode,
                       sx, sy, sw, sh, dx, dy, dw, dh, image_draw);

end:
   if (src) ector_buffer_unmap(src_fb->buffer, src, src_len);
   if (dst) ector_buffer_unmap(cmd->output->buffer, dst, dst_len);
   return ret;
}

static Eina_Bool
_filter_blend_cpu_alpha(Evas_Filter_Command *cmd)
{
   return _filter_blend_cpu_generic_do(cmd, _image_draw_cpu_alpha_alpha);
}

static Eina_Bool
_filter_blend_cpu_alpha_rgba(Evas_Filter_Command *cmd)
{
   return _filter_blend_cpu_generic_do(cmd, _image_draw_cpu_alpha_rgba);
}

static Eina_Bool
_filter_blend_cpu_rgba_alpha(Evas_Filter_Command *cmd)
{
   return _filter_blend_cpu_generic_do(cmd, _image_draw_cpu_rgba_alpha);
}

static Eina_Bool
_filter_blend_cpu_rgba(Evas_Filter_Command *cmd)
{
   return _filter_blend_cpu_generic_do(cmd, _image_draw_cpu_rgba_rgba);
}

static Eina_Bool
_mapped_blend(void *drawctx,
              const void *src_map, unsigned int src_stride,
              void *dst_map, unsigned int dst_stride,
              Evas_Filter_Fill_Mode fillmode,
              int sx, int sy,
              int sw, int sh,
              int dx, int dy,
              int dw, int dh,
              draw_func image_draw)
{
   int right = 0, bottom = 0, left = 0, top = 0;
   int row, col, rows, cols;
   Eina_Bool ret = EINA_TRUE;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((sx == 0) && (sy == 0), EINA_FALSE);

   if (fillmode == EVAS_FILTER_FILL_MODE_NONE)
     {
        _clip_to_target(&sx, &sy, sw, sh, dx, dy, dw, dh, &dx, &dy, &rows, &cols);
        XDBG("blend: %d,%d,%d,%d --> %d,%d,%d,%d (from %dx%d to %dx%d +%d,%d)",
             0, 0, sw, sh, dx, dy, cols, rows, sw, sh, dw, dh, dx, dy);
        image_draw(drawctx,
                   src_map, src_stride, dst_map, dst_stride,
                   sx, sy, cols, rows, // src
                   dx, dy, cols, rows, // dst
                   EINA_TRUE, // smooth
                   EINA_FALSE); // Not async
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
        dx = 0;
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
        dy = 0;
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
                  src_h = sh;
                  dst_h = dh;
                  dst_y = 0;
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
                       src_w = sw;
                       dst_w = dw;
                       dst_x = 0;
                    }
                  else
                    {
                       dst_x = left + dx + col * sw;
                       src_w = MIN(dw - dst_x, sw);
                       dst_w = src_w;
                    }
               }
             if (src_w <= 0 || dst_w <= 0) break;

             XDBG("blend: [%d,%d] %d,%d,%dx%d --> %d,%d,%dx%d "
                  "(src %dx%d, dst %dx%d)",
                  col, row, src_x, src_y, src_w, src_h,
                  dst_x, dst_y, dst_w, dst_h,
                  sw, sh, dw, dh);
             image_draw(drawctx,
                        src_map, src_stride, dst_map, dst_stride,
                        src_x, src_y, src_w, src_h,
                        dst_x, dst_y, dst_w, dst_h,
                        EINA_TRUE, EINA_FALSE);
          }
     }
   return ret;
}

Software_Filter_Func
eng_filter_blend_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);

   if (cmd->input->alpha_only)
     {
        if (cmd->output->alpha_only)
          return _filter_blend_cpu_alpha;
        else
          return _filter_blend_cpu_alpha_rgba;
     }
   else
     {
        if (cmd->output->alpha_only)
          return _filter_blend_cpu_rgba_alpha;
        else
          return _filter_blend_cpu_rgba;
     }
}
