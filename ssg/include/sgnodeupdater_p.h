#ifndef SGNODEUPDATER_P_H
#define SGNODEUPDATER_P_H

#include"sgglobal.h"
#include"sgnode.h"
#include "sgnodevisitor_p.h"

class SG_EXPORT SGNodeUpdater : public SGNodeVisitor
{
public:
    SGNodeUpdater();
    ~SGNodeUpdater();
    bool visit(SGTransformNode *) override;
    void endVisit(SGTransformNode *) override;
    bool visit(SGClipNode *) override;
    void endVisit(SGClipNode *) override;
    bool visit(SGGeometryNode *) override;
    void endVisit(SGGeometryNode *) override;
    bool visit(SGOpacityNode *) override;
    void endVisit(SGOpacityNode *) override;
    bool visit(SGRootNode *) override;
    void endVisit(SGRootNode *) override;
    void visitChildren(SGNode *node) override;

    bool isNodeBlocked(SGNode *n, SGNode *root) const;
    void updateStates(SGNode *n);

    void visitNode(SGNode *node);
};
#endif // SGNODEUPDATER_P_H
