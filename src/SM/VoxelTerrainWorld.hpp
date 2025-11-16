#pragma once

#include <memory>
#include <functional>

#include "Util.hpp"
#include "Types.hpp"
#include "XXHash.hpp"
#include "PhysicsBase.hpp"
#include "VoxelTerrainChunk.hpp"
#include "VoxelTerrainPhysicsProxy.hpp"
#include "Bounds.hpp"

namespace SM {
	class VoxelTerrainWorld {
		public:
			using ChunkVoxelCallback = std::function<void(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex)>;

			enum ChunkFlags : uint8 {
				ChunkFlag_SlotEmpty = 0x0,
				ChunkFlag_SlotUsed = 0x1,
				ChunkFlag_Updated = 0x2
			};

			inline uint16 getId() const {return m_pPhysicsBase->getWorld()->getID();};
			inline PhysicsBase* getPhysicsBase() const {return m_pPhysicsBase;};

			inline uint32 getChunkIndex(const i32Vec3& index3D) const {
				auto it = m_mapChunkIndices3D.find(index3D);
				if ( it == m_mapChunkIndices3D.end() )
					return VoxelConstants::InvalidChunkIndex;
				return it->second;
			}

			inline VoxelTerrainChunk* getChunk(uint32 uChunkIndex) const {
				if ( uChunkIndex < m_voxelChunkCount && (m_pArrChunkFlags[uChunkIndex] & ChunkFlag_SlotUsed) )
					return m_pArrChunks[uChunkIndex];
				return nullptr;
			};

			inline VoxelTerrainPhysicsProxy* getChunkProxy(uint32 uChunkIndex) const {
				if ( uChunkIndex < m_voxelChunkCount && (m_pArrChunkFlags[uChunkIndex] & ChunkFlag_SlotUsed) )
					return m_pArrChunkProxies[uChunkIndex];
				return nullptr;
			}

			inline void updateChunk(const i32Vec3& vChunkIndex, bool makeDirtySphere = true);

			uint16 getClosestVoxel(const Vec3& vPosition, i32Vec3* pChunkIndex = nullptr, i32Vec3* pVoxelIndex = nullptr) const;

			inline Bounds getBounds() const {
				constexpr float MetersPerCell = World::MetersPerCell;
				const World::CellBounds& cellBounds = m_pPhysicsBase->getWorld()->getCellBounds();
				return {Vec3(
					float(cellBounds.minX) * MetersPerCell,
					float(cellBounds.minY) * MetersPerCell,
					VoxelConstants::WorldBoundsMinZ
				), Vec3(
					float(cellBounds.maxX) * MetersPerCell + MetersPerCell,
					float(cellBounds.maxY) * MetersPerCell + MetersPerCell,
					VoxelConstants::WorldBoundsMaxZ
				)};
			};
			IntBounds getChunkBounds() const {
				constexpr int CellChunksXY = VoxelConstants::ChunksPerCellXY;
				const World::CellBounds& cellBounds = m_pPhysicsBase->getWorld()->getCellBounds();
				return {i32Vec3(
					cellBounds.minX * CellChunksXY,
					cellBounds.minY * CellChunksXY,
					VoxelConstants::ChunkIndexMinZ
				), i32Vec3(
					cellBounds.maxX * CellChunksXY + (CellChunksXY - 1),
					cellBounds.maxY * CellChunksXY + (CellChunksXY - 1),
					VoxelConstants::ChunkIndexMaxZ
				)};
			};

			VoxelTerrainChunk* getOrCreateChunk(const i32Vec3& vChunkIndex, bool optional);
			void createAndIterateChunksAndVoxels(const IntBounds& bounds, bool createChunks, bool clearEdgeVoxels, ChunkVoxelCallback&& cb);

		private:
			char _pad0[0x8];
			PhysicsBase* m_pPhysicsBase;
			char _pad1[0x90];
			uint32 m_voxelMaterialSetHash;
			char _pad2[0x4];
			XXHashMap<i32Vec3, uint32> m_mapChunkIndices3D;
			char _pad3[0x70];
			uint32 m_voxelChunkCount;
			char _pad4[0x4];
			ChunkFlags* m_pArrChunkFlags;
			char _pad5[0x8];
			VoxelTerrainChunk** m_pArrChunks;
			char _pad6[0x10];
			VoxelTerrainPhysicsProxy** m_pArrChunkProxies;
			char _pad7[0xC8];
			XXHashSet<i32Vec3> m_setChunksToUpdate;
			char _pad8[0x40];
	};
	ASSERT_SIZE(VoxelTerrainWorld, 0x2D8);
}
