#include"sgjson.h"
#include "rapidjson/document.h"
#include <iostream>
#include "lottiecomposition.h"
#include "lottiemodel.h"

RAPIDJSON_DIAG_PUSH
#ifdef __GNUC__
RAPIDJSON_DIAG_OFF(effc++)
#endif

// This example demonstrates JSON token-by-token parsing with an API that is
// more direct; you don't need to design your logic around a handler object and
// callbacks. Instead, you retrieve values from the JSON stream by calling
// GetInt(), GetDouble(), GetString() and GetBool(), traverse into structures
// by calling EnterObject() and EnterArray(), and skip over unwanted data by
// calling SkipValue(). When you know your JSON's structure, this can be quite
// convenient.
//
// If you aren't sure of what's next in the JSON data, you can use PeekType() and
// PeekValue() to look ahead to the next object before reading it.
//
// If you call the wrong retrieval method--e.g. GetInt when the next JSON token is
// not an int, EnterObject or EnterArray when there isn't actually an object or array
// to read--the stream parsing will end immediately and no more data will be delivered.
//
// After calling EnterObject, you retrieve keys via NextObjectKey() and values via
// the normal getters. When NextObjectKey() returns null, you have exited the
// object, or you can call SkipObject() to skip to the end of the object
// immediately. If you fetch the entire object (i.e. NextObjectKey() returned  null),
// you should not call SkipObject().
//
// After calling EnterArray(), you must alternate between calling NextArrayValue()
// to see if the array has more data, and then retrieving values via the normal
// getters. You can call SkipArray() to skip to the end of the array immediately.
// If you fetch the entire array (i.e. NextArrayValue() returned null),
// you should not call SkipArray().
//
// This parser uses in-situ strings, so the JSON buffer will be altered during the
// parse.

using namespace rapidjson;


class LookaheadParserHandler {
public:
    bool Null() { st_ = kHasNull; v_.SetNull(); return true; }
    bool Bool(bool b) { st_ = kHasBool; v_.SetBool(b); return true; }
    bool Int(int i) { st_ = kHasNumber; v_.SetInt(i); return true; }
    bool Uint(unsigned u) { st_ = kHasNumber; v_.SetUint(u); return true; }
    bool Int64(int64_t i) { st_ = kHasNumber; v_.SetInt64(i); return true; }
    bool Uint64(uint64_t u) { st_ = kHasNumber; v_.SetUint64(u); return true; }
    bool Double(double d) { st_ = kHasNumber; v_.SetDouble(d); return true; }
    bool RawNumber(const char*, SizeType, bool) { return false; }
    bool String(const char* str, SizeType length, bool) { st_ = kHasString; v_.SetString(str, length); return true; }
    bool StartObject() { st_ = kEnteringObject; return true; }
    bool Key(const char* str, SizeType length, bool) { st_ = kHasKey; v_.SetString(str, length); return true; }
    bool EndObject(SizeType) { st_ = kExitingObject; return true; }
    bool StartArray() { st_ = kEnteringArray; return true; }
    bool EndArray(SizeType) { st_ = kExitingArray; return true; }

protected:
    LookaheadParserHandler(char* str);
    void ParseNext();

protected:
    enum LookaheadParsingState {
        kInit,
        kError,
        kHasNull,
        kHasBool,
        kHasNumber,
        kHasString,
        kHasKey,
        kEnteringObject,
        kExitingObject,
        kEnteringArray,
        kExitingArray
    };

    Value v_;
    LookaheadParsingState st_;
    Reader r_;
    InsituStringStream ss_;

    static const int parseFlags = kParseDefaultFlags | kParseInsituFlag;
};

LookaheadParserHandler::LookaheadParserHandler(char* str) : v_(), st_(kInit), r_(), ss_(str) {
    r_.IterativeParseInit();
    ParseNext();
}

void LookaheadParserHandler::ParseNext() {
    if (r_.HasParseError()) {
        st_ = kError;
        return;
    }

    if (!r_.IterativeParseNext<parseFlags>(ss_, *this)) {
        sgCritical<<"Lottie file parsing error";
        RAPIDJSON_ASSERT(0);
    }
}

