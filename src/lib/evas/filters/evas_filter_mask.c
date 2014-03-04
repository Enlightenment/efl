/* Implementation of some masking functions for the software engine */

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_filter.h"
#include "evas_filter_private.h"
#include "evas_blend_private.h"


// Naming convention: _func_engine_incolor_maskcolor_outcolor()
static Eina_Bool _mask_cpu_alpha_alpha_alpha(Evas_Filter_Command *cmd);
static Eina_Bool _mask_cpu_alpha_rgba_rgba(Evas_Filter_Command *cmd);
static Eina_Bool _mask_cpu_alpha_alpha_rgba(Evas_Filter_Command *cmd);
static Eina_Bool _mask_cpu_rgba_alpha_rgba(Evas_Filter_Command *cmd);
static Eina_Bool _mask_cpu_rgba_rgba_rgba(Evas_Filter_Command *cmd);

Evas_Filter_Apply_Func
evas_filter_mask_cpu_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->mask, NULL);

   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input->backing, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output->backing, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->mask->backing, NULL);

   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->input->w == cmd->output->w, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->input->h == cmd->output->h, NULL);

   if (cmd->input->alpha_only)
     {
        if (cmd->mask->alpha_only && cmd->output->alpha_only)
          return _mask_cpu_alpha_alpha_alpha;
        else if (!cmd->mask->alpha_only && !cmd->output->alpha_only)
          return _mask_cpu_alpha_rgba_rgba;
        else if (cmd->mask->alpha_only && !cmd->output->alpha_only)
          return _mask_cpu_alpha_alpha_rgba;
     }
   else
     {
        if (cmd->mask->alpha_only && !cmd->output->alpha_only)
          return _mask_cpu_rgba_alpha_rgba;
        else if (!cmd->mask->alpha_only && !cmd->output->alpha_only)
          return _mask_cpu_rgba_rgba_rgba;
     }

   CRI("If input or mask is RGBA, then output must also be RGBA: %s [%s] %s",
       cmd->input->alpha_only ? "alpha" : "rgba",
       cmd->mask->alpha_only ? "alpha" : "rgba",
       cmd->output->alpha_only ? "alpha" : "rgba");
   return NULL;
}

