#include "sgrle.h"
#include"sgglobal.h"
#include<sgrect.h>
#include<cstdlib>
#include<vector>
#include<algorithm>
#include<ostream>

struct SGRleHelper
{
   ushort        alloc;
   ushort        size;
   SGRle::Span  *spans;
};

#define SGMIN(a,b) ((a) < (b) ? (a) : (b))
#define SGMAX(a,b) ((a) > (b) ? (a) : (b))

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
rleIntersectWithRle(SGRleHelper *tmp_clip,
                    int clip_offset_x,
                    int clip_offset_y,
                    SGRleHelper *tmp_obj,
                    SGRleHelper *result)
{
    SGRle::Span *out = result->spans;
    int available = result->alloc;
    SGRle::Span *spans = tmp_obj->spans;
    SGRle::Span *end = tmp_obj->spans + tmp_obj->size;
    SGRle::Span *clipSpans = tmp_clip->spans;
    SGRle::Span *clipEnd = tmp_clip->spans + tmp_clip->size;
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
        x = SGMAX(sx1, cx1);
        len = SGMIN(sx2, cx2) - x;
        if (len)
          {
             out->x = SGMAX(sx1, cx1);
             out->len = ( SGMIN(sx2, cx2) - out->x);
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
rleIntersectWithRect(const SGRect &clip,
                     SGRleHelper *tmp_obj,
                     SGRleHelper *result)
{
   SGRle::Span *out = result->spans;
   int available = result->alloc;
   SGRle::Span *spans = tmp_obj->spans;
   SGRle::Span *end = tmp_obj->spans + tmp_obj->size;
   short minx, miny, maxx, maxy;

   minx = clip.left();
   miny = clip.top();
   maxx = clip.right() - 1;
   maxy = clip.bottom() - 1;

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
             out->len = SGMIN(spans->len - (minx - spans->x), maxx - minx + 1);
             out->x = minx;
          }
        else
          {
             out->x = spans->x;
             out->len = SGMIN(spans->len, (maxx - spans->x + 1));
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


class SGRleImpl
{
public:
    inline SGRleImpl():m_bbox(),m_spans(){}
    SGRleImpl &operator=(const SGRleImpl &);
    void addSpan(const SGRle::Span *span, int count);
    void updateBbox();
    bool operator ==(const SGRleImpl &) const;
    void intersected(const SGRect &r, SGRleImpl &result);
    friend SGDebug& operator<<(SGDebug& os, const SGRleImpl& object);
public:
    SGRect                   m_bbox;
    std::vector<SGRle::Span> m_spans;// array of Spanlines.
    SGPoint                  m_offset;
};

inline static void
copyArrayToVector(const SGRle::Span *span, int count, std::vector<SGRle::Span> &v)
{
    // make sure enough memory available
    v.reserve(v.size() + count);
    std::copy(span, span + count, back_inserter(v));
}

SGDebug& operator<<(SGDebug& os, const SGRleImpl& o)
{
    os<<"[bbox="<< o.m_bbox<<"]"<<"[offset="<<o.m_offset<<"]"<<
        "[span count ="<<o.m_spans.size()<<"]";
    os<<"[rle spans = {x y len coverage}";
    for(auto sp : o.m_spans)
        os<<"{"<<sp.x<<" "<<sp.y<<" "<<sp.len<<" "<<sp.coverage<<"}";
    os<<"]";
    return os;
}


const int SPAN_BUF_SIZE = 256;

void SGRleImpl::intersected(const SGRect &r, SGRleImpl &result)
{
    SGRect clip = r.translated(-m_offset.x(), -m_offset.y());

    std::array<SGRle::Span, 256> arrayResult;
    SGRleHelper tresult, tmp_obj;
    SGRle::Span SPAN_BUF[SPAN_BUF_SIZE];

    //setup the tresult object
    tresult.size = SPAN_BUF_SIZE;
    tresult.alloc = SPAN_BUF_SIZE;
    tresult.spans = SPAN_BUF;

    // setup tmp object
    tmp_obj.size = m_spans.size();
    tmp_obj.spans = m_spans.data();

    // run till all the spans are processed
    while (tmp_obj.size)
      {
         rleIntersectWithRect(clip, &tmp_obj, &tresult);
         if (tresult.size) {
             copyArrayToVector(tresult.spans, tresult.size, result.m_spans);
         }
         tresult.size = 0;
      }
    result.updateBbox();
}


SGRleImpl &SGRleImpl::operator=(const SGRleImpl &other)
{
    m_spans = other.m_spans;
    m_bbox = other.m_bbox;
    return *this;
}

bool SGRleImpl::operator ==(const SGRleImpl &other) const
{
    if (m_offset != other.m_offset)
        return false;

    if (m_spans.size() != other.m_spans.size())
        return false;
    const SGRle::Span *spans = m_spans.data();
    const SGRle::Span *o_spans = other.m_spans.data();
    int sz = m_spans.size();

    for (int i = 0; i < sz; i++) {
        if (spans[i].x != o_spans[i].x ||
            spans[i].y != o_spans[i].y ||
            spans[i].len != o_spans[i].len ||
            spans[i].coverage != o_spans[i].coverage)
            return false;
    }

    return true;

}


void SGRleImpl::updateBbox()
{
    int i, l = 0, t = 0, r = 0, b = 0, sz;
    l = std::numeric_limits<int>::max();
    const SGRle::Span *span = m_spans.data();

    m_bbox = SGRect();
    sz = m_spans.size();
    if (sz)
      {
         t = span[0].y;
         b = span[sz-1].y;
         for (i = 0; i < sz; i++)
           {
              if (span[i].x < l) l = span[i].x;
              if (span[i].x + span[i].len > r) r = span[i].x + span[i].len;
           }
         m_bbox = SGRect(l, t, r - l, b - t + 1);
      }
}

void SGRleImpl::addSpan(const SGRle::Span *span, int count)
{
    copyArrayToVector(span, count, m_spans);
    updateBbox();
}

struct SGRleData
{
    RefCount    ref;
    SGRleImpl   impl;
};

static const struct SGRleData shared_empty = {RefCount(-1),
                                              SGRleImpl()};

inline void SGRle::cleanUp(SGRleData *d)
{
    delete d;
}

void SGRle::detach()
{
    if (d->ref.isShared())
        *this = copy();
}

SGRle SGRle::copy() const
{
    SGRle other;

    other.d = new SGRleData;
    other.d->impl = d->impl;
    other.d->ref.setOwned();
    return other;
}

SGRle::~SGRle()
{
    if (!d->ref.deref())
        cleanUp(d);
}

SGRle::SGRle()
    : d(const_cast<SGRleData*>(&shared_empty))
{
}

SGRle::SGRle(const SGRle &other)
{
    d = other.d;
    d->ref.ref();
}

SGRle::SGRle(SGRle &&other): d(other.d)
{
    other.d = const_cast<SGRleData*>(&shared_empty);
}

SGRle &SGRle::operator=(const SGRle &other)
{
    other.d->ref.ref();
    if (!d->ref.deref())
        cleanUp(d);

    d = other.d;
    return *this;
}

inline SGRle &SGRle::operator=(SGRle &&other)
{
    std::swap(d, other.d); return *this;
}

bool SGRle::isEmpty()const
{
    return (d == &shared_empty || d->impl.m_spans.empty());
}

void SGRle::addSpan(const SGRle::Span *span, int count)
{
    detach();
    d->impl.addSpan(span, count);
}

SGRect SGRle::boundingRect() const
{
    if(isEmpty())
        return SGRect();
    return d->impl.m_bbox;
}

bool SGRle::operator ==(const SGRle &other) const
{
    if (isEmpty())
        return other.isEmpty();
    if (other.isEmpty())
        return isEmpty();

    if (d == other.d)
        return true;
    else
        return d->impl == other.d->impl;
}

void SGRle::translate(const SGPoint &p)
{
    detach();
    d->impl.m_offset = p;
}

SGRle SGRle::intersected(const SGRect &r) const
{
    if (isEmpty() || r.isEmpty())
        return SGRle();

    SGRle result;
    result.detach();
    d->impl.intersected(r, result.d->impl);
    return std::move(result);
}

SGRle  &SGRle::intersect(const SGRect &r)
{
    if (isEmpty() || r.isEmpty())
        return *this = SGRle();

    SGRle result;
    result.detach();
    d->impl.intersected(r, result.d->impl);
    return *this = result;
}

SGDebug& operator<<(SGDebug& os, const SGRle& o)
{
    os<<"[RLE: [dptr = "<<"]"<<"[ref = "<<o.d->ref.count()<<"]"<<o.d->impl<<"]";
    return os;
}



