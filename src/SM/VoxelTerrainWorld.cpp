
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include "VoxelTerrainWorld.hpp"
#include "Console.hpp"
#include "VoxelUtils.hpp"
#include "CreationManager.hpp"
#include "PhysicsUtils.hpp"
#include "ModDataManager.hpp"

using namespace SM;
using namespace VoxelConstants;
using namespace DLL;

constexpr uintptr Offset_CreateChunk = 0x0a39510;

using CreateChunkFunc = uint32(*)(VoxelTerrainWorld*, bool, const i32Vec3*, VoxelTerrainChunk**);
CreateChunkFunc g_CreateChunk = nullptr;



uint16 VoxelTerrainWorld::getClosestVoxel(const Vec3& vPosition, i32Vec3* pChunkIndex, i32Vec3* pVoxelIndex) const {
	const i32Vec3 vClosestVoxelIndex = WorldPositionToClosestVoxelIndex(vPosition);

	SM_ASSERT(
		vClosestVoxelIndex.x < VoxelsPerChunkAxis &&
		vClosestVoxelIndex.y < VoxelsPerChunkAxis &&
		vClosestVoxelIndex.z < VoxelsPerChunkAxis
	);

	const i32Vec3 vChunkIndex = i32Vec3(glm::floor(vPosition / float(MetersPerChunkAxis)));
	const uint32 uChunkIndex = getChunkIndex(vChunkIndex);
	if ( uChunkIndex == InvalidChunkIndex )
		return 0xFFFF;

	const VoxelTerrainChunk* pChunk = getChunk(uChunkIndex);
	if ( pChunk == nullptr )
		return 0xFFFF;

	if ( pChunkIndex )
		*pChunkIndex = vChunkIndex;
	if ( pVoxelIndex )
		*pVoxelIndex = vClosestVoxelIndex;

	return pChunk->voxels[VoxelIndex3To1(vClosestVoxelIndex)];
}

void VoxelTerrainWorld::updateChunk(const i32Vec3& vChunkIndex, bool makeDirtySphere) {
	uint32 uChunkIndex = getChunkIndex(vChunkIndex);
	if ( uChunkIndex != VoxelConstants::InvalidChunkIndex ) {
		ChunkFlags flags = m_pArrChunkFlags[uChunkIndex];
		if ( flags & ChunkFlag_SlotUsed ) {
			m_pArrChunkFlags[uChunkIndex] = ChunkFlags(flags | ChunkFlag_Updated);;
			m_setChunksToUpdate.emplace(vChunkIndex);
		}
	}
	if ( makeDirtySphere ) {
		const float halfChunk = float(VoxelConstants::MetersPerChunkAxis / 2);
		const Vec3 vCenter = Vec3(vChunkIndex) * float(VoxelConstants::MetersPerChunkAxis) + Vec3(halfChunk);

		SM::World* pWorld = getPhysicsBase()->getWorld();
		pWorld->addDirtySphere(vCenter, float(VoxelConstants::MetersPerChunkAxis / 2));

		if ( pWorld->isRenderWorld() )
			SM::RenderStateManager::Get()->addDirtySphere(vCenter, 1.0f - float(VoxelConstants::MetersPerChunkAxis / 2));
	}
}

VoxelTerrainChunk* VoxelTerrainWorld::getOrCreateChunk(const i32Vec3& vChunkIndex, bool optional) {
	ResolveGlobal(CreateChunk);

	const IntBounds bounds = getChunkBounds();
	if ( glm::any(glm::lessThan(vChunkIndex, bounds.min)) || glm::any(glm::greaterThan(vChunkIndex, bounds.max)) ) {
		SM_ERROR("Chunk index out of bounds!! {{{}, {}, {}}}", vChunkIndex.x, vChunkIndex.y, vChunkIndex.z);
		return nullptr;
	}

	uint32 uChunkIndex = getChunkIndex(vChunkIndex);
	if ( uChunkIndex != InvalidChunkIndex ) {
		VoxelTerrainChunk* pChunk = getChunk(uChunkIndex);
		if ( pChunk != nullptr )
			return pChunk;
	}

	if ( optional )
		return nullptr;

	VoxelTerrainChunk* pChunk = VoxelTerrainManager::AllocateVoxelChunk();

	uChunkIndex = g_CreateChunk(this, true, &vChunkIndex, &pChunk);
	if ( uChunkIndex == InvalidChunkIndex ) {
		SM_ERROR("Voxel chunk allocation failed! Chunk index: {{{}, {}, {}}}", vChunkIndex.x, vChunkIndex.y, vChunkIndex.z);
		// The game already deallocates the chunk if this happens
		return nullptr;
	}

	return pChunk;
}

void VoxelTerrainWorld::createAndIterateChunksAndVoxels(const IntBounds& bounds, bool createChunks, bool clearEdgeVoxels, ChunkVoxelCallback&& cb) {
	const IntBounds worldBounds = getChunkBounds();
	const IntBounds clampedBounds = {
		glm::clamp(bounds.min, worldBounds.min, bounds.max),
		glm::clamp(bounds.max, bounds.min, worldBounds.max)
	};

	ITERATE_BOUNDS_BEGIN(clampedBounds, cx, cy, cz)
		const i32Vec3 vChunkIndex = {cx, cy, cz};

		VoxelTerrainChunk* pChunk = getOrCreateChunk(vChunkIndex, !createChunks);
		if ( pChunk == nullptr )
			continue;

		for ( uint8 vz = 0; vz < VoxelsPerChunkAxis; ++vz ) {
			for ( uint8 vy = 0; vy < VoxelsPerChunkAxis; ++vy ) {
				for ( uint8 vx = 0; vx < VoxelsPerChunkAxis; ++vx ) {
					const i32Vec3 vVoxelIndex = {vx, vy, vz};
					const Vec3 vVoxelPosition = (Vec3(vChunkIndex) * float(MetersPerChunkAxis)) + Vec3(vVoxelIndex);
					cb(vVoxelPosition, pChunk, VoxelIndex3To1(vVoxelIndex));
				}
			}
		}

		if ( clearEdgeVoxels ) {
			const glm::bvec3 vMinEdges = {
				cx == worldBounds.min.x,
				cy == worldBounds.min.y,
				cz == worldBounds.min.z
			};
			const glm::bvec3 vMaxEdges = {
				cx == worldBounds.max.x,
				cy == worldBounds.max.y,
				cz == worldBounds.max.z
			};
			ClearChunkEdgeVoxels(*pChunk, vMinEdges, vMaxEdges);
		}

		updateChunk(vChunkIndex, false);
	ITERATE_BOUNDS_END;

	// Chunk physics mesh update takes some time - body detachment is triggered from VoxelTerrainNetChunk::update hook

	const Vec3 chunkPosMin = Vec3(clampedBounds.min) * float(MetersPerChunkAxis);
	const Vec3 chunkPosMax = Vec3(clampedBounds.max) * float(MetersPerChunkAxis) + Vec3(MetersPerChunkAxis);
	const Vec3 halfExtents = (chunkPosMax - chunkPosMin) / 2.0f;
	const Vec3 areaCenter = chunkPosMin + halfExtents;
	const float radius = glm::max(halfExtents.x, glm::max(halfExtents.y, halfExtents.z));

	World* pWorld = getPhysicsBase()->getWorld();
	pWorld->addDirtySphere(areaCenter, 1.0f - radius);

	if ( pWorld->isRenderWorld() )
		RenderStateManager::Get()->addDirtySphere(areaCenter, 1.0f - radius);
}
