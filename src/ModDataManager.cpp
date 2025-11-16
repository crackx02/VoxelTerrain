
#include "BulletCollision/CollisionShapes/btCollisionShape.h"

#include "ModDataManager.hpp"
#include "SM/VoxelTerrainManager.hpp"
#include "SM/VoxelTerrainWorld.hpp"
#include "SM/CreationManager.hpp"
#include "SM/Console.hpp"

using namespace DLL;
using namespace PhysicsUtils;
using namespace SM;
using namespace VoxelConstants;

ModDataManager* DLL::gModDataManager = nullptr;

ModDataManager::~ModDataManager() {
	gModDataManager = nullptr;
};

void ModDataManager::update() {
	VoxelTerrainManager* pVoxelTerrainManager = VoxelTerrainManager::Get();
	for ( auto& [world, setChunks] : m_mapSetUpdatedWorldChunks ) {
		if ( setChunks.empty() )
			continue;

		VoxelTerrainWorld* pWorld = pVoxelTerrainManager->getWorld(world);
		if ( pWorld == nullptr )
			continue;

		PhysicsBase* pPhysics = pWorld->getPhysicsBase();

		for ( const i32Vec3& vChunkIndex : setChunks ) {
			btVector3 vMin = ToBT(vChunkIndex * i32Vec3(MetersPerChunkAxis));
			btVector3 vMax = vMin + ToBT(Vec3(MetersPerChunkAxis));

			pPhysics->getTickWorldBroadphase()->aabbTest(vMin, vMax, m_findStaticBodiesCallback);
			pPhysics->getCollisionWorldBroadphase()->aabbTest(vMin, vMax, m_findDynamicBodiesCallback);
		}

		setChunks.clear();
	}

	for ( const auto& pBody : m_findStaticBodiesCallback.vecBodies )
		m_findStaticBodiesCallback.pCreationManager->queueDetachDisconnectedShapesTask(pBody);
	for ( const auto& pBody : m_findDynamicBodiesCallback.vecBodies )
		pBody->update();

	m_findStaticBodiesCallback.clear();
	m_findDynamicBodiesCallback.clear();
}

void ModDataManager::initialize() {
	m_restrictionAreaCount = 0;
	m_mapRestrictionAreas.clear();
	m_mapWorldRestrictions.clear();
	m_mapSetUpdatedWorldChunks.clear();
	m_mapWorldVoxelTerrainState.clear();
	m_findStaticBodiesCallback = {};
	m_findDynamicBodiesCallback = {};
	m_findStaticBodiesCallback.pCreationManager = CreationManager::Get();
	m_findDynamicBodiesCallback.pCreationManager = CreationManager::Get();
}

void ModDataManager::addWorld(uint16 world, bool bVoxelTerrainEnabled) {
	m_mapWorldRestrictions.emplace(world, WorldRestrictions());
	m_mapSetUpdatedWorldChunks.emplace(world, SM::XXHashSet<i32Vec3>());
	m_mapWorldVoxelTerrainState.emplace(world, bVoxelTerrainEnabled);
	SM_LOG("Added world {} to ModDataManager", world);
}

void ModDataManager::removeWorld(uint16 world) {
	SM_LOG("Removing world {} from ModDataManager", world);
	auto itState = m_mapWorldVoxelTerrainState.find(world);
	if ( itState == m_mapWorldVoxelTerrainState.end() ) {
		SM_ASSERT(m_mapWorldRestrictions.find(world) == m_mapWorldRestrictions.end());
		SM_ASSERT(m_mapSetUpdatedWorldChunks.find(world) == m_mapSetUpdatedWorldChunks.end());
		SM_ERROR("Failed to find world with id {}", world);
		return;
	}
	m_mapWorldVoxelTerrainState.erase(itState);
	auto itRes = m_mapWorldRestrictions.find(world);
	SM_ASSERT(itRes != m_mapWorldRestrictions.end());
	for ( const auto& [vChunkIndex, vecAreas] : itRes->second.mapChunkRestrictionAreas ) {
		for ( const RestrictionArea* pArea : vecAreas )
			m_mapRestrictionAreas.erase(pArea->getId());
	}
	m_mapWorldRestrictions.erase(itRes);
	m_mapSetUpdatedWorldChunks.erase(world);
}