class LottieParser : protected LookaheadParserHandler {
public:
    LottieParser(char* str) : LookaheadParserHandler(str) {}

    bool EnterObject();
    bool EnterArray();
    const char* NextObjectKey();
    bool NextArrayValue();
    int GetInt();
    double GetDouble();
    const char* GetString();
    bool GetBool();
    void GetNull();

    void SkipObject();
    void SkipArray();
    void SkipValue();
    Value* PeekValue();
    int PeekType(); // returns a rapidjson::Type, or -1 for no value (at end of object/array)

    bool IsValid() { return st_ != kError; }

    void Skip(const char *key);
    SGRect getRect();
    LottieBlendMode getBlendMode();
    CapStyle getLineCap();
    JoinStyle getLineJoin();
    LottieTrimObject::TrimType getTrimType();

    LottieComposition *parseComposition();
    void parseLayers(LottieComposition *comp);
    LottieLayer *parseLayer();
    void parseShapesAttr(LottieLayer *layer);
    void parseObject(LottieGroupObject *parent);
    LottieObject* parseObjectTypeAttr();
    LottieObject *parseGroupObject();
    LottieObject *parseRectObject();
    LottieObject *parseEllipseObject();
    LottieObject *parseShapeObject();

    LottieTransform *parseTransformObject();
    LottieObject *parseFillObject();
    LottieObject *parseGradientFillObject();
    LottieObject *parseStrokeObject();
    LottieObject *parseGradientStrokeObject();
    LottieObject *parseTrimObject();
    LottieObject *parseReapeaterObject();

    SGPointF parseInperpolatorPoint();
    void parseArrayValue(SGPointF &pt);
    void parseArrayValue(LottieColor &pt);
    void parseArrayValue(float &val);
    void parseArrayValue(int &val);
    void getValue(SGPointF &val);
    void getValue(float &val);
    void getValue(LottieColor &val);
    void getValue(int &val);
    void getValue(LottieShape &shape);
    template<typename T>
    void parseKeyFrame(LottieAnimInfo<T> &obj);
    template<typename T>
    void parseProperty(LottieAnimatable<T> &obj);

    void parseShapeKeyFrame(LottieAnimInfo<LottieShape> &obj);
    void parseShapeProperty(LottieAnimatable<LottieShape> &obj);
    void parseArrayValue(std::vector<SGPointF> &v);
protected:
    LottieComposition *compRef;
    void SkipOut(int depth);
};

bool LottieParser::EnterObject() {
    if (st_ != kEnteringObject) {
        st_  = kError;
        RAPIDJSON_ASSERT(false);
        return false;
    }

    ParseNext();
    return true;
}

bool LottieParser::EnterArray() {
    if (st_ != kEnteringArray) {
        st_  = kError;
        RAPIDJSON_ASSERT(false);
        return false;
    }

    ParseNext();
    return true;
}

const char* LottieParser::NextObjectKey() {
    if (st_ == kHasKey) {
        const char* result = v_.GetString();
        ParseNext();
        return result;
    }

    /* SPECIAL CASE
     * The parser works with a prdefined rule that it will be only
     * while (NextObjectKey()) for each object but in case of our nested group
     * object we can call multiple time NextObjectKey() while exiting the object
     * so ignore those and don't put parser in the error state.
     * */
    if (st_ == kExitingArray || st_ == kEnteringObject ) {
        sgDebug<<"O: Exiting nested loop";
        return 0;
    }

    if (st_ != kExitingObject) {
        RAPIDJSON_ASSERT(false);
        st_ = kError;
        return 0;
    }

    ParseNext();
    return 0;
}

bool LottieParser::NextArrayValue() {
    if (st_ == kExitingArray) {
        ParseNext();
        return false;
    }

    /* SPECIAL CASE
     * same as  NextObjectKey()
     */
    if (st_ == kExitingObject) {
        sgDebug<<"A: Exiting nested loop";
        return 0;
    }


    if (st_ == kError || st_ == kHasKey) {
        RAPIDJSON_ASSERT(false);
        st_ = kError;
        return false;
    }

    return true;
}

