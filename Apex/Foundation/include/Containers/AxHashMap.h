#pragma once

#include "Memory/MemoryManager.h"
#include <optional>
#include <unordered_dense.h>

#include "String/AxString.h"

namespace apex {

	template <
		typename KeyType,
		typename ValueType,
		typename Hasher = ankerl::unordered_dense::hash<KeyType>,
		typename KeyEqual = std::equal_to<KeyType>>
	using AxDenseHashMap = ankerl::unordered_dense::map<KeyType, ValueType, Hasher, KeyEqual, mem::StdAllocator<std::pair<KeyType, ValueType>>>;

}


namespace ankerl::unordered_dense {
	template <> struct hash<apex::AxString> {
	    using is_avalanching = void;
	    auto operator()(apex::AxString const& str) const noexcept -> uint64_t {
	        return detail::wyhash::hash(str.data(), sizeof(char) * str.size());
	    }
	};
}
