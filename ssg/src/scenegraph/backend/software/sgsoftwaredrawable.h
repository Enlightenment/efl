#ifndef SGSOFTWAREDRAWABLE_H
#define SGSOFTWAREDRAWABLE_H

#include"sgnode.h"

class SGRectNode;

class SGSoftwareDrawable
{
public:
    enum NodeType {
        Invalid = -1,
        Rect,
        Shape
    };

    SGSoftwareDrawable(NodeType type, SGNode *node);
    ~SGSoftwareDrawable();

    void update();

    //SGRegion renderNode(QPainter *painter, bool forceOpaquePainting = false);
    SGRect boundingRectMin() const { return m_boundingRect; }
    SGRect boundingRectMax() const { return m_boundingRect; }
    NodeType type() const { return m_nodeType; }
    bool isOpaque() const { return m_isOpaque; }
    bool isDirty() const { return m_isDirty; }
    bool isDirtyRegionEmpty() const;

    void setTransform(const SGMatrix &transform);
    void setClipRegion(const SGRegion &clipRegion, bool hasClipRegion = true);
    void setOpacity(float opacity);
    SGMatrix transform() const { return m_transform; }
    SGRegion clipRegion() const { return m_clipRegion; }
    float opacity() const { return m_opacity; }

//    void markGeometryDirty();
//    void markMaterialDirty();

    void addDirtyRegion(const SGRegion &dirtyRegion, bool forceDirty = true);
    void subtractDirtyRegion(const SGRegion &dirtyRegion);

    SGRegion previousDirtyRegion(bool wasRemoved = false) const;
    SGRegion dirtyRegion() const;

private:
    union RenderableNodeHandle {
        SGRectNode *rectNode;
    };

    const NodeType m_nodeType;
    RenderableNodeHandle m_handle;

    bool m_isOpaque;

    bool m_isDirty;
    SGRegion m_dirtyRegion;
    SGRegion m_previousDirtyRegion;

    SGMatrix m_transform;
    SGRegion m_clipRegion;
    bool m_hasClipRegion;
    float m_opacity;

    SGRect m_boundingRect;
};

#endif  //SGSOFTWAREDRAWABLE_H
