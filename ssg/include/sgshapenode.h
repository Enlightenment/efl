#ifndef SGSHAPENODE_H
#define SGSHAPENODE_H

#include "sgnode.h"

class SGGradient;

class SGShapeNode : public SGGeometryNode
{
public:
    FillRule fillRule() const;
    void setFillRule(FillRule fillRule);
    JoinStyle joinStyle() const;
    void setJoinStyle(JoinStyle style);
    CapStyle capStyle() const;
    void setCapStyle(CapStyle style);
    StrokeStyle strokeStyle() const;
    void setStrokeStyle(StrokeStyle style);
    double strokeWidth() const;
    void setStrokeWidth(double w);

    SGColor fillColor() const;
    void setFillColor(const SGColor &color);
    SGColor strokeColor() const;
    void setStrokeColor(const SGColor &color);

    SGGradient *fillGradient() const;
    void setFillGradient(SGGradient *gradient);
};

#endif // SGSHAPENODE_H
