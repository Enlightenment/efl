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

    void parseGroupItem(FINode *group);
    void parseEllipse(FINode *parent);
    LottieComposition *parseComposition();
    void parseLayers(LottieComposition *comp);
    LottieLayer *parseLayer();
    void parseShapesAttr(LottieLayer *layer);
    void parseObject(LottieGroupObj *parent);
    LottieObject* parseObjectTypeAttr();
    LottieObject *parseGroupObject();
    LottieObject *parseRectObject();
    LottieObject *parseEllipseObject();

    void parseArrayValue(SGPointF &pt);
    void parseArrayValue(float &val);
    template<typename T>
    void parseKeyFrame(LottieProperty<T,true> &obj);
    template<typename T>
    void parseProperty(LottiePropertyHelper<T> &obj);

protected:
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
    if (st_ == kExitingObject || st_ == kEnteringArray) {
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
//        if(key)
//            sgWarning<<"Lottie ARRAY attribute not supported : "<<key;
        EnterArray();
        SkipArray();
    } else if (PeekType() == kObjectType) {
//        if(key)
//            sgWarning<<"Lottie OBJECT attribute not supported : "<<key;
        EnterObject();
        SkipObject();
    } else {
        SkipValue();
//        if(key)
//            sgWarning<<"Lottie VALUE attribute not supported : "<<key;
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
        }
        else {
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
    sgDebug<<"parse LAYER: S";
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
        } else if (0 == strcmp(key, "shapes")) { /* Shape list of items */
            parseShapesAttr(layer);
        } else {
            sgWarning<<"Layer Attribute Skipped : "<<key;
            Skip(key);
        }
    }
       sgDebug<<"parse LAYER: E";
  return layer;
}

void LottieParser::parseShapesAttr(LottieLayer *layer)
{
    sgDebug<<"ENTER SHAPE ATTR";
    RAPIDJSON_ASSERT(PeekType() == kArrayType);
    EnterArray();
    while (NextArrayValue()) {
        parseObject(layer);
    }
   sgDebug<<"EXIT SHAPE ATTR";
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
    } else {
        sgDebug<<"The Object Type not yet handled = "<< type;
        return nullptr;
    }
}

void
LottieParser::parseObject(LottieGroupObj *parent)
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
    LottieGroupObj *group = new LottieGroupObj();
    sgDebug<<"ENTER GROUP item";
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "it")) {
            sgDebug<<"ENTER IT attribute";
            RAPIDJSON_ASSERT(PeekType() == kArrayType);
            EnterArray();
            while (NextArrayValue()) {
                RAPIDJSON_ASSERT(PeekType() == kObjectType);
                parseObject(group);
            }
          sgDebug<<"EXIT IT attribute";
        } else {
            Skip(key);
        }
    }
  sgDebug<<"EXIT GROUP item";
  return group;
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/shapes/rect.json
 */
LottieObject *
LottieParser::parseRectObject()
{
    LottiePropertyHelper<float> roundness(0);
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "p")) {
            Skip(key);
        } else if (0 == strcmp(key, "s")) {
            Skip(key);
        } else if (0 == strcmp(key, "r")) {
            parseProperty(roundness);
        }  else if (0 == strcmp(key, "d")) {
            Skip(key);
        } else {
            Skip(key);
        }
    }
    return nullptr;
}

LottieObject *
LottieParser::parseEllipseObject()
{
    sgDebug<<"parse EL item START:";
    LottiePropertyHelper<SGPointF> pos = LottiePropertyHelper<SGPointF>(SGPointF());
    LottiePropertyHelper<SGPointF> size = LottiePropertyHelper<SGPointF>(SGPointF());
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "p")) {
            parseProperty(pos);
        } else if (0 == strcmp(key, "s")) {
            parseProperty(size);
        } else {
            Skip(key);
        }
    }

    if (!pos.mAnimation && !size.mAnimation) {
        auto obj = new LottieEllipseObject<false, false>();
        obj->mPos = std::move(pos.mProperty.mValue);
        obj->mSize = std::move(size.mProperty.mValue);
        return obj;
    } else {
        auto obj = new LottieEllipseObject<true, true>();
        obj->mPos = std::move(pos.mProperty);
        obj->mSize = std::move(size.mProperty);
        return obj;
    }
    sgDebug<<"parse EL item END:";
    return nullptr;
}


void LottieParser::parseArrayValue(SGPointF &pt)
{
    float val[10];
    int i=0;
    while (NextArrayValue()) {
        val[i++] = GetDouble();
    }
    sgDebug<<"Value parsed as point / size"<<i;

    pt.setX(val[0]);
    pt.setY(val[1]);
}

void LottieParser::parseArrayValue(float &val)
{
    sgDebug<<"Value parsed as single val";
    val = GetDouble();
}

template<typename T>
void LottieParser::parseKeyFrame(LottieProperty<T,true> &obj)
{
    EnterObject();
    LottieKeyFrame<T> keyframe;
     while (const char* key = NextObjectKey()) {
         if (0 == strcmp(key, "i")) {
             sgDebug<<"i";
              Skip(key);
         } else if (0 == strcmp(key, "o")) {
            sgDebug<<"o";
             Skip(key);
         } else if (0 == strcmp(key, "n")) {
             sgDebug<<"n";
              Skip(key);
         } else if (0 == strcmp(key, "t")) {
             keyframe.mStartFrame = GetDouble();
         } else if (0 == strcmp(key, "s")) {
             if (PeekType() == kArrayType)
                 EnterArray();
             parseArrayValue(keyframe.mStartValue);
         } else if (0 == strcmp(key, "e")) {
             if (PeekType() == kArrayType)
                 EnterArray();
             parseArrayValue(keyframe.mEndValue);
         } else {
             Skip(key);
         }
     }

     if (!obj.mKeyFrames.empty()) {
         // update the endFrame value of current keyframe
         obj.mKeyFrames.back().mEndFrame = keyframe.mStartFrame;
     }
   obj.mKeyFrames.push_back(keyframe);
}

template<typename T>
void LottieParser::parseProperty(LottiePropertyHelper<T> &obj)
{
    EnterObject();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "a")) {
            obj.mAnimation = GetBool();
            sgDebug<<"animation property :"<< obj.mAnimation;
        } else if (0 == strcmp(key, "k")) {
            RAPIDJSON_ASSERT(PeekType() == kArrayType);
            EnterArray();
            while (NextArrayValue()) {
                // for key frame
                if (PeekType() == kObjectType) {
                    obj.mAnimation = true;
                    parseKeyFrame(obj.mProperty);
                } else if (PeekType() == kNumberType) {
                    parseArrayValue(obj.mProperty.mValue);
                } else {
                    sgDebug<<"Something is really wrong here ++++++++";
                    Skip(nullptr);
                }
            }
        }  else {
            sgDebug<<"Property ignored :";
            Skip(key);
        }
    }
}

SGJson::SGJson(const char *data)
{
    LottieParser r(const_cast<char *>(data));
    r.parseComposition();
}

RAPIDJSON_DIAG_POP