int LottieParser::GetInt() {
    if (st_ != kHasNumber || !v_.IsInt()) {
        st_ = kError;
        RAPIDJSON_ASSERT(false);
        return 0;
    }

    int result = v_.GetInt();
    ParseNext();
    return result;
}

double LottieParser::GetDouble() {
    if (st_ != kHasNumber) {
        st_  = kError;
        RAPIDJSON_ASSERT(false);
        return 0.;
    }

    double result = v_.GetDouble();
    ParseNext();
    return result;
}

bool LottieParser::GetBool() {
    if (st_ != kHasBool) {
        st_  = kError;
        RAPIDJSON_ASSERT(false);
        return false;
    }

    bool result = v_.GetBool();
    ParseNext();
    return result;
}

void LottieParser::GetNull() {
    if (st_ != kHasNull) {
        st_  = kError;
        return;
    }

    ParseNext();
}

const char* LottieParser::GetString() {
    if (st_ != kHasString) {
        st_  = kError;
        RAPIDJSON_ASSERT(false);
        return 0;
    }

    const char* result = v_.GetString();
    ParseNext();
    return result;
}

void LottieParser::SkipOut(int depth) {
    do {
        if (st_ == kEnteringArray || st_ == kEnteringObject) {
            ++depth;
        }
        else if (st_ == kExitingArray || st_ == kExitingObject) {
            --depth;
        }
        else if (st_ == kError) {
            RAPIDJSON_ASSERT(false);
            return;
        }

        ParseNext();
    }
    while (depth > 0);
}

void LottieParser::SkipValue() {
    SkipOut(0);
}

void LottieParser::SkipArray() {
    SkipOut(1);
}

void LottieParser::SkipObject() {
    SkipOut(1);
}

Value* LottieParser::PeekValue() {
    if (st_ >= kHasNull && st_ <= kHasKey) {
        return &v_;
    }

    return 0;
}

int LottieParser::PeekType() {
    if (st_ >= kHasNull && st_ <= kHasKey) {
        return v_.GetType();
    }

    if (st_ == kEnteringArray) {
        return kArrayType;
    }

    if (st_ == kEnteringObject) {
        return kObjectType;
    }

    return -1;
}

void LottieParser::Skip(const char *key)
{
    if (PeekType() == kArrayType) {
        EnterArray();
        SkipArray();
    } else if (PeekType() == kObjectType) {
        EnterObject();
        SkipObject();
    } else {
        SkipValue();
    }
}

LottieBlendMode
LottieParser::getBlendMode()
{
    RAPIDJSON_ASSERT(PeekType() == kNumberType);
    LottieBlendMode mode = LottieBlendMode::Normal;

    switch (GetInt()) {
    case 1:
        mode = LottieBlendMode::Multiply;
        break;
    case 2:
        mode = LottieBlendMode::Screen;
        break;
    case 3:
        mode = LottieBlendMode::OverLay;
        break;
    default:
        break;
    }
    return mode;
}
SGRect LottieParser::getRect()
{
    SGRect r;
    RAPIDJSON_ASSERT(PeekType() == kObjectType);
    EnterObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "l")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            r.setLeft(GetInt());
        } else if (0 == strcmp(key, "r")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            r.setRight(GetInt());
        } else if (0 == strcmp(key, "t")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            r.setTop(GetInt());
        } else if (0 == strcmp(key, "b")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            r.setBottom(GetInt());
        } else {
            RAPIDJSON_ASSERT(false);
        }
    }
    return r;
}

LottieComposition *LottieParser::parseComposition()
{
    RAPIDJSON_ASSERT(PeekType() == kObjectType);
    EnterObject();
    LottieComposition *comp = new LottieComposition();
    compRef = comp;
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "w")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            comp->mBound.setWidth(GetInt());
        } else if (0 == strcmp(key, "h")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            comp->mBound.setHeight(GetInt());
        } else if (0 == strcmp(key, "ip")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            comp->mStartFrame = GetDouble();
        } else if (0 == strcmp(key, "op")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            comp->mEndFrame = GetDouble();
        } else if (0 == strcmp(key, "fr")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            comp->mFrameRate = GetDouble();
        } else if (0 == strcmp(key, "layers")) {
            parseLayers(comp);
        } else {
            sgWarning<<"Composition Attribute Skipped : "<<key;
            Skip(key);
        }
    }
    return comp;
}

