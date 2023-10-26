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
	    union Float32
	    {
            /* Floating point representation helper struct
             * https://stackoverflow.com/a/3423299
             */

            using raw_type         = float32;
            using bits_type        = uint32;
            using signed_bits_type = int32;

			static constexpr size_t kMaxUlps          = 4;

            static constexpr size_t kBitCount         = sizeof(raw_type) * 8;
            static constexpr size_t kFractionBitCount = std::numeric_limits<raw_type>::digits - 1;
            static constexpr size_t kExponentBitCount = kBitCount - 1 - kFractionBitCount;

            static constexpr uint32 kSignBitMask      = static_cast<uint32>(1) << (kBitCount - 1);
            static constexpr uint32 kFractionBitMask  = ~static_cast<uint32>(0) >> (kExponentBitCount + 1);
            static constexpr uint32 kExponentBitMask  = ~(kSignBitMask | kFractionBitMask);

            static_assert(kFractionBitMask == 0x007fffff);
            static_assert(kExponentBitMask == 0x7f800000);

		    Float32(float32 num = 0.0f) : f(num) {}

            [[nodiscard]] bits_type signBit() const { return bits & kSignBitMask; }
            [[nodiscard]] bits_type fractionBits() const { return bits & kFractionBitMask; }
            [[nodiscard]] bits_type exponentBits() const { return bits & kExponentBitMask; }

	    	bool isNegative() const { return static_cast<bool>(signBit()); }
            bool isNan() const { return (exponentBits() == kExponentBitMask) && (fractionBits() != 0); }

            bool almostEqual(Float32 const& rhs) const
		    {
			    if (isNan() || rhs.isNan()) return false;

                return _distanceBetweenNumbers(bits, rhs.bits) <= kMaxUlps;
		    }

            static bits_type _biasedRepresentation(bits_type const& b)
		    {
			    if (b & kSignBitMask)
			    {
				    return ~b + 1;
			    }
		    	else
			    {
				    return kSignBitMask | b;
			    }
		    }

            static bits_type _distanceBetweenNumbers(bits_type const& b1, bits_type const& b2)
		    {
			    const bits_type biased1 = _biasedRepresentation(b1);
			    const bits_type biased2 = _biasedRepresentation(b2);

                return (biased1 >= biased2) ? (biased1 - biased2) : (biased2 - biased1);
		    }

	    private:
            raw_type         f;
            bits_type        bits;
	    };
    }

    inline bool floatCompareApproximatelyEqual(float32 a, float32 b)
    {
        return fabsf(a - b) <= ( (a == 0 || b == 0) ? constants::float32_EPSILON : ((fabsf(a) < fabsf(b) ? fabsf(b) : fabsf(a)) * constants::float32_EPSILON ));
    }

    inline bool floatCompareAlmostEqual(float32 a, float32 b)
    {
	    detail::Float32 A { a };
        detail::Float32 B { b };

        return A.almostEqual(B);
    }

	inline bool floatCompareNearZero(float32 const& v)
	{
		return fabsf(v) < constants::float32_EPSILON;
	}

    template <typename T>
    concept numeric = std::integral<T> || std::floating_point<T> || std::is_pointer_v<T> || std::is_enum_v<T>;

    template <typename T>
    concept empty = std::is_empty_v<T>;

    // TODO: Create a new Timestep class that provides conversion to nanos, micros, millis and seconds
    using Timestep = float32;

}
