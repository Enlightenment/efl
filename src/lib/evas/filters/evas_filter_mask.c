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
          return evas_filter_blend_cpu_func_get(cmd); // Check this. Merge?
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
    * 1. Copy source to destination
    * 2. Render mask into destination using alpha function
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
   src = in->mask.data;
   dst = out->mask.data;
   msk = mask->mask.data;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((w > 0) && (mw > 0), EINA_FALSE);

   stepsize  = MIN(mw, w);
   stepcount = w / stepsize;

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
             msk = mask->mask.data;
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
   int w, h, mw, mh, x, y, my;
   int stepsize, stepcount, step;
   DATA32 color2;

   /* Mechanism:
    * 1. Render mask to span using input as mask
    * 2. Render span into destination
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
   src = in->mask.data;
   dst = out->image.data;
   msk = mask->image.data;

   color2 = ARGB_JOIN(cmd->draw.A, cmd->draw.R, cmd->draw.G, cmd->draw.B);

   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msk, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((w > 0) && (mw > 0), EINA_FALSE);

   stepsize  = MIN(mw, w);
   stepcount = w / stepsize;
   span = malloc(stepsize * sizeof(DATA32));

   func1 = evas_common_gfx_func_composite_pixel_mask_span_get(mask, out, 1, EVAS_RENDER_COPY);
   func2 = evas_common_gfx_func_composite_pixel_color_span_get(mask, color2, out, 1, op);

   for (y = 0, my = 0; y < h; y++, my++, msk += stepsize)
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

        x = stepsize * stepcount;
        if (x < w)
          {
             memset(span, 0, (w - x) * sizeof(DATA32));
             func1(msk, src, 0, span, w - x);
             func2(span, NULL, color2, dst, w - x);
             dst += w - x;
             src += w - x;
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
   int w, h, mw, mh, x, y, my;
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
   src = in->mask.data;
   dst = out->image.data;
   msk = mask->mask.data;
   color = ARGB_JOIN(cmd->draw.A, cmd->draw.R, cmd->draw.G, cmd->draw.B);

   EINA_SAFETY_ON_FALSE_RETURN_VAL((w > 0) && (mw > 0), EINA_FALSE);

   stepsize  = MIN(mw, w);
   stepcount = w / stepsize;
   span = malloc(stepsize * sizeof(DATA8));

   func = evas_common_gfx_func_composite_mask_color_span_get(color, out, 1, op);
   span_func = evas_common_alpha_func_get(EVAS_RENDER_MASK);

   for (y = 0, my = 0; y < h; y++, my++, msk += stepsize)
     {
        if (my >= mh)
          {
             my = 0;
             msk = mask->mask.data;
          }

        for (step = 0; step < stepcount; step++, dst += stepsize, src += stepsize)
          {
             memcpy(span, msk, stepsize * sizeof(DATA8));
             span_func(src, span, stepsize);
             func(NULL, span, color, dst, stepsize);
          }

        x = stepsize * stepcount;
        if (x < w)
          {
             memcpy(span, msk, (w - x) * sizeof(DATA8));
             span_func(src, span, w - x);
             func(NULL, span, color, dst, w - x);
             dst += w - x;
             src += w - x;
          }
     }

   free(span);
   return EINA_TRUE;
}
