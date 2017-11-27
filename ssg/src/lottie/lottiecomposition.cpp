#include"lottiecomposition.h"
#include"sgrect.h"

/*
 * The Model class for the lottie file
 * This class is responsible for parsing the lottie file
 * and creating the model data.
 *
 */

class LottieCompositionData
{
public:
    friend class LottieComposition;
    void parseLayers(const JsonObj &json, LottieComposition &composition);
    void fromJsonSync(JsonDoc &json, LottieComposition &comp);
    void init(SGRect bounds, long startFrame, long endFrame, float frameRate,
              float dpScale, int major, int minor, int patch);
    RefCount         ref;
    SGRect           mBounds;
    long             mStartFrame;
    long             mEndFrame;
    float            mFrameRate;
    float            mDpScale;
    /* Bodymovin version */
    int              mMajorVersion;
    int              mMinorVersion;
    int              mPatchVersion;
};

void LottieCompositionData::init(SGRect bounds, long startFrame, long endFrame, float frameRate,
                                 float dpScale, int major, int minor, int patch)
{
    mBounds = bounds;
    mStartFrame = startFrame;
    mEndFrame = endFrame;
    mFrameRate = frameRate;
    mDpScale = dpScale;
    mMajorVersion = major;
    mMinorVersion = minor;
    mPatchVersion = patch;
}

void LottieCompositionData::fromJsonSync(JsonDoc &json, LottieComposition &comp)
{
    SGRect bounds;
    float scale = 1.0;
    int width = json.optInt("w", -1);
    int height = json.optInt("h", -1);

    if (width != -1 && height != -1) {
      int scaledWidth = (int) (width * scale);
      int scaledHeight = (int) (height * scale);
      bounds = SGRect(0, 0, scaledWidth, scaledHeight);
    }

    long startFrame = json.optFloat("ip", 0);
    long endFrame = json.optFloat("op", 0);
    float frameRate = json.optFloat("fr", 0);
    int major= 5, minor = 1, patch = 0;
    //FIXME get the version info from the jason file
    init(bounds, startFrame, endFrame, frameRate, scale, major, minor, patch);

    if (json.HasMember("layers"))
        parseLayers(json["layers"], comp);
}

void LottieCompositionData::parseLayers(const JsonObj &json, LottieComposition &composition)
{
    for(auto &jsonLayer : json.GetArray()) {
        sgDebug<<"layer found";
    }
}
inline void LottieComposition::cleanUp(LottieCompositionData *d)
{
    delete d;
}

LottieComposition::~LottieComposition()
{
    if (!d->ref.deref())
        cleanUp(d);
}

LottieComposition::LottieComposition()
{
    d = new LottieCompositionData();
}

LottieComposition::LottieComposition(const LottieComposition &other)
{
    d = other.d;
    d->ref.ref();
}

LottieComposition::LottieComposition(LottieComposition &&other): d(other.d)
{
    other.d = nullptr;
}

LottieComposition &LottieComposition::operator=(const LottieComposition &other)
{
    other.d->ref.ref();
    if (!d->ref.deref())
        delete d;

    d = other.d;
    return *this;
}

inline LottieComposition &LottieComposition::operator=(LottieComposition &&other)
{
    std::swap(d, other.d); return *this;
}

long LottieComposition::getStartFrame() const
{
    return d->mStartFrame;
}

long LottieComposition::getEndFrame() const
{
    return d->mEndFrame;
}

SGRect LottieComposition::getBounds() const
{
    return d->mBounds;
}

float LottieComposition::getDpScale() const
{
    return d->mDpScale;
}

int LottieComposition::getMajorVersion() const
{
    return d->mMajorVersion;
}

int LottieComposition::getMinorVersion() const
{
    return d->mMinorVersion;
}

int LottieComposition::getPatchVersion() const
{
    return d->mPatchVersion;
}

long LottieComposition::getDuration() const
{
    long frameDuration = d->mEndFrame - d->mStartFrame;
    return (long) (frameDuration / d->mFrameRate * 1000);
}

LottieComposition LottieComposition::fromJsonSync(JsonDoc &json)
{
    LottieComposition composition;
    composition.d->fromJsonSync(json, composition);
    composition.d->ref.setOwned();
    return composition;
}
