#ifndef LOTTIEMODEL_H
#define LOTTIEMODEL_H

#include<vector>
#include<unordered_map>
#include"sgpoint.h"
#include"sgrect.h"


class LottieComposition;
class LottieLayer;
class LottieTransform;
class LottieShapeGroup;
class LottieShapeObject;
class LottieRectObject;
class LottieEllipseObject;
class LottieTrimObject;
class LottieRepeaterObject;
class LottieFillObject;
class LottieStrokeObject;
class LottieGroupObject;

class LottieObjectVisitor
{
public:
    virtual ~LottieObjectVisitor() {}
    virtual void visit(LottieComposition *) = 0;
    virtual void visit(LottieLayer *) = 0;
    virtual void visit(LottieTransform *) = 0;
    virtual void visit(LottieShapeGroup *) = 0;
    virtual void visit(LottieShapeObject *) = 0;
    virtual void visit(LottieRectObject *) = 0;
    virtual void visit(LottieEllipseObject *) = 0;
    virtual void visit(LottieTrimObject *) = 0;
    virtual void visit(LottieRepeaterObject *) = 0;
    virtual void visit(LottieFillObject *) = 0;
    virtual void visit(LottieStrokeObject *) = 0;
    virtual void visitChildren(LottieGroupObject *) = 0;
};

class LottieColor
{
public:
    float r;
    float g;
    float b;
};

class LottieInterpolater
{
public:
  SGPointF mInTangent;
  SGPointF mOutTangent;
};

template<typename T>
class LottieKeyFrame
{
public:
    LottieKeyFrame():mStartValue(),
                     mEndValue(),
                     mStartFrame(0),
                     mEndFrame(0),
                     mInterpolator(nullptr),
                     mInTangent(),
                     mOutTangent(),
                     mPathKeyFrame(false){}
public:
    T                   mStartValue;
    T                   mEndValue;
    int                 mStartFrame;
    int                 mEndFrame;
    LottieInterpolater *mInterpolator;

    /* this is for interpolating position along a path
     * Need to move to other place because its only applicable
     * for positional property.
     */
    SGPointF            mInTangent;
    SGPointF            mOutTangent;
    bool                mPathKeyFrame;
};

template<typename T>
class LottieAnimInfo
{
public:
    std::vector<LottieKeyFrame<T>> mKeyFrames;
};

template<typename T>
class LottieAnimatable
{
public:
    LottieAnimatable():mValue(),mAnimInfo(nullptr){}
    LottieAnimatable(const T &value): mValue(value), mAnimInfo(nullptr){}
    constexpr bool isStatic() const {return ((mAnimInfo==nullptr) ? true : false);}
public:
    T                     mValue;
    LottieAnimInfo<T>    *mAnimInfo;
    int                   mPropertyIndex; /* "ix" */
};

enum class LottieBlendMode
{
    Normal = 0,
    Multiply = 1,
    Screen = 2,
    OverLay = 3
};

class LottieObjectVisitor;
class LottieObject
{
public:
    enum class Type {
        Composition = 1,
        Layer,
        ShapeGroup,
        Transform,
        Fill,
        Stroke,
        GFill,
        GStroke,
        Rect,
        Ellipse,
        Shape,
        Star,
        Trim,
        Repeater
    };
    virtual void accept(LottieObjectVisitor *){}
    virtual ~LottieObject(){}
    LottieObject(LottieObject::Type  type): mType(type){}
    bool isStatic();
public:
    LottieObject::Type  mType;
};

class LottieGroupObject: public LottieObject
{
public:
    LottieGroupObject(LottieObject::Type  type):LottieObject(type){}
public:
    std::vector<LottieObject *> mChildren;
};

class LottieShapeGroup : public LottieGroupObject
{
public:
    void accept(LottieObjectVisitor *visitor) override
    {visitor->visit(this); visitor->visitChildren(this);}

    LottieShapeGroup():LottieGroupObject(LottieObject::Type::ShapeGroup){}
};

class LottieTransform;
class LottieComposition : public LottieGroupObject
{
public:
    void accept(LottieObjectVisitor *visitor) override
    {visitor->visit(this); visitor->visitChildren(this);}
    LottieComposition():LottieGroupObject(LottieObject::Type::Composition){}
public:
    SGRect               mBound;
    bool                 mAnimation = false;
    long                 mStartFrame = 0;
    long                 mEndFrame = 0;
    float                mFrameRate;
    long                 mStartTime;
    LottieBlendMode      mBlendMode;
    float                mTimeStreatch;
    std::unordered_map<std::string, LottieInterpolater*> mInterpolatorCache;
};

class LottieLayer : public LottieGroupObject
{
public:
    void accept(LottieObjectVisitor *visitor) override
    {visitor->visit(this); visitor->visitChildren(this);}
    LottieLayer():LottieGroupObject(LottieObject::Type::Layer),mParentId(-1),mId(-1){}

public:
    SGRect               mBound;
    int                  mlayerType; //lottie layer type  (solid/shape/precomp)
    int                  mParentId; // Lottie the id of the parent in the composition
    int                  mId;  // Lottie the group id  used for parenting.
    int                  mGroupType; //lottie layer type  (solid/shape/precomp)
    long                 mStartFrame = 0;
    long                 mEndFrame = 0;
    long                 mStartTime;
    LottieBlendMode      mBlendMode;
    float                mTimeStreatch;
    LottieTransform     *mTransform;
};

