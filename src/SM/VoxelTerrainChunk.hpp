#pragma once

#include "Types.hpp"
#include "Util.hpp"
#include "VoxelTerrainConstants.hpp"

namespace SM {
	struct VoxelTerrainChunk {
		static constexpr int VoxelsPerAxis = VoxelConstants::VoxelsPerChunkAxis;
		uint8 voxels[VoxelsPerAxis * VoxelsPerAxis * VoxelsPerAxis];
	};
	ASSERT_SIZE(VoxelTerrainChunk, 0x1331);
}
