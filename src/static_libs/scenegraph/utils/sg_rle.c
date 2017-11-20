#include"sg_rle.h"

typedef struct _SG_Rle_Impl
{
   Eina_Rect        bbox;
   unsigned short   alloc;
   unsigned short   size;
   SG_Span         *spans;// array of Scanlines.
}SG_Rle_Impl;

static SG_Rle_Impl EMPTY_RLE_IMPL = {{}, 0, 0, 0};

static void
rle_impl_update_bounding_box(SG_Rle_Impl *impl)
{
    int i, l = INT_MAX, t = 0, r = 0, b = 0, sz;
    SG_Span *span = impl->spans;

    impl->bbox = EINA_RECT(0,0,0,0);
    sz = impl->size;
    if (sz)
      {
         t = span[0].y;
         b = span[sz-1].y;
         for (i = 0; i < sz; i++)
           {
              if (span[i].x < l) l = span[i].x;
              if (span[i].x + span[i].len > r) r = span[i].x + span[i].len;
           }
         impl->bbox.x = l;
         impl->bbox.y = t;
         impl->bbox.w = r - l;
         impl->bbox.h = b - t + 1;
      }
}

static inline int
_div_255(int x) { return (x + (x>>8) + 0x80) >> 8; }

/*
 * This function will clip a rle list with another rle object
 * tmp_clip  : The rle list that will be use to clip the rle
 * tmp_obj   : holds the list of spans that has to be clipped
 * result    : will hold the result after the processing
 * NOTE: if the algorithm runs out of the result buffer list
 *       it will stop and update the tmp_obj with the span list
 *       that are yet to be processed as well as the tpm_clip object
 *       with the unprocessed clip spans.
 */
