#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"

#include "draw.h"

static void
_blend_color_argb(int count, const SW_FT_Span *spans, void *user_data)
{
   Span_Data *sd = user_data;
   uint32_t color, *buffer, *target;
   const int pix_stride = sd->raster_buffer->stride / 4;

   // multiply the color with mul_col if any
   color = DRAW_MUL4_SYM(sd->color, sd->mul_col);
   RGBA_Comp_Func_Solid comp_func = efl_draw_func_solid_span_get(sd->op, color);

   // move to the offset location
   buffer = sd->raster_buffer->pixels.u32 + ((pix_stride * sd->offy) + sd->offx);

    while (count--)
      {
          target = buffer + ((pix_stride * spans->y) + spans->x);
          comp_func(target, spans->len, color, spans->coverage);
          ++spans;
      }
}

static void
_blend_color_argb_with_maskA(int count, const SW_FT_Span *spans, void *user_data)
{
   Span_Data *sd = user_data;
   const int pix_stride = sd->raster_buffer->stride / 4;
   Ector_Software_Buffer_Base_Data *mask = sd->mask;

   // multiply the color with mul_col if any
   uint32_t color = DRAW_MUL4_SYM(sd->color, sd->mul_col);
   RGBA_Comp_Func_Solid comp_func = efl_draw_func_solid_span_get(sd->op, color);

   // move to the offset location
   uint32_t *buffer =
         sd->raster_buffer->pixels.u32 + ((pix_stride * sd->offy) + sd->offx);
   uint32_t *mbuffer = mask->pixels.u32;

   //Temp buffer for intermediate processing
   int tsize = sd->raster_buffer->generic->w;
   uint32_t *tbuffer = alloca(sizeof(uint32_t) * tsize);

   while (count--)
     {
        uint32_t *target = buffer + ((pix_stride * spans->y) + spans->x);
        uint32_t *mtarget =
              mbuffer + ((mask->generic->w * spans->y) + spans->x);
        uint32_t *temp = tbuffer;
        memset(temp, 0x00, sizeof(uint32_t) * spans->len);
        comp_func(temp, spans->len, color, spans->coverage);

        //masking
        for (int i = 0; i < spans->len; i++)
          {
             *temp = draw_mul_256(((*mtarget)>>24), *temp);
             int alpha = 255 - ((*temp) >> 24);
             *target = *temp + draw_mul_256(alpha, *target);
             ++temp;
             ++mtarget;
             ++target;
          }
        ++spans;
     }
}

static void
_blend_color_argb_with_maskInvA(int count, const SW_FT_Span *spans, void *user_data)
{
   Span_Data *sd = user_data;
   const int pix_stride = sd->raster_buffer->stride / 4;
   Ector_Software_Buffer_Base_Data *mask = sd->mask;

   // multiply the color with mul_col if any
   uint32_t color = DRAW_MUL4_SYM(sd->color, sd->mul_col);
   RGBA_Comp_Func_Solid comp_func = efl_draw_func_solid_span_get(sd->op, color);

   // move to the offset location
   uint32_t *buffer =
         sd->raster_buffer->pixels.u32 + ((pix_stride * sd->offy) + sd->offx);
   uint32_t *mbuffer = mask->pixels.u32;

   //Temp buffer for intermediate processing
   int tsize = sd->raster_buffer->generic->w;
   uint32_t *tbuffer = alloca(sizeof(uint32_t) * tsize);

   while (count--)
     {
        uint32_t *target = buffer + ((pix_stride * spans->y) + spans->x);
        uint32_t *mtarget =
              mbuffer + ((mask->generic->w * spans->y) + spans->x);
        uint32_t *temp = tbuffer;
        memset(temp, 0x00, sizeof(uint32_t) * spans->len);
        comp_func(temp, spans->len, color, spans->coverage);

        //masking
        for (int i = 0; i < spans->len; i++)
          {
             if (*mtarget)
                *temp = draw_mul_256((255 - ((*mtarget)>>24)), *temp);
             int alpha = 255 - ((*temp) >> 24);
             *target = *temp + draw_mul_256(alpha, *target);
             ++temp;
             ++mtarget;
             ++target;
          }
        ++spans;
     }
}

#define BLEND_GRADIENT_BUFFER_SIZE 2048

typedef void (*src_fetch) (unsigned int *buffer, Span_Data *data, int y, int x, int length);

