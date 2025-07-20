#pragma once
#include <optional>
#include <unordered_dense.h>

namespace apex {

	template <
		typename KeyType,
		typename ValueType,
		typename Hasher = ankerl::unordered_dense::hash<KeyType>,
		typename KeyEqual = std::equal_to<KeyType>>
	using AxDenseHashMap = ankerl::unordered_dense::map<KeyType, ValueType, Hasher, KeyEqual, mem::StdAllocator<std::pair<KeyType, ValueType>>>;

}
