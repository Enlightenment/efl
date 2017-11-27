#ifndef SGREGION_H
#define SGREGION_H
#include"sgdebug.h"
#include <sgglobal.h>
#include<sgrect.h>
#include<sgpoint.h>
#include<utility>

typedef struct pixman_region  region_type_t;
typedef region_type_t SGRegionPrivate;

class SG_EXPORT SGRegion
{
public:
    SGRegion();
    SGRegion(int x, int y, int w, int h);
    SGRegion(const SGRect &r);
    SGRegion(const SGRegion &region);
    SGRegion(SGRegion &&other): d(other.d) { other.d = const_cast<SGRegionData*>(&shared_empty); }
    ~SGRegion();
    SGRegion &operator=(const SGRegion &);
    inline SGRegion &operator=(SGRegion &&other)
    { std::swap(d, other.d); return *this; }
    bool isEmpty() const;
    bool contains(const SGRect &r) const;
    SGRegion united(const SGRect &r) const;
    SGRegion united(const SGRegion &r) const;
    SGRegion intersected(const SGRect &r) const;
    SGRegion intersected(const SGRegion &r) const;
    SGRegion subtracted(const SGRegion &r) const;
    void translate(const SGPoint &p);
    inline void translate(int dx, int dy);
    SGRegion translated(const SGPoint &p) const;
    inline SGRegion translated(int dx, int dy) const;
    int rectCount() const;
    SGRect rectAt(int index) const;

    SGRegion operator+(const SGRect &r) const;
    SGRegion operator+(const SGRegion &r) const;
    SGRegion operator-(const SGRegion &r) const;
    SGRegion& operator+=(const SGRect &r);
    SGRegion& operator+=(const SGRegion &r);
    SGRegion& operator-=(const SGRegion &r);

    SGRect boundingRect() const noexcept;
    bool intersects(const SGRegion &region) const;

    bool operator==(const SGRegion &r) const;
    inline bool operator!=(const SGRegion &r) const { return !(operator==(r)); }
    friend SGDebug& operator<<(SGDebug& os, const SGRegion& o);
private:
    bool within(const SGRect &r) const;
    SGRegion copy() const;
    void detach();

    struct SGRegionData {
        RefCount ref;
        SGRegionPrivate *rgn;
    };

    struct SGRegionData *d;
    static const struct SGRegionData shared_empty;
    static void cleanUp(SGRegionData *x);
};
inline void SGRegion::translate(int dx, int dy)
{
    translate(SGPoint(dx,dy));
}

inline SGRegion SGRegion::translated(int dx, int dy) const
{
    return translated(SGPoint(dx,dy));
}
#endif //SGREGION_H
