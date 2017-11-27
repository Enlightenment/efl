#include"sgsoftwarerenderer.h"
#include"sgsoftwarerectnode.h"
#include"sgsoftwarenodeupdater.h"

SGSoftwareRenderer::SGSoftwareRenderer(SGRenderContext *context)
    : SGRenderer(context)
    //, m_background(new SGSoftwareRectNode)
    , m_nodeUpdater(new SGSoftwareNodeUpdater(this))
{
    // Setup special background node
    auto backgroundRenderable = new SGSoftwareDrawable(SGSoftwareDrawable::Rect, m_background);
    addNodeMapping(m_background, backgroundRenderable);
}

SGSoftwareRenderer::~SGSoftwareRenderer()
{
    // Cleanup RenderableNodes
    delete m_background;

    // clean the hash m_nodes;

    delete m_nodeUpdater;
}

SGSoftwareDrawable *SGSoftwareRenderer::renderableNode(SGNode *node) const
{
    auto search = m_nodes.find(node);
    if (search != m_nodes.end()) {
        return search->second;
    }
    return nullptr;
}

void SGSoftwareRenderer::addNodeMapping(SGNode *node, SGSoftwareDrawable *renderableNode)
{
    m_nodes.insert({node, renderableNode});
}

void SGSoftwareRenderer::appendRenderableNode(SGSoftwareDrawable *node)
{
    m_renderableNodes.push_back(node);
}

SGRegion SGSoftwareRenderer::optimizeRenderList()
{
    // Iterate through the renderlist from front to back
    // Objective is to update the dirty status and rects.
    for (auto i = m_renderableNodes.rbegin(); i != m_renderableNodes.rend(); ++i) {
        auto node = *i;
        if (!m_dirtyRegion.isEmpty()) {
            // See if the current dirty regions apply to the current node
            node->addDirtyRegion(m_dirtyRegion, true);
        }

        if (!m_obscuredRegion.isEmpty()) {
            // Don't try to paint things that are covered by opaque objects
            node->subtractDirtyRegion(m_obscuredRegion);
        }

        // Keep up with obscured regions
        if (node->isOpaque()) {
            m_obscuredRegion += node->boundingRectMin();
        }

        if (node->isDirty()) {
            // Don't paint things outside of the rendering area
            if (!m_background->rect().contains(node->boundingRectMax(), /*proper*/ true)) {
                // Some part(s) of node is(are) outside of the rendering area
                SGRegion renderArea(m_background->rect());
                SGRegion outsideRegions = node->dirtyRegion().subtracted(renderArea);
                if (!outsideRegions.isEmpty())
                    node->subtractDirtyRegion(outsideRegions);
            }

            // Get the dirty region's to pass to the next nodes
            if (node->isOpaque()) {
                // if isOpaque, subtract node's dirty rect from m_dirtyRegion
                m_dirtyRegion -= node->boundingRectMin();
            } else {
                // if isAlpha, add node's dirty rect to m_dirtyRegion
                m_dirtyRegion += node->dirtyRegion();
            }
            // if previousDirtyRegion has content outside of boundingRect add to m_dirtyRegion
            SGRegion prevDirty = node->previousDirtyRegion();
            if (!prevDirty.isEmpty())
                m_dirtyRegion += prevDirty;
        }
    }

    // Empty dirtyRegion (for second pass)
    m_dirtyRegion = SGRegion();
    m_obscuredRegion = SGRegion();

    // Iterate through the renderlist from back to front
    // Objective is to make sure all non-opaque items are painted when an item under them is dirty
    for (auto j = m_renderableNodes.begin(); j != m_renderableNodes.end(); ++j) {
        auto node = *j;

        if (!node->isOpaque() && !m_dirtyRegion.isEmpty()) {
            // Only blended nodes need to be updated
            node->addDirtyRegion(m_dirtyRegion, true);
        }

        m_dirtyRegion += node->dirtyRegion();
    }

    SGRegion updateRegion = m_dirtyRegion;

    // Empty dirtyRegion
    m_dirtyRegion = SGRegion();
    m_obscuredRegion = SGRegion();

    return updateRegion;
}

void SGSoftwareRenderer::nodeChanged(SGNode *node, SGNode::DirtyFlag state)
{
        if (state & SGNode::DirtyState::Geometry) {
            nodeGeometryUpdated(node);
        }
        if (state & SGNode::DirtyState::Material) {
            nodeMaterialUpdated(node);
        }
        if (state & SGNode::DirtyState::Matrix) {
            nodeMatrixUpdated(node);
        }
        if (state & SGNode::DirtyState::NodeAdded) {
            nodeAdded(node);
        }
        if (state & SGNode::DirtyState::NodeRemoved) {
            nodeRemoved(node);
        }
        if (state & SGNode::DirtyState::Opacity) {
            nodeOpacityUpdated(node);
        }
        if (state & SGNode::DirtyState::SubtreeBlocked) {
            m_nodeUpdater->updateNodes(node);
        }
        if (state & SGNode::DirtyState::ForceUpdate) {
            m_nodeUpdater->updateNodes(node);
        }
        SGRenderer::nodeChanged(node, state);
}

void SGSoftwareRenderer::nodeAdded(SGNode *node)
{
    m_nodeUpdater->updateNodes(node);
}

void SGSoftwareRenderer::nodeRemoved(SGNode *node)
{

    auto renderable = renderableNode(node);
    // remove mapping
    if (renderable != nullptr) {
        // Need to mark this region dirty in the other nodes
        SGRegion dirtyRegion = renderable->previousDirtyRegion(true);
        if (dirtyRegion.isEmpty())
            dirtyRegion = renderable->boundingRectMax();
        m_dirtyRegion += dirtyRegion;
        m_nodes.erase(node);
        delete renderable;
    }

    // Remove all children nodes as well
    for (SGNode *child = node->firstChild(); child; child = child->nextSibling()) {
        nodeRemoved(child);
    }

    m_nodeUpdater->updateNodes(node, true);
}

void SGSoftwareRenderer::nodeMatrixUpdated(SGNode *node)
{
    // Update children nodes
    m_nodeUpdater->updateNodes(node);
}

void SGSoftwareRenderer::nodeOpacityUpdated(SGNode *node)
{
    // Update children nodes
    m_nodeUpdater->updateNodes(node);
}

void SGSoftwareRenderer::nodeGeometryUpdated(SGNode *node)
{

    // Mark node as dirty
    auto renderable = renderableNode(node);
    if (renderable != nullptr) {
        //renderable->markGeometryDirty();
    } else {
        m_nodeUpdater->updateNodes(node);
    }
}

void SGSoftwareRenderer::nodeMaterialUpdated(SGNode *node)
{
    // Mark node as dirty
    auto renderable = renderableNode(node);
    if (renderable != nullptr) {
        //renderable->markMaterialDirty();
    } else {
        m_nodeUpdater->updateNodes(node);
    }
}

