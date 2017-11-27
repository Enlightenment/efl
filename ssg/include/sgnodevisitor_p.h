#ifndef SGNODEVISITOR_P_H
#define SGNODEVISITOR_P_H

class SGTransformNode;
class SGClipNode;
class SGGeometryNode;
class SGOpacityNode;
class SGRootNode;
class SGNode;

class SGNodeVisitor
{
public:
    virtual ~SGNodeVisitor() {}
    virtual bool visit(SGTransformNode *) = 0;
    virtual void endVisit(SGTransformNode *) = 0;
    virtual bool visit(SGClipNode *) = 0;
    virtual void endVisit(SGClipNode *) = 0;
    virtual bool visit(SGGeometryNode *) = 0;
    virtual void endVisit(SGGeometryNode *) = 0;
    virtual bool visit(SGOpacityNode *) = 0;
    virtual void endVisit(SGOpacityNode *) = 0;
    virtual bool visit(SGRootNode *) = 0;
    virtual void endVisit(SGRootNode *) = 0;
    virtual void visitChildren(SGNode *) = 0;
};

#endif // SGNODEVISITOR_P_H
