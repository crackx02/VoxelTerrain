
#include <mutex>
#include <algorithm>
#include <filesystem>
#include <fstream>

#include "DirectoryManager.hpp"
#include "Console.hpp"

using namespace SM;

DirectoryManager** DirectoryManager::_selfPtr = (DirectoryManager**)0x1267810;

bool DirectoryManager::resolvePath(const std::string& path, std::string& resolved) {
	if ( path.find("..") != std::string::npos ) {
		SM_ERROR("Illegal file path: {}", path);
		return false;
	}

	const auto firstSlash = path.find_first_of('/');
	const std::string key = path.substr(0, firstSlash);
	
	const std::string* pResolved = nullptr;
	{
		std::scoped_lock lock(m_lock);
		auto it = m_mapContentPaths.find(key);
		if ( it != m_mapContentPaths.end() )
			pResolved = &it->second;
	}

	resolved.clear();
	
	if ( pResolved == nullptr )
		return false;

	resolved = *pResolved + path.substr(firstSlash, std::string::npos);

	std::replace(resolved.begin(), resolved.end(), '\\', '/');

	return true;
}

bool DirectoryManager::readFile(const std::string& path, std::string& data, bool resolve) {
	std::string resolvedPath;
	if ( resolve ) {
		if ( !resolvePath(path, resolvedPath) ) {
			SM_ERROR("Failed to resolve '{}'", path);
			return false;
		}
	}

	try {
		if ( !std::filesystem::exists(resolvedPath) )
			return false;

		const uint64 size = std::filesystem::file_size(resolvedPath);
		data.clear();
		data.resize(size);
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		file.open(resolvedPath, std::ios::binary);
		file.read(data.data(), size);
		file.close();
		return true;

	} catch ( const std::exception& e ) {
		SM_ERROR("File read exception: '{}': {}", path, e.what());
		data.clear();
		return false;
	}
}
