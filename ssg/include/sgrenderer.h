#ifndef SGRENDERER_H
#define SGRENDERER_H

#include<unordered_set>

#include"sgglobal.h"
#include"sgnode.h"

class SGRenderContext;
class SGRendererPrivate;
class SGNodeUpdater;

class SG_EXPORT SGRenderer
{
public:
    enum ClearModeBit
    {
        ClearColorBuffer    = 0x0001,
        ClearDepthBuffer    = 0x0002,
        ClearStencilBuffer  = 0x0004
    };
    typedef sgFlag<ClearModeBit> ClearMode;

    SGRenderer(SGRenderContext *context);
    virtual ~SGRenderer();

    void setRootNode(SGRootNode *node);
    SGRootNode *rootNode() const;
    void setDeviceRect(const SGRect &rect);
    SGRect deviceRect() const;

    void setViewportRect(const SGRect &rect);
    SGRect viewportRect() const;

//    void setProjectionMatrix(const SGMatrix &matrix);
//    SGMatrix projectionMatrix() const;

//    void setClearColor(const QColor &color);
//    QColor clearColor() const;

//    void setClearMode(ClearMode mode);
//    ClearMode clearMode() const;
    SGNodeUpdater *nodeUpdater() const;
    void setNodeUpdater(SGNodeUpdater *updater);

    virtual void renderScene();

protected:
    explicit SGRenderer();
    virtual void nodeChanged(SGNode *node, SGNode::DirtyFlag flag) = 0;
    virtual void render() = 0;
    virtual void preprocess();
    void addNodesToPreprocess(SGNode *node);
    void removeNodesToPreprocess(SGNode *node);

private:
    friend class SGRendererPrivate;
    friend class SGRootNode;
    SGRendererPrivate *d;
};

#endif // SGRENDERER_H
