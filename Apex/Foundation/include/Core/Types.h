#pragma once
#include <cfloat>
#include <cmath>
#include <limits>
#include <concepts>

namespace apex {

    // using Bool  = bool;

	using s8  = char;
    using s16 = short;
    using s32 = int;
    using s64 = long long;

    using u8  = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using u64 = unsigned long long;

    using f32 = float;
    using f64 = double;

    // additional types
	using byte = u8;
    using sbyte = s8;

    static_assert(sizeof(bool)  == 1, "Integer type `Bool` does not have correct size!");

    static_assert(sizeof(s8)  == 1, "Integer type `s8` does not have correct size!");
    static_assert(sizeof(s16) == 2, "Integer type `s16` does not have correct size!");
    static_assert(sizeof(s32) == 4, "Integer type `s32` does not have correct size!");
    static_assert(sizeof(s64) == 8, "Integer type `s64` does not have correct size!");

    static_assert(sizeof(u8)  == 1, "Integer type `u8` does not have correct size!");
    static_assert(sizeof(u16) == 2, "Integer type `u16` does not have correct size!");
    static_assert(sizeof(u32) == 4, "Integer type `u32` does not have correct size!");
    static_assert(sizeof(u64) == 8, "Integer type `u64` does not have correct size!");

    static_assert(sizeof(f32) == 4, "Floating type `f32` does not have correct size!");
    static_assert(sizeof(f64) == 8, "Floating type `f64` does not have correct size!");

    struct Constants
    {
	    static constexpr s8  s8_MAX  = 0x7fi8;
	    static constexpr s8  s8_MIN  = 0x80i8;
	    static constexpr s16 s16_MAX = 0x7fffi16;
	    static constexpr s16 s16_MIN = 0x8000i16;
	    static constexpr s32 s32_MAX = 0x7fffffffi32;
	    static constexpr s32 s32_MIN = 0x80000000i32;
	    static constexpr s64 s64_MAX = 0x7fffffffffffffffi64;
	    static constexpr s64 s64_MIN = 0x8000000000000000i64;

	    static constexpr u8  u8_MAX  = 0xffui8;
	    static constexpr u16 u16_MAX = 0xffffui16;
	    static constexpr u32 u32_MAX = 0xffffffffui32;
	    static constexpr u64 u64_MAX = 0xffffffffffffffffui64;

        static constexpr f32 f32_MAX        = FLT_MAX;
        static constexpr f32 f32_MIN        = FLT_MIN;
        static constexpr f32 f32_MAX_2_EXP  = FLT_MAX_EXP;
        static constexpr f32 f32_MIN_2_EXP  = FLT_MIN_EXP;
        static constexpr f32 f32_MAX_10_EXP = FLT_MAX_10_EXP;
        static constexpr f32 f32_MIN_10_EXP = FLT_MIN_10_EXP;
        static constexpr f32 f32_EPSILON    = FLT_EPSILON;
        static constexpr f32 f32_INFINITY   = std::numeric_limits<f32>::infinity();
        static constexpr f32 f32_PI         = 3.1415926535897932385f;

        static constexpr f64 f64_MAX        = DBL_MAX;
        static constexpr f64 f64_MIN        = DBL_MIN;
        static constexpr f64 f64_MAX_2_EXP  = DBL_MAX_EXP;
        static constexpr f64 f64_MIN_2_EXP  = DBL_MIN_EXP;
        static constexpr f64 f64_MAX_10_EXP = DBL_MAX_10_EXP;
        static constexpr f64 f64_MIN_10_EXP = DBL_MIN_10_EXP;
        static constexpr f64 f64_EPSILON    = DBL_EPSILON;
        static constexpr f64 f64_INFINITY   = std::numeric_limits<f64>::infinity();
        static constexpr f64 f64_PI         = 3.1415926535897932385;
    };

	namespace detail
    {
	    union Float32
	    {
            /* Floating point representation helper struct
             * https://stackoverflow.com/a/3423299
             */

            using raw_type         = f32;
            using bits_type        = u32;
            using signed_bits_type = s32;

			static constexpr size_t kMaxUlps          = 4;

            static constexpr size_t kBitCount         = sizeof(raw_type) * 8;
            static constexpr size_t kFractionBitCount = std::numeric_limits<raw_type>::digits - 1;
            static constexpr size_t kExponentBitCount = kBitCount - 1 - kFractionBitCount;

            static constexpr u32 kSignBitMask      = static_cast<u32>(1) << (kBitCount - 1);
            static constexpr u32 kFractionBitMask  = ~static_cast<u32>(0) >> (kExponentBitCount + 1);
            static constexpr u32 kExponentBitMask  = ~(kSignBitMask | kFractionBitMask);

            static_assert(kFractionBitMask == 0x007fffff);
            static_assert(kExponentBitMask == 0x7f800000);

		    Float32(f32 num = 0.0f) : f(num) {}

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
			    return kSignBitMask | b;
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

    inline bool floatCompareApproximatelyEqual(f32 a, f32 b)
    {
        return fabsf(a - b) <= ( (a == 0 || b == 0) ? Constants::f32_EPSILON : ((fabsf(a) < fabsf(b) ? fabsf(b) : fabsf(a)) * Constants::f32_EPSILON ));
    }

    inline bool floatCompareAlmostEqual(f32 a, f32 b)
    {
	    detail::Float32 A { a };
        detail::Float32 B { b };

        return A.almostEqual(B);
    }

	inline bool floatCompareNearZero(f32 const& v)
	{
		return fabsf(v) < Constants::f32_EPSILON;
	}

    /**
     * @brief checks if T is a numeric type
     */
    template <typename T>
    concept numeric = std::integral<T> || std::floating_point<T> || std::is_pointer_v<T> || std::is_enum_v<T>;

    /**
     * @brief checks if T is an empty class
     */
    template <typename T>
    concept empty = std::is_empty_v<T>;

    /**
	 * @brief checks if T2 is a non-array type convertible to T
	 */
	template <typename T2, typename T> 
	concept convertible_to = std::negation_v<std::is_array<T2>> && (std::convertible_to<T2, T> || std::derived_from<T2, T>);

	/**
	 * @brief checks if U is a pointer type convertible to T[]
	 */
	template <typename T, typename U> 
	concept ptr_convertible_to_array = std::same_as<U, T*> || (std::is_pointer_v<U> && std::convertible_to<std::remove_pointer_t<U> (*)[], T (*)[]>);

	/**
	 * @brief checks if U is an array type convertible to T[]
	 */
	template <typename T, typename U> 
	concept array_convertible_to_array = std::is_array_v<U> && std::is_pointer_v<U> && std::convertible_to<std::remove_pointer_t<U> (*)[], T (*)[]>;

    /**
     * @brief default delete function for UniquePtr/SharedPtr
     */
	template <typename T> requires (!std::is_array_v<T>)
	void default_delete(T* ptr)
	{
		delete ptr;
	}

	/**
	 * \brief  default delete function for array types for UniquePtr/SharedPtr
	 */
	template <typename T> requires (std::is_array_v<T>)
	void default_delete(std::remove_extent_t<T>* ptr)
	{
		delete[] ptr;
	}

    // TODO: Create a new Timestep class that provides conversion to nanos, micros, millis and seconds
    using Timestep = f32;

}