static void
rle_impl_intersect_with_rle(SG_Rle_Impl *tmp_clip,
                            int clip_offset_x,
                            int clip_offset_y,
                            SG_Rle_Impl *tmp_obj,
                            SG_Rle_Impl *result)
{
    SG_Span *out = result->spans;
    int available = result->alloc;
    SG_Span *spans = tmp_obj->spans;
    SG_Span *end = tmp_obj->spans + tmp_obj->size;
    SG_Span *clipSpans = tmp_clip->spans;
    SG_Span *clipEnd = tmp_clip->spans + tmp_clip->size;
    int sx1, sx2, cx1, cx2, x, len;


   while (available && spans < end )
     {
        if (clipSpans >= clipEnd)
          {
             spans = end;
             break;
          }
        if ((clipSpans->y + clip_offset_y) > spans->y)
          {
             ++spans;
             continue;
          }
        if (spans->y != (clipSpans->y + clip_offset_y))
          {
             ++clipSpans;
             continue;
          }
        //assert(spans->y == (clipSpans->y + clip_offset_y));
        sx1 = spans->x;
        sx2 = sx1 + spans->len;
        cx1 = (clipSpans->x + clip_offset_x);
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
             out->len = ( MIN(sx2, cx2) - out->x);
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

   // update the span list that yet to be processed
   tmp_obj->spans = spans;
   tmp_obj->size = end - spans;

   // update the clip list that yet to be processed
   tmp_clip->spans = clipSpans;
   tmp_clip->size = clipEnd - clipSpans;

   // update the result
   result->size = result->alloc - available;
}


/*
 * This function will clip a rle list with a given rect
 * clip      : The clip rect that will be use to clip the rle
 * tmp_obj   : holds the list of spans that has to be clipped
 * result    : will hold the result after the processing
 * NOTE: if the algorithm runs out of the result buffer list
 *       it will stop and update the tmp_obj with the span list
 *       that are yet to be processed
 */
static void
rle_impl_intersect_with_rect(const Eina_Rect clip,
                             SG_Rle_Impl *tmp_obj,
                             SG_Rle_Impl *result)
{
   SG_Span *out = result->spans;
   int available = result->alloc;
   SG_Span *spans = tmp_obj->spans;
   SG_Span *end = tmp_obj->spans + tmp_obj->size;
   short minx, miny, maxx, maxy;

   minx = clip.x;
   miny = clip.y;
   maxx = minx + clip.w - 1;
   maxy = miny + clip.h - 1;

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

   // update the span list that yet to be processed
   tmp_obj->spans = spans;
   tmp_obj->size = end - spans;

   // update the result
   result->size = result->alloc - available;
}

const int SPAN_BUF_SIZE = 256;

static void
rle_impl_intersected_rect(const SG_Rle_Impl *obj,
                          const Eina_Rect r,
                          const SG_Span_Cb cb,
                          void *user_data)
{
    SG_Rle_Impl result, tmp_obj;
    SG_Span SPAN_BUF[SPAN_BUF_SIZE];

    if (!obj || !obj->size || !cb) return;

    //setup the result object
    result.size = SPAN_BUF_SIZE;
    result.alloc = SPAN_BUF_SIZE;
    result.spans = SPAN_BUF;
    // setup tmp object
    tmp_obj.size = obj->size;
    tmp_obj.spans = obj->spans;

    // run till all the spans are processed
    while (tmp_obj.size)
      {
         rle_impl_intersect_with_rect(r, &tmp_obj, &result);
         if (result.size)
           cb(result.spans, result.size, user_data);
         result.size = 0;
      }
}

static void
rle_impl_intersect_rect_inplace(SG_Rle_Impl *obj,
                                const Eina_Rect r)
{
    SG_Rle_Impl result, tmp_obj;
    int final_size = 0;

    if (!obj || !obj->size) return;

    //setup the result object as the obj
    result.size = obj->size;
    result.alloc = obj->alloc;
    result.spans = obj->spans;
    // setup tmp object
    tmp_obj.size = obj->size;
    tmp_obj.spans = obj->spans;

    // run till all the spans are processed
    while (tmp_obj.size)
      {
         rle_impl_intersect_with_rect(r, &tmp_obj, &result);
         if (result.size)
             final_size += result.size;
         result.size = 0;
      }

    obj->size = final_size;
    rle_impl_update_bounding_box(obj);
}

static void
rle_impl_intersected_rle(const SG_Rle_Impl *obj,
                         const SG_Rle_Impl *clip,
                         int clip_offset_x,
                         int clip_offset_y,
                         const SG_Span_Cb cb,
                         void *user_data)
{
    SG_Rle_Impl result, tmp_obj, tmp_clip;
    SG_Span SPAN_BUF[SPAN_BUF_SIZE];

    if (!obj || !obj->size || !cb) return;

    //setup the result object
    result.size = SPAN_BUF_SIZE;
    result.alloc = SPAN_BUF_SIZE;
    result.spans = SPAN_BUF;
    // setup tmp object
    tmp_obj.size = obj->size;
    tmp_obj.spans = obj->spans;

    // setup tmp clip
    tmp_clip.size = clip->size;
    tmp_clip.spans = clip->spans;

    // run till all the spans are processed
    while (tmp_obj.size)
      {
         rle_impl_intersect_with_rle(&tmp_clip, clip_offset_x, clip_offset_y, &tmp_obj, &result);
         if (result.size)
           cb(result.spans, result.size, user_data);
         result.size = 0;
      }
}

static SG_Rle_Impl *
rle_impl_copy(SG_Rle_Impl *rle)
{
    SG_Rle_Impl *copy_impl = calloc(1, sizeof(SG_Rle_Impl));
    if (rle->size)
    {
        copy_impl->spans = malloc(sizeof(SG_Span) * rle->size);
        memcpy(copy_impl->spans, rle->spans, sizeof(SG_Span) * rle->size);

        copy_impl->alloc = copy_impl->size = rle->size;
        copy_impl->bbox = rle->bbox;
    }
    return copy_impl;
}

static void
rle_impl_free(SG_Rle_Impl *rle)
{
    if (!rle) return;

    if (rle->size)
      free(rle->spans);

    free(rle);
}

static void
rle_impl_append_spans(SG_Rle_Impl *rle, const SG_Span *spans, int count)
{
    if(!rle->spans)
    {
        rle->spans = malloc(sizeof(SG_Span) * count);
        memcpy(rle->spans, spans, sizeof(SG_Span) * count);
        rle->alloc = rle->size = count;
    }
    else
    {
        int new_size = rle->size + count;
        rle->spans = (SG_Span *) realloc(rle->spans, new_size * sizeof(SG_Span));
        // copy the new spans to the allocated memory
        SG_Span *last_span = (rle->spans + rle->size);
        memcpy(last_span,spans, count * sizeof(SG_Span));
        //update size
        rle->alloc = rle->size = new_size;
    }
    rle_impl_update_bounding_box(rle);
}

/* SG_Rle_Impl class End  */

/* SG_Rle class Start  */

inline static SG_Rle rle_copy(const SG_Rle *obj);
static void detach(SG_Rle *obj);

struct _SG_Rle_Data
{
    int               ref;
    SG_Rle_Impl      *impl;
};

inline static SG_Rle
rle_ref(SG_Rle *obj)
{
    obj->data->ref++;
    return *obj;
}

inline static Eina_Bool
rle_empty(const SG_Rle *obj)
{
    if (!obj->data->impl || !obj->data->impl->size)
        return EINA_TRUE;
    return EINA_FALSE;
}

inline static void
rle_clear(SG_Rle *obj)
{
    sg_rle_free(obj);
    *obj = sg_rle_new();
}

static void
rle_translate(SG_Rle *obj, int x, int y)
{
    obj->x = x;
    obj->y = y;
}

static void
rle_intersected_rect(const SG_Rle *obj, Eina_Rect r, SG_Span_Cb cb, void *user_data)
{
    /* take care of the translation matrix applied to the RLE data
       either we can apply the translation to the RLE data or apply
       the inverse translation to the clip rect, hence applying inverse
       to the clip rect.
     */
    r.x = r.x - obj->x;
    r.x = r.y - obj->y;
    rle_impl_intersected_rect(obj->data->impl, r, cb, user_data);
}

static void
rle_intersected_rle(const SG_Rle *obj, const SG_Rle clip, SG_Span_Cb cb, void *user_data)
{
    int offx, offy;
    /* take care of the translation matrix applied to the RLE data
     * we will apply the translation to the clip rle object
     * effective offset = clip_offset - obj_offset
     */
    offx = clip.x - obj->x;
    offy = clip.y - obj->y;
    rle_impl_intersected_rle(obj->data->impl, clip.data->impl, offx, offy, cb, user_data);
}

static void
rle_intersect_rect(SG_Rle *obj, Eina_Rect r)
{
   detach(obj);
   r.x = r.x - obj->x;
   r.y = r.y - obj->y;
   rle_impl_intersect_rect_inplace(obj->data->impl, r);
   /* now the rle data is updated with the translation
    * so reset the translation value
    */
   obj->x = 0;
   obj->y = 0;
}

static Eina_Rect
rle_bounding_rect(const SG_Rle *obj)
{
    if (obj->data->impl)
        return obj->data->impl->bbox;
    return EINA_RECT(0, 0, 0, 0);

}

static void
rle_append_spans(SG_Rle *obj, const SG_Span *spans, int count)
{
    detach(obj);
    rle_impl_append_spans(obj->data->impl, spans, count);
}


static SG_Rle_Data EMPTY_RLE_DATA = {2, &EMPTY_RLE_IMPL};
static SG_Rle_Func   SG_RLE_VTABLE =
{
    rle_copy,
    rle_ref,
    rle_empty,
    rle_clear,
    rle_translate,
    rle_intersected_rect,
    rle_intersected_rle,
    rle_intersect_rect,
    rle_bounding_rect,
    rle_append_spans
};

static void
detach(SG_Rle *obj)
{
    if ( (obj->data->ref != 1) && (obj->data->ref != 0) )
      {
         //unref
         obj->data->ref -= 1;
         // copy
         *obj = rle_copy(obj);
      }
}

inline static SG_Rle
rle_copy(const SG_Rle *obj)
{
    SG_Rle r;

    r.fn = &SG_RLE_VTABLE;
    r.data = calloc(1, sizeof(SG_Rle_Data));
    r.data->ref = 1;
    r.data->impl = rle_impl_copy(obj->data->impl);

    return r;
}

SG_Rle sg_rle_new(void)
{
    SG_Rle obj;

    obj.fn = &SG_RLE_VTABLE;
    obj.data = &EMPTY_RLE_DATA;
    obj.data->ref++;
    return obj;
}

void sg_rle_free(SG_Rle *obj)
{
    obj->data->ref--;
    if (!obj->data->ref)
      {
         rle_impl_free(obj->data->impl);
         free(obj->data);
      }
}

/* SG_Rle class Start  */