void ModDataManager::setGlobalRestrictions(uint16 world, RestrictionFlags flags) {
	auto itWorld = m_mapWorldRestrictions.find(world);
	if ( itWorld == m_mapWorldRestrictions.end() ) {
		SM_ERROR("Failed to find world with id {}", world);
		return;
	}
	itWorld->second.globalRestrictions = flags;
}

RestrictionFlags ModDataManager::getGlobalRestrictions(uint16 world) {
	auto itWorld = m_mapWorldRestrictions.find(world);
	if ( itWorld == m_mapWorldRestrictions.end() ) {
		SM_ERROR("Failed to find world with id {}", world);
		return RestrictionFlags::None;
	}
	return itWorld->second.globalRestrictions;
}

RestrictionArea* ModDataManager::createRestrictionArea(uint16 world, const SM::Bounds& bounds, RestrictionFlags flags) {
	auto itWorld = m_mapWorldRestrictions.find(world);
	if ( itWorld == m_mapWorldRestrictions.end() ) {
		SM_ERROR("Failed to find world with id {}", world);
		return nullptr;
	}
	WorldRestrictions& res = itWorld->second;

	const uint32 id = ++m_restrictionAreaCount;
	auto itInsertedArea = m_mapRestrictionAreas.emplace(id, RestrictionArea(id, world, bounds, flags));
	SM_ASSERT(itInsertedArea.second);
	RestrictionArea* pArea = &itInsertedArea.first->second;

	const IntBounds chunkBounds(
		i32Vec3(glm::floor(bounds.min / float(MetersPerChunkAxis))),
		i32Vec3(glm::floor(bounds.max / float(MetersPerChunkAxis)))
	);

	ITERATE_BOUNDS_BEGIN(chunkBounds, cx, cy, cz);
		const i32Vec3 vChunkIndex = {cx, cy, cz};
		auto itChunk = res.mapChunkRestrictionAreas.try_emplace(vChunkIndex, std::vector<RestrictionArea*>());
		itChunk.first->second.emplace_back(pArea);
	ITERATE_BOUNDS_END;

	return pArea;
}

void ModDataManager::destroyRestrictionArea(uint32 id) {
	auto itThisArea = m_mapRestrictionAreas.find(id);
	if ( itThisArea == m_mapRestrictionAreas.end() ) {
		SM_ERROR("Failed to find build control area with id {}", id);
		return;
	}
	RestrictionArea& area = itThisArea->second;

	auto itWorld = m_mapWorldRestrictions.find(area.getWorld());
	SM_ASSERT(itWorld != m_mapWorldRestrictions.end());

	WorldRestrictions& res = itWorld->second;

	const IntBounds chunkBounds = area.getChunkBounds();
	ITERATE_BOUNDS_BEGIN(chunkBounds, cx, cy, cz);
		const i32Vec3 vChunkIndex = {cx, cy, cz};

		auto itChunk = res.mapChunkRestrictionAreas.find(vChunkIndex);
		SM_ASSERT(itChunk != res.mapChunkRestrictionAreas.end());
		auto& vecChunkAreas = itChunk->second;

		for ( auto itArea = vecChunkAreas.begin(); itArea != vecChunkAreas.end(); ++itArea )
			if ( *itArea == &area ) {
				vecChunkAreas.erase(itArea);
				break;
			}
	ITERATE_BOUNDS_END;

	m_mapRestrictionAreas.erase(itThisArea);
}

RestrictionFlags ModDataManager::getRestrictionsAt(uint16 world, const Vec3& vPosition) const {
	auto itWorld = m_mapWorldRestrictions.find(world);
	SM_ASSERT(itWorld != m_mapWorldRestrictions.end());
	const WorldRestrictions& res = itWorld->second;

	const i32Vec3 vChunkIndex = i32Vec3(glm::floor(vPosition / float(MetersPerChunkAxis)));
	auto itChunk = res.mapChunkRestrictionAreas.find(vChunkIndex);
	if ( itChunk == res.mapChunkRestrictionAreas.end() )
		return res.globalRestrictions;

	RestrictionFlags flags = res.globalRestrictions;
	RestrictionFlags overrideFlags = RestrictionFlags::None;
	for ( const auto& itArea : itChunk->second ) {
		const RestrictionArea& area = *itArea;
		if ( area.isActive() && area.getBounds().testPoint(vPosition) ) {
			flags = area.getFlags();
			if ( flags & RestrictionFlags::Override )
				overrideFlags |= flags;
		}
	}
	return flags | overrideFlags;
}

