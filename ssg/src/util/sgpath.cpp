#include"sgpath.h"
#include<vector>
#include<cassert>
#include"sgdebug.h"

struct SGPathData
{
    RefCount                      ref;
    std::vector<SGPointF>         m_points;
    std::vector<SGPath::Element>  m_elements;
    int                           m_segments;
};

static const struct SGPathData shared_empty = {RefCount(-1),
                                               std::vector<SGPointF>(),
                                               std::vector<SGPath::Element>(),
                                               0};

inline void SGPath::cleanUp(SGPathData *d)
{
    delete d;
}

void SGPath::detach()
{
    if (d->ref.isShared())
        *this = copy();
}

SGPath SGPath::copy() const
{
    SGPath other;

    other.d = new SGPathData;
    other.d->m_points = d->m_points;
    other.d->m_elements = d->m_elements;
    other.d->m_segments = d->m_segments;
    other.d->ref.setOwned();
    return other;
}

SGPath::~SGPath()
{
    if (!d->ref.deref())
        cleanUp(d);
}

SGPath::SGPath()
    : d(const_cast<SGPathData*>(&shared_empty))
{
}

SGPath::SGPath(const SGPath &other)
{
    d = other.d;
    d->ref.ref();
}

SGPath::SGPath(SGPath &&other): d(other.d)
{
    other.d = const_cast<SGPathData*>(&shared_empty);
}

SGPath &SGPath::operator=(const SGPath &other)
{
    other.d->ref.ref();
    if (!d->ref.deref())
        cleanUp(d);

    d = other.d;
    return *this;
}

inline SGPath &SGPath::operator=(SGPath &&other)
{
    std::swap(d, other.d); return *this;
}

bool SGPath::isEmpty()const
{
    return d->m_elements.empty();
}

void SGPath::close()
{
    if (isEmpty()) return;
    detach();
    d->m_elements.push_back(Element::Close);
    d->m_segments++;
}

void SGPath::moveTo(const SGPointF &p)
{
    detach();
    d->m_elements.push_back(Element::MoveTo);
    d->m_points.push_back(p);
}

void SGPath::lineTo(const SGPointF &p)
{
    assert(!isEmpty());
    detach();
    d->m_elements.push_back(Element::LineTo);
    d->m_points.push_back(p);
}

void SGPath::cubicTo(const SGPointF &ctrlPt1, const SGPointF &ctrlPt2, const SGPointF &endPt)
{
    assert(!isEmpty());
    detach();
    d->m_elements.push_back(Element::CubicTo);
    d->m_points.push_back(ctrlPt1);
    d->m_points.push_back(ctrlPt2);
    d->m_points.push_back(endPt);
}

void SGPath::reserve(int num_elm)
{
    detach();
    d->m_elements.reserve(num_elm);
    d->m_points.reserve(num_elm);
}

const std::vector<SGPath::Element> &SGPath::elements() const
{
    return d->m_elements;
}
const std::vector<SGPointF> &SGPath::points() const
{
    return d->m_points;
}
int SGPath::segments() const
{
    return d->m_segments + 1;
}



