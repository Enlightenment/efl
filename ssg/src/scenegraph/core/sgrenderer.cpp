#include"sgrenderer.h"
#include"sgnodeupdater_p.h"
#include<algorithm>
#include<cassert>


class SGRendererPrivate
{
    friend class SGRenderer;
public:
    SGRenderContext *m_context;
    SGRootNode *m_root_node;

    //QColor m_clear_color;
    SGRenderer::ClearMode m_clear_mode;

    uint m_mirrored : 1;

    std::unordered_set<SGNode *> m_nodes_to_preprocess;
    std::unordered_set<SGNode *> m_nodes_dont_preprocess;

    SGNodeUpdater *m_node_updater;

    double m_current_opacity;

    bool m_changed_emitted;
    bool m_is_rendering;
    bool m_is_preprocessing;
};

SGRenderer::SGRenderer(SGRenderContext *context)
{
    d = new SGRendererPrivate();
    d->m_current_opacity = 1,
    d->m_changed_emitted = false;
    d->m_is_preprocessing = false;
    d->m_is_rendering = false;
    d->m_context = context;
    d->m_node_updater = new SGNodeUpdater();
}

SGRenderer::~SGRenderer()
{
    setRootNode(nullptr);
    delete d;
}


/*!
    Sets the \a node as the root of the SGNode scene
    that you want to render. You need to provide a \a node
    before trying to render the scene.

    \note This doesn't take ownership of \a node.

    \sa rootNode()
*/
void SGRenderer::setRootNode(SGRootNode *node)
{
    if (d->m_root_node == node)
        return;
    if (d->m_root_node) {
        auto itr = find(d->m_root_node->m_renderers.begin(), d->m_root_node->m_renderers.end(), this);
        if (itr != d->m_root_node->m_renderers.end())
           d->m_root_node->m_renderers.erase(itr);
        nodeChanged(d->m_root_node, SGNode::DirtyState::NodeRemoved);
    }
    d->m_root_node = node;
    if (d->m_root_node) {
        d->m_root_node->m_renderers.push_back(this);
        nodeChanged(d->m_root_node, SGNode::DirtyState::NodeAdded);
    }
}

/*!
    Returns the root of the SGNode scene.

    \sa setRootNode()
*/
SGRootNode *SGRenderer::rootNode() const
{
    return d->m_root_node;
}

SGNodeUpdater *SGRenderer::nodeUpdater() const
{
    return d->m_node_updater;
}

void SGRenderer::setNodeUpdater(SGNodeUpdater *updater)
{
    if (d->m_node_updater)
        delete d->m_node_updater;
    d->m_node_updater = updater;
}


/*!
    Updates internal data structures and emits the sceneGraphChanged() signal.

    If \a flags contains the SGNode::DirtyState::NodeRemoved flag, the node might be
    in the process of being destroyed. It is then not safe to downcast the node
    pointer.
*/

void SGRenderer::nodeChanged(SGNode *node, SGNode::DirtyFlag state)
{
    if (state & SGNode::DirtyState::NodeAdded)
        addNodesToPreprocess(node);
    if (state & SGNode::DirtyState::NodeRemoved)
        removeNodesToPreprocess(node);
    if (state & SGNode::DirtyState::UsePreprocess) {
        if (node->flags() & SGNode::Flag::UsePreprocess)
            d->m_nodes_to_preprocess.insert(node);
        else
            d->m_nodes_to_preprocess.erase(node);
    }

    if (!d->m_changed_emitted && !d->m_is_rendering) {
        // Premature overoptimization to avoid excessive signal emissions
        d->m_changed_emitted = true;
        //sceneGraphChanged();
    }
}

void SGRenderer::addNodesToPreprocess(SGNode *node)
{
    for (SGNode *c = node->firstChild(); c; c = c->nextSibling())
        addNodesToPreprocess(c);
    if (node->flags() & SGNode::Flag::UsePreprocess)
        d->m_nodes_to_preprocess.insert(node);
}

void SGRenderer::removeNodesToPreprocess(SGNode *node)
{
    for (SGNode *c = node->firstChild(); c; c = c->nextSibling())
        removeNodesToPreprocess(c);
    if (node->flags() & SGNode::Flag::UsePreprocess) {
        d->m_nodes_to_preprocess.erase(node);

        // If preprocessing *now*, mark the node as gone.
        if (d->m_is_preprocessing)
            d->m_nodes_dont_preprocess.insert(node);
    }
}

void SGRenderer::renderScene()
{
    if (!rootNode())
        return;

    d->m_is_rendering = true;

    preprocess();

    render();

    d->m_is_rendering = false;
    d->m_changed_emitted = false;

}


void SGRenderer::preprocess()
{
    d->m_is_preprocessing = true;

    SGRootNode *root = rootNode();
    assert(root);

    // We need to take a copy here, in case any of the preprocess calls deletes a node that
    // is in the preprocess list and thus, changes the m_nodes_to_preprocess behind our backs
    // For the default case, when this does not happen, the cost is negligible.
    std::unordered_set<SGNode *> items = d->m_nodes_to_preprocess;

    for (auto n : items) {

        // If we are currently preprocessing, check this node hasn't been
        // deleted or something. we don't want a use-after-free!
        if (d->m_nodes_dont_preprocess.find(n) != d->m_nodes_dont_preprocess.end()) // skip
            continue;
        if (!nodeUpdater()->isNodeBlocked(n, root)) {
            n->preprocess();
        }
    }

    nodeUpdater()->updateStates(root);

    d->m_is_preprocessing = false;
    d->m_nodes_dont_preprocess.clear();
}