class LottieTransform : public LottieObject
{
public:
    void accept(LottieObjectVisitor *visitor) final
    {visitor->visit(this);}
    LottieTransform():LottieObject(LottieObject::Type::Transform),
                      mRotation(0),
                      mScale(SGPointF(100, 100)),
                      mPosition(SGPointF(0, 0)),
                      mAnchor(SGPointF(0, 0)),
                      mOpacity(100),
                      mSkew(0),
                      mSkewAxis(0){}
public:
    LottieAnimatable<float>     mRotation;  /* "r" */
    LottieAnimatable<SGPointF>  mScale;     /* "s" */
    LottieAnimatable<SGPointF>  mPosition;  /* "p" */
    LottieAnimatable<SGPointF>  mAnchor;    /* "a" */
    LottieAnimatable<float>     mOpacity;   /* "o" */
    LottieAnimatable<float>     mSkew;      /* "sk" */
    LottieAnimatable<float>     mSkewAxis;  /* "sa" */
};

class LottieFillObject : public LottieObject
{
public:
    void accept(LottieObjectVisitor *visitor) final
    {visitor->visit(this);}
    LottieFillObject():LottieObject(LottieObject::Type::Fill){}
public:
    LottieAnimatable<LottieColor>     mColor;   /* "c" */
    LottieAnimatable<int>             mOpacity;  /* "o" */
    bool                              mEnabled = true; /* "fillEnabled" */
};

class LottieStrokeObject : public LottieObject
{
public:
    void accept(LottieObjectVisitor *visitor) final
    {visitor->visit(this);}
    LottieStrokeObject():LottieObject(LottieObject::Type::Stroke){}
public:
    LottieAnimatable<LottieColor>     mColor;      /* "c" */
    LottieAnimatable<int>             mOpacity;    /* "o" */
    LottieAnimatable<float>           mWidth;      /* "w" */
    CapStyle                          mCapStyle;   /* "lc" */
    JoinStyle                         mJoinStyle;  /* "lj" */
    float                             mMeterLimit; /* "ml" */
    bool                              mEnabled = true;    /* "fillEnabled" */
};

class LottieShape
{
public:
    void process(bool closed =false);
    std::vector<SGPointF>    mInPoint;
    std::vector<SGPointF>    mOutPoint;
    std::vector<SGPointF>    mVertices;
    std::vector<SGPointF>    mPoints;
    int                      mSegments;
};

class LottieShapeObject : public LottieObject
{
public:
    void accept(LottieObjectVisitor *visitor) final
    {visitor->visit(this);}
    void process();
    LottieShapeObject():LottieObject(LottieObject::Type::Shape){}
public:
    LottieAnimatable<LottieShape>    mShape;
    bool                             mClosed = false;
};

class LottieRectObject : public LottieObject
{
public:
    void accept(LottieObjectVisitor *visitor) final
    {visitor->visit(this);}
    LottieRectObject():LottieObject(LottieObject::Type::Rect),
                       mPos(SGPointF(0,0)),
                       mSize(SGPointF(0,0)),
                       mRound(0){}
public:
    LottieAnimatable<SGPointF>    mPos;
    LottieAnimatable<SGPointF>    mSize;
    LottieAnimatable<float>       mRound;
};

class LottieEllipseObject : public LottieObject
{
public:
    void accept(LottieObjectVisitor *visitor) final
    {visitor->visit(this);}
    LottieEllipseObject():LottieObject(LottieObject::Type::Ellipse),
                          mPos(SGPointF(0,0)),
                          mSize(SGPointF(0,0)){}
public:
    LottieAnimatable<SGPointF>   mPos;
    LottieAnimatable<SGPointF>   mSize;
};

class LottieTrimObject : public LottieObject
{
public:
    void accept(LottieObjectVisitor *visitor) final
    {visitor->visit(this);}
    enum class TrimType {
        Simultaneously,
        Individually
    };
    LottieTrimObject():LottieObject(LottieObject::Type::Trim),
                       mStart(0),
                       mEnd(0),
                       mOffset(0),
                       mTrimType(TrimType::Simultaneously){}
public:
    LottieAnimatable<float>             mStart;
    LottieAnimatable<float>             mEnd;
    LottieAnimatable<float>             mOffset;
    LottieTrimObject::TrimType          mTrimType;
};

class LottieRepeaterObject : public LottieObject
{
public:
    void accept(LottieObjectVisitor *visitor) final
    {visitor->visit(this);}
    LottieRepeaterObject():LottieObject(LottieObject::Type::Repeater),
                           mCopies(0),
                           mOffset(0),
                           mTransform(nullptr){}
public:
    LottieAnimatable<float>             mCopies;
    LottieAnimatable<float>             mOffset;
    LottieTransform                    *mTransform;
};



#endif // LOTTIEMODEL_H