void LottieParser::parseLayers(LottieComposition *composition)
{
    RAPIDJSON_ASSERT(PeekType() == kArrayType);
    EnterArray();
    while (NextArrayValue()) {
        auto layer = parseLayer();
        composition->mChildren.push_back(layer);
    }
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/layers/shape.json
 *
 */
LottieLayer * LottieParser::parseLayer()
{
    RAPIDJSON_ASSERT(PeekType() == kObjectType);
    //sgDebug<<"parse LAYER: S";
    LottieLayer *layer = new LottieLayer();
    EnterObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "ty")) {    /* Type of layer: Shape. Value 4.*/
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            layer->mGroupType = GetInt();
        } else if (0 == strcmp(key, "ind")) { /*Layer index in AE. Used for parenting and expressions.*/
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            layer->mId = GetInt();
        } else if (0 == strcmp(key, "parent")) { /*Layer Parent. Uses "ind" of parent.*/
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            layer->mParentId = GetInt();
        }else if (0 == strcmp(key, "sr")) { // "Layer Time Stretching"
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            layer->mTimeStreatch = GetDouble();
        } else if (0 == strcmp(key, "ip")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            layer->mStartFrame = GetDouble();
        } else if (0 == strcmp(key, "op")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            layer->mEndFrame = GetDouble();
        }  else if (0 == strcmp(key, "st")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            layer->mStartTime = GetDouble();
        } else if (0 == strcmp(key, "bounds")) {
            layer->mBound = getRect();
        } else if (0 == strcmp(key, "bm")) {
            layer->mBlendMode = getBlendMode();
        } else if (0 == strcmp(key, "ks")) {
            RAPIDJSON_ASSERT(PeekType() == kObjectType);
            EnterObject();
            layer->mTransform = parseTransformObject();
        } else if (0 == strcmp(key, "shapes")) {
            parseShapesAttr(layer);
        } else {
            sgWarning<<"Layer Attribute Skipped : "<<key;
            Skip(key);
        }
    }
       //sgDebug<<"parse LAYER: E";
  return layer;
}

void LottieParser::parseShapesAttr(LottieLayer *layer)
{
    //sgDebug<<"ENTER SHAPE ATTR";
    RAPIDJSON_ASSERT(PeekType() == kArrayType);
    EnterArray();
    while (NextArrayValue()) {
        parseObject(layer);
    }
   //sgDebug<<"EXIT SHAPE ATTR";
}

LottieObject*
LottieParser::parseObjectTypeAttr()
{
    RAPIDJSON_ASSERT(PeekType() == kStringType);
    const char *type = GetString();
    if (0 == strcmp(type, "gr")) {
        return parseGroupObject();
    } else if (0 == strcmp(type, "rc")) {
        return parseRectObject();
    } else if (0 == strcmp(type, "el")) {
        return parseEllipseObject();
    } else if (0 == strcmp(type, "tr")) {
        return parseTransformObject();
    } else if (0 == strcmp(type, "fl")) {
        return parseFillObject();
    } else if (0 == strcmp(type, "st")) {
        return parseStrokeObject();
    } else if (0 == strcmp(type, "sh")) {
        return parseShapeObject();
    }  else if (0 == strcmp(type, "tm")) {
        return parseTrimObject();
    } else {
        sgDebug<<"The Object Type not yet handled = "<< type;
        return nullptr;
    }
}

void
LottieParser::parseObject(LottieGroupObject *parent)
{
    RAPIDJSON_ASSERT(PeekType() == kObjectType);
    EnterObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "ty")) {
            auto child = parseObjectTypeAttr();
            if (child)
                parent->mChildren.push_back(child);
        } else {
            Skip(key);
        }
    }
}

