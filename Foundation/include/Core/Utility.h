#pragma once
#include <algorithm>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif


#define UNUSED_ARG(x) 


namespace apex {

	template <typename T>
	const T& min(const T& a, const T& b)
	{
		return std::min(a, b);
	}

	template <typename T>
	const T& max(const T& a, const T& b)
	{
		return std::max(a, b);
	}

	template <typename T>
	void memmove_s(T* const dst, const size_t dstCount, const T* const src, const size_t srcCount)
	{
		(void)::memmove_s(dst, sizeof(T) * dstCount, src, sizeof(T) * srcCount);
	}

	template <typename T>
	void memcpy_s(void* const dst, const size_t dstCount, const void* const src, const size_t srcCount)
	{
		(void)::memcpy_s(dst, sizeof(T) * dstCount, src, sizeof(T) * srcCount);
	}

}
