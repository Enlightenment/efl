#include"ftraster_p.h"
#include"sw_ft_raster.h"
#include"sw_ft_stroker.h"
#include"sgpath.h"
#include"sgmatrix.h"
#include<cstring>
#include"sgdebug.h"

struct FTOutline
{
public:
    FTOutline() = delete;
    FTOutline(int points, int segments)
    {
        ft.points = new SW_FT_Vector[points + segments];
        ft.tags   = new char[points + segments];
        ft.contours = new short[segments];
        ft.n_points = ft.n_contours = 0;
    }
    void moveTo(const SGPointF &pt);
    void lineTo(const SGPointF &pt);
    void cubicTo(const SGPointF &ctr1, const SGPointF &ctr2, const SGPointF end);
    void close();
    void end();
    void transform(const SGMatrix &m);
    ~FTOutline()
    {
        delete[] ft.points;
        delete[] ft.tags;
        delete[] ft.contours;
    }
    SW_FT_Outline  ft;
    bool           closed;
};

#define TO_FT_COORD(x) ((x) * 64) // to freetype 26.6 coordinate.

void FTOutline::transform(const SGMatrix &m)
{
    SGPointF pt;
    if (m.isIdentity()) return;
    for (auto i = 0; i < ft.n_points; i++) {
        pt = m.map(SGPointF(ft.points[i].x/64.0, ft.points[i].y/64.0));
        ft.points[i].x = TO_FT_COORD(pt.x());
        ft.points[i].y = TO_FT_COORD(pt.y());
    }
}

void FTOutline::moveTo(const SGPointF &pt)
{
    ft.points[ft.n_points].x = TO_FT_COORD(pt.x());
    ft.points[ft.n_points].y = TO_FT_COORD(pt.y());
    ft.tags[ft.n_points] = SW_FT_CURVE_TAG_ON;
    if (ft.n_points) {
        ft.contours[ft.n_contours] = ft.n_points - 1;
        ft.n_contours++;
    }
    ft.n_points++;
    closed = false;
}

void FTOutline::lineTo(const SGPointF &pt)
{
    ft.points[ft.n_points].x = TO_FT_COORD(pt.x());
    ft.points[ft.n_points].y = TO_FT_COORD(pt.y());
    ft.tags[ft.n_points] = SW_FT_CURVE_TAG_ON;
    ft.n_points++;
    closed = false;
}

void FTOutline::cubicTo(const SGPointF &cp1, const SGPointF &cp2, const SGPointF ep)
{
    ft.points[ft.n_points].x = TO_FT_COORD(cp1.x());
    ft.points[ft.n_points].y = TO_FT_COORD(cp1.y());
    ft.tags[ft.n_points] = SW_FT_CURVE_TAG_CUBIC;
    ft.n_points++;

    ft.points[ft.n_points].x = TO_FT_COORD(cp2.x());
    ft.points[ft.n_points].y = TO_FT_COORD(cp2.y());
    ft.tags[ft.n_points] = SW_FT_CURVE_TAG_CUBIC;
    ft.n_points++;

    ft.points[ft.n_points].x = TO_FT_COORD(ep.x());
    ft.points[ft.n_points].y = TO_FT_COORD(ep.y());
    ft.tags[ft.n_points] = SW_FT_CURVE_TAG_ON;
    ft.n_points++;
    closed = false;
}
void FTOutline::close()
{
    int index;
    if (ft.n_contours) {
        index = ft.contours[ft.n_contours - 1] + 1;
    } else {
        index = 0;
    }

    // make sure atleast 1 point exists in the segment.
    if (ft.n_points == index) {
        closed = false;
        return;
    }

    ft.points[ft.n_points].x = ft.points[index].x;
    ft.points[ft.n_points].y = ft.points[index].y;
    ft.tags[ft.n_points] = SW_FT_CURVE_TAG_ON;
    ft.n_points++;
    closed = true;
}

void FTOutline::end()
{
    if (ft.n_points) {
        ft.contours[ft.n_contours] = ft.n_points - 1;
        ft.n_contours++;
    }
}

struct FTRasterPrivate
{
public:
    SGRle generateFillInfoAsync(const SW_FT_Outline *outline);
    SGRle generateStrokeInfoAsync(const SW_FT_Outline *outline, SW_FT_Stroker_LineCap cap,
                                     SW_FT_Stroker_LineJoin join, int width, SW_FT_Bool closed);

    std::mutex        m_rasterAcess;
    std::mutex        m_strokerAcess;
    SW_FT_Raster      m_raster;
    SW_FT_Stroker     m_stroker;
};

struct SpanInfo
{
  SGRle::Span *spans;
  int          size;
};

static void
rleGenerationCb( int count, const SW_FT_Span*  spans,void *user)
{
   SGRle *rle = (SGRle *) user;
   SGRle::Span *rleSpan = (SGRle::Span *)spans;
   rle->addSpan(rleSpan, count);
}