LottieObject *
LottieParser::parseGroupObject()
{
    LottieShapeGroup *group = new LottieShapeGroup();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "it")) {
            RAPIDJSON_ASSERT(PeekType() == kArrayType);
            EnterArray();
            while (NextArrayValue()) {
                RAPIDJSON_ASSERT(PeekType() == kObjectType);
                parseObject(group);
            }
        } else {
            Skip(key);
        }
    }
  return group;
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/shapes/rect.json
 */
LottieObject *
LottieParser::parseRectObject()
{
    LottieRectObject *obj = new LottieRectObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "p")) {
            parseProperty(obj->mPos);
        } else if (0 == strcmp(key, "s")) {
            parseProperty(obj->mSize);
        } else if (0 == strcmp(key, "r")) {
            parseProperty(obj->mRound);
        }  else if (0 == strcmp(key, "d")) {
            Skip(key);
        } else {
            Skip(key);
        }
    }
    return nullptr;
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/shapes/ellipse.json
 */
LottieObject *
LottieParser::parseEllipseObject()
{
    LottieEllipseObject *obj = new LottieEllipseObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "p")) {
            parseProperty(obj->mPos);
        } else if (0 == strcmp(key, "s")) {
            parseProperty(obj->mSize);
        } else {
            Skip(key);
        }
    }
    return obj;
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/properties/shape.json
 */
LottieObject *
LottieParser::parseShapeObject()
{
    //sgDebug<<"parse Shape START:";
    LottieShapeObject *obj = new LottieShapeObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "ks")) {
            parseShapeProperty(obj->mShape);
        }  else if (0 == strcmp(key, "closed")) {
            obj->mClosed = GetBool();
        } else {
            sgDebug<<"Shape property ignored :"<<key;
            Skip(key);
        }
    }
    //sgDebug<<"parse Shape item END:";
    obj->process();
    return obj;
}

LottieTrimObject::TrimType
LottieParser::getTrimType()
{
    RAPIDJSON_ASSERT(PeekType() == kNumberType);
    switch (GetInt()) {
    case 1:
        return LottieTrimObject::TrimType::Simultaneously;
        break;
    case 2:
        return LottieTrimObject::TrimType::Individually;
        break;
    default:
        RAPIDJSON_ASSERT(0);
        break;
    }
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/shapes/trim.json
 */
LottieObject *
LottieParser::parseTrimObject()
{
    LottieTrimObject *obj = new LottieTrimObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "s")) {
            parseProperty(obj->mStart);
        } else if (0 == strcmp(key, "e")) {
            parseProperty(obj->mEnd);
        } else if (0 == strcmp(key, "o")) {
            parseProperty(obj->mOffset);
        }  else if (0 == strcmp(key, "m")) {
            obj->mTrimType = getTrimType();
        } else {
            sgDebug<<"Trim property ignored :"<<key;
            Skip(key);
        }
    }
    return obj;
}

LottieObject *
LottieParser::parseReapeaterObject()
{
    LottieRepeaterObject *obj = new LottieRepeaterObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "c")) {
            parseProperty(obj->mCopies);
        } else if (0 == strcmp(key, "o")) {
            parseProperty(obj->mOffset);
        } else if (0 == strcmp(key, "tr")) {
            obj->mTransform = parseTransformObject();
        } else {
            sgDebug<<"Repeater property ignored :"<<key;
            Skip(key);
        }
    }
    return obj;
}


/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/shapes/transform.json
 */
LottieTransform *
LottieParser::parseTransformObject()
{
    LottieTransform *obj = new LottieTransform();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "a")) {
            parseProperty(obj->mAnchor);
        } else if (0 == strcmp(key, "p")) {
            parseProperty(obj->mPosition);
        } else if (0 == strcmp(key, "r")) {
            parseProperty(obj->mRotation);
        } else if (0 == strcmp(key, "s")) {
            parseProperty(obj->mScale);
        } else if (0 == strcmp(key, "sk")) {
            parseProperty(obj->mSkew);
        }  else if (0 == strcmp(key, "sa")) {
            parseProperty(obj->mSkewAxis);
        } else if (0 == strcmp(key, "o")) {
            parseProperty(obj->mOpacity);
        } else {
            Skip(key);
        }
    }
    return obj;
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/shapes/fill.json
 */
