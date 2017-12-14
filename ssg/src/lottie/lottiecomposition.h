#ifndef LOTTIECOMPOSITION_H
#define LOTTIECOMPOSITION_H

#include"rapidjson/document.h"
#include"sgrect.h"
#include"sgpoint.h"
#include"sgdebug.h"
#include<vector>

#include"lottiemodel.h"

// Format Indipendent class to generate data after parsing svg file

struct FIDefNode;
struct FIGradinet;
struct FIColor
{
    int red;
    int green;
    int blue;
};

struct FIPaint
{
   FIColor       mColor;
   bool          mNone;
   bool          mCurColor;
   FIGradinet   *mGradient;
   std::string   mRef;
};

struct FIFill
{
    enum class Mode {
        OddEven,
        Winding
    };
    enum class Flag {
        Paint    = 0x1,
        Opacity  = 0x2,
        Gradient = 0x4,
        FillRule = 0x8
    };
    FIFill::Flag   mFlag;
    FIPaint        mPaint;
    int            mOpacity;
    FIFill::Mode   mMode;
};

struct FIStroke
{
    struct Dash {

    };
    enum class Cap {
      Butt,
      Round,
      Square
    };
    enum class Join {
      Bevel,
      Round,
      Meter
    };
    enum class Flag {
        Paint    = 0x1,
        Opacity  = 0x2,
        Gradient = 0x4,
        Scale    = 0x8,
        Width    = 0x10,
        Cap      = 0x20,
        Join     = 0x40,
        Dash     = 0x80
    };
   FIStroke::Flag       mFlags;
   FIPaint              mPaint;
   int                  mOpacity;
   float                mScale;
   float                mWidth;
   float                mCentered;
   FIStroke::Cap        mCap;
   FIStroke::Join       mJoin;
   FIStroke::Dash      *mDash;
};

class FINodeProperty
{
    FIFill     mfill;
    FIStroke   mstroke;
};
struct FINode
{
    ~FINode(){}
    FINode(FINode *parent = nullptr) {
        if (parent) {
            mParent = parent;
            mParent->mChildren.push_back(this);
        }
    }
    enum class Type {
        Doc,
        Group,
        Def,
        Ellipse,
        Circle,
        Rect,
        Line,
        Path
    };
    FINode::Type           mType;
    FINode                *mParent;
    std::string            name;
    FINodeProperty        *property;
    std::vector<FINode *>  mChildren;

};

struct FIGroupNode : public FINode
{
    FIGroupNode(FINode *parent = nullptr):FINode(parent) { mType = FINode::Type::Group;}
    int                    mParentId; // Lottie the id of the parent in the composition
    int                    mId;  // Lottie the group id  used for parenting.
    int                    mGroupType; //lottie layer type  (solid/shape/precomp)
};

struct FIDocNode : public FINode
{
    FIDocNode(FINode *parent = nullptr):FINode(parent) { mType = FINode::Type::Doc;}
    SGRect      mBound;
    FIDefNode  *mDef;
    bool        mpreserveAspect = false;
    bool        mAnimation = false;
    float       mStartTime = 0;
    float       mEndTime = 0;
    float       mFrameRate;
};

struct FIEllipseNode : public FINode
{
    FIEllipseNode(FINode *parent = nullptr):FINode(parent) { mType = FINode::Type::Ellipse;}
    SGPointF mCenter;
    float    mRadiusX;
    float    mRadiusY;
};

struct FICircleNode : public FINode
{
    FICircleNode(FINode *parent = nullptr):FINode(parent) { mType = FINode::Type::Circle;}
    SGPointF mCenter;
    float    mRadius;
};

struct FIRectNode : public FINode
{
    FIRectNode(FINode *parent = nullptr):FINode(parent) { mType = FINode::Type::Rect;}
    SGRect  mRect;
    float   mRadiusX;
    float   mRadiusY;
};

template<bool sizeAnim>
struct LNode : public FINode
{
    LottieIntProperty<sizeAnim> mSizeProperty;
};

struct FILineNode : public FINode
{
    FILineNode(FINode *parent = nullptr):FINode(parent) { mType = FINode::Type::Line;}
    SGPointF mStartPt;
    SGPointF mEndPt;
};

struct FIPathNode : public FINode
{
    FIPathNode(FINode *parent = nullptr):FINode(parent) { mType = FINode::Type::Path;}
    std::vector<float> mPoints;
};

struct FIDefNode : public FINode
{
    FIDefNode(FINode *parent = nullptr):FINode(parent) { mType = FINode::Type::Def;}
    std::vector<FIGradinet> mGradients;
};

struct FIGradinet
{
    enum class Type {
        Linear,
        Radial
    };
    enum class Spread {
        Pad,
        Repeat,
        Reflect
    };

   FIGradinet::Type     mType;
   std::string          mId;
   bool                 mUserSpace;
   FIGradinet::Spread   mSpread;
   std::vector<float>   mStops;
};

struct FILinearGradinet : FIGradinet
{
    FILinearGradinet()
    {
        mType = FIGradinet::Type::Linear;
    }
    SGPointF mStartPt;
    SGPointF mEndPt;
};

struct FIRadialGradinet : FIGradinet
{
    FIRadialGradinet()
    {
        mType = FIGradinet::Type::Radial;
    }
    SGPointF mCenter;
    SGPointF mFocal;
    float    mRadius;
};


using namespace rapidjson;

class LottieCompositionData;
class LottieComposition
{
public:
    static LottieComposition fromJsonSync(JsonDoc &json);
    ~LottieComposition();
    LottieComposition(const LottieComposition &path);
    LottieComposition(LottieComposition &&other);
    LottieComposition &operator=(const LottieComposition &);
    LottieComposition &operator=(LottieComposition &&other);

    long getStartFrame() const;
    long getEndFrame() const;
    SGRect getBounds() const;
    float getDpScale() const;
    int getMajorVersion() const;
    int getMinorVersion() const;
    int getPatchVersion() const;
    long getDuration() const;

    friend SGDebug& operator<<(SGDebug& os, const LottieComposition& o);
private:
    void cleanUp(LottieCompositionData *x);
    LottieComposition();
    friend class LottieCompositionData;
    LottieCompositionData *d;
};

inline SGDebug& operator<<(SGDebug& os, const LottieComposition& o)
{
    os<<"{startFrame:"<<o.getStartFrame()<<", endFrame: "<<o.getEndFrame()<<", Bound :"<<o.getBounds()<<", Duration:"<<o.getDuration()<<"}";
    return os;
}

#endif // LOTTIECOMPOSITION_H
