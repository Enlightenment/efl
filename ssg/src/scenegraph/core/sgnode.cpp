#include<sgnode.h>
#include<sgrenderer.h>
#include<cassert>
#include"sgnodevisitor_p.h"

SGNode::SGNode(NodeType type)
    : m_parent(0)
    , m_type(type)
    , m_firstChild(0)
    , m_lastChild(0)
    , m_nextSibling(0)
    , m_previousSibling(0)
    , m_nodeFlags(Flag::OwnedByParent)
{
}
SGNode::~SGNode()
{
    destroy();
}

bool SGNode::isSubtreeBlocked() const
{
    return false;
}

void SGNode::destroy()
{
    if (m_parent) {
        m_parent->removeChildNode(this);
        assert(m_parent == 0);
    }
    while (m_firstChild) {
        SGNode *child = m_firstChild;
        removeChildNode(child);
        assert(child->m_parent == 0);
        if (child->flags() & Flag::OwnedByParent)
            delete child;
    }

    assert(m_firstChild == 0 && m_lastChild == 0);
}

void SGNode::prependChildNode(SGNode *node)
{
    assert(!node->m_parent && "SGNode::prependChildNode, SGNode already has a parent");

    if (m_firstChild)
        m_firstChild->m_previousSibling = node;
    else
        m_lastChild = node;
    node->m_nextSibling = m_firstChild;
    m_firstChild = node;
    node->m_parent = this;

    node->markDirty(DirtyState::NodeAdded);
}

void SGNode::appendChildNode(SGNode *node)
{
    assert(!node->m_parent && "SGNode::appendChildNode, SGNode already has a parent");

    if (m_lastChild)
        m_lastChild->m_nextSibling = node;
    else
        m_firstChild = node;
    node->m_previousSibling = m_lastChild;
    m_lastChild = node;
    node->m_parent = this;

    node->markDirty(DirtyState::NodeAdded);
}

void SGNode::insertChildNodeBefore(SGNode *node, SGNode *before)
{
    assert(!node->m_parent && "SGNode::appendChildNode, SGNode already has a parent");
    assert(before && before->m_parent == this && "SGNode::insertChildNodeBefore, The parent of \'before\' is wrong");

    SGNode *previous = before->m_previousSibling;
    if (previous)
        previous->m_nextSibling = node;
    else
        m_firstChild = node;
    node->m_previousSibling = previous;
    node->m_nextSibling = before;
    before->m_previousSibling = node;
    node->m_parent = this;

    node->markDirty(DirtyState::NodeAdded);
}

void SGNode::insertChildNodeAfter(SGNode *node, SGNode *after)
{
    assert(!node->m_parent && "SGNode::appendChildNode, SGNode already has a parent");
    assert(after && after->m_parent == this && "SGNode::insertChildNodeBefore, The parent of 'after' is wrong");

    SGNode *next = after->m_nextSibling;
    if (next)
        next->m_previousSibling = node;
    else
        m_lastChild = node;
    node->m_nextSibling = next;
    node->m_previousSibling = after;
    after->m_nextSibling = node;
    node->m_parent = this;

    node->markDirty(DirtyState::NodeAdded);
}

void SGNode::removeChildNode(SGNode *node)
{
    assert(node->parent() == this);

    SGNode *previous = node->m_previousSibling;
    SGNode *next = node->m_nextSibling;
    if (previous)
        previous->m_nextSibling = next;
    else
        m_firstChild = next;
    if (next)
        next->m_previousSibling = previous;
    else
        m_lastChild = previous;
    node->m_previousSibling = 0;
    node->m_nextSibling = 0;

    node->markDirty(DirtyState::NodeRemoved);
    node->m_parent = 0;
}

void SGNode::removeAllChildNodes()
{
    while (m_firstChild) {
        SGNode *node = m_firstChild;
        m_firstChild = node->m_nextSibling;
        node->m_nextSibling = 0;
        if (m_firstChild)
            m_firstChild->m_previousSibling = 0;
        else
            m_lastChild = 0;
        node->markDirty(DirtyState::NodeRemoved);
        node->m_parent = 0;
    }
}

void SGNode::reparentChildNodesTo(SGNode *newParent)
{
    for (SGNode *c = firstChild(); c; c = firstChild()) {
        removeChildNode(c);
        newParent->appendChildNode(c);
    }
}

int SGNode::childCount() const
{
    int count = 0;
    SGNode *n = m_firstChild;
    while (n) {
        ++count;
        n = n->m_nextSibling;
    }
    return count;
}


SGNode *SGNode::childAtIndex(int i) const
{
    SGNode *n = m_firstChild;
    while (i && n) {
        --i;
        n = n->m_nextSibling;
    }
    return n;
}

void SGNode::setFlag(NodeFlag  f, bool enabled)
{
    if (bool(m_nodeFlags & f) == enabled)
        return;
    m_nodeFlags ^= f;
    int changedFlag = f & Flag::UsePreprocess;
    if (changedFlag)
        markDirty(DirtyState::UsePreprocess);
}

void SGNode::setFlags(NodeFlag f, bool enabled)
{
    NodeFlag oldFlags = m_nodeFlags;
    if (enabled)
        m_nodeFlags |= f;
    else
        m_nodeFlags &= ~f;
    int changedFlags = (oldFlags ^ m_nodeFlags) & Flag::UsePreprocess;
    if (changedFlags)
        markDirty(DirtyState::UsePreprocess);
}