LottieObject *
LottieParser::parseFillObject()
{
    LottieFillObject *obj = new LottieFillObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "c")) {
            parseProperty(obj->mColor);
        } else if (0 == strcmp(key, "o")) {
            parseProperty(obj->mOpacity);
        } else if (0 == strcmp(key, "fillEnabled")) {
            obj->mEnabled = GetBool();
        } else {
            sgWarning<<"Fill property skipped = "<<key;
            Skip(key);
        }
    }
    return obj;
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/helpers/lineCap.json
 */
CapStyle LottieParser::getLineCap()
{
    RAPIDJSON_ASSERT(PeekType() == kNumberType);
    switch (GetInt()) {
    case 1:
        return CapStyle::Flat;
        break;
    case 2:
        return CapStyle::Round;
        break;
    default:
        return CapStyle::Square;
        break;
    }
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/helpers/lineJoin.json
 */
JoinStyle LottieParser::getLineJoin()
{
    RAPIDJSON_ASSERT(PeekType() == kNumberType);
    switch (GetInt()) {
    case 1:
        return JoinStyle::Miter;
        break;
    case 2:
        return JoinStyle::Round;
        break;
    default:
        return JoinStyle::Bevel;
        break;
    }
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/shapes/stroke.json
 */
LottieObject *
LottieParser::parseStrokeObject()
{
    LottieStrokeObject *obj = new LottieStrokeObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "c")) {
            parseProperty(obj->mColor);
        } else if (0 == strcmp(key, "o")) {
            parseProperty(obj->mOpacity);
        } else if (0 == strcmp(key, "w")) {
            parseProperty(obj->mWidth);
        } else if (0 == strcmp(key, "fillEnabled")) {
            obj->mEnabled = GetBool();
        } else if (0 == strcmp(key, "lc")) {
            obj->mCapStyle = getLineCap();
        } else if (0 == strcmp(key, "lj")) {
            obj->mJoinStyle = getLineJoin();
        } else if (0 == strcmp(key, "ml")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            obj->mMeterLimit = GetDouble();
        } else {
            sgWarning<<"Stroke property skipped = "<<key;
            Skip(key);
        }
    }
    return obj;
}

void LottieParser::parseArrayValue(LottieColor &color)
{
    float val[4];
    int i=0;
    while (NextArrayValue()) {
        val[i++] = GetDouble();
    }

    color.r = val[0];
    color.g = val[1];
    color.b = val[2];
}

void LottieParser::parseArrayValue(SGPointF &pt)
{
    float val[4];
    int i=0;
    while (NextArrayValue()) {
        val[i++] = GetDouble();
    }
//    sgDebug<<"Value parsed as point / size"<<i;

    pt.setX(val[0]);
    pt.setY(val[1]);
}

void LottieParser::parseArrayValue(float &val)
{
    val = GetDouble();
}

void LottieParser::parseArrayValue(int &val)
{
    val = GetInt();
}

void LottieParser::parseArrayValue(std::vector<SGPointF> &v)
{
    RAPIDJSON_ASSERT(PeekType() == kArrayType);
    EnterArray();
    while (NextArrayValue()) {
        RAPIDJSON_ASSERT(PeekType() == kArrayType);
        EnterArray();
        while (NextArrayValue()) {
            v.push_back(SGPointF(GetDouble(), GetDouble()));
        }
    }
}

void LottieParser::getValue(SGPointF &pt)
{
    float val[4];
    int i=0;
    RAPIDJSON_ASSERT(PeekType() == kArrayType);
    EnterArray();
    while (NextArrayValue()) {
        val[i++] = GetDouble();
    }
    pt.setX(val[0]);
    pt.setY(val[1]);
}

void LottieParser::getValue(float &val)
{
    RAPIDJSON_ASSERT(PeekType() == kArrayType);
    EnterArray();
    while (NextArrayValue()) {
        val = GetDouble();
    }
}

void LottieParser::getValue(LottieColor &color)
{
    float val[4];
    int i=0;
    RAPIDJSON_ASSERT(PeekType() == kArrayType);
    EnterArray();
    while (NextArrayValue()) {
        val[i++] = GetDouble();
    }
    color.r = val[0];
    color.g = val[1];
    color.b = val[2];
}

