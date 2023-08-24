#pragma once
#include <cfloat>
#include <cmath>
#include <limits>

namespace apex {

    // using Bool  = bool;

	using int8  = char;
    using int16 = short;
    using int32 = int;
    using int64 = long long;

    using uint8  = unsigned char;
    using uint16 = unsigned short;
    using uint32 = unsigned int;
    using uint64 = unsigned long long;

    using float32 = float;
    using float64 = double;

    // additional types
	using byte = uint8;
    using sbyte = int8;

    static_assert(sizeof(bool)  == 1, "Integer type `Bool` does not have correct size!");

    static_assert(sizeof(int8)  == 1, "Integer type `int8` does not have correct size!");
    static_assert(sizeof(int16) == 2, "Integer type `int16` does not have correct size!");
    static_assert(sizeof(int32) == 4, "Integer type `int32` does not have correct size!");
    static_assert(sizeof(int64) == 8, "Integer type `int64` does not have correct size!");

    static_assert(sizeof(uint8)  == 1, "Integer type `uint8` does not have correct size!");
    static_assert(sizeof(uint16) == 2, "Integer type `uint16` does not have correct size!");
    static_assert(sizeof(uint32) == 4, "Integer type `uint32` does not have correct size!");
    static_assert(sizeof(uint64) == 8, "Integer type `uint64` does not have correct size!");

    static_assert(sizeof(float32) == 4, "Floating type `float32` does not have correct size!");
    static_assert(sizeof(float64) == 8, "Floating type `float64` does not have correct size!");

    namespace constants
    {
	    constexpr int8  int8_MAX  = 0x7fi8;
	    constexpr int8  int8_MIN  = 0x80i8;
	    constexpr int16 int16_MAX = 0x7fffi16;
	    constexpr int16 int16_MIN = 0x8000i16;
	    constexpr int32 int32_MAX = 0x7fffffffi32;
	    constexpr int32 int32_MIN = 0x80000000i32;
	    constexpr int64 int64_MAX = 0x7fffffffffffffffi64;
	    constexpr int64 int64_MIN = 0x8000000000000000i64;

	    constexpr uint8  uint8_MAX  = 0xffui8;
	    constexpr uint16 uint16_MAX = 0xffffui16;
	    constexpr uint32 uint32_MAX = 0xffffffffui32;
	    constexpr uint64 uint64_MAX = 0xffffffffffffffffui64;

        constexpr float32 float32_MAX        = FLT_MAX;
        constexpr float32 float32_MIN        = FLT_MIN;
        constexpr float32 float32_MAX_2_EXP  = FLT_MAX_EXP;
        constexpr float32 float32_MIN_2_EXP  = FLT_MIN_EXP;
        constexpr float32 float32_MAX_10_EXP = FLT_MAX_10_EXP;
        constexpr float32 float32_MIN_10_EXP = FLT_MIN_10_EXP;
        constexpr float32 float32_EPSILON    = FLT_EPSILON;
        constexpr float32 float32_INFINITY   = std::numeric_limits<float32>::infinity();
        constexpr float32 float32_PI         = 3.1415926535897932385f;
        constexpr int64   float32_ULP_DIFF     = 4;

        constexpr float64 float64_MAX        = DBL_MAX;
        constexpr float64 float64_MIN        = DBL_MIN;
        constexpr float64 float64_MAX_2_EXP  = DBL_MAX_EXP;
        constexpr float64 float64_MIN_2_EXP  = DBL_MIN_EXP;
        constexpr float64 float64_MAX_10_EXP = DBL_MAX_10_EXP;
        constexpr float64 float64_MIN_10_EXP = DBL_MIN_10_EXP;
        constexpr float64 float64_EPSILON    = DBL_EPSILON;
        constexpr float64 float64_INFINITY   = std::numeric_limits<float64>::infinity();
        constexpr float64 float64_PI         = 3.1415926535897932385;
    }

	namespace detail
    {
	    union Float
	    {
		    Float(float num = 0.0f) : f(num) {}

            bool isNegative() const { return (i >> 31) != 0; }
            int32 rawMantissa() const { return i & ((i << 23) - 1); }
            int32 rawExponent() const { return (i >> 23) & 0xff; }

            int32 i;
            float32 f;
	    };
    }

    inline bool floatCompareAlmostEqual(float A, float B, int maxUlpsDiff = constants::float32_ULP_DIFF)
    {
	    detail::Float a { A };
        detail::Float b { B };

        if (a.isNegative() != b.isNegative())
        {
            // check for +0 == -0
            return (A == B);
        }

        const int ulpsDiff = abs(a.i - b.i);
        return (ulpsDiff <= maxUlpsDiff);
    }
}
