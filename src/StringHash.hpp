#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>

namespace DLL {
	struct StringHash {
		using is_transparent = void;
		[[nodiscard]]
		size_t operator()(const char* str) const {
			return std::hash<std::string_view>{}(str);
		}
		[[nodiscard]]
		size_t operator()(std::string_view str) const {
			return std::hash<std::string_view>{}(str);
		}
		[[nodiscard]]
		size_t operator()(const std::string& str) const {
			return std::hash<std::string>{}(str);
		}
	};

	template<typename T>
	using StringHashMap = std::unordered_map<std::string, T, StringHash, std::equal_to<>>;
	using StringHashSet = std::unordered_set<std::string, StringHash, std::equal_to<>>;
}
