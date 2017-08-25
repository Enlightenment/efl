#include "gl_engine_filter.h"

// sqrt(2 * M_PI)
#define SQRT_2_PI 2.506628274631

static inline double
_radius_to_sigma(double radius)
{
   // In theory, sqrt(radius / 3.0) but that means the outer pixel at radius
   // pixels away from the center have ~0.001 weight.

   // This is an experimental value - to be adjusted!
   return /*sqrt*/ (radius / 3.0);
}

static inline double
_gaussian_val(double a EINA_UNUSED, double b, double x)
{
   return /*a * */ exp(-(x*x/b));
}

static void
_gaussian_calc(double *values, int max_index, double radius)
{
   // Gaussian: f(x) = a * exp(-(x^2 / b))
   // sigma is such that variance v = sigma^2
   // v is such that after 3 v the value is almost 0 (ressembles a radius)
   // a = 1 / (sigma * sqrt (2 * pi))
   // b = 2 * sigma^2
   // The constant a is not required since we always calculate the dividor

   double a, b, sigma;
   int k;

   sigma = _radius_to_sigma(radius);
   a = 1.0 / (sigma * SQRT_2_PI);
   b = 2.0 * sigma * sigma;

   for (k = 0; k <= max_index; k++)
     {
        values[k] = _gaussian_val(a, b, k);
        XDBG("Gauss %d: %f", k, values[k]);
     }
}

static int
_gaussian_interpolate(double **weights, double **offsets, double radius)
{
   int k, count, max_index;
   double *w, *o;
   double *values;

   max_index = (int) ceil(radius);
   if (max_index & 0x1) max_index++;
   values = alloca((max_index + 1) * sizeof(*values));
    _gaussian_calc(values, max_index, radius);

   count = (max_index / 2) + 1;
   *offsets = o = calloc(1, count * sizeof(*o));
   *weights = w = calloc(1, count * sizeof(*w));

   // Center pixel's weight
   k = 0;
   o[k] = 0.0;
   w[k] = values[0];
   XDBG("Interpolating weights %d: w %f o %f", k, w[k], o[k]);

   // Left & right pixels' interpolated weights
   for (k = 1; k < count; k++)
     {
        double w1, w2;

        w1 = values[(k - 1) * 2 + 1];
        w2 = values[(k - 1) * 2 + 2];
        w[k] = w1 + w2;
        if (EINA_DBL_EQ(w[k], 0.0)) continue;
        o[k] = w2 / w[k];
        XDBG("Interpolating weights %d: %f %f -> w %f o %f", k, w1, w2, w[k], o[k]);
     }

   return count;
}

static inline Eina_Rectangle
_rect(int x, int y, int w, int h, int maxw, int maxh)
{
   Eina_Rectangle rect;

   if (x < 0)
     {
        w -= (-x);
        x = 0;
     }
   if (y < 0)
     {
        h -= (-y);
        y = 0;
     }
   if ((x + w) > maxw) w = maxw - x;
   if ((y + h) > maxh) h = maxh - y;
   if (w < 0) w = 0;
   if (h < 0) h = 0;

   rect.x = x;
   rect.y = y;
   rect.w = w;
   rect.h = h;
   return rect;
}

#define S_RECT(_x, _y, _w, _h) _rect(_x, _y, _w, _h, s_w, s_h)
#define D_RECT(_x, _y, _w, _h) _rect(_x, _y, _w, _h, d_w, d_h)

static inline void
_output_scale_get(Evas_Filter_Context *ctx, Evas_Filter_Command *cmd,
                  double *scale_x, double *scale_y)
{
   Evas_Filter_Buffer *fb;
   Eina_List *li;

   EINA_LIST_FOREACH(ctx->buffers, li, fb)
     if (fb->id == EVAS_FILTER_BUFFER_OUTPUT_ID)
       {
          *scale_x = (double) cmd->output->w / (double) fb->w;
          *scale_y = (double) cmd->output->h / (double) fb->h;
       }
}