static void
_blend_gradient(int count, const SW_FT_Span *spans, void *user_data)
{
   RGBA_Comp_Func comp_func;
   Span_Data *data = (Span_Data *)(user_data);
   src_fetch fetchfunc = NULL;
   unsigned int buffer[BLEND_GRADIENT_BUFFER_SIZE], *target, *destbuffer;
   int length, l;
   const int pix_stride = data->raster_buffer->stride / 4;

   // FIXME: Get the proper composition function using ,color, ECTOR_OP etc.
   if (data->type == LinearGradient) fetchfunc = &fetch_linear_gradient;
   if (data->type == RadialGradient) fetchfunc = &fetch_radial_gradient;

   if (!fetchfunc)
     return;

   comp_func = efl_draw_func_span_get(data->op, data->mul_col, data->gradient->alpha);

   // move to the offset location
   destbuffer = data->raster_buffer->pixels.u32 + ((pix_stride * data->offy) + data->offx);

   while (count--)
     {
        target = destbuffer + ((pix_stride * spans->y) + spans->x);
        length = spans->len;
        while (length)
          {
             l = MIN(length, BLEND_GRADIENT_BUFFER_SIZE);
             fetchfunc(buffer, data, spans->y, spans->x, l);
             comp_func(target, buffer, l, data->mul_col, spans->coverage);
             target += l;
             length -= l;
          }
        ++spans;
     }
}

/*!
    \internal
    spans must be sorted on y
*/
static const
SW_FT_Span *_intersect_spans_rect(const Eina_Rectangle *clip,
                                  const SW_FT_Span *spans,
                                  const SW_FT_Span *end,
                                  SW_FT_Span **out_spans,
                                  int available)
{
   SW_FT_Span *out = *out_spans;
   short minx, miny, maxx, maxy;
   minx = clip->x;
   miny = clip->y;
   maxx = minx + clip->w - 1;
   maxy = miny + clip->h - 1;

   while (available && spans < end )
     {
        if (spans->y > maxy)
          {
             spans = end;// update spans so that we can breakout
             break;
          }
        if (spans->y < miny
            || spans->x > maxx
            || spans->x + spans->len <= minx)
          {
             ++spans;
             continue;
          }
        if (spans->x < minx)
          {
             out->len = MIN(spans->len - (minx - spans->x), maxx - minx + 1);
             out->x = minx;
          }
        else
          {
             out->x = spans->x;
             out->len = MIN(spans->len, (maxx - spans->x + 1));
          }
        if (out->len != 0)
          {
             out->y = spans->y;
             out->coverage = spans->coverage;
             ++out;
          }
        ++spans;
        --available;
     }

   *out_spans = out;
   return spans;
}

static inline int
_div_255(int x) { return (x + (x>>8) + 0x80) >> 8; }

static const
SW_FT_Span *_intersect_spans_region(const Shape_Rle_Data *clip,
                                    int *currentClip,
                                    const SW_FT_Span *spans,
                                    const SW_FT_Span *end,
                                    SW_FT_Span **out_spans,
                                    int available)
{
   SW_FT_Span *out = *out_spans;
   int sx1, sx2, cx1, cx2, x, len;

   const SW_FT_Span *clipSpans = clip->spans + *currentClip;
   const SW_FT_Span *clipEnd = clip->spans + clip->size;

   while (available && spans < end )
     {
        if (clipSpans >= clipEnd)
          {
             spans = end;
             break;
          }
        if (clipSpans->y > spans->y)
          {
             ++spans;
             continue;
          }
        if (spans->y != clipSpans->y)
          {
             ++clipSpans;
             continue;
          }
        //assert(spans->y == clipSpans->y);
        sx1 = spans->x;
        sx2 = sx1 + spans->len;
        cx1 = clipSpans->x;
        cx2 = cx1 + clipSpans->len;

        if (cx1 < sx1 && cx2 < sx1)
          {
             ++clipSpans;
             continue;
          }
        else if (sx1 < cx1 && sx2 < cx1)
          {
             ++spans;
             continue;
          }
        x = MAX(sx1, cx1);
        len = MIN(sx2, cx2) - x;
        if (len)
          {
             out->x = MAX(sx1, cx1);
             out->len = MIN(sx2, cx2) - out->x;
             out->y = spans->y;
             out->coverage = _div_255(spans->coverage * clipSpans->coverage);
             ++out;
             --available;
          }
        if (sx2 < cx2)
          {
             ++spans;
          }
        else
          {
             ++clipSpans;
          }
     }

   *out_spans = out;
   *currentClip = clipSpans - clip->spans;
   return spans;
}

