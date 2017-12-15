#include "lottiemodel.h"

/*
 * Convert the AE shape format to
 * list of bazier curves
 */
void LottieShapeObject::process()
{
    if (mShape.isStatic()) {
        mShape.mValue.process(mClosed);
    } else {
         for (auto &keyframe : mShape.mAnimInfo->mKeyFrames) {
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
void LottieShape::process(bool closed)
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