static Eina_Bool
_mask_cpu_alpha_alpha_alpha(Evas_Filter_Command *cmd)
{
   Alpha_Gfx_Func func;
   RGBA_Image *in, *out, *mask;
   DATA8 *src, *dst, *msk;
   int render_op = cmd->draw.render_op;
   int w, h, mw, mh, x, y, my;
   int stepsize, stepcount, step;

   /* Mechanism:
    * 1. Stretch mask as requested in fillmode
    * 2. Copy source to destination
    * 3. Render mask into destination using alpha function
    *
    * FIXME: Could probably be optimized into a single op :)
    */

   in = (RGBA_Image *) cmd->input->backing;
   out = (RGBA_Image *) cmd->output->backing;
   mask = (RGBA_Image *) cmd->mask->backing;

   w = cmd->input->w;
   h = cmd->input->h;
   mw = cmd->mask->w;
   mh = cmd->mask->h;
   src = in->image.data8;
   dst = out->image.data8;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((w > 0) && (mw > 0), EINA_FALSE);

   stepsize  = MIN(mw, w);
   stepcount = w / stepsize;

   // Stretch if necessary.
   if ((mw != w || mh != h) && (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_XY))
     {
        Evas_Filter_Buffer *fb;

        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
          mw = w;
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
          mh = h;

        BUFFERS_LOCK();
        fb = evas_filter_buffer_scaled_get(cmd->ctx, cmd->mask, mw, mh);
        BUFFERS_UNLOCK();

        EINA_SAFETY_ON_NULL_RETURN_VAL(fb, EINA_FALSE);
        fb->locked = EINA_FALSE;
        mask = fb->backing;
     }

   msk = mask->image.data8;

   // First pass: copy to dest
   if (src != dst)
     memcpy(dst, src, w * h * sizeof(DATA8));

   // Second pass: apply render op
   func = evas_common_alpha_func_get(render_op);
   for (y = 0, my = 0; y < h; y++, my++, msk += mw)
     {
        if (my >= mh)
          {
             my = 0;
             msk = mask->image.data8;
          }

        for (step = 0; step < stepcount; step++, dst += stepsize)
          func(msk, dst, stepsize);

        x = stepsize * stepcount;
        if (x < w)
          {
             func(msk, dst, w - x);
             dst += w - x;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_mask_cpu_rgba_alpha_rgba(Evas_Filter_Command *cmd)
{
   Evas_Filter_Buffer *fb;
   Eina_Bool ok;

   /* Mechanism:
    * 1. Swap input and mask
    * 2. Apply mask operation for alpha+rgba+rgba
    * 3. Swap input and mask
    */

   fb = cmd->input;
   cmd->input = cmd->mask;
   cmd->mask = fb;

   ok = _mask_cpu_alpha_rgba_rgba(cmd);

   fb = cmd->input;
   cmd->input = cmd->mask;
   cmd->mask = fb;

   return ok;
}

static Eina_Bool
_mask_cpu_alpha_rgba_rgba(Evas_Filter_Command *cmd)
{
   RGBA_Gfx_Func func1, func2;
   RGBA_Image *in, *out, *mask;
   DATA8 *src;
   DATA32 *dst, *msk, *span;
   int op = cmd->draw.render_op;
   int w, h, mw, mh, y, my, r;
   int stepsize, stepcount, step;
   DATA32 color2;

   /* Mechanism:
    * 1. Stretch mask as requested in fillmode
    * 2. Render mask to span using input as mask
    * 3. Render span into destination
    *
    * FIXME: Could probably be optimized into a single op :)
    */

   in = (RGBA_Image *) cmd->input->backing;
   out = (RGBA_Image *) cmd->output->backing;
   mask = (RGBA_Image *) cmd->mask->backing;

   w = cmd->input->w;
   h = cmd->input->h;
   mw = cmd->mask->w;
   mh = cmd->mask->h;
   src = in->image.data8;
   dst = out->image.data;

   // Stretch if necessary.
   if ((mw != w || mh != h) && (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_XY))
     {
        Evas_Filter_Buffer *fb;

        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
          mw = w;
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
          mh = h;

        BUFFERS_LOCK();
        fb = evas_filter_buffer_scaled_get(cmd->ctx, cmd->mask, mw, mh);
        BUFFERS_UNLOCK();

        EINA_SAFETY_ON_NULL_RETURN_VAL(fb, EINA_FALSE);
        fb->locked = EINA_FALSE;
        mask = fb->backing;
     }

   color2 = ARGB_JOIN(cmd->draw.A, cmd->draw.R, cmd->draw.G, cmd->draw.B);
   msk = mask->image.data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msk, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((w > 0) && (mw > 0), EINA_FALSE);

   stepsize  = MIN(mw, w);
   stepcount = w / stepsize;
   span = malloc(stepsize * sizeof(DATA32));

   func1 = evas_common_gfx_func_composite_pixel_mask_span_get(mask, out, 1, EVAS_RENDER_COPY);
   func2 = evas_common_gfx_func_composite_pixel_color_span_get(mask, color2, out, 1, op);

   // Apply mask using Gfx functions
   for (y = 0, my = 0; y < h; y++, my++, msk += mw)
     {
        if (my >= mh)
          {
             my = 0;
             msk = mask->image.data;
          }

        for (step = 0; step < stepcount; step++, dst += stepsize, src += stepsize)
          {
             memset(span, 0, stepsize * sizeof(DATA32));
             func1(msk, src, 0, span, stepsize);
             func2(span, NULL, color2, dst, stepsize);
          }

        r = w - (stepsize * stepcount);
        if (r > 0)
          {
             memset(span, 0, r * sizeof(DATA32));
             func1(msk, src, 0, span, r);
             func2(span, NULL, color2, dst, r);
             dst += r;
             src += r;
          }
     }

   free(span);
   return EINA_TRUE;
}

static Eina_Bool
_mask_cpu_alpha_alpha_rgba(Evas_Filter_Command *cmd)
{
   RGBA_Gfx_Func func;
   Alpha_Gfx_Func span_func;
   RGBA_Image *in, *out, *mask;
   DATA8 *src, *msk, *span;
   DATA32 *dst;
   DATA32 color;
   int op = cmd->draw.render_op;
   int w, h, mw, mh, y, my, r;
   int stepsize, stepcount, step;

   /* Mechanism:
    * 1. Copy mask to span buffer (1 line)
    * 2. Multiply source by span (so that: span = mask * source)
    * 3. Render span to destination using color (blend)
    *
    * FIXME: Could probably be optimized into a single op :)
    */

   in = (RGBA_Image *) cmd->input->backing;
   out = (RGBA_Image *) cmd->output->backing;
   mask = (RGBA_Image *) cmd->mask->backing;

   w = cmd->input->w;
   h = cmd->input->h;
   mw = cmd->mask->w;
   mh = cmd->mask->h;
   src = in->image.data8;
   dst = out->image.data;
   color = ARGB_JOIN(cmd->draw.A, cmd->draw.R, cmd->draw.G, cmd->draw.B);

   EINA_SAFETY_ON_FALSE_RETURN_VAL((w > 0) && (mw > 0), EINA_FALSE);

   // Stretch if necessary.
   if ((mw != w || mh != h) && (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_XY))
     {
        Evas_Filter_Buffer *fb;

        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
          mw = w;
        if (cmd->draw.fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
          mh = h;

        BUFFERS_LOCK();
        fb = evas_filter_buffer_scaled_get(cmd->ctx, cmd->mask, mw, mh);
        BUFFERS_UNLOCK();

        EINA_SAFETY_ON_NULL_RETURN_VAL(fb, EINA_FALSE);
        fb->locked = EINA_FALSE;
        mask = fb->backing;
     }

   msk = mask->image.data8;
   stepsize  = MIN(mw, w);
   stepcount = w / stepsize;
   span = malloc(stepsize * sizeof(DATA8));

   func = evas_common_gfx_func_composite_mask_color_span_get(color, out, 1, op);
   span_func = evas_common_alpha_func_get(EVAS_RENDER_MASK);

   for (y = 0, my = 0; y < h; y++, my++, msk += mw)
     {
        if (my >= mh)
          {
             my = 0;
             msk = mask->image.data8;
          }

        for (step = 0; step < stepcount; step++, dst += stepsize, src += stepsize)
          {
             memcpy(span, msk, stepsize * sizeof(DATA8));
             span_func(src, span, stepsize);
             func(NULL, span, color, dst, stepsize);
          }

        r = w - (stepsize * stepcount);
        if (r > 0)
          {
             memcpy(span, msk, r * sizeof(DATA8));
             span_func(src, span, r);
             func(NULL, span, color, dst, r);
             dst += r;
             src += r;
          }
     }

   free(span);
   return EINA_TRUE;
}

static Eina_Bool
_mask_cpu_rgba_rgba_rgba(Evas_Filter_Command *cmd)
{
   Evas_Filter_Command fake_cmd;
   Evas_Filter_Apply_Func blend;
   Evas_Filter_Buffer *fb;
   int w, h;

   fake_cmd = *cmd;
   w = cmd->input->w;
   h = cmd->input->h;

   /* Blend 2 rgba images into rgba destination.
    * Mechanism:
    * 1. Copy input to temp (COPY)
    * 2. Blend mask to temp (MUL)
    * 3. Blend temp to output (render_op)
    */

   // Copy
   BUFFERS_LOCK();
   fb = evas_filter_buffer_scaled_get(cmd->ctx, cmd->input, w, h);
   BUFFERS_UNLOCK();
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, EINA_FALSE);
   fb->locked = EINA_TRUE;

   // Mask --> Temp
   fake_cmd.input = cmd->mask;
   fake_cmd.mask = NULL;
   fake_cmd.output = fb;
   fake_cmd.draw.render_op = EVAS_RENDER_MUL;
   blend = evas_filter_blend_cpu_func_get(&fake_cmd);
   EINA_SAFETY_ON_NULL_RETURN_VAL(blend, EINA_FALSE);
   blend(&fake_cmd);

   // Temp --> Output
   fake_cmd.draw.render_op = EVAS_RENDER_BLEND;
   fake_cmd.input = fb;
   fake_cmd.output = cmd->output;
   blend = evas_filter_blend_cpu_func_get(&fake_cmd);
   EINA_SAFETY_ON_NULL_RETURN_VAL(blend, EINA_FALSE);
   blend(&fake_cmd);

   fb->locked = EINA_FALSE;
   return EINA_TRUE;
}
