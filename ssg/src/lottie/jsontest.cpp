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

template <typename T>
struct LottiePropertyHelper
{
  LottiePropertyHelper(const T &value):mAnimation(false), mStatic(value){}
  bool mAnimation;
  LottieProperty<T, false>  mStatic;
  LottieProperty<T,true> mDyanmic;
};

struct LottieFloatPropertyHelper
{
    LottieFloatPropertyHelper(float initialValue):
        mAnimation(false), mStatic(initialValue){}
    bool mAnimation;
    LottieFloatProperty<false> mStatic;
    LottieFloatProperty<true> mDyanmic;
};

struct LottiePointFPropertyHelper
{
    LottiePointFPropertyHelper(const SGPointF &initialValue):
        mAnimation(false), mStatic(initialValue){}
    bool mAnimation;
    LottiePointFProperty<false> mStatic;
    LottiePointFProperty<true> mDyanmic;
};


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
    FIDocNode * parseDoc();
    FINode * parseLayers();
    FINode * parseLayer();
    void parseItems(FINode *layer);
    void parseItems1(FINode *layer);
    void parseGroupItem(FINode *group);
    void parseEllipse(FINode *parent);
    void parseRectObject();
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
        return false;
    }

    ParseNext();
    return true;
}

bool LottieParser::EnterArray() {
    if (st_ != kEnteringArray) {
        st_  = kError;
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

    if (st_ != kExitingObject) {
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

    if (st_ == kError || st_ == kExitingObject || st_ == kHasKey) {
        st_ = kError;
        return false;
    }

    return true;
}

int LottieParser::GetInt() {
    if (st_ != kHasNumber || !v_.IsInt()) {
        st_ = kError;
        return 0;
    }

    int result = v_.GetInt();
    ParseNext();
    return result;
}

double LottieParser::GetDouble() {
    if (st_ != kHasNumber) {
        st_  = kError;
        return 0.;
    }

    double result = v_.GetDouble();
    ParseNext();
    return result;
}

bool LottieParser::GetBool() {
    if (st_ != kHasBool) {
        st_  = kError;
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

using namespace std;

void LottieParser::Skip(const char *key)
{
    if (PeekType() == kArrayType) {
        if(key)
            sgWarning<<"Lottie ARRAY attribute not supported : "<<key;
        EnterArray();
        SkipArray();
    } else if (PeekType() == kObjectType) {
        if(key)
            sgWarning<<"Lottie OBJECT attribute not supported : "<<key;
        EnterObject();
        SkipObject();
    } else {
        SkipValue();
        if(key)
            sgWarning<<"Lottie VALUE attribute not supported : "<<key;
    }
    /*
    if (PeekType() == kArrayType) {
        cout<<"\ntry skipping array : ";
        if (key)
            cout<<key;
        EnterArray();
        while (NextArrayValue())
            Skip(nullptr);

    } else if (PeekType() == kObjectType) {
        cout<<"\ntry skipping Object : ";
        EnterObject();
        while (const char* objkey = NextObjectKey())
            Skip(objkey);
    } else {
        cout<<"\ntry skipping Value : ";
        if (key)
            cout<<key;
        if (PeekType() == kNumberType) {
            cout << ": skipped number";
            GetDouble();
        } else if (PeekType() == kStringType) {
            cout << ": skipped string";
            GetString();
        } else if(PeekType() == kTrueType || PeekType() == kFalseType) {
            cout << ": skipped boolean";
            GetBool();
        } else if (PeekType() == kNullType) {
            cout << ": skipped null";
            GetNull();
        }
    } */
}

FIDocNode * LottieParser::parseDoc()
{
    RAPIDJSON_ASSERT(PeekType() == kObjectType);
    EnterObject();
    FIDocNode *doc = new FIDocNode();
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "w")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            doc->mBound.setWidth(GetInt());
        } else if (0 == strcmp(key, "h")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            doc->mBound.setHeight(GetInt());
        } else if (0 == strcmp(key, "ip")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            doc->mStartTime = GetDouble();
        } else if (0 == strcmp(key, "op")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            doc->mEndTime = GetDouble();
        } else if (0 == strcmp(key, "fr")) {
            RAPIDJSON_ASSERT(PeekType() == kNumberType);
            doc->mFrameRate = GetDouble();
        } else if (0 == strcmp(key, "layers")) {
            RAPIDJSON_ASSERT(PeekType() == kArrayType);
            FIGroupNode *composition = new FIGroupNode();
            parseLayers();
        }
        else {
            Skip(key);
        }
    }
    return nullptr;
}

FINode * LottieParser::parseLayers()
{
    FIGroupNode *composition = new FIGroupNode();
    EnterArray();
    int i = 1;
    while (NextArrayValue()) {
        RAPIDJSON_ASSERT(PeekType() == kObjectType);
        sgDebug<<"ENTER LAYER: "<<i;
        parseLayer();
        sgDebug<<"EXIT LAYER: "<<i;
        i++;
    }
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/layers/shape.json
 *
 */
FINode * LottieParser::parseLayer()
{
    FIGroupNode *layer = new FIGroupNode();
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
            Skip(key);
        } else if (0 == strcmp(key, "nm")) { /*After Effects Layer Name. Used for expressions.*/
            RAPIDJSON_ASSERT(PeekType() == kStringType);
            layer->name = GetString();
        } else if (0 == strcmp(key, "shapes")) { /* Shape list of items */
            sgDebug<<"ENTER SHAPE";
            RAPIDJSON_ASSERT(PeekType() == kArrayType);
            EnterArray();
            while (NextArrayValue()) {
                if (PeekType() == kObjectType) {
                    parseItems(layer);
                } else {
                    Skip(nullptr);
                }
            }
           sgDebug<<"EXIT SHAPE";
        } else {
            Skip(key);
        }
    }
}

