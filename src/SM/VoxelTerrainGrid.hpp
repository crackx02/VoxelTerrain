#pragma once

#include <vector>

#include "Types.hpp"
#include "Util.hpp"
#include "VoxelTerrainChunk.hpp"

namespace SM {
	struct VoxelChunkPtr {
		i32Vec3 index3D;
		VoxelTerrainChunk* pChunk;
	};
	ASSERT_SIZE(VoxelChunkPtr, 0x18);

	struct VoxelTerrainGrid {
		i32Vec3 boundsMin;
		i32Vec3 boundsMax;
		char _pad[0x40];
		std::vector<VoxelChunkPtr> vecChunks;
	};
	ASSERT_SIZE(VoxelTerrainGrid, 0x70);
}
