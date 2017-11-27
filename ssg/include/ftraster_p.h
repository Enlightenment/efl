#ifndef FTRASTER_P_H
#define FTRASTER_P_H
#include"sgrle.h"
#include<future>

struct FTOutline;
class SGPath;

struct FTRasterPrivate;
class FTRaster
{
public:
    ~FTRaster();
    static FTRaster &instance()
    {
        static FTRaster Singleton;
        return Singleton;
    }
    FTRaster(const FTRaster &other) = delete;
    FTRaster(FTRaster&&) = delete;
    FTRaster& operator=(FTRaster const&) = delete;
    FTRaster& operator=(FTRaster &&) = delete;

    static FTOutline *toFTOutline(const SGPath &path);
    static void deleteFTOutline(FTOutline *);
    std::future<SGRle> generateFillInfo(const FTOutline *);
    std::future<SGRle> generateStrokeInfo(const FTOutline *, CapStyle cap, JoinStyle join, double width);
private:
    FTRaster();
    FTRasterPrivate *d;
};
#endif // FTRASTER_P_H
