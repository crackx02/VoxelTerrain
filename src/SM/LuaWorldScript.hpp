#pragma once

#include <string>

#include "Util.hpp"

namespace SM {
	struct LuaWorldScript {
		char _pad0[0x40];
		std::string sVoxelMaterialSet;
		char _pad1[0x27];
		bool bEnableVoxelTerrain;
		char _pad2[0x4];
		bool bIndoors;
		bool bStatic;
		char _pad3[0x20];
	};
	ASSERT_SIZE(LuaWorldScript, 0xB0);
}