SGRle FTRasterPrivate::generateFillInfoAsync(const SW_FT_Outline *outline)
{
    m_rasterAcess.lock();
    SGRle rle;
    SW_FT_Raster_Params params;

    params.flags = SW_FT_RASTER_FLAG_DIRECT | SW_FT_RASTER_FLAG_AA ;
    params.gray_spans = &rleGenerationCb;
    params.user = &rle;
    params.source = outline;

    sw_ft_grays_raster.raster_render(m_raster, &params);

    m_rasterAcess.unlock();

    return rle;
}

SGRle FTRasterPrivate::generateStrokeInfoAsync(const SW_FT_Outline *outline, SW_FT_Stroker_LineCap cap,
                                                  SW_FT_Stroker_LineJoin join, int width, SW_FT_Bool closed)
{
    m_strokerAcess.lock();
    uint points,contors;
    SW_FT_Outline strokeOutline = { 0, 0, nullptr, nullptr, nullptr, 0 };

    SW_FT_Stroker_Set(m_stroker, width, cap, join, 0);
    SW_FT_Stroker_ParseOutline(m_stroker, outline, !closed);
    SW_FT_Stroker_GetCounts(m_stroker,&points, &contors);

    strokeOutline.points = (SW_FT_Vector *) calloc(points, sizeof(SW_FT_Vector));
    strokeOutline.tags = (char *) calloc(points, sizeof(char));
    strokeOutline.contours = (short *) calloc(contors, sizeof(short));

    SW_FT_Stroker_Export(m_stroker, &strokeOutline);

    m_strokerAcess.unlock();

    SGRle rle = generateFillInfoAsync(&strokeOutline);

    // cleanup the outline data.
    free(strokeOutline.points);
    free(strokeOutline.tags);
    free(strokeOutline.contours);

    return rle;
}


FTRaster::FTRaster()
{
    d = new FTRasterPrivate;
    sw_ft_grays_raster.raster_new(&d->m_raster);
    SW_FT_Stroker_New(&d->m_stroker);
    SW_FT_Stroker_Set(d->m_stroker, 1 << 6,
                      SW_FT_STROKER_LINECAP_BUTT, SW_FT_STROKER_LINEJOIN_MITER, 0);
}

FTRaster::~FTRaster()
{
    sw_ft_grays_raster.raster_done(d->m_raster);
    SW_FT_Stroker_Done(d->m_stroker);
}

void FTRaster::deleteFTOutline(FTOutline *outline)
{
    delete outline;
}

FTOutline *FTRaster::toFTOutline(const SGPath &path)
{
    if (path.isEmpty())
        return nullptr;

    const std::vector<SGPath::Element> &elements = path.elements();
    const std::vector<SGPointF> &points = path.points();

    FTOutline *outline = new FTOutline(points.size(), path.segments());

    int index = 0;
    for(auto element : elements) {
        switch (element){
        case SGPath::Element::MoveTo:
            outline->moveTo(points[index]);
            index++;
            break;
        case SGPath::Element::LineTo:
            outline->lineTo(points[index]);
            index++;
            break;
        case SGPath::Element::CubicTo:
            outline->cubicTo(points[index], points[index+1], points[index+2]);
            index = index+3;
            break;
        case SGPath::Element::Close:
            outline->close();
            break;
        default:
            break;
        }
    }
    outline->end();
    return outline;
}

std::future<SGRle> FTRaster::generateFillInfo(const FTOutline *outline)
{
    return std::async(std::launch::async, &FTRasterPrivate::generateFillInfoAsync, d, &outline->ft);
}

std::future<SGRle> FTRaster::generateStrokeInfo(const FTOutline *outline, CapStyle cap, JoinStyle join, double width)
{
    SW_FT_Stroker_LineCap ftCap;
    SW_FT_Stroker_LineJoin ftJoin;
    int ftWidth;
    SW_FT_Bool ftbool = (SW_FT_Bool) outline->closed;

    // map strokeWidth to freetype. It uses as the radius of the pen not the diameter
    width = width/2.0;
    // convert to freetype co-ordinate
    ftWidth = int(width * 64);

    // map to freetype capstyle
    switch (cap)
      {
         case CapStyle::Square:
           ftCap = SW_FT_STROKER_LINECAP_SQUARE;
           break;
         case CapStyle::Round:
           ftCap = SW_FT_STROKER_LINECAP_ROUND;
           break;
         default:
           ftCap = SW_FT_STROKER_LINECAP_BUTT;
           break;
      }
    switch (join)
      {
         case JoinStyle::Bevel:
           ftJoin = SW_FT_STROKER_LINEJOIN_BEVEL;
           break;
         case JoinStyle::Round:
           ftJoin = SW_FT_STROKER_LINEJOIN_ROUND;
           break;
         default:
           ftJoin = SW_FT_STROKER_LINEJOIN_MITER;
           break;
      }

    return std::async(std::launch::async, &FTRasterPrivate::generateStrokeInfoAsync, d,
                      &outline->ft, ftCap, ftJoin, ftWidth, ftbool);
}
