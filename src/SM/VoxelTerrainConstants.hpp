#pragma once

#include "Types.hpp"

namespace SM {
	namespace VoxelConstants {
		// These need to be multiple of 16
		constexpr float WorldBoundsMinZ = -2048.0f;
		constexpr float WorldBoundsMaxZ = 2048.0f;
		// These can be larger but it's useless as the world ceiling and floor kill area stop it early
		constexpr int ChunkIndexMinZ = -125;
		constexpr int ChunkIndexMaxZ = 63;
		constexpr int ChunksPerCellXY = 4;
		constexpr int MetersPerChunkAxis = 16;
		constexpr int VoxelsPerChunkAxis = 17;
		constexpr uint32 InvalidChunkIndex = 0x00FFFFFF;
		constexpr uint8 MaxVoxelDensity = 63;
		constexpr const char* StandardMaterialSetPath = "/VoxelMeshes/Materials/voxel_materialset_standard.json";

		static_assert(ChunkIndexMinZ * MetersPerChunkAxis >= WorldBoundsMinZ);
		static_assert(ChunkIndexMaxZ* MetersPerChunkAxis + MetersPerChunkAxis <= WorldBoundsMaxZ);
	}
}