void LottieParser::parseItems(FINode *parent)
{
    FINode::Type nodeType = FINode::Type::Doc;
    EnterObject();
    //FINode *item;
    sgDebug<<"ENTER Parse item";
    while (const char* key = NextObjectKey()) {
        if (0 == strcmp(key, "ty")) {
            const char *type = GetString();
            sgDebug<<"Shape type :"<<type;
            if (0 == strcmp(type, "gr")) {
                nodeType = FINode::Type::Group;
            } else if (0 == strcmp(type, "rc")) {
                parseRectObject();
            } else if (0 == strcmp(type, "el")) {
                parseEllipse(parent);
            } else if (0 == strcmp(type, "sh")) {
                nodeType = FINode::Type::Path;
            } else {
            }
        } else if (0 == strcmp(key, "it")) { /*After Effects Layer Name. Used for expressions.*/
            sgDebug<<"ENTER GROUP item";
            RAPIDJSON_ASSERT(PeekType() == kArrayType);
            if (nodeType == FINode::Type::Group)
                parseGroupItem(parent);
            else {
                assert(0);
            }
          sgDebug<<"EXIT GROUP item";
        } else {
            Skip(key);
        }

    }
    sgDebug<<"EXIT Parse item";
}

/*
 * https://github.com/airbnb/lottie-web/blob/master/docs/json/shapes/rect.json
 */
void LottieParser::parseRectObject()
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
    pt.setX(val[1]);
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
         } else if (0 == strcmp(key, "i")) {
            sgDebug<<"o";
             Skip(key);
         } else if (0 == strcmp(key, "n")) {
             sgDebug<<"n";
              Skip(key);
         } else if (0 == strcmp(key, "t")) {
             sgDebug<<"t";
              Skip(key);
         } else if (0 == strcmp(key, "s")) {
             if (PeekType() == kArrayType)
                 EnterArray();
             parseArrayValue(keyframe.mStartValue);
             sgDebug<<" S consumed";
         } else if (0 == strcmp(key, "e")) {
             if (PeekType() == kArrayType)
                 EnterArray();
             parseArrayValue(keyframe.mEndValue);
             sgDebug<<"E consumed";
         } else {
             Skip(key);
         }
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
                    parseKeyFrame(obj.mDyanmic);
                } else if (PeekType() == kNumberType) {
                    parseArrayValue(obj.mStatic.mValue);
                } else {
                    sgDebug<<"Something is really wrong here ++++++++";
                    Skip(nullptr);
                }
            }
        }  else {
            sgDebug<<"PointFProperty ignored :";
            Skip(key);
        }
    }
}

void LottieParser::parseGroupItem(FINode *parent)
{
    FINode *group = new FIGroupNode(parent);
    EnterArray();
    while (NextArrayValue()) {
        RAPIDJSON_ASSERT(PeekType() == kObjectType);
        parseItems(group);
    }
}

void LottieParser::parseEllipse(FINode *parent)
{
    sgDebug<<"parse el item :";
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
}


SGJson::SGJson(const char *data)
{
    using namespace std;

    LottieParser r(const_cast<char *>(data));
    r.parseDoc();
}

RAPIDJSON_DIAG_POP
