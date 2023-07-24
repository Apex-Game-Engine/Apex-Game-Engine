#pragma once
#include <algorithm>

// #define min(a, b) (((a) < (b)) ? (a) : (b))
// #define max(a, b) (((a) > (b)) ? (a) : (b))


namespace apex {

	template <typename T>
	const T& min(const T& a, const T& b)
	{
		return std::min(a, b);
	}

	template <typename T>
	const T& max(const T& a, const T& b)
	{
		return std::min(a, b);
	}

}
