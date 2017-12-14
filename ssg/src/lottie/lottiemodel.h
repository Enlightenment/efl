#ifndef LOTTIEMODEL_H
#define LOTTIEMODEL_H

#include<vector>
#include"sgpoint.h"
#include"sgrect.h"

template<typename T>
class LottieKeyFrame
{
public:
    T     mStartValue;
    T     mEndValue;
    int   mStartFrame;
    int   mEndFrame;
};

template<typename T, bool animation = true>
class LottieProperty
{
public:
    LottieProperty(){}
    constexpr bool staticType() const {return false;}
    constexpr bool isStatic() const {return mKeyFrames.empty();}
public:
    T     mValue;
    std::vector<LottieKeyFrame<T>> mKeyFrames;
};

template<typename T>
class LottieProperty<T, false>
{
public:
    LottieProperty(){}
    LottieProperty<T,false>(T initialvalue): mValue(initialvalue){}
    constexpr bool staticType() const {return true;}
    constexpr bool isStatic() const {return true;}
public:
    T     mValue;
};

// Template aliasing for easy of use.
template<bool animation>
using LottieIntProperty = LottieProperty<int, animation>;

template<bool animation>
using LottieFloatProperty = LottieProperty<float, animation>;

template <typename T>
struct LottiePropertyHelper
{
  LottiePropertyHelper(const T &value):mAnimation(false){ mProperty.mValue = value;}
  bool mAnimation;
  LottieProperty<T,true> mProperty;
};

template<bool animation>
using LottiePointFProperty = LottieProperty<SGPointF, animation>;

enum class LottieBlendMode
{
    Normal = 0,
    Multiply = 1,
    Screen = 2,
    OverLay = 3
};
class LottieObject
{
public:
    enum class Type {
        Composition,
        Layer,
        Group,
        Fill,
        Stroke,
        GFill,
        GStroke,
        Rect,
        Ellipse,
        Shape,
        Star
    };

    bool isStatic();
public:
    LottieObject::Type  mType;
};

class LottieGroupObj : public LottieObject
{
public:
    std::vector<LottieObject *> mChildren;
    int                         mParentId; // Lottie the id of the parent in the composition
    int                         mId;  // Lottie the group id  used for parenting.
};

class LottieComposition : public LottieGroupObj
{
public:
    SGRect      mBound;
    bool        mpreserveAspect = false;
    bool        mAnimation = false;
    long        mStartFrame = 0;
    long        mEndFrame = 0;
    float       mFrameRate;
};

class LottieLayer : public LottieGroupObj
{
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
};

class LottieMatrix : public LottieGroupObj
{
public:
    LottieFloatProperty<true>   mRotation;  /* "r" */
    LottiePointFProperty<true>  mScale;     /* "s" */
    LottiePointFProperty<true>  mPosition;  /* "p" */
    LottiePointFProperty<true>  mAnchor;    /* "a" */
    LottieFloatProperty<true>   mOpacity;   /* "o" */
    LottiePointFProperty<true>  mSkew;      /* "sk" */
    LottieFloatProperty<true>   mSkewAxis;  /* "sa" */
};

template<bool pos, bool size, bool roundness>
class LottieRectObject : public LottieObject
{
public:
    LottiePointFProperty<pos>       mPos;
    LottiePointFProperty<size>      mSize;
    LottieFloatProperty<roundness>  mRound;
};

template<bool pos, bool size>
class LottieEllipseObject : public LottieObject
{
public:
    LottiePointFProperty<pos>       mPos;
    LottiePointFProperty<size>      mSize;
};



#endif // LOTTIEMODEL_H
