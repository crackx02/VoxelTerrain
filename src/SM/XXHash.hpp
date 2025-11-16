#pragma once

#include <unordered_map>
#include <unordered_set>

#include "xxh3.h"

namespace SM {
	template <typename T>
	struct XXHash {
		std::size_t operator()(const T& t) const noexcept {
			return XXH3_64bits(&t, sizeof(T));
		}
	};

	template <typename K, typename V>
	using XXHashMap = std::unordered_map<K, V, XXHash<K>>;
	template <typename K>
	using XXHashSet = std::unordered_set<K, XXHash<K>>;
}
