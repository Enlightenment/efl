#ifndef SGRECT_H
#define SGRECT_H
#include"sgglobal.h"
#include"sgdebug.h"
class SG_EXPORT SGRect
{
public:
    SG_CONSTEXPR SGRect(): x1(0), y1(0), x2(-1), y2(-1) {}
    SG_CONSTEXPR SGRect(int left, int top, int width, int height);
    SG_CONSTEXPR inline bool isEmpty() const;

    SG_CONSTEXPR inline int left() const ;
    SG_CONSTEXPR inline int top() const ;
    SG_CONSTEXPR inline int right() const ;
    SG_CONSTEXPR inline int bottom() const ;
    SG_CONSTEXPR inline int width() const ;
    SG_CONSTEXPR inline int height() const ;
    SG_CONSTEXPR inline int x() const ;
    SG_CONSTEXPR inline int y() const ;
    inline void setLeft(int l) {x1 = l;}
    inline void setTop(int t) {y1 = t;}
    inline void setRight(int r) {x2 = r;}
    inline void setBottom(int b) {y2 = b;}
    inline void setWidth(int w) {x2 = x1 + w;}
    inline void setHeight(int h) {y2 = y1 + h;}
    inline SGRect translated(int dx, int dy) const ;
    inline void translate(int dx, int dy);
    inline bool contains(const SGRect &r, bool proper = false);
    friend SG_CONSTEXPR inline bool operator==(const SGRect &, const SGRect &) noexcept;
    friend SG_CONSTEXPR inline bool operator!=(const SGRect &, const SGRect &) noexcept;
    friend SGDebug& operator<<(SGDebug& os, const SGRect& o);
private:
    int x1;
    int y1;
    int x2;
    int y2;
};
inline SGDebug& operator<<(SGDebug& os, const SGRect& o)
{
    os<<"{R "<<o.x()<<","<<o.y()<<","<<o.width()<<","<<o.height()<<"}";
    return os;
}
SG_CONSTEXPR inline bool operator==(const SGRect &r1, const SGRect &r2) noexcept
{
    return r1.x1==r2.x1 && r1.x2==r2.x2 && r1.y1==r2.y1 && r1.y2==r2.y2;
}

SG_CONSTEXPR inline bool operator!=(const SGRect &r1, const SGRect &r2) noexcept
{
    return r1.x1!=r2.x1 || r1.x2!=r2.x2 || r1.y1!=r2.y1 || r1.y2!=r2.y2;
}

SG_CONSTEXPR inline bool SGRect::isEmpty() const
{ return x1 > x2 || y1 > y2; }

SG_CONSTEXPR inline int SGRect::x() const
{ return x1; }

SG_CONSTEXPR inline int SGRect::y() const
{ return y1; }

SG_CONSTEXPR inline int SGRect::left() const
{ return x1; }

SG_CONSTEXPR inline int SGRect::top() const
{ return y1; }

SG_CONSTEXPR inline int SGRect::right() const
{ return x2; }

SG_CONSTEXPR inline int SGRect::bottom() const
{ return y2; }
SG_CONSTEXPR inline int SGRect::width() const
{ return x2 - x1; }
SG_CONSTEXPR inline int SGRect::height() const
{ return y2 - y1; }

inline SGRect SGRect::translated(int dx, int dy) const
{ return SGRect(x1+dx, y1+dy, x2-x1, y2-y1); }

inline void SGRect::translate(int dx, int dy)
{
    x1 += dx;
    y1 += dy;
    x2 += dx;
    y2 += dy;
}
inline bool SGRect::contains(const SGRect &r, bool proper)
{
    if (!proper) {
        if ((x1 <= r.x1) &&
            (x2 >= r.x2) &&
            (y1 <= r.y1) &&
            (y2 >= r.y2))
            return true;
        return false;
    }else {
        if ((x1 < r.x1) &&
            (x2 > r.x2) &&
            (y1 < r.y1) &&
            (y2 > r.y2))
            return true;
        return false;
    }

}
SG_CONSTEXPR inline SGRect::SGRect(int left, int top, int width, int height):
    x1(left), y1(top), x2(width + left), y2(height + top){}


#endif // SGRECT_H
