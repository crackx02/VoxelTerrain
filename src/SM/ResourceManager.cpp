
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include "ResourceManager.hpp"
#include "Util.hpp"

using namespace SM;

constexpr uintptr Offset_LoadTexArray = 0x057b6a0;

using LoadTexArrayFunc = TextureResource * (*)(
	ResourceManager*, const std::vector<std::string>*,
	int, int, TextureResourceLayout, int, int
);
LoadTexArrayFunc g_LoadTexArray = nullptr;



ResourceManager** ResourceManager::_selfPtr = (ResourceManager**)0x1267818;

TextureResource* ResourceManager::createTextureArrayResource(
	const std::vector<std::string>& vecFilePaths,
	int iWidth, int iHeight, TextureResourceLayout eLayout
) {
	ResolveGlobal(LoadTexArray);
	return g_LoadTexArray(this, &vecFilePaths, iWidth, iHeight, eLayout, 0, 0);
}
