#pragma once

#include <vector>
#include <string>

#include "TextureResource.hpp"
#include "Util.hpp"

namespace SM {
	struct TerrainGroundMaterialSet {
		uint32 hash;
		char _pad[0x4];
		std::vector<std::string> vecMaterialNames;
		TextureResource* pDifTexArray = nullptr;
		TextureResource* pAsgTexArray = nullptr;
		TextureResource* pNorTexArray = nullptr;
		std::string sFilepath;
	};
	ASSERT_SIZE(TerrainGroundMaterialSet, 0x58);
}
