#include<sgrectnode.h>
#include"sgsoftwarenodeupdater.h"
#include"sgsoftwaredrawable.h"

SGSoftwareNodeUpdater::SGSoftwareNodeUpdater(SGSoftwareRenderer *renderer)
    : m_renderer(renderer)
{
    m_opacityState.push(1.0f);
    // by default there is no clip
    m_clipState.push(SGRegion());
    m_hasClip = false;
    m_transformState.push(SGMatrix());
}
SGSoftwareNodeUpdater::~SGSoftwareNodeUpdater()
{

}

bool SGSoftwareNodeUpdater::visit(SGTransformNode *node)
{
    m_transformState.push(node->matrix() * m_transformState.top());
    m_stateMap[node] = currentState(node);
    return true;
}

void SGSoftwareNodeUpdater::endVisit(SGTransformNode *)
{
    m_transformState.pop();
}

bool SGSoftwareNodeUpdater::visit(SGClipNode *node)
{
    // Make sure to translate the clip rect into world coordinates
    if (m_clipState.size() == 0 || m_clipState.top().isEmpty()) {
        m_clipState.push(m_transformState.top().map(SGRegion(node->clipRect())));
        m_hasClip = true;
    } else {
        const SGRegion transformedClipRect = m_transformState.top().map(SGRegion(node->clipRect()));
        m_clipState.push(transformedClipRect.intersected(m_clipState.top()));
    }
    m_stateMap[node] = currentState(node);
    return true;
}

void SGSoftwareNodeUpdater::endVisit(SGClipNode *)
{
    m_clipState.pop();
    if (m_clipState.size() == 0 || m_clipState.top().isEmpty())
        m_hasClip = false;
}

bool SGSoftwareNodeUpdater::visit(SGOpacityNode *node)
{
    m_opacityState.push(m_opacityState.top() * node->opacity());
    m_stateMap[node] = currentState(node);
    return true;
}

void SGSoftwareNodeUpdater::endVisit(SGOpacityNode *)
{
    m_opacityState.pop();
}

bool SGSoftwareNodeUpdater::visit(SGRootNode *node)
{
    m_stateMap[node] = currentState(node);
    return true;
}

void SGSoftwareNodeUpdater::endVisit(SGRootNode *)
{
}

bool SGSoftwareNodeUpdater::visit(SGGeometryNode *node)
{
    if (SGRectNode *rectNode = dynamic_cast<SGRectNode *>(node)) {
        return updateRenderableNode(SGSoftwareDrawable::Rect, rectNode);
    } else {
        // We dont know, so skip
        return false;
    }
}

void SGSoftwareNodeUpdater::endVisit(SGGeometryNode *)
{
}

void SGSoftwareNodeUpdater::updateNodes(SGNode *node, bool isNodeRemoved)
{
    m_opacityState = std::stack<float>();
    m_clipState = std::stack<SGRegion>();
    m_transformState = std::stack<SGMatrix>();

    auto parentNode = node->parent();
    // If the node was deleted, it will have no parent
    // check if the state map has the previous parent
    if ((!parentNode || isNodeRemoved ) && (m_stateMap.find(node) != m_stateMap.end()))
        parentNode = m_stateMap[node].parent;

    // If we find a parent, use its state for updating the new children
    if (parentNode && (m_stateMap.find(parentNode) != m_stateMap.end())) {
        auto state = m_stateMap[parentNode];
        m_opacityState.push(state.opacity);
        m_transformState.push(state.transform);
        m_clipState.push(state.clip);
        m_hasClip = state.hasClip;
    } else {
        // There is no parent, and no previous parent, so likely a root node
        m_opacityState.push(1.0f);
        m_transformState.push(SGMatrix());
        m_clipState.push(SGRegion());
        m_hasClip = false;
    }

    // If the node is being removed, then cleanup the state data
    // Then just visit the children without visiting the now removed node
    if (isNodeRemoved) {
        m_stateMap.erase(node);
        return;
    }

    node->accept(this);
}

SGSoftwareNodeUpdater::NodeState SGSoftwareNodeUpdater::currentState(SGNode *node) const
{
    NodeState state;
    state.opacity = m_opacityState.top();
    state.clip = m_clipState.top();
    state.hasClip = m_hasClip;
    state.transform = m_transformState.top();
    state.parent = node->parent();
    return state;
}