void SGNode::markDirty(DirtyFlag bits)
{
    int renderableCountDiff = 0;
    if (bits & DirtyState::NodeAdded)
        renderableCountDiff += m_subtreeRenderableCount;
    if (bits & DirtyState::NodeRemoved)
        renderableCountDiff -= m_subtreeRenderableCount;

    SGNode *p = m_parent;
    while (p) {
        p->m_subtreeRenderableCount += renderableCountDiff;
        if (p->type() == NodeType::Root)
            static_cast<SGRootNode *>(p)->notifyNodeChange(this, bits);
        p = p->m_parent;
    }
}

SGBasicGeometryNode::SGBasicGeometryNode(NodeType type)
    : SGNode(type)
    , m_geometry(0)
    , m_matrix(0)
    , m_clip_list(0)
{
}

SGBasicGeometryNode::~SGBasicGeometryNode()
{
//    if (flags() & Flag::OwnsGeometry)
//        delete m_geometry;
}

void SGBasicGeometryNode::setGeometry(SGGeometry *geometry)
{
//    if ((flags() & Flag::OwnsGeometry) != 0 && m_geometry != geometry)
//        delete m_geometry;
    m_geometry = geometry;
    markDirty(DirtyState::Geometry);
}


SGGeometryNode::SGGeometryNode()
    : SGBasicGeometryNode(NodeType::Geometry)
    , m_render_order(0)
    , m_opacity(1)
{
}

SGGeometryNode::~SGGeometryNode()
{
}

void SGGeometryNode::setRenderOrder(int order)
{
    m_render_order = order;
}


void SGGeometryNode::setInheritedOpacity(double opacity)
{
    m_opacity = opacity;
}

SGClipNode::SGClipNode()
    : SGBasicGeometryNode(NodeType::Clip)
    , m_is_rectangular(false)
{
}


SGClipNode::~SGClipNode()
{
}




void SGClipNode::setIsRectangular(bool rectHint)
{
    m_is_rectangular = rectHint;
}

void SGClipNode::setClipRect(const SGRect &rect)
{
    m_clip_rect = rect;
}

SGTransformNode::SGTransformNode()
    : SGNode(NodeType::Transform)
{
}


SGTransformNode::~SGTransformNode()
{
}

void SGTransformNode::setMatrix(const SGMatrix &matrix)
{
    m_matrix = matrix;
    markDirty(DirtyState::Matrix);
}

void SGTransformNode::setCombinedMatrix(const SGMatrix &matrix)
{
    m_combined_matrix = matrix;
}

SGRootNode::SGRootNode()
    : SGNode(NodeType::Root)
{
}


SGRootNode::~SGRootNode()
{
    while (!m_renderers.empty())
        m_renderers.back()->setRootNode(nullptr);
    destroy(); // Must call destroy() here because markDirty() casts this to SGRootNode.
}

void SGRootNode::notifyNodeChange(SGNode *node, DirtyFlag flag)
{
    for (uint i=0; i<m_renderers.size(); ++i) {
        m_renderers.at(i)->nodeChanged(node, flag);
    }
}

const double OPACITY_THRESHOLD = 0.001;

SGOpacityNode::SGOpacityNode()
    : SGNode(NodeType::Opacity)
    , m_opacity(1)
    , m_combined_opacity(1)
{
}


SGOpacityNode::~SGOpacityNode()
{
}

void SGOpacityNode::setOpacity(double opacity)
{
    //opacity = qBound<double>(0, opacity, 1);
    if (m_opacity == opacity)
        return;
    DirtyFlag dirtyFlag = DirtyState::Opacity;

    if ((m_opacity < OPACITY_THRESHOLD && opacity >= OPACITY_THRESHOLD)     // blocked to unblocked
        || (m_opacity >= OPACITY_THRESHOLD && opacity < OPACITY_THRESHOLD)) // unblocked to blocked
        dirtyFlag |= DirtyState::SubtreeBlocked;

    m_opacity = opacity;
    markDirty(dirtyFlag);
}


void SGOpacityNode::setCombinedOpacity(double opacity)
{
    m_combined_opacity = opacity;
}

bool SGOpacityNode::isSubtreeBlocked() const
{
    return m_opacity < OPACITY_THRESHOLD;
}

void SGClipNode::accept(SGNodeVisitor *visitor)
{if (visitor->visit(this)) visitor->visitChildren(this); visitor->endVisit(this);}

void SGTransformNode::accept(SGNodeVisitor *visitor)
{if (visitor->visit(this)) visitor->visitChildren(this); visitor->endVisit(this);}

void SGOpacityNode::accept(SGNodeVisitor *visitor)
{if (visitor->visit(this)) visitor->visitChildren(this); visitor->endVisit(this);}

void SGRootNode::accept(SGNodeVisitor *visitor)
{if (visitor->visit(this)) visitor->visitChildren(this); visitor->endVisit(this);}

void SGGeometryNode::accept(SGNodeVisitor *visitor)
{if (visitor->visit(this)) visitor->visitChildren(this); visitor->endVisit(this);}






