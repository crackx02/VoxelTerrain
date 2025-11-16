#pragma once

#include "XXHash.hpp"
#include "VoxelTerrainWorld.hpp"
#include "TerrainGroundMaterialSet.hpp"
#include "Types.hpp"
#include "Util.hpp"

namespace SM {
	class VoxelTerrainManager {
		public:
			static VoxelTerrainManager** _selfPtr;
			inline static VoxelTerrainManager* Get() {return *_selfPtr;}

			static VoxelTerrainChunk* AllocateVoxelChunk();

			inline VoxelTerrainWorld* getWorld(uint16 id) {
				auto it = m_mapWorlds.find(id);
				if ( it == m_mapWorlds.end() )
					return nullptr;
				return &it->second;
			}

			void loadStandardMaterialSet();

		private:
			XXHashMap<uint16, VoxelTerrainWorld> m_mapWorlds;
			char _pad1[0x50];
			TerrainGroundMaterialSet m_standardVoxelMaterialSet;
			char _pad2[0x40];
	};
	ASSERT_SIZE(VoxelTerrainManager, 0x128);
}
