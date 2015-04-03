#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"
#include "ector_blend_private.h"

static void
_blend_color_argb(int count, const SW_FT_Span *spans, void *userData)
{
   Span_Data *data = (Span_Data *)(userData);

   // multiply the color with mul_col if any
   uint color = ECTOR_MUL4_SYM(data->color, data->mul_col);
   Eina_Bool solidSource = ((color >> 24) == 255);

   // move to the offset location
   uint *buffer = data->raster_buffer.buffer + (data->raster_buffer.width * data->offy + data->offx);

   if (solidSource)
     {
        while (count--)
          {
             uint *target = buffer + (data->raster_buffer.width * spans->y + spans->x);
             if (spans->coverage == 255)
               {
                  _ector_memfill(target, color, spans->len);
               }
             else
               {
                  uint c = ECTOR_MUL_256(color, spans->coverage);
                  int ialpha = 255 - spans->coverage;
                  for (int i = 0; i < spans->len; ++i)
                    target[i] = c + ECTOR_MUL_256(target[i], ialpha);
               }
             ++spans;
          }
        return;
     }

   while (count--)
     {
        uint *target = buffer + (data->raster_buffer.width * spans->y + spans->x);
        uint c =  ECTOR_MUL_256(color, spans->coverage);
        int ialpha = (~c) >> 24;

        for (int i = 0; i < spans->len; ++i)
            target[i] = c + ECTOR_MUL_256(target[i], ialpha);
        ++spans;
     }
}

int buffer_size = 2048;

typedef void (*src_fetch) (unsigned int *buffer, Span_Data *data, int y, int x, int length);

