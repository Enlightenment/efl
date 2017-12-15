#ifndef SGPOINT_H
#define SGPOINT_H

#include"sgglobal.h"

class SG_EXPORT SGPointF
{
public:
    constexpr inline SGPointF() noexcept :mx(0), my(0){}
    constexpr inline SGPointF(float x, float y) noexcept :mx(x), my(y){}
    constexpr inline float x() const noexcept {return mx;}
    constexpr inline float y() const noexcept {return my;}
    inline void setX(float x) {mx = x;}
    inline void setY(float y) {my = y;}
    inline SGPointF &operator+=(const SGPointF &p) noexcept;
    inline SGPointF &operator-=(const SGPointF &p) noexcept;
    friend const  SGPointF operator+(const SGPointF & p1, const SGPointF & p2) {
        return SGPointF(p1.mx + p2.mx , p1.my + p2.my);
    }
private:
    float mx;
    float my;
};

inline SGPointF &SGPointF::operator+=(const SGPointF &p) noexcept
{ mx+=p.mx; my+=p.my; return *this; }

inline SGPointF &SGPointF::operator-=(const SGPointF &p) noexcept
{ mx-=p.mx; my-=p.my; return *this;  }

class SG_EXPORT SGPoint
{
public:
    constexpr inline SGPoint() noexcept :mx(0), my(0){}
    constexpr inline SGPoint(int x, int y) noexcept :mx(x), my(y){}
    constexpr inline int x() const noexcept {return mx;}
    constexpr inline int y() const noexcept {return my;}
    inline void setX(int x) {mx = x;}
    inline void setY(int y) {my = y;}
    inline SGPoint &operator+=(const SGPoint &p) noexcept;
    inline SGPoint &operator-=(const SGPoint &p) noexcept;
    constexpr inline bool operator==(const SGPoint &o) const;
    constexpr inline bool operator!=(const SGPoint &o) const { return !(operator==(o)); }
    inline friend SGDebug& operator<<(SGDebug& os, const SGPoint& o);
private:
    int mx;
    int my;
};
inline SGDebug& operator<<(SGDebug& os, const SGPoint& o)
{
    os<<"{P "<<o.x()<<","<<o.y()<<"}";
    return os;
}
constexpr inline bool SGPoint::operator ==(const SGPoint &o) const
{
    return (mx == o.x() && my == o.y());
}

inline SGPoint &SGPoint::operator+=(const SGPoint &p) noexcept
{ mx+=p.mx; my+=p.my; return *this; }

inline SGPoint &SGPoint::operator-=(const SGPoint &p) noexcept
{ mx-=p.mx; my-=p.my; return *this;  }

#endif // SGPOINT_H
