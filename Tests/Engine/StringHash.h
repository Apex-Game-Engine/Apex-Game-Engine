#pragma once
#include "Core/Types.h"

namespace apex::internal
{
    constexpr uint32 fnv_prime_32 = 16777619;
    constexpr uint64 fnv_prime_64 = 1099511628211U;

    constexpr uint32 fnv_offset_basis_32 = 2166136261;
    constexpr uint64 fnv_offset_basis_64 = 14695981039346656037U;

    inline constexpr uint64 hash_fnv1a_64(const char* const str, const uint64 hash = fnv_offset_basis_64)
    {
        // hash = fnv_offset_basis_64
        // hash = hash ^ str[idx]
        // hash = hash * fnv_prime_64
        return (str[0] == '\0') ? hash : hash_fnv1a_64(&str[1], (hash ^ str[0]) * fnv_prime_64);
    }

    inline constexpr uint32 hash_fnv1a_32(const char* const str, const uint32 hash = fnv_offset_basis_32)
    {
	    return (str[0] == '\0') ? hash : hash_fnv1a_32(&str[1], (hash ^ str[0]) * fnv_prime_32);
	}
}
