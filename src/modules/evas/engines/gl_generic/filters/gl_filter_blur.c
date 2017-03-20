#include "gl_engine_filter.h"

// sqrt(2 * M_PI)
#define SQRT_2_PI 2.506628274631

static inline double
_radius_to_sigma(double radius)
{
   // FIXME: This was supposed to be sqrt(r/3) ~ or something close
   return /*sqrt*/ (radius / 3.0);
}

static inline double
_gaussian_val(double a, double b, double x)
{
   return a * exp(-(x*x/b));
}

static void
_gaussian_calc(double *values, int count, double radius)
{
   // f(x) = a * exp(-(x^2 / b))
   // sigma is such that variance v = sigma^2
   // v is such that after 3 v the value is almost 0 (ressembles a radius)
   // a = 1 / (sigma * sqrt (2 * pi))
   // b = 2 * sigma^2

   // FIXME: Some of this math doesn't fit right (values too small too fast)

   double a, b, sigma;
   int k;

   sigma = _radius_to_sigma(radius);
   a = 1.0 / (sigma * SQRT_2_PI);
   b = 2.0 * sigma * sigma;

   for (k = 0; k <= count; k++)
     {
        values[k] = _gaussian_val(a, b, k);
        ERR("Gauss %d: %f", k, values[k]);
     }
}

static int
_gaussian_interpolate(GLfloat **weights, GLfloat **offsets, double radius)
{
   int k, num, count;
   GLfloat *w, *o;
   double *values;

   count = (int) ceil(radius);
   if (count & 0x1) count++;
   values = alloca((count + 1) * sizeof(*values));
    _gaussian_calc(values, count, radius);

   num = (count / 2) + 1;
   *offsets = o = calloc(1, num * sizeof(*o));
   *weights = w = calloc(1, num * sizeof(*w));

   // Center pixel's weight
   k = 0;
   o[k] = 0.0;
   w[k] = values[0];
   ERR("Interpolating weights %d: w %f o %f", k, w[k], o[k]);

   // Left & right pixels' interpolated weights
   for (k = 1; k < num; k++)
     {
        double w1, w2;

        w1 = values[(k - 1) * 2 + 1];
        w2 = values[(k - 1) * 2 + 2];
        w[k] = w1 + w2;
        if (EINA_DBL_EQ(w[k], 0.0)) continue;
        o[k] = (w2 / w[k]) + (k - 1.0) * 2.0;
        ERR("Interpolating weights %d: %f %f -> w %f o %f", k, w1, w2, w[k], o[k]);
     }

   return num;
}

static Eina_Bool
_gl_filter_blur(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd)
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *image, *surface;
   RGBA_Draw_Context *dc_save;
   Eina_Bool horiz;
   double sx, sy, sw, sh, ssx, ssy, ssw, ssh, dx, dy, dw, dh, radius;
   int nx, ny, nw, nh, count = 0;
   GLfloat *weights, *offsets;

   DEBUG_TIME_BEGIN();

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

   sx = 0;
   sy = 0;
   sw = cmd->input->w;
   sh = cmd->input->h;
   dx = cmd->draw.ox;
   dy = cmd->draw.oy;
   dw = cmd->output->w;
   dh = cmd->output->h;

   nx = dx; ny = dy; nw = dw; nh = dh;
   RECTS_CLIP_TO_RECT(nx, ny, nw, nh, 0, 0, cmd->output->w, cmd->output->h);
   ssx = (double)sx + ((double)(sw * (nx - dx)) / (double)(dw));
   ssy = (double)sy + ((double)(sh * (ny - dy)) / (double)(dh));
   ssw = ((double)sw * (double)(nw)) / (double)(dw);
   ssh = ((double)sh * (double)(nh)) / (double)(dh);

   dc_save = gc->dc;
   gc->dc = evas_common_draw_context_new();
   evas_common_draw_context_set_multiplier(gc->dc, cmd->draw.R, cmd->draw.G, cmd->draw.B, cmd->draw.A);

   // FIXME: Don't render to same FBO as input! This is not supposed to work!
   if (cmd->input == cmd->output)
     gc->dc->render_op = EVAS_RENDER_COPY;
   else
     gc->dc->render_op = _gfx_to_evas_render_op(cmd->draw.rop);

   count = _gaussian_interpolate(&weights, &offsets, radius);
   evas_gl_common_filter_blur_push(gc, image->tex, ssx, ssy, ssw, ssh, dx, dy, dw, dh,
                                   weights, offsets, count, horiz);
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
gl_filter_blur_func_get(Render_Engine_GL_Generic *re EINA_UNUSED, Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);

   // 1D blurs only, radius != 0
   EINA_SAFETY_ON_FALSE_RETURN_VAL((!cmd->blur.dx) ^ (!cmd->blur.dy), NULL);

   return _gl_filter_blur;
}
