#ifndef SGSOFTWARERENDERER_H
#define SGSOFTWARERENDERER_H

#include"sgrenderer.h"
#include<unordered_map>

class SGSoftwareDrawable;
class SGSoftwareNodeUpdater;
class SGRectNode;

class SG_EXPORT SGSoftwareRenderer : public SGRenderer
{
public:
    SGSoftwareRenderer(SGRenderContext *context);
    virtual ~SGSoftwareRenderer();

    SGSoftwareDrawable *renderableNode(SGNode *node) const;
    void addNodeMapping(SGNode *node, SGSoftwareDrawable *renderableNode);
    void appendRenderableNode(SGSoftwareDrawable *node);

    void nodeChanged(SGNode *node, SGNode::DirtyFlag flag) override;

    void markDirty();

    void buildRenderList();
    SGRegion optimizeRenderList();

    void setBackgroundColor(const SGColor &color);
    SGColor backgroundColor();

private:
    void nodeAdded(SGNode *node);
    void nodeRemoved(SGNode *node);
    void nodeGeometryUpdated(SGNode *node);
    void nodeMaterialUpdated(SGNode *node);
    void nodeMatrixUpdated(SGNode *node);
    void nodeOpacityUpdated(SGNode *node);

    std::unordered_map<SGNode*, SGSoftwareDrawable*> m_nodes;
    std::vector<SGSoftwareDrawable*> m_renderableNodes;

    SGRectNode *m_background;

    SGRegion m_dirtyRegion;
    SGRegion m_obscuredRegion;

    SGSoftwareNodeUpdater *m_nodeUpdater;
};

#endif //SGSOFTWARERENDERER_H
