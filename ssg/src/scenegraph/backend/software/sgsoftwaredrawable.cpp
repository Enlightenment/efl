#include"sgsoftwaredrawable.h"
#include"sgglobal.h"
#include"sgmatrix.h"
#include"sgsoftwarerectnode.h"

SGSoftwareDrawable::SGSoftwareDrawable(NodeType type, SGNode *node)
    : m_nodeType(type)
    , m_isOpaque(true)
    , m_isDirty(true)
    , m_hasClipRegion(false)
    , m_opacity(1.0f)
{
    switch (m_nodeType) {
    case SGSoftwareDrawable::NodeType::Rect:
        m_handle.rectNode = static_cast<SGSoftwareRectNode*>(node);
        break;
    default:
        break;
    }
}

SGSoftwareDrawable::~SGSoftwareDrawable()
{

}

bool SGSoftwareDrawable::isDirtyRegionEmpty() const
{
    return m_dirtyRegion.isEmpty();
}

void SGSoftwareDrawable::setTransform(const SGMatrix &transform)
{
    if (m_transform == transform)
        return;
    m_transform = transform;
    update();
}

void SGSoftwareDrawable::setClipRegion(const SGRegion &clipRect, bool hasClipRegion)
{
    if (m_clipRegion == clipRect && m_hasClipRegion == hasClipRegion)
        return;

    m_clipRegion = clipRect;
    m_hasClipRegion = hasClipRegion;
    update();
}

void SGSoftwareDrawable::setOpacity(float opacity)
{
    if (sgCompare(m_opacity, opacity))
        return;

    m_opacity = opacity;
    update();
}


void SGSoftwareDrawable::addDirtyRegion(const SGRegion &dirtyRegion, bool forceDirty)
{
    // Check if the dirty region applies to this node
    if (dirtyRegion.intersects(m_boundingRect)) {
        if (forceDirty)
            m_isDirty = true;
        m_dirtyRegion += dirtyRegion.intersected(m_boundingRect);
    }
}

void SGSoftwareDrawable::subtractDirtyRegion(const SGRegion &dirtyRegion)
{
    if (m_isDirty) {
        // Check if this rect concerns us
        if (dirtyRegion.intersects(m_boundingRect)) {
            m_dirtyRegion -= dirtyRegion;
            if (m_dirtyRegion.isEmpty())
                m_isDirty = false;
        }
    }
}

SGRegion SGSoftwareDrawable::previousDirtyRegion(bool wasRemoved) const
{
    // When removing a node, the boundingRect shouldn't be subtracted
    // because a deleted node has no valid boundingRect
    if (wasRemoved)
        return m_previousDirtyRegion;

    return m_previousDirtyRegion.subtracted(SGRegion(m_boundingRect));
}

SGRegion SGSoftwareDrawable::dirtyRegion() const
{
    return m_dirtyRegion;
}

void SGSoftwareDrawable::update()
{
    // Update the Node properties
    m_isDirty = true;

    SGRect boundingRect;

    switch (m_nodeType) {
    case SGSoftwareDrawable::NodeType::Rect:
//        if (m_handle.simpleRectNode->color().alpha() == 255 && !m_transform.isRotating())
//            m_isOpaque = true;
//        else
//            m_isOpaque = false;

//        boundingRect = m_handle.simpleRectNode->rect();
        break;
    default:
        break;
    }

    const SGRect transformedRect = m_transform.map(boundingRect);
    //m_boundingRect = toRectMax(transformedRect);

    if (m_hasClipRegion && m_clipRegion.rectCount() <= 1) {
        // If there is a clipRegion, and it is empty, the item wont be rendered
        if (m_clipRegion.isEmpty()) {
            m_boundingRect = SGRect();
        } else {
//            m_boundingRectMin = m_boundingRectMin.intersected(m_clipRegion.rects().constFirst());
//            m_boundingRectMax = m_boundingRectMax.intersected(m_clipRegion.rects().constFirst());
        }
    }

    // Overrides
    if (m_opacity < 1.0f)
        m_isOpaque = false;

    m_dirtyRegion = SGRegion(m_boundingRect);
}