void LottieParser::getValue(int &val)
{
    RAPIDJSON_ASSERT(PeekType() == kNumberType);
    val = GetInt();
}

void LottieParser::getValue(LottieShape &obj)
{
    /*
     * The shape object could be wrapped by a array
     * if its part of the keyframe object
     */
    bool arrayWrapper = (PeekType() == kArrayType);
    if (arrayWrapper)
         EnterArray();

    RAPIDJSON_ASSERT(PeekType() == kObjectType);
    EnterObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "i")) {
            parseArrayValue(obj.mInPoint);
        } else if (0 == strcmp(key, "o")) {
            parseArrayValue(obj.mOutPoint);
        } else if (0 == strcmp(key, "v")) {
            parseArrayValue(obj.mVertices);
        } else {
            RAPIDJSON_ASSERT(0);
            Skip(nullptr);
        }
    }
    // exit properly from the array
    if (arrayWrapper)
        NextArrayValue();
}

SGPointF
LottieParser::parseInperpolatorPoint()
{
    SGPointF cp;
    RAPIDJSON_ASSERT(PeekType() == kObjectType);
    EnterObject();
    while(const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "x")) {
            if (PeekType() == kNumberType) {
                cp.setX(GetDouble());
            } else {
                RAPIDJSON_ASSERT(PeekType() == kArrayType);
                EnterArray();
                while (NextArrayValue()) {
                    cp.setX(GetDouble());
                }
            }
        }
        if (0 == strcmp(key, "y")) {
            if (PeekType() == kNumberType) {
                cp.setY(GetDouble());
            } else {
                RAPIDJSON_ASSERT(PeekType() == kArrayType);
                EnterArray();
                while (NextArrayValue()) {
                    cp.setY(GetDouble());
                }
            }
        }
    }
    return cp;
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/properties/multiDimensionalKeyframed.json
 */
template<typename T>
void LottieParser::parseKeyFrame(LottieAnimInfo<T> &obj)
{
    EnterObject();
    LottieKeyFrame<T> keyframe;
    SGPointF inTangent;
    SGPointF outTangent;
    const char *interpolatorKey = nullptr;
     while (const char* key = NextObjectKey()) {
         if (0 == strcmp(key, "i")) {
             inTangent = parseInperpolatorPoint();
         } else if (0 == strcmp(key, "o")) {
             outTangent = parseInperpolatorPoint();
         } else if (0 == strcmp(key, "n")) {
             if (PeekType() == kStringType) {
                interpolatorKey = GetString();
             } else {
                RAPIDJSON_ASSERT(PeekType() == kArrayType);
                EnterArray();
                while (NextArrayValue()) {
                    RAPIDJSON_ASSERT(PeekType() == kStringType);
                    interpolatorKey = GetString();
                }
             }
             continue;
         } else if (0 == strcmp(key, "t")) {
             keyframe.mStartFrame = GetDouble();
         } else if (0 == strcmp(key, "s")) {
             getValue(keyframe.mStartValue);
             continue;
         } else if (0 == strcmp(key, "e")) {
             getValue(keyframe.mEndValue);
             continue;
         } else if (0 == strcmp(key, "ti")) {
             keyframe.mPathKeyFrame = true;
             getValue(keyframe.mInTangent);
             continue;
         } else if (0 == strcmp(key, "to")) {
             keyframe.mPathKeyFrame = true;
             getValue(keyframe.mOutTangent);
             continue;
         } else {
             sgDebug<<"key frame property skipped = "<<key;
             Skip(key);
         }
     }

     if (!obj.mKeyFrames.empty()) {
         // update the endFrame value of current keyframe
         obj.mKeyFrames.back().mEndFrame = keyframe.mStartFrame;
     }

     // Try to find the interpolator from cache
     if (interpolatorKey) {
         auto search = compRef->mInterpolatorCache.find(interpolatorKey);
         if (search != compRef->mInterpolatorCache.end()) {
             keyframe.mInterpolator = search->second;
         } else {
             keyframe.mInterpolator = new LottieInterpolater();
             keyframe.mInterpolator->mInTangent = inTangent;
             keyframe.mInterpolator->mOutTangent = outTangent;
             compRef->mInterpolatorCache[interpolatorKey] = keyframe.mInterpolator;
         }
     } else {
         /* this is the last key frame just skip it  */
         return;
     }
   obj.mKeyFrames.push_back(keyframe);
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/properties/shapeKeyframed.json
 */

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/properties/shape.json
 */
void
LottieParser::parseShapeProperty(LottieAnimatable<LottieShape> &obj)
{
    EnterObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "k")) {
            if (PeekType() == kArrayType) {
                EnterArray();
                while (NextArrayValue()) {
                    RAPIDJSON_ASSERT(PeekType() == kObjectType);
                    if (!obj.mAnimInfo)
                        obj.mAnimInfo = new LottieAnimInfo<LottieShape>();
                    parseKeyFrame(*obj.mAnimInfo);
                }
            } else {
                getValue(obj.mValue);
            }
        } else {
            sgDebug<<"shape property ignored = "<<key;
            Skip(nullptr);
        }
    }
}

