#ifndef SGGLOBAL_H
#define SGGLOBAL_H

#include<iostream>
#include <cmath>
#include<cstdint>
#include<type_traits>
#include<utility>

typedef uint32_t         uint;
typedef uint16_t         ushort;
typedef uint8_t          uchar;


#define SG_VISIBILITY_AVAILABLE

#  ifdef SG_OS_WIN
#    define SG_EXPORT     __declspec(dllexport)
#    define SG_IMPORT     __declspec(dllimport)
#  elif defined(SG_VISIBILITY_AVAILABLE)
#    define SG_EXPORT     __attribute__((visibility("default")))
#    define SG_EXPORT     __attribute__((visibility("default")))
#  endif

#define SG_UNUSED __attribute__((__unused__))
#define SG_REQUIRED_RESULT __attribute__ ((__warn_unused_result__))

#define SG_CONSTEXPR constexpr
#define SG_NOTHROW   noexcept

#include"sgdebug.h"

class RefCount
{
public:
    inline RefCount(){}
    inline RefCount(int i):atomic(i){}
    inline bool ref() {
        int count = atomic;
        if (count == 0) // !isSharable
            return false;
        if (count != -1) // !isStatic
            atomic++;
        return true;
    }
    inline bool deref() {
        int count = atomic;
        if (count == 0) // !isSharable
            return false;
        if (count == -1) // isStatic
            return true;
        return --atomic;
    }
    bool isShared() const
    {
        int count = atomic;
        return (count != 1) && (count != 0);
    }
    bool isStatic() const
    {
        // Persistent object, never deleted
        return atomic == -1;
    }
    inline int count()const{return atomic;}
    void setOwned() { atomic = 1; }
    void setUnsharable() { atomic = 0; }
private:
    int atomic;
};

template <typename T>
SG_CONSTEXPR inline const T &sgMin(const T &a, const T &b) { return (a < b) ? a : b; }
template <typename T>
SG_CONSTEXPR inline const T &sgMax(const T &a, const T &b) { return (a < b) ? b : a; }

static inline bool sgCompare(double p1, double p2)
{
    return (std::abs(p1 - p2) * 1000000000000. <= sgMin(std::abs(p1), std::abs(p2)));
}

static inline bool sgCompare(float p1, float p2)
{
    return (std::abs(p1 - p2) * 100000.f <= sgMin(std::abs(p1), std::abs(p2)));
}

static inline bool sgIsNull(double d)
{
    return std::abs(d) <= 0.000000000001;
}

static inline bool sgIsNull(float f)
{
    return std::abs(f) <= 0.00001f;
}

class sgFlagHelper
{
    int i;
public:
    constexpr inline sgFlagHelper(int ai) noexcept : i(ai) {}
    constexpr inline operator int() const noexcept { return i; }

    constexpr inline sgFlagHelper(uint ai) noexcept : i(int(ai)) {}
    constexpr inline sgFlagHelper(short ai) noexcept : i(int(ai)) {}
    constexpr inline sgFlagHelper(ushort ai) noexcept : i(int(uint(ai))) {}
    constexpr inline operator uint() const noexcept { return uint(i); }
};

template<typename Enum>
class sgFlag
{
public:
    static_assert((sizeof(Enum) <= sizeof(int)),
                  "sgFlag only supports int as storage so bigger type will overflow");
    static_assert((std::is_enum<Enum>::value), "sgFlag is only usable on enumeration types.");

    typedef typename std::conditional<
            std::is_unsigned<typename std::underlying_type<Enum>::type>::value,
            unsigned int,
            signed int
        >::type Int;

    typedef Enum enum_type;
    // compiler-generated copy/move ctor/assignment operators are fine!

    constexpr inline sgFlag(Enum f) noexcept : i(Int(f)) {}
    constexpr inline sgFlag() noexcept : i(0) {}
    constexpr inline sgFlag(sgFlagHelper f) noexcept : i(f) {}

     inline sgFlag &operator&=(int mask) noexcept { i &= mask; return *this; }
     inline sgFlag &operator&=(uint mask) noexcept { i &= mask; return *this; }
     inline sgFlag &operator&=(Enum mask) noexcept { i &= Int(mask); return *this; }
     inline sgFlag &operator|=(sgFlag f) noexcept { i |= f.i; return *this; }
     inline sgFlag &operator|=(Enum f) noexcept { i |= Int(f); return *this; }
     inline sgFlag &operator^=(sgFlag f) noexcept { i ^= f.i; return *this; }
     inline sgFlag &operator^=(Enum f) noexcept { i ^= Int(f); return *this; }

    constexpr inline operator Int() const noexcept { return i; }

    constexpr inline sgFlag operator|(sgFlag f) const  { return sgFlag(sgFlagHelper(i | f.i)); }
    constexpr inline sgFlag operator|(Enum f) const noexcept { return sgFlag(sgFlagHelper(i | Int(f))); }
    constexpr inline sgFlag operator^(sgFlag f) const noexcept { return sgFlag(sgFlagHelper(i ^ f.i)); }
    constexpr inline sgFlag operator^(Enum f) const noexcept { return sgFlag(sgFlagHelper(i ^ Int(f))); }
    constexpr inline sgFlag operator&(int mask) const noexcept { return sgFlag(sgFlagHelper(i & mask)); }
    constexpr inline sgFlag operator&(uint mask) const noexcept { return sgFlag(sgFlagHelper(i & mask)); }
    constexpr inline sgFlag operator&(Enum f) const noexcept { return sgFlag(sgFlagHelper(i & Int(f))); }
    constexpr inline sgFlag operator~() const noexcept { return sgFlag(sgFlagHelper(~i)); }

    constexpr inline bool operator!() const noexcept { return !i; }

    constexpr inline bool testFlag(Enum f) const noexcept { return (i & Int(f)) == Int(f) && (Int(f) != 0 || i == Int(f) ); }
    inline sgFlag &setFlag(Enum f, bool on = true) noexcept
    {
        return on ? (*this |= f) : (*this &= ~f);
    }

    Int i;
};


class SG_EXPORT SGColor
{
    inline SGColor() noexcept {a = r = g = b = 0;}
    inline SGColor(int red, int green, int blue, int alpha = 255) noexcept { r = red; g = green; b = blue; a = alpha; }
    inline int red() const noexcept {return r;}
    inline int green() const noexcept{return g;}
    inline int blue() const noexcept{return b;}
    inline int alpha() const noexcept{return a;}
    inline void setRed(int red) noexcept {r = red;}
    inline void setGreen(int green)noexcept {g = green;}
    inline void setBlue(int blue)noexcept {b = blue;}
    inline void setAlpha(int alpha)noexcept {a = alpha;}

public:
    ushort a;
    ushort r;
    ushort g;
    ushort b;
};

enum class FillRule {
    OddEven,
    Winding
};

enum class JoinStyle {
    Miter,
    Bevel,
    Round
};
enum class CapStyle {
    Flat,
    Square,
    Round
};
enum StrokeStyle {
    SolidLine,
    DashLine
};

#endif //SGGLOBAL_H