static void
_blend_gradient(int count, const SW_FT_Span *spans, void *userData)
{
    Span_Data *data = (Span_Data *)(userData);
    src_fetch fetchfunc = NULL;

    if(data->type == LinearGradient) fetchfunc = &fetch_linear_gradient;
    if(data->type == RadialGradient) fetchfunc = &fetch_radial_gradient;

    unsigned int buffer[buffer_size];

    // move to the offset location
    unsigned int *destbuffer = data->raster_buffer.buffer + (data->raster_buffer.width * data->offy + data->offx);

    while (count--)
      {
         unsigned int *target = destbuffer + (data->raster_buffer.width * spans->y + spans->x);
         int length = spans->len;
         while (length)
           {
              int l = MIN(length, buffer_size);
              fetchfunc(buffer, data, spans->y, spans->x, l);
              if (data->mul_col == 0xffffffff)
                _ector_comp_func_source_over(target, buffer, l, spans->coverage); // TODO use proper composition func
              else
                _ector_comp_func_source_over_mul_c(target, buffer, data->mul_col, l, spans->coverage);
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
SW_FT_Span *_intersect_spans_rect(const Eina_Rectangle *clip, const SW_FT_Span *spans, const SW_FT_Span *end,
                                  SW_FT_Span **outSpans, int available)
{
   SW_FT_Span *out = *outSpans;
   const short minx = clip->x;
   const short miny = clip->y;
   const short maxx = minx + clip->w - 1;
   const short maxy = miny + clip->h - 1;

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

    *outSpans = out;

    return spans;
}

static inline int
_div_255(int x) { return (x + (x>>8) + 0x80) >> 8; }

static const
SW_FT_Span *_intersect_spans_region(const Shape_Rle_Data *clip, int *currentClip,
                                       const SW_FT_Span *spans, const SW_FT_Span *end,
                                       SW_FT_Span **outSpans, int available)
{
    SW_FT_Span *out = *outSpans;

    const SW_FT_Span *clipSpans = clip->spans + *currentClip;
    const SW_FT_Span *clipEnd = clip->spans + clip->size;

    while (available && spans < end ) {
        if (clipSpans >= clipEnd) {
            spans = end;
            break;
        }
        if (clipSpans->y > spans->y) {
            ++spans;
            continue;
        }
        if (spans->y != clipSpans->y) {
            ++clipSpans;
            continue;
        }
        //assert(spans->y == clipSpans->y);

        int sx1 = spans->x;
        int sx2 = sx1 + spans->len;
        int cx1 = clipSpans->x;
        int cx2 = cx1 + clipSpans->len;

        if (cx1 < sx1 && cx2 < sx1) {
            ++clipSpans;
            continue;
        } else if (sx1 < cx1 && sx2 < cx1) {
            ++spans;
            continue;
        }
        int x = MAX(sx1, cx1);
        int len = MIN(sx2, cx2) - x;
        if (len) {
            out->x = MAX(sx1, cx1);
            out->len = MIN(sx2, cx2) - out->x;
            out->y = spans->y;
            out->coverage = _div_255(spans->coverage * clipSpans->coverage);
            ++out;
            --available;
        }
        if (sx2 < cx2) {
            ++spans;
        } else {
            ++clipSpans;
        }
    }

    *outSpans = out;
    *currentClip = clipSpans - clip->spans;
    return spans;
}

static void
_span_fill_clipRect(int spanCount, const SW_FT_Span *spans, void *userData)
{
    const int NSPANS = 256;
    int clip_count, i;
    SW_FT_Span cspans[NSPANS];
    Span_Data *fillData = (Span_Data *) userData;
    Clip_Data clip = fillData->clip;

    clip_count = eina_array_count(clip.clips);
    for (i = 0; i < clip_count ; i ++)
      {
        Eina_Rectangle *rect = (Eina_Rectangle *)eina_array_data_get(clip.clips, i);
        Eina_Rectangle tmpRect;

        // invert transform the offset
        tmpRect.x = rect->x - fillData->offx;
        tmpRect.y = rect->y - fillData->offy;
        tmpRect.w = rect->w;
        tmpRect.h = rect->h;
        const SW_FT_Span *end = spans + spanCount;

        while (spans < end)
          {
             SW_FT_Span *clipped = cspans;
             spans = _intersect_spans_rect(&tmpRect,spans, end, &clipped, NSPANS);
             if (clipped - cspans)
               fillData->unclipped_blend(clipped - cspans, cspans, fillData);
          }
      }
}

static void
_span_fill_clipPath(int spanCount, const SW_FT_Span *spans, void *userData)
{
    const int NSPANS = 256;
    int current_clip = 0;
    SW_FT_Span cspans[NSPANS];
    Span_Data *fillData = (Span_Data *) userData;
    Clip_Data clip = fillData->clip;

    //TODO take clip path offset into account.
    
    const SW_FT_Span *end = spans + spanCount;
    while (spans < end)
      {
         SW_FT_Span *clipped = cspans;
         spans = _intersect_spans_region(clip.path, &current_clip, spans, end, &clipped, NSPANS);
         if (clipped - cspans)
           fillData->unclipped_blend(clipped - cspans, cspans, fillData);
      }
}

static void
_adjust_span_fill_methods(Span_Data *spdata)
{
   switch(spdata->type)
     {
      case None:
         spdata->unclipped_blend = 0;
         break;
      case Solid:
         spdata->unclipped_blend = &_blend_color_argb;
         break;
      case LinearGradient:
      case RadialGradient:
         spdata->unclipped_blend = &_blend_gradient;
         break;
      case Image:
         spdata->unclipped_blend = 0;//&_blend_image;
         break;
     }

   // setup clipping
   if (!spdata->unclipped_blend)
     {
        spdata->blend = 0;
     }
   else if (!spdata->clip.enabled)
     {
        spdata->blend = spdata->unclipped_blend;
     }
   else if (spdata->clip.hasRectClip)
     {
        spdata->blend = &_span_fill_clipRect;
     }
   else
     {
        spdata->blend = &_span_fill_clipPath;
     }
}



void ector_software_rasterizer_init(Software_Rasterizer *rasterizer)
{
   // initialize the rasterizer and stroker
   unsigned char* renderPool = (unsigned char*) malloc(1024 * 100);
   sw_ft_grays_raster.raster_new(&rasterizer->raster);
   sw_ft_grays_raster.raster_reset(rasterizer->raster, renderPool, 1024*100);

   SW_FT_Stroker_New(&rasterizer->stroker);
   SW_FT_Stroker_Set(rasterizer->stroker, 1<<6,SW_FT_STROKER_LINECAP_BUTT,SW_FT_STROKER_LINEJOIN_MITER,0);

   //initialize the span data.
   rasterizer->fillData.raster_buffer.buffer = NULL;
   rasterizer->fillData.clip.enabled = EINA_FALSE;
   rasterizer->fillData.unclipped_blend = 0;
   rasterizer->fillData.blend = 0;
}

void ector_software_rasterizer_done(Software_Rasterizer *rasterizer)
{
   sw_ft_grays_raster.raster_done(rasterizer->raster);
   SW_FT_Stroker_Done(rasterizer->stroker);
   //TODO free the pool memory
}


void ector_software_rasterizer_stroke_set(Software_Rasterizer *rasterizer, double width,
                                          Efl_Gfx_Cap cap_style, Efl_Gfx_Join join_style)
{
   SW_FT_Stroker_LineCap cap;
   SW_FT_Stroker_LineJoin join;

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

   int stroke_width = (int)(width * 64);
   SW_FT_Stroker_Set(rasterizer->stroker, stroke_width, cap, join, 0);
}

static void
_rle_generation_cb( int count, const SW_FT_Span*  spans,void *user)
{
   Shape_Rle_Data *rle = (Shape_Rle_Data *) user;
   int newsize = rle->size + count;

   // allocate enough memory for new spans
   // alloc is required to prevent free and reallocation
   // when the rle needs to be regenerated because of attribute change.
   if(rle->alloc < newsize)
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
ector_software_rasterizer_generate_rle_data(Software_Rasterizer *rasterizer, SW_FT_Outline *outline)
{
   Shape_Rle_Data *rle_data = (Shape_Rle_Data *) calloc(1, sizeof(Shape_Rle_Data));
   SW_FT_Raster_Params params;

   params.flags = SW_FT_RASTER_FLAG_DIRECT | SW_FT_RASTER_FLAG_AA ;
   params.gray_spans = &_rle_generation_cb;
   params.user = rle_data;
   params.source = outline;

   sw_ft_grays_raster.raster_render(rasterizer->raster, &params);

   return rle_data;
}

Shape_Rle_Data *
ector_software_rasterizer_generate_stroke_rle_data(Software_Rasterizer *rasterizer, SW_FT_Outline *outline, Eina_Bool closePath)
{
   uint points,contors;

   SW_FT_Stroker_ParseOutline(rasterizer->stroker, outline, !closePath);
   SW_FT_Stroker_GetCounts(rasterizer->stroker,&points, &contors);

   SW_FT_Outline strokeOutline = {0};
   strokeOutline.points = (SW_FT_Vector *) calloc(points, sizeof(SW_FT_Vector));
   strokeOutline.tags = (char *) calloc(points, sizeof(char));
   strokeOutline.contours = (short *) calloc(contors, sizeof(short));

   SW_FT_Stroker_Export(rasterizer->stroker, &strokeOutline);

   Shape_Rle_Data *rle_data = ector_software_rasterizer_generate_rle_data(rasterizer, &strokeOutline);

   // cleanup the outline data.
   free(strokeOutline.points);
   free(strokeOutline.tags);
   free(strokeOutline.contours);

   return rle_data;
}

void ector_software_rasterizer_destroy_rle_data(Shape_Rle_Data *rle)
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
     {
        eina_matrix3_inverse(rasterizer->transform, &rasterizer->fillData.inv);
     }
   else
     {
        eina_matrix3_identity(&rasterizer->fillData.inv);
        eina_matrix3_identity(&rasterizer->fillData.inv);
     }
}

void ector_software_rasterizer_transform_set(Software_Rasterizer *rasterizer, Eina_Matrix3 *t)
{
   rasterizer->transform = t;
}

void ector_software_rasterizer_clip_rect_set(Software_Rasterizer *rasterizer, Eina_Array *clips)
{
   if (clips)
     {
        rasterizer->fillData.clip.clips = clips;
        rasterizer->fillData.clip.hasRectClip = EINA_TRUE;
        rasterizer->fillData.clip.enabled = EINA_TRUE;
     }
   else
     {
        rasterizer->fillData.clip.clips = NULL;
        rasterizer->fillData.clip.hasRectClip = EINA_FALSE;
        rasterizer->fillData.clip.enabled = EINA_FALSE;
     }
}

void ector_software_rasterizer_clip_shape_set(Software_Rasterizer *rasterizer, Shape_Rle_Data *clip)
{
   rasterizer->fillData.clip.path = clip;
   rasterizer->fillData.clip.hasPathClip = EINA_TRUE;
   rasterizer->fillData.clip.enabled = EINA_TRUE;
}

void ector_software_rasterizer_color_set(Software_Rasterizer *rasterizer, int r, int g, int b, int a)
{
   uint color = ECTOR_ARGB_JOIN(a, r, g, b);

   rasterizer->fillData.color = _ector_premultiply(color);
   rasterizer->fillData.type = Solid;
}
void ector_software_rasterizer_linear_gradient_set(Software_Rasterizer *rasterizer, Ector_Renderer_Software_Gradient_Data *linear)
{
   rasterizer->fillData.gradient = linear;
   rasterizer->fillData.type = LinearGradient;
}
void ector_software_rasterizer_radial_gradient_set(Software_Rasterizer *rasterizer, Ector_Renderer_Software_Gradient_Data *radial)
{
   rasterizer->fillData.gradient = radial;
   rasterizer->fillData.type = RadialGradient;
}


void ector_software_rasterizer_draw_rle_data(Software_Rasterizer *rasterizer,
                                             int x, int y, uint mul_col, Ector_Rop op, Shape_Rle_Data* rle)
{
   // check for NULL rle data
   if (!rle) return;

   rasterizer->fillData.offx = x;
   rasterizer->fillData.offy = y;
   rasterizer->fillData.mul_col = mul_col;
   rasterizer->fillData.op = op;

   _setup_span_fill_matrix(rasterizer);
   _adjust_span_fill_methods(&rasterizer->fillData);

   if(rasterizer->fillData.blend)
     rasterizer->fillData.blend(rle->size, rle->spans, &rasterizer->fillData);
}