const std::vector<RestrictionArea*>* ModDataManager::getRestrictionAreasInChunk(uint16 world, const i32Vec3& vChunkIndex) const {
	auto itWorld = m_mapWorldRestrictions.find(world);
	SM_ASSERT(itWorld != m_mapWorldRestrictions.end());
	const WorldRestrictions& res = itWorld->second;

	auto itChunk = res.mapChunkRestrictionAreas.find(vChunkIndex);
	if ( itChunk == res.mapChunkRestrictionAreas.end() )
		return nullptr;
	return &itChunk->second;
}

bool ModDataManager::isVoxelTerrainEnabled(uint16 world) {
	auto it = m_mapWorldVoxelTerrainState.find(world);
	if ( it != m_mapWorldVoxelTerrainState.end() )
		return it->second;
	SM_ERROR("Failed to find world with id {}", world);
	return false;
};

void ModDataManager::onChunkUpdated(uint16 world, const i32Vec3& vChunkIndex) {
	auto it = m_mapSetUpdatedWorldChunks.find(world);
	SM_ASSERT(it != m_mapSetUpdatedWorldChunks.end());
	it->second.emplace(vChunkIndex);
}



void ModDataManager::updateRestrictionAreaChunkBounds(RestrictionArea* pArea, const SM::Bounds& newBounds) {
	auto itWorld = m_mapWorldRestrictions.find(pArea->getWorld());
	SM_ASSERT(itWorld != m_mapWorldRestrictions.end());
	WorldRestrictions& res = itWorld->second;
	auto& mapChunks = res.mapChunkRestrictionAreas;

	const IntBounds oldChunkBounds = pArea->getChunkBounds();
	const IntBounds newChunkBounds = IntBounds(
		i32Vec3(glm::floor(newBounds.min / float(MetersPerChunkAxis))),
		i32Vec3(glm::floor(newBounds.max / float(MetersPerChunkAxis)))
	);

	// Avoid reassigning chunks that don't need to be reassigned
	if ( oldChunkBounds.testOverlap(newChunkBounds) ) {
		const IntBounds overlap = oldChunkBounds.getOverlap(newChunkBounds);
		ITERATE_BOUNDS_BEGIN(oldChunkBounds, cx, cy, cz);
			const i32Vec3 vChunkIndex = {cx, cy, cz};

			if ( overlap.testPoint(vChunkIndex) )
				continue;

			auto itChunk = mapChunks.find(vChunkIndex);
			SM_ASSERT(itChunk != mapChunks.end());
			auto& vecChunkAreas = itChunk->second;

			for ( auto itArea = vecChunkAreas.begin(); itArea != vecChunkAreas.end(); ++itArea ) {
				if ( *itArea == pArea ) {
					vecChunkAreas.erase(itArea);
					break;
				}
			}
		ITERATE_BOUNDS_END;

		ITERATE_BOUNDS_BEGIN(newChunkBounds, cx, cy, cz);
			const i32Vec3 vChunkIndex = {cx, cy, cz};

			if ( overlap.testPoint(vChunkIndex) )
				continue;

			auto itChunk = mapChunks.try_emplace(vChunkIndex, std::vector<RestrictionArea*>());
			itChunk.first->second.emplace_back(pArea);
		ITERATE_BOUNDS_END;

	} else {
		ITERATE_BOUNDS_BEGIN(oldChunkBounds, cx, cy, cz);
			const i32Vec3 vChunkIndex = {cx, cy, cz};

			auto itChunk = mapChunks.find(vChunkIndex);
			SM_ASSERT(itChunk != mapChunks.end());
			auto& vecChunkAreas = itChunk->second;

			for ( auto itArea = vecChunkAreas.begin(); itArea != vecChunkAreas.end(); ++itArea ) {
				if ( *itArea == pArea ) {
					vecChunkAreas.erase(itArea);
					break;
				}
			}
		ITERATE_BOUNDS_END;

		ITERATE_BOUNDS_BEGIN(newChunkBounds, cx, cy, cz);
			const i32Vec3 vChunkIndex = {cx, cy, cz};

			auto itChunk = mapChunks.try_emplace(vChunkIndex, std::vector<RestrictionArea*>());
			itChunk.first->second.emplace_back(pArea);
		ITERATE_BOUNDS_END;
	}
}
