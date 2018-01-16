#ifndef SGJSON_H
#define SGJSON_H

#include "lottiemodel.h"
class SGJson
{
public:
    SGJson(){}
    SGJson(const char *data);
public:
    std::shared_ptr<LottieComposition> mComposition;
};
#endif // SGJSON_H