/*
 * https://github.com/airbnb/lottie-web/tree/master/docs/json/properties
 */
template<typename T>
void LottieParser::parseProperty(LottieAnimatable<T> &obj)
{
    EnterObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "k")) {
            if (PeekType() == kNumberType) {
                /*single value property with no animation*/
                parseArrayValue(obj.mValue);
            } else {
                RAPIDJSON_ASSERT(PeekType() == kArrayType);
                EnterArray();
                while (NextArrayValue()) {
                    /* property with keyframe info*/
                    if (PeekType() == kObjectType) {
                        if (!obj.mAnimInfo)
                            obj.mAnimInfo = new LottieAnimInfo<T>();
                        parseKeyFrame(*obj.mAnimInfo);
                    } else {
                        /* Read before modifying.
                         * as there is no way of knowing if the
                         * value of the array is either array of numbers
                         * or array of object without entering the array
                         * thats why this hack is there
                         */
                        RAPIDJSON_ASSERT(PeekType() == kNumberType);
                        /*multi value property with no animation*/
                        parseArrayValue(obj.mValue);
                        /*break here as we already reached end of array*/
                        break;
                    }
                }
            }
        }  else if (0 == strcmp(key, "ix")){
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            obj.mPropertyIndex = GetInt();
        } else {
            Skip(key);
        }
    }
}

class LottieObjectInspector : public LottieObjectVisitor
{
public:
    void visit(LottieComposition *obj) {
        sgDebug<<"[COMP: START]";
    }
    void visit(LottieLayer *obj) {
        sgDebug<<"[LAYER: "<<"type: "<<obj->mGroupType<<" id: "<<obj->mId<<" parent: "<<obj->mParentId;
    }
    void visit(LottieTransform *) {
        sgDebug<<"[TRANSFORM: ]";
    }
    void visit(LottieShapeGroup *obj) {
        sgDebug<<"[SHAPE GROP: START]";
    }
    void visit(LottieShapeObject *) {
        sgDebug<<"[SHAPE: ]";
    }
    void visit(LottieRectObject *) {
        sgDebug<<"[RECT: ]";
    }
    void visit(LottieEllipseObject *) {
        sgDebug<<"[ELLIPSE: ]";
    }
    void visit(LottieTrimObject *) {
        sgDebug<<"[TRIM: ]";
    }
    void visit(LottieRepeaterObject *) {
        sgDebug<<"[REPEATER: ]";
    }
    void visit(LottieFillObject *) {
        sgDebug<<"[FILL: ]";
    }
    void visit(LottieStrokeObject *) {
        sgDebug<<"[STROKE: ]";
    }
    void visitChildren(LottieGroupObject *obj) {
        for(auto child :obj->mChildren)
            child->accept(this);
    }
};

SGJson::SGJson(const char *data)
{
    LottieParser r(const_cast<char *>(data));

    LottieComposition *comp = r.parseComposition();
    LottieObjectInspector inspector;
    comp->accept(&inspector);
}

RAPIDJSON_DIAG_POP
