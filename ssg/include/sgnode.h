#ifndef SGNODE_H
#define SGNODE_H

#include<sgglobal.h>
#include<sgregion.h>
#include<sgmatrix.h>
#include<vector>

class SGGeometry;
class SGRect;
class SGRenderer;
class SGNodeVisitor;
class SG_EXPORT SGNode
{
public:
    enum class NodeType {
        Basic,
        Geometry,
        Transform,
        Clip,
        Opacity,
        Root,
    };

    enum class Flag {
        // Lower 16 bites reserved for general node
        OwnedByParent               = 0x0001,
        UsePreprocess               = 0x0002,
        // 0x00ff0000 bits reserved for node subclasses
        OwnsGeometry                = 0x00010000,
        IsVisitableNode             = 0x01000000
    };
    typedef sgFlag<Flag> NodeFlag;
    enum class DirtyState {
        SubtreeBlocked         = 0x0080,
        Matrix                 = 0x0100,
        NodeAdded              = 0x0400,
        NodeRemoved            = 0x0800,
        Geometry               = 0x1000,
        Material               = 0x2000,
        Opacity                = 0x4000,
        ForceUpdate            = 0x8000,
        UsePreprocess          = 0x0002,
        PropagationMask        = Matrix
                               | NodeAdded
                               | Opacity
                               | ForceUpdate

    };
    typedef sgFlag<DirtyState> DirtyFlag;
    SGNode();
    virtual ~SGNode();

    SGNode *parent() const { return m_parent; }

    void removeChildNode(SGNode *node);
    void removeAllChildNodes();
    void prependChildNode(SGNode *node);
    void appendChildNode(SGNode *node);
    void insertChildNodeBefore(SGNode *node, SGNode *before);
    void insertChildNodeAfter(SGNode *node, SGNode *after);
    void reparentChildNodesTo(SGNode *newParent);

    int childCount() const;
    SGNode *childAtIndex(int i) const;
    SGNode *firstChild() const { return m_firstChild; }
    SGNode *lastChild() const { return m_lastChild; }
    SGNode *nextSibling() const { return m_nextSibling; }
    SGNode* previousSibling() const { return m_previousSibling; }

    inline NodeType type() const { return m_type; }

    void markDirty(DirtyFlag flags);

    virtual bool isSubtreeBlocked() const;

    NodeFlag flags() const { return m_nodeFlags; }
    void setFlag(NodeFlag, bool = true);
    void setFlags(NodeFlag, bool = true);

    virtual void preprocess() { }
    virtual void accept(SGNodeVisitor *){}
protected:
    SGNode(NodeType type);
private:
    friend class SGRootNode;
    friend class SGRenderer;

    void init();
    void destroy();

    SGNode *m_parent;
    NodeType m_type;
    SGNode *m_firstChild;
    SGNode *m_lastChild;
    SGNode *m_nextSibling;
    SGNode *m_previousSibling;
    int m_subtreeRenderableCount;

    NodeFlag m_nodeFlags;
};

class SGClipNode;

class SG_EXPORT SGBasicGeometryNode : public SGNode
{
public:
    ~SGBasicGeometryNode();

    void setGeometry(SGGeometry *geometry);
    const SGGeometry *geometry() const { return m_geometry; }
    SGGeometry *geometry() { return m_geometry; }

    const SGMatrix *matrix() const { return m_matrix; }
    const SGClipNode *clipList() const { return m_clip_list; }

    void setRendererMatrix(const SGMatrix *m) { m_matrix = m; }
    void setRendererClipList(const SGClipNode *c) { m_clip_list = c; }

protected:
    SGBasicGeometryNode(NodeType type);

private:
    friend class SGNodeUpdater;

    SGGeometry *m_geometry;

    int m_reserved_start_index;
    int m_reserved_end_index;

    const SGMatrix *m_matrix;
    const SGClipNode *m_clip_list;
};

class SG_EXPORT SGGeometryNode : public SGBasicGeometryNode
{
public:
    SGGeometryNode();
    ~SGGeometryNode();

    void setRenderOrder(int order);
    int renderOrder() const { return m_render_order; }

    void setInheritedOpacity(double opacity);
    double inheritedOpacity() const { return m_opacity; }
    void accept(SGNodeVisitor *visitor) override;
private:
    friend class SGNodeUpdater;

    int m_render_order;

    double m_opacity;
};

class SG_EXPORT SGClipNode : public SGBasicGeometryNode
{
public:
    SGClipNode();
    ~SGClipNode();

    void setIsRectangular(bool rectHint);
    bool isRectangular() const { return m_is_rectangular; }

    void setClipRect(const SGRect &);
    SGRect clipRect() const { return m_clip_rect; }
    void accept(SGNodeVisitor *visitor) override;
private:
    bool     m_is_rectangular;

    SGRect m_clip_rect;
};

class SG_EXPORT SGTransformNode : public SGNode
{
public:
    SGTransformNode();
    ~SGTransformNode();

    void setMatrix(const SGMatrix &matrix);
    const SGMatrix &matrix() const { return m_matrix; }

    void setCombinedMatrix(const SGMatrix &matrix);
    const SGMatrix &combinedMatrix() const { return m_combined_matrix; }
    void accept(SGNodeVisitor *visitor) override;
private:
    SGMatrix m_matrix;
    SGMatrix m_combined_matrix;
};

class SG_EXPORT SGRootNode : public SGNode
{
public:
    SGRootNode();
    ~SGRootNode();
    void accept(SGNodeVisitor *visitor) override;
private:
    void notifyNodeChange(SGNode *node, DirtyFlag flags);

    friend class SGNode;
    friend class SGGeometryNode;
    friend class SGRenderer;

    std::vector<SGRenderer *> m_renderers;
};


class SG_EXPORT SGOpacityNode : public SGNode
{
public:
    SGOpacityNode();
    ~SGOpacityNode();

    void setOpacity(double opacity);
    double opacity() const { return m_opacity; }

    void setCombinedOpacity(double opacity);
    double combinedOpacity() const { return m_combined_opacity; }

    bool isSubtreeBlocked() const override;
    void accept(SGNodeVisitor *visitor) override;
private:
    double m_opacity;
    double m_combined_opacity;
};


#endif //SGNODE_H
