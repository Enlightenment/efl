#include"sgnodeupdater_p.h"

SGNodeUpdater::SGNodeUpdater()
{
}

SGNodeUpdater::~SGNodeUpdater()
{
}

/*!
    Returns true if \a node is has something that blocks it in the chain from
    \a node to \a root doing a full state update pass.

    This function does not process dirty states, simply does a simple traversion
    up to the top.

    The function assumes that \a root exists in the parent chain of \a node.
 */

bool SGNodeUpdater::isNodeBlocked(SGNode *node, SGNode *root) const
{
    while (node != root && node != 0) {
        if (node->isSubtreeBlocked())
            return true;
        node = node->parent();
    }
    return false;
}

void SGNodeUpdater::updateStates(SGNode *n)
{

}

bool SGNodeUpdater::visit(SGTransformNode *) { return true; }
void SGNodeUpdater::endVisit(SGTransformNode *){}
bool SGNodeUpdater::visit(SGClipNode *) { return true; }
void SGNodeUpdater::endVisit(SGClipNode *){}
bool SGNodeUpdater::visit(SGGeometryNode *) { return true; }
void SGNodeUpdater::endVisit(SGGeometryNode *){}
bool SGNodeUpdater::visit(SGOpacityNode *) { return true; }
void SGNodeUpdater::endVisit(SGOpacityNode *){}
bool SGNodeUpdater::visit(SGRootNode *) { return true; }
void SGNodeUpdater::endVisit(SGRootNode *){}

void SGNodeUpdater::visitChildren(SGNode *n)
{
    for (SGNode *c = n->firstChild(); c; c = c->nextSibling())
        c->accept(this);
}

//void SGNodeUpdater::visitNode(SGNode *node)
//{
//    switch (node->type()) {
//    case SGNode::NodeType::Clip: {
//        SGClipNode *c = static_cast<SGClipNode*>(node);
//        if (visit(c))
//            visitChildren(c);
//        endVisit(c);
//        break;
//    }
//    case SGNode::NodeType::Transform: {
//        SGTransformNode *c = static_cast<SGTransformNode*>(node);
//        if (visit(c))
//            visitChildren(c);
//        endVisit(c);
//        break;
//    }
//    case SGNode::NodeType::Opacity: {
//        SGOpacityNode *c = static_cast<SGOpacityNode*>(node);
//        if (visit(c))
//            visitChildren(c);
//        endVisit(c);
//        break;
//    }
//    case SGNode::NodeType::Geometry: {
//        if (node->flags() & SGNode::Flag::IsVisitableNode) {
//            node->accept(this);
//        } else {
//            SGGeometryNode *c = static_cast<SGGeometryNode*>(node);
//            if (visit(c))
//                visitChildren(c);
//            endVisit(c);
//        }
//        break;
//    }
//    case SGNode::NodeType::Root: {
//        SGRootNode *root = static_cast<SGRootNode*>(node);
//        if (visit(root))
//            visitChildren(root);
//        endVisit(root);
//        break;
//    }
//    default:
//        break;
//    }
//}