static void
_span_fill_clipRect(int span_count, const SW_FT_Span *spans, void *user_data)
{
   const int NSPANS = 256;
   int clip_count, i;
   SW_FT_Span cspans[NSPANS];
   Span_Data *fill_data = (Span_Data *) user_data;
   Clip_Data clip = fill_data->clip;
   SW_FT_Span *clipped;
   Eina_Rectangle *rect;
   Eina_Rectangle tmp_rect;

   clip_count = eina_array_count(clip.clips);

   for (i = 0; i < clip_count; i++)
     {
        rect = (Eina_Rectangle *)eina_array_data_get(clip.clips, i);

        // invert transform the offset
        tmp_rect.x = rect->x - fill_data->offx;
        tmp_rect.y = rect->y - fill_data->offy;
        tmp_rect.w = rect->w;
        tmp_rect.h = rect->h;
        const SW_FT_Span *end = spans + span_count;

        while (spans < end)
          {
             clipped = cspans;
             spans = _intersect_spans_rect(&tmp_rect, spans, end, &clipped, NSPANS);
             if (clipped - cspans)
               fill_data->unclipped_blend(clipped - cspans, cspans, fill_data);
          }
     }
}

static void
_span_fill_clipPath(int span_count, const SW_FT_Span *spans, void *user_data)
{
   const int NSPANS = 256;
   int current_clip = 0;
   SW_FT_Span cspans[NSPANS];
   Span_Data *fill_data = (Span_Data *) user_data;
   Clip_Data clip = fill_data->clip;
   SW_FT_Span *clipped;

   // FIXME: Take clip path offset into account.
   const SW_FT_Span *end = spans + span_count;
   while (spans < end)
     {
        clipped = cspans;
        spans = _intersect_spans_region(clip.path, &current_clip, spans, end, &clipped, NSPANS);
        if (clipped - cspans)
          fill_data->unclipped_blend(clipped - cspans, cspans, fill_data);
     }
}

static void
_adjust_span_fill_methods(Span_Data *spdata)
{
   //Blending Function
   switch(spdata->type)
     {
        case None:
          spdata->unclipped_blend = NULL;
          break;
        case Solid:
          {
             if (spdata->mask)
                {
                   if (spdata->mask_op == 2)
                     spdata->unclipped_blend = &_blend_color_argb_with_maskInvA;
                   else
                     spdata->unclipped_blend = &_blend_color_argb_with_maskA;
                }
             else
                spdata->unclipped_blend = &_blend_color_argb;
           }
          break;
        case LinearGradient:
        case RadialGradient:
          spdata->unclipped_blend = &_blend_gradient;
          break;
     }

   // Clipping Function
   if (spdata->clip.enabled)
     {
        if (spdata->clip.type == 0)
          spdata->blend = &_span_fill_clipRect;
        else
          spdata->blend = &_span_fill_clipPath;
     }
   else
     spdata->blend = spdata->unclipped_blend;
}

void ector_software_thread_init(Ector_Software_Thread *thread)
{
   // initialize the rasterizer and stroker
   sw_ft_grays_raster.raster_new(&thread->raster);

   SW_FT_Stroker_New(&thread->stroker);
   SW_FT_Stroker_Set(thread->stroker, 1 << 6,
                     SW_FT_STROKER_LINECAP_BUTT, SW_FT_STROKER_LINEJOIN_MITER, 0);
}

void ector_software_rasterizer_init(Software_Rasterizer *rasterizer)
{
   //initialize the span data.
   rasterizer->fill_data.clip.enabled = EINA_FALSE;
   rasterizer->fill_data.unclipped_blend = 0;
   rasterizer->fill_data.blend = 0;
   efl_draw_init();
   ector_software_gradient_init();
}

void ector_software_thread_shutdown(Ector_Software_Thread *thread)
{
   sw_ft_grays_raster.raster_done(thread->raster);
   SW_FT_Stroker_Done(thread->stroker);
}