static Eina_Bool
_gl_filter_blur(Render_Output_GL_Generic *re, Evas_Filter_Command *cmd)
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *image, *surface;
   RGBA_Draw_Context *dc_save;
   Eina_Bool horiz;
   double sx, sy, sw, sh, ssx, ssy, ssw, ssh, dx, dy, dw, dh, radius;
   double s_w, s_h, d_w, d_h, scale_x = 1.0, scale_y = 1.0;
   Eina_Rectangle s_ob, d_ob, s_region[4], d_region[4];
   int nx, ny, nw, nh, regions, count = 0;
   double *weights, *offsets;

   DEBUG_TIME_BEGIN();

   s_w = cmd->input->w;
   s_h = cmd->input->h;
   d_w = cmd->output->w;
   d_h = cmd->output->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(s_w && s_h && d_w && d_h, EINA_FALSE);

   re->window_use(re->software.ob);
   gc = re->window_gl_context_get(re->software.ob);

   image = evas_ector_buffer_drawable_image_get(cmd->input->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);

   surface = evas_ector_buffer_render_image_get(cmd->output->buffer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, EINA_FALSE);

   evas_gl_common_context_target_surface_set(gc, surface);

   if (cmd->blur.dx)
     {
        horiz = EINA_TRUE;
        radius = cmd->blur.dx;
     }
   else
     {
        horiz = EINA_FALSE;
        radius = cmd->blur.dy;
     }

   DBG("blur %d @%p -> %d @%p (%.0fpx %s)",
       cmd->input->id, cmd->input->buffer,
       cmd->output->id, cmd->output->buffer,
       radius, horiz ? "X" : "Y");

   dc_save = gc->dc;
   gc->dc = evas_common_draw_context_new();
   evas_common_draw_context_set_multiplier(gc->dc, cmd->draw.R, cmd->draw.G, cmd->draw.B, cmd->draw.A);

   // FIXME: Don't render to same FBO as input! This is not supposed to work!
   if (cmd->input == cmd->output)
     gc->dc->render_op = EVAS_RENDER_COPY;
   else
     gc->dc->render_op = _gfx_to_evas_render_op(cmd->draw.rop);

   count = _gaussian_interpolate(&weights, &offsets, radius);

   _output_scale_get(cmd->ctx, cmd, &scale_x, &scale_y);
   d_ob = cmd->ctx->obscured.effective;
   d_ob.x *= scale_x;
   d_ob.y *= scale_y;
   d_ob.w *= scale_x;
   d_ob.h *= scale_y;
   s_ob.x = d_ob.x * s_w / d_w;
   s_ob.y = d_ob.y * s_h / d_h;
   s_ob.w = d_ob.w * s_w / d_w;
   s_ob.h = d_ob.h * s_h / d_h;
   if (!d_ob.w || !d_ob.h)
     {
        s_region[0] = S_RECT(0, 0, s_w, s_h);
        d_region[0] = D_RECT(0, 0, d_w, d_h);
        regions = 1;
     }
   else if (horiz)
     {
        // top (full), left, right, bottom (full)
        s_region[0] = S_RECT(0, 0, s_w, s_ob.y);
        d_region[0] = D_RECT(0, 0, d_w, d_ob.y);
        s_region[1] = S_RECT(0, s_ob.y, s_ob.x, s_ob.h);
        d_region[1] = D_RECT(0, d_ob.y, d_ob.x, d_ob.h);
        s_region[2] = S_RECT(s_ob.x + s_ob.w, s_ob.y, s_w - s_ob.x - s_ob.w, s_ob.h);
        d_region[2] = D_RECT(d_ob.x + d_ob.w, d_ob.y, d_w - d_ob.x - d_ob.w, d_ob.h);
        s_region[3] = S_RECT(0, s_ob.y + s_ob.h, s_w, s_h - s_ob.y - s_ob.h);
        d_region[3] = D_RECT(0, d_ob.y + d_ob.h, d_w, d_h - d_ob.y - d_ob.h);
        regions = 4;
     }
   else
     {
        // left (full), top, bottom, right (full)
        s_region[0] = S_RECT(0, 0, s_ob.x, s_h);
        d_region[0] = D_RECT(0, 0, d_ob.x, d_h);
        s_region[1] = S_RECT(s_ob.x, 0, s_ob.w, s_ob.y);
        d_region[1] = D_RECT(d_ob.x, 0, d_ob.w, d_ob.y);
        s_region[2] = S_RECT(s_ob.x, s_ob.y + s_ob.h, s_ob.w, s_h - s_ob.y - s_ob.h);
        d_region[2] = D_RECT(d_ob.x, d_ob.y + d_ob.h, d_ob.w, d_h - d_ob.y - d_ob.h);
        s_region[3] = S_RECT(s_ob.x + s_ob.w, 0, s_w - s_ob.x - s_ob.w, s_h);
        d_region[3] = D_RECT(d_ob.x + d_ob.w, 0, d_w - d_ob.x - d_ob.w, d_h);
        regions = 4;
     }

   for (int k = 0; k < regions; k++)
     {
        sx = s_region[k].x;
        sy = s_region[k].y;
        sw = s_region[k].w;
        sh = s_region[k].h;

        dx = d_region[k].x + cmd->draw.ox;
        dy = d_region[k].y + cmd->draw.oy;
        dw = d_region[k].w;
        dh = d_region[k].h;

        nx = dx; ny = dy; nw = dw; nh = dh;
        RECTS_CLIP_TO_RECT(nx, ny, nw, nh, 0, 0, d_w, d_h);
        ssx = (double)sx + ((double)(sw * (nx - dx)) / (double)(dw));
        ssy = (double)sy + ((double)(sh * (ny - dy)) / (double)(dh));
        ssw = ((double)sw * (double)(nw)) / (double)(dw);
        ssh = ((double)sh * (double)(nh)) / (double)(dh);

        evas_gl_common_filter_blur_push(gc, image->tex, ssx, ssy, ssw, ssh, dx, dy, dw, dh,
                                        weights, offsets, count, radius, horiz);
     }

   free(weights);
   free(offsets);

   evas_common_draw_context_free(gc->dc);
   gc->dc = dc_save;

   evas_ector_buffer_engine_image_release(cmd->input->buffer, image);
   evas_ector_buffer_engine_image_release(cmd->output->buffer, surface);

   DEBUG_TIME_END();

   return EINA_TRUE;
}

GL_Filter_Apply_Func
gl_filter_blur_func_get(Render_Output_GL_Generic *re EINA_UNUSED, Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);

   // 1D blurs only, radius != 0
   EINA_SAFETY_ON_FALSE_RETURN_VAL((!cmd->blur.dx) ^ (!cmd->blur.dy), NULL);

   return _gl_filter_blur;
}
