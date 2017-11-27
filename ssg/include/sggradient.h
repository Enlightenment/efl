#ifndef SGGRADIENT_H
#define SGGRADIENT_H

#include "sgglobal.h"
#include<vector>

class SGGradientStop
{
    SGGradientStop(double pos, const SGColor &color):m_pos(pos), m_color(color){}
    inline double position(){return m_pos;}
    inline double color(){return m_color;}
public:
    double  m_pos;
    SGColor m_color;
};

typedef std::vector<SGGradientStop> SGGradientStops;

class SGGradient
{
public:
    enum class SpreadMode {
        Pad,
        Repeat,
        Reflect
    };
    SpreadMode spread() const;
    void setSpread(SpreadMode mode);
private:
    SpreadMode       m_spread;
    SGGradientStops  m_stops;
};

class SGLinearGradient : public SGGradient
{
 public:
    double x1() const;
    void setX1(double v);
    double y1() const;
    void setY1(double v);
    double x2() const;
    void setX2(double v);
    double y2() const;
    void setY2(double v);
private:
    double m_X1, m_Y1;
    double m_X2, m_Y2;
};

class SGRadialGradient : public SGGradient
{
 public:
    double centerX() const;
    void setCenterX(double v);

    double centerY() const;
    void setCenterY(double v);

    double centerRadius() const;
    void setCenterRadius(double v);

    double focalX() const;
    void setFocalX(double v);

    double focalY() const;
    void setFocalY(double v);

    double focalRadius() const;
    void setFocalRadius(double v);
private:
    double m_centerX, m_centerY;
    double m_focalX, m_focalY;
    double m_centerRadius = 0;
    double m_focalRadius = 0;
};

#endif // SGGRADIENT_H
