
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

#include "Texture.hpp"
#include "SM/DirectoryManager.hpp"
#include "SM/Console.hpp"

using namespace DLL;
using namespace SM;

Texture::Texture(const std::string& path) {
	std::string imageData;
	if ( !DirectoryManager::Get()->readFile(path, imageData) ) {
		SM_ERROR("Failed to read texture file '{}'", path);
		return;
	}

	int sx = 0;
	int sy = 0;
	int comp = 0;
	stbi_set_flip_vertically_on_load(true);
	float* pData = stbi_loadf_from_memory((stbi_uc*)imageData.data(), int(imageData.length()), &sx, &sy, &comp, 3);
	if ( pData == nullptr ) {
		SM_ERROR("Failed to load texture '{}': {}", path, stbi_failure_reason());
		return;
	}

	if ( sx < 2 || sy < 2 ) {
		SM_ERROR("texture too small, minimum size 2x2, was {}x{}", sx, sy);
		stbi_image_free(pData);
		return;
	}

	m_vSize = {uint32(sx), uint32(sy)};
	m_vBounds = m_vSize - 1;
	m_vConversionRatio = Vec2(m_vSize - 1) / Vec2(m_vSize);
	m_vecPixels.insert(m_vecPixels.begin(), (Vec3*)pData, ((Vec3*)pData) + sx * sy);
	m_bValid = true;

	stbi_image_free(pData);
}
