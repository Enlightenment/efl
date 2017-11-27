#ifndef SGRECTNODE_H
#define SGRECTNODE_H
#include<sgnode.h>

class SGRectNode : public SGGeometryNode
{
public:
    virtual void setRect(const SGRect &rect) = 0;
    inline void setRect(int x, int y, int w, int h) noexcept { setRect(SGRect(x, y, w, h)); }
    virtual SGRect rect() const noexcept = 0;

    virtual void setColor(const SGColor &color) = 0;
    virtual SGColor color() const = 0;
};
#endif // SGRECTNODE_H
