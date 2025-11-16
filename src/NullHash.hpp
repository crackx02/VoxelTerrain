#pragma once

#include <unordered_map>
#include <unordered_set>

namespace DLL {
	template <typename T>
	struct NullHash {
		std::size_t operator()(const T& t) const noexcept {
			return size_t(T);
		}
	};

	template <typename K, typename V>
	using NullHashMap = std::unordered_map<K, V, NullHash<K>>;
	template <typename K>
	using NullHashSet = std::unordered_set<K, NullHash<K>>;
}
