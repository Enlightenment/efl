#ifndef SGSOFTWARENODEUPDATER_H
#define SGSOFTWARENODEUPDATER_H

#include"sgnode.h"
#include "sgnodeupdater_p.h"
#include"sgsoftwaredrawable.h"
#include"sgsoftwarerenderer.h"

#include<stack>
#include<unordered_map>

class SGSoftwareRenderer;

class SGSoftwareNodeUpdater : public SGNodeUpdater
{
public:
    SGSoftwareNodeUpdater(SGSoftwareRenderer *renderer);
    virtual ~SGSoftwareNodeUpdater();

    bool visit(SGTransformNode *);
    void endVisit(SGTransformNode *);
    bool visit(SGClipNode *);
    void endVisit(SGClipNode *);
    bool visit(SGGeometryNode *);
    void endVisit(SGGeometryNode *);
    bool visit(SGOpacityNode *);
    void endVisit(SGOpacityNode *);
    bool visit(SGRootNode *);
    void endVisit(SGRootNode *) ;
    void updateNodes(SGNode *node, bool isNodeRemoved = false);

    struct NodeState {
        float opacity;
        SGRegion clip;
        bool hasClip;
        SGMatrix transform;
        SGNode *parent;
    };

private:

    NodeState currentState(SGNode *node) const;

    template<class NODE>
    bool updateRenderableNode(SGSoftwareDrawable::NodeType type, NODE *node);

    SGSoftwareRenderer *m_renderer;
    std::stack<float> m_opacityState;
    std::stack<SGRegion> m_clipState;
    bool m_hasClip;
    std::stack<SGMatrix> m_transformState;
    std::unordered_map<SGNode*,NodeState> m_stateMap;
};

template<class NODE>
bool SGSoftwareNodeUpdater::updateRenderableNode(SGSoftwareDrawable::NodeType type, NODE *node)
{
    //Check if we already know about node
    auto renderableNode = m_renderer->renderableNode(node);
    if (renderableNode == nullptr) {
        renderableNode = new SGSoftwareDrawable(type, node);
        m_renderer->addNodeMapping(node, renderableNode);
    }

    //Update the node
    renderableNode->setTransform(m_transformState.top());
    renderableNode->setOpacity(m_opacityState.top());
    renderableNode->setClipRegion(m_clipState.top(), m_hasClip);

    renderableNode->update();
    m_stateMap[node] = currentState(node);

    return true;
}

#endif  //SGSOFTWARENODEUPDATER_H
