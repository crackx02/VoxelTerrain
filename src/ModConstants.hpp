#pragma once

#define MAKE_VERSION(major, minor, patch) ((major << 16) | (minor << 8) | patch)

namespace DLL {
	// VoxelTerrain helper mod has multiplayer & dll version checks
	constexpr const char* VoxelHelperModUuid = "1cf785ff-18e3-40f9-89b5-d7816bf835f4";
	constexpr const char* ModVersionString = "1.0.0";
	constexpr int ModVersionInt = MAKE_VERSION(1, 0, 0);
}
