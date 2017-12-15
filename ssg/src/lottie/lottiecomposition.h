#ifndef LOTTIECOMPOSITION_H
#define LOTTIECOMPOSITION_H

#include"rapidjson/document.h"
#include"sgrect.h"
#include"sgpoint.h"
#include"sgdebug.h"
#include<vector>

//using namespace rapidjson;

//class LottieCompositionData;
//class LottieComposition
//{
//public:
//    static LottieComposition fromJsonSync(JsonDoc &json);
//    ~LottieComposition();
//    LottieComposition(const LottieComposition &path);
//    LottieComposition(LottieComposition &&other);
//    LottieComposition &operator=(const LottieComposition &);
//    LottieComposition &operator=(LottieComposition &&other);

//    long getStartFrame() const;
//    long getEndFrame() const;
//    SGRect getBounds() const;
//    float getDpScale() const;
//    int getMajorVersion() const;
//    int getMinorVersion() const;
//    int getPatchVersion() const;
//    long getDuration() const;

//    friend SGDebug& operator<<(SGDebug& os, const LottieComposition& o);
//private:
//    void cleanUp(LottieCompositionData *x);
//    LottieComposition();
//    friend class LottieCompositionData;
//    LottieCompositionData *d;
//};

//inline SGDebug& operator<<(SGDebug& os, const LottieComposition& o)
//{
//    os<<"{startFrame:"<<o.getStartFrame()<<", endFrame: "<<o.getEndFrame()<<", Bound :"<<o.getBounds()<<", Duration:"<<o.getDuration()<<"}";
//    return os;
//}

#endif // LOTTIECOMPOSITION_H
