#ifndef SGPATH_H
#define SGPATH_H
#include "sgglobal.h"
#include "sgpoint.h"
#include<vector>

struct SGPathData;
class SGPath
{
public:
    enum class Element : uchar {
        MoveTo,
        LineTo,
        CubicTo,
        Close
    };
    ~SGPath();
    SGPath();
    SGPath(const SGPath &path);
    SGPath(SGPath &&other);
    SGPath &operator=(const SGPath &);
    SGPath &operator=(SGPath &&other);
    bool isEmpty()const;
    void moveTo(const SGPointF &p);
    inline void moveTo(float x, float y);
    void lineTo(const SGPointF &p);
    inline void lineTo(float x, float y);
    void cubicTo(const SGPointF &ctrlPt1, const SGPointF &ctrlPt2, const SGPointF &endPt);
    inline void cubicTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x, float ctrlPt2y,
                        float endPtx, float endPty);
    void close();
    void reserve(int num_elm);
private:
    friend class FTRaster;
    const std::vector<SGPath::Element> &elements() const;
    const std::vector<SGPointF> &points() const;
    int segments() const;
    SGPath copy() const;
    void detach();
    void cleanUp(SGPathData *x);
    SGPathData *d;
};

inline void SGPath::lineTo(float x, float y)
{
    lineTo(SGPointF(x,y));
}

inline void SGPath::moveTo(float x, float y)
{
    moveTo(SGPointF(x,y));
}

inline void SGPath::cubicTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x, float ctrlPt2y,
                    float endPtx, float endPty)
{
      cubicTo(SGPointF(ctrlPt1x, ctrlPt1y), SGPointF(ctrlPt2x, ctrlPt2y), SGPointF(endPtx, endPty));
}

#endif // SGPATH_H
