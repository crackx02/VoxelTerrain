#pragma once

#include <string>
#include <unordered_map>

#include "SRWLock.hpp"
#include "Util.hpp"

namespace SM {
	class DirectoryManager {
		public:
			static DirectoryManager** _selfPtr;
			inline static DirectoryManager* Get() {return *_selfPtr;}

			bool resolvePath(const std::string& path, std::string& resolved);
			bool readFile(const std::string& path, std::string& data, bool resolve = true);

		private:
			SRWLock m_lock;
			std::unordered_map<std::string, std::string> m_mapContentPaths;
	};
	ASSERT_SIZE(DirectoryManager, 0x48);
}
