#ifndef SGCONTEXT_H
#define SGCONTEXT_H

#include"sgglobal.h"

class SGRectangleNode;
class SGImageNode;
class SGGlyphNode;
class SGLayer;
class SGRenderContext;

class SG_EXPORT SGContext
{

public:
    enum AntialiasingMethod {
        UndecidedAntialiasing,
        VertexAntialiasing,
        MsaaAntialiasing
    };

    explicit SGContext();
    virtual ~SGContext();

    virtual void renderContextInitialized(SGRenderContext *renderContext);
    virtual void renderContextInvalidated(SGRenderContext *renderContext);
    virtual SGRenderContext *createRenderContext() = 0;

    virtual SGRectangleNode *createRectangleNode() = 0;
    virtual SGImageNode *createImageNode() = 0;
    virtual SGGlyphNode *createGlyphNode(SGRenderContext *rc) = 0;
    virtual SGLayer *createLayer(SGRenderContext *renderContext) = 0;

    static SGContext *createDefaultContext();
    //static SGRenderLoop *createWindowManager();

    static void setBackend(const char *backend);
    static const char *backend();
};

class SG_EXPORT SGRenderContext
{
public:
    enum CreateTextureFlags {
        CreateTexture_Alpha       = 0x1,
        CreateTexture_Atlas       = 0x2,
        CreateTexture_Mipmap      = 0x4
    };

    SGRenderContext(SGContext *context);
    virtual ~SGRenderContext();

    SGContext *sceneGraphContext() const { return m_sg; }
    virtual bool isValid() const { return true; }

    virtual void initialize(void *context);
    virtual void invalidate();
    virtual void renderNextFrame(SGRenderer *renderer, uint fboId) = 0;
    virtual void endSync();

    virtual SGTexture *createTexture() const = 0;
    virtual SGRenderer *createRenderer() = 0;

    virtual void setAttachToGraphicsContext(bool attach) { Q_UNUSED(attach); }

    virtual int maxTextureSize() const = 0;

    void registerFontengineForCleanup(QFontEngine *engine);

protected:
    // Hold m_sg with QPointer in the rare case it gets deleted before us.
    SGContext  *m_sg;
};


#endif // SGCONTEXT_H
