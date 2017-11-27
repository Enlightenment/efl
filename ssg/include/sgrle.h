#ifndef SGRLE_H
#define SGRLE_H
#include<sgglobal.h>
#include<sgrect.h>
#include<sgpoint.h>

struct SGRleData;
class SGRle
{
public:
    struct Span
    {
      short  x;
      short  y;
      ushort len;
      ushort coverage;
    };
    ~SGRle();
    SGRle();
    SGRle(const SGRle &other);
    SGRle(SGRle &&other);
    SGRle &operator=(const SGRle &);
    SGRle &operator=(SGRle &&other);
    bool isEmpty()const;
    SGRect boundingRect() const;
    void addSpan(const SGRle::Span *span, int count);
    bool operator ==(const SGRle &other) const;
    void translate(const SGPoint &p);
    void translate(int x, int y);
    SGRle intersected(const SGRect &r) const;
    SGRle intersected(const SGRle &other) const;
    SGRle &intersect(const SGRect &r);
    friend SGDebug& operator<<(SGDebug& os, const SGRle& object);
private:
    SGRle copy() const;
    void detach();
    void cleanUp(SGRleData *x);
    SGRleData *d;
};
inline void SGRle::translate(int x, int y)
{
    translate(SGPoint(x,y));
}
#endif // SGRLE_H
