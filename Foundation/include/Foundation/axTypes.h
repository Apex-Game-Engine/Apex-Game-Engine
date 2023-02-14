#pragma once
#include <cfloat>

namespace apex {

    using b8  = bool;

	using i8  = char;
    using i16 = short;
    using i32 = int;
    using i64 = long long;

    using u8  = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using u64 = unsigned long long;

    using f32 = float;
    using f64 = double;

    static_assert(sizeof(b8)  == 1, "Integer type `b8` does not have correct size!");

    static_assert(sizeof(i8)  == 1, "Integer type `i8` does not have correct size!");
    static_assert(sizeof(i16) == 2, "Integer type `i16` does not have correct size!");
    static_assert(sizeof(i32) == 4, "Integer type `i32` does not have correct size!");
    static_assert(sizeof(i64) == 8, "Integer type `i64` does not have correct size!");

    static_assert(sizeof(u8)  == 1, "Integer type `u8` does not have correct size!");
    static_assert(sizeof(u16) == 2, "Integer type `u16` does not have correct size!");
    static_assert(sizeof(u32) == 4, "Integer type `u32` does not have correct size!");
    static_assert(sizeof(u64) == 8, "Integer type `u64` does not have correct size!");

    static_assert(sizeof(f32) == 4, "Floating type `f32` does not have correct size!");
    static_assert(sizeof(f64) == 8, "Floating type `f64` does not have correct size!");

    namespace constants
    {
	    constexpr i8  i8_MAX  = 0x7fi8;
	    constexpr i8  i8_MIN  = 0x80i8;
	    constexpr i16 i16_MAX = 0x7fffi16;
	    constexpr i16 i16_MIN = 0x8000i16;
	    constexpr i32 i32_MAX = 0x7fffffffi32;
	    constexpr i32 i32_MIN = 0x80000000i32;
	    constexpr i64 i64_MAX = 0x7fffffffffffffffi64;
	    constexpr i64 i64_MIN = 0x8000000000000000i64;

	    constexpr u8  u8_MAX  = 0xffui8;
	    constexpr u16 u16_MAX = 0xffffui16;
	    constexpr u32 u32_MAX = 0xffffffffui32;
	    constexpr u64 u64_MAX = 0xffffffffffffffffui64;

        constexpr f32 f32_MAX        = FLT_MAX;
        constexpr f32 f32_MIN        = FLT_MIN;
        constexpr f32 f32_MAX_2_EXP  = FLT_MAX_EXP;
        constexpr f32 f32_MIN_2_EXP  = FLT_MIN_EXP;
        constexpr f32 f32_MAX_10_EXP = FLT_MAX_10_EXP;
        constexpr f32 f32_MIN_10_EXP = FLT_MIN_10_EXP;
        constexpr f32 f32_EPSILON    = FLT_EPSILON;

        constexpr f64 f64_MAX        = DBL_MAX;
        constexpr f64 f64_MIN        = DBL_MIN;
        constexpr f64 f64_MAX_2_EXP  = DBL_MAX_EXP;
        constexpr f64 f64_MIN_2_EXP  = DBL_MIN_EXP;
        constexpr f64 f64_MAX_10_EXP = DBL_MAX_10_EXP;
        constexpr f64 f64_MIN_10_EXP = DBL_MIN_10_EXP;
        constexpr f64 f64_EPSILON    = DBL_EPSILON;
    }

}
