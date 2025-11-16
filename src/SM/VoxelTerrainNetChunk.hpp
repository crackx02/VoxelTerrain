#pragma once

#include "NetObj.hpp"
#include "VoxelTerrainChunk.hpp"
#include "Types.hpp"

namespace SM {
	class VoxelTerrainWorld;
	class VoxelTerrainNetChunk : public NetObj {
		public:
			inline uint16 getWorld() const {return m_world;};
			inline const i32Vec3& getChunkIndex() const {return m_vChunkIndex;};

		private:
			uint16 m_world;
			char _pad0[0x2];
			i32Vec3 m_vChunkIndex;
			VoxelTerrainWorld* m_pVoxelWorld;
			uint32 m_uChunkIndex;
			char _pad1[0x4];
			VoxelTerrainChunk* m_pChunkAtUnload;
			char _pad2[0x8];
	};
	ASSERT_SIZE(VoxelTerrainNetChunk, 0x60);
}
