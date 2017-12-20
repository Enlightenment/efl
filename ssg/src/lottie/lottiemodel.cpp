#include "lottiemodel.h"
#include<stack>


class LottieRepeaterProcesser : public LottieObjectVisitor
{
public:
    LottieRepeaterProcesser():mRepeaterFound(false){}
    void visit(LottieComposition *obj) {}
    void visit(LottieLayer *obj) {}
    void visit(LottieTransform *) {}
    void visit(LottieShapeGroup *obj) {}
    void visit(LottieShapeObject *) {}
    void visit(LottieRectObject *) {}
    void visit(LottieEllipseObject *) {}
    void visit(LottieTrimObject *) {}
    void visit(LottieRepeaterObject *) { mRepeaterFound = true;}
    void visit(LottieFillObject *) {}
    void visit(LottieStrokeObject *) {}
    void visitChildren(LottieGroupObject *obj) {
        for(auto child :obj->mChildren) {
            child.get()->accept(this);
            if (mRepeaterFound) {
                LottieRepeaterObject *repeater = static_cast<LottieRepeaterObject *>(child.get());
                std::shared_ptr<LottieShapeGroup> sharedShapeGroup= std::make_shared<LottieShapeGroup>();
                LottieShapeGroup *shapeGroup = sharedShapeGroup.get();
                repeater->mChildren.push_back(sharedShapeGroup);
                // copy all the child of the object till repeater and
                // move that in to a group and then add that group to
                // the repeater object.
                for(auto cpChild :obj->mChildren) {
                    if (cpChild == child)
                        break;
                    // we shouldn't copy the trim as trim operation is
                    // already applied to the objects.
                    if (cpChild.get()->type() == LottieObject::Type::Trim)
                        continue;
                    shapeGroup->mChildren.push_back(cpChild);
                }
                mRepeaterFound = false;
            }
        }
    }
public:
    bool mRepeaterFound;
};

class LottiePathOperationProcesser : public LottieObjectVisitor
{
public:
    LottiePathOperationProcesser():mPathOperator(false), mPaintNode(false), mPathOperationCount(0){}
    void visit(LottieComposition *obj) {}
    void visit(LottieLayer *obj) {}
    void visit(LottieTransform *) {}
    void visit(LottieShapeGroup *obj) {}
    void visit(LottieShapeObject *) {mPaintNode = true;}
    void visit(LottieRectObject *) {mPaintNode = true;}
    void visit(LottieEllipseObject *) { mPaintNode = true;}
    void visit(LottieTrimObject *) {
        mPathOperator = true;
    }
    void visit(LottieRepeaterObject *) {}
    void visit(LottieFillObject *) {}
    void visit(LottieStrokeObject *) {}
    void visitChildren(LottieGroupObject *obj) {
        mPathOperator = false;
        mPaintNode = false;
        mPathOperationList.push_back(std::vector<std::shared_ptr<LottieObject>>());
        for (auto i = obj->mChildren.rbegin(); i != obj->mChildren.rend(); ++i) {
            auto child = *i;
            child.get()->accept(this);
            if (mPathOperator) {
                mPathOperationList.back().push_back(child);
                mPathOperationCount++;
            }
            if (mPaintNode && mPathOperationCount) {
               // put it in the list
               updatePaintObject(static_cast<LottieDrawableObject *>(child.get()));
            }
            mPathOperator = false;
            mPaintNode = false;
        }
        mPathOperationCount -= mPathOperationList.back().size();
        mPathOperationList.pop_back();
    }

    void updatePaintObject(LottieDrawableObject *drawable) {
        sgDebug<<"For each path START";
        for (auto i = mPathOperationList.rbegin(); i != mPathOperationList.rend(); ++i) {
            auto trimList = *i;
            sgDebug<<"For each level START";
            for (auto j = trimList.rbegin(); j != trimList.rend(); ++j) {
                drawable->mPathOperations.push_back(*j);
                sgDebug<<"Added path operaton to ";
            }
            sgDebug<<"For each level END";
        }
        sgDebug<<"For each path END";
    }
public:
    bool mPathOperator;
    bool mPaintNode;
    std::vector<std::vector<std::shared_ptr<LottieObject>>> mPathOperationList;
    int mPathOperationCount;
};


void LottieComposition::processRepeaterObjects()
{
    LottieRepeaterProcesser visitor;
    accept(&visitor);
}

void LottieComposition::processPathOperatorObjects()
{
    LottiePathOperationProcesser visitor;
    accept(&visitor);
}

/*
 * makes a deep copy of the object as well as all its children
 *
 */
LottieGroupObject::LottieGroupObject(const LottieGroupObject &other):LottieObject(other.mType)
{
    sgDebug<<"We Shouldn't come here ************************";
    for(auto child: other.mChildren) {
        mChildren.push_back(child);
    }
}

/*
 * Convert the AE shape format to
 * list of bazier curves
 */
void LottieShapeObject::process()
{
    if (mShape.isStatic()) {
        mShape.mValue.process(mClosed);
    } else {
         for (auto &keyframe : mShape.mAnimInfo.get()->mKeyFrames) {
            keyframe.mStartValue.process(mClosed);
            keyframe.mEndValue.process(mClosed);
        }
    }
}

/*
 * Convert the AE shape format to
 * list of bazier curves
 * The final structure will be M+size*C
 */
void LottieShapeData::process(bool closed)
{
    if (mInPoint.size() != mOutPoint.size() ||
        mInPoint.size() != mVertices.size()) {
        sgCritical<<"The Shape data are corrupted";
        mInPoint = std::vector<SGPointF>();
        mOutPoint = std::vector<SGPointF>();
        mVertices = std::vector<SGPointF>();
    }
    int size = mVertices.size();
    mPoints.reserve(3*size + 4);
    mPoints.push_back(mVertices[0]);
    for (int i =1; i <size ; i++ ) {
        mPoints.push_back(mVertices[i-1] + mOutPoint[i-1]); // CP1 = start + outTangent
        mPoints.push_back(mVertices[i] + mInPoint[i]); // CP2 = end + inTangent
        mPoints.push_back(mVertices[i]); //end point
    }

    if (closed) {
        mPoints.push_back(mVertices[size-1] + mOutPoint[size-1]); // CP1 = start + outTangent
        mPoints.push_back(mVertices[0] + mInPoint[0]); // CP2 = end + inTangent
        mPoints.push_back(mVertices[0]); //end point
    }

    // below data no more needed
    mInPoint = std::vector<SGPointF>();
    mOutPoint = std::vector<SGPointF>();
    mVertices = std::vector<SGPointF>();
}

void LottieShape::process(bool closed)
{
    if (mShapeData)
        mShapeData.get()->process(closed);
}

