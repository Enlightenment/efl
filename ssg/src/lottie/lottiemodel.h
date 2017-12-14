#ifndef LOTTIEMODEL_H
#define LOTTIEMODEL_H

#include<vector>
#include"sgpoint.h"

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
    constexpr bool isStatic() const {return false;}
public:
    std::vector<LottieKeyFrame<T>> mKeyFrames;
};

template<typename T>
class LottieProperty<T, false>
{
public:
    LottieProperty(){}
    LottieProperty<T,false>(T initialvalue): mValue(initialvalue){}
    constexpr bool isStatic() const {return true;}
public:
    T     mValue;
};

// Template aliasing for easy of use.
template<bool animation>
using LottieIntProperty = LottieProperty<int, animation>;

template<bool animation>
using LottieFloatProperty = LottieProperty<float, animation>;

template<bool animation>
using LottiePointFProperty = LottieProperty<SGPointF, animation>;

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

class LottieGroup : public LottieObject
{
public:
    std::vector<LottieObject *> mChildren;
    int                         mParentId; // Lottie the id of the parent in the composition
    int                         mId;  // Lottie the group id  used for parenting.
};

//class LottieComposition : public LottieGroupObject
//{
//public:

//};

class LottieLayer : public LottieGroup
{
public:
    int         mlayerType; //lottie layer type  (solid/shape/precomp)
};

class LottieMatrix : public LottieGroup
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
class LottieRect : public LottieObject
{
public:
    LottiePointFProperty<pos>       mPos;
    LottiePointFProperty<size>      mSize;
    LottieFloatProperty<roundness>  mRound;
};



#endif // LOTTIEMODEL_H