void ector_software_rasterizer_stroke_set(Ector_Software_Thread *thread,
                                          Software_Rasterizer *rasterizer EINA_UNUSED, double width,
                                          Efl_Gfx_Cap cap_style, Efl_Gfx_Join join_style,
                                          Eina_Matrix3 *m)
{
   SW_FT_Stroker_LineCap cap;
   SW_FT_Stroker_LineJoin join;
   int stroke_width;
   double scale_factor = 1.0;
   if (m)
     {
        // get the minimum scale factor from matrix
        scale_factor =  m->xx < m->yy ? m->xx : m->yy;
     }
   width = width * scale_factor;
   width = width/2.0; // as free type uses it as the radius of the
                      // pen not the diameter.
   // convert to freetype co-ordinate
   stroke_width = (int)(width * 64);

   switch (cap_style)
     {
        case EFL_GFX_CAP_SQUARE:
          cap = SW_FT_STROKER_LINECAP_SQUARE;
          break;
        case EFL_GFX_CAP_ROUND:
          cap = SW_FT_STROKER_LINECAP_ROUND;
          break;
        default:
          cap = SW_FT_STROKER_LINECAP_BUTT;
          break;
     }

   switch (join_style)
     {
        case EFL_GFX_JOIN_BEVEL:
          join = SW_FT_STROKER_LINEJOIN_BEVEL;
          break;
        case EFL_GFX_JOIN_ROUND:
          join = SW_FT_STROKER_LINEJOIN_ROUND;
          break;
        default:
          join = SW_FT_STROKER_LINEJOIN_MITER;
          break;
     }
   SW_FT_Stroker_Set(thread->stroker, stroke_width, cap, join, 0);
}

static void
_rle_generation_cb( int count, const SW_FT_Span*  spans,void *user)
{
   Shape_Rle_Data *rle = (Shape_Rle_Data *) user;
   int newsize = rle->size + count;

   // allocate enough memory for new spans
   // alloc is required to prevent free and reallocation
   // when the rle needs to be regenerated because of attribute change.
   if (rle->alloc < newsize)
     {
        rle->spans = (SW_FT_Span *) realloc(rle->spans, newsize * sizeof(SW_FT_Span));
        rle->alloc = newsize;
     }

   // copy the new spans to the allocated memory
   SW_FT_Span *lastspan = (rle->spans + rle->size);
   memcpy(lastspan,spans, count * sizeof(SW_FT_Span));

   // update the size
   rle->size = newsize;
}

Shape_Rle_Data *
ector_software_rasterizer_generate_rle_data(Ector_Software_Thread *thread,
                                            Software_Rasterizer *rasterizer EINA_UNUSED,
                                            SW_FT_Outline *outline)
{
   int i, rle_size;
   int l = 0, t = 0, r = 0, b = 0;
   Shape_Rle_Data *rle_data = (Shape_Rle_Data *) calloc(1, sizeof(Shape_Rle_Data));
   SW_FT_Raster_Params params;
   SW_FT_Span* span;

   params.flags = SW_FT_RASTER_FLAG_DIRECT | SW_FT_RASTER_FLAG_AA ;
   params.gray_spans = &_rle_generation_cb;
   params.user = rle_data;
   params.source = outline;

   sw_ft_grays_raster.raster_render(thread->raster, &params);

   // update RLE bounding box.
   span = rle_data->spans;
   rle_size = rle_data->size;
   if (rle_size)
     {
        t = span[0].y;
        b = span[rle_size-1].y;
        for (i = 0; i < rle_size; i++)
          {
             if (span[i].x < l) l = span[i].x;
             if (span[i].x + span[i].len > r) r = span[i].x + span[i].len;
          }
        rle_data->bbox.x = l;
        rle_data->bbox.y = t;
        rle_data->bbox.w = r - l;
        rle_data->bbox.h = b - t + 1;
     }
   return rle_data;
}

Shape_Rle_Data *
ector_software_rasterizer_generate_stroke_rle_data(Ector_Software_Thread *thread,
                                                   Software_Rasterizer *rasterizer,
                                                   SW_FT_Outline *outline,
                                                   Eina_Bool closePath)
{
   uint32_t points,contors;
   Shape_Rle_Data *rle_data;
   SW_FT_Outline strokeOutline = { 0, 0, NULL, NULL, NULL, 0 };

   SW_FT_Stroker_ParseOutline(thread->stroker, outline, !closePath);
   SW_FT_Stroker_GetCounts(thread->stroker,&points, &contors);

   strokeOutline.points = (SW_FT_Vector *) calloc(points, sizeof(SW_FT_Vector));
   strokeOutline.tags = (char *) calloc(points, sizeof(char));
   strokeOutline.contours = (short *) calloc(contors, sizeof(short));

   SW_FT_Stroker_Export(thread->stroker, &strokeOutline);

   rle_data = ector_software_rasterizer_generate_rle_data(thread, rasterizer, &strokeOutline);

   // cleanup the outline data.
   free(strokeOutline.points);
   free(strokeOutline.tags);
   free(strokeOutline.contours);

   return rle_data;
}

void
ector_software_rasterizer_destroy_rle_data(Shape_Rle_Data *rle)
{
   if (rle)
     {
        if (rle->spans)
          free(rle->spans);
        free(rle);
     }
}

static
void _setup_span_fill_matrix(Software_Rasterizer *rasterizer)
{
   if (rasterizer->transform)
     eina_matrix3_inverse(rasterizer->transform, &rasterizer->fill_data.inv);
   else
     eina_matrix3_identity(&rasterizer->fill_data.inv);
}

void
ector_software_rasterizer_transform_set(Software_Rasterizer *rasterizer, Eina_Matrix3 *t)
{
   rasterizer->transform = t;
}

void
ector_software_rasterizer_clip_rect_set(Software_Rasterizer *rasterizer, Eina_Array *clips)
{
   if (clips)
     {
        rasterizer->fill_data.clip.clips = clips;
        rasterizer->fill_data.clip.type = 0;
        rasterizer->fill_data.clip.enabled = EINA_TRUE;
     }
   else
     {
        rasterizer->fill_data.clip.clips = NULL;
        rasterizer->fill_data.clip.type = 0;
        rasterizer->fill_data.clip.enabled = EINA_FALSE;
     }
}

void
ector_software_rasterizer_clip_shape_set(Software_Rasterizer *rasterizer, Shape_Rle_Data *clip)
{
   if (clip)
     {
        rasterizer->fill_data.clip.path = clip;
        rasterizer->fill_data.clip.type = 1;
        rasterizer->fill_data.clip.enabled = EINA_TRUE;
     }
   else
     {
        rasterizer->fill_data.clip.path = NULL;
        rasterizer->fill_data.clip.type = 0;
        rasterizer->fill_data.clip.enabled = EINA_FALSE;
     }
}

void
ector_software_rasterizer_color_set(Software_Rasterizer *rasterizer, int r, int g, int b, int a)
{
   rasterizer->fill_data.color = DRAW_ARGB_JOIN(a, r, g, b);
   rasterizer->fill_data.type = Solid;
}

void ector_software_rasterizer_linear_gradient_set(Software_Rasterizer *rasterizer,
                                                   Ector_Renderer_Software_Gradient_Data *linear)
{
   rasterizer->fill_data.gradient = linear;
   rasterizer->fill_data.type = LinearGradient;
}

void
ector_software_rasterizer_radial_gradient_set(Software_Rasterizer *rasterizer,
                                              Ector_Renderer_Software_Gradient_Data *radial)
{
   rasterizer->fill_data.gradient = radial;
   rasterizer->fill_data.type = RadialGradient;
}

void
ector_software_rasterizer_draw_rle_data(Software_Rasterizer *rasterizer,
                                        int x, int y, uint32_t mul_col,
                                        Efl_Gfx_Render_Op op, Shape_Rle_Data* rle,
                                        Ector_Buffer *mask,
                                        int mask_op)
{
   if (!rle) return;

   rasterizer->fill_data.offx = x;
   rasterizer->fill_data.offy = y;
   rasterizer->fill_data.mul_col = mul_col;
   rasterizer->fill_data.op = op;
   rasterizer->fill_data.mask =
         mask ? efl_data_scope_get(mask, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN) : NULL;
   rasterizer->fill_data.mask_op = mask_op;

   _setup_span_fill_matrix(rasterizer);
   _adjust_span_fill_methods(&rasterizer->fill_data);

   if (rasterizer->fill_data.blend)
     rasterizer->fill_data.blend(rle->size, rle->spans, &rasterizer->fill_data);
}
