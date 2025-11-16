
#include <algorithm>
#include <functional>
#include <iostream>

#include "wrap_VoxelTerrain.hpp"
#include "wrap_VoxelShared.hpp"
#include "wrap_RestrictionArea.hpp"
#include "VoxelUtils.hpp"
#include "ApplySDFMethods.hpp"
#include "LuaUtils.hpp"
#include "ModDataManager.hpp"
#include "MeshVoxelizer.hpp"
#include "ModConstants.hpp"
#include "SM/VoxelTerrainConstants.hpp"
#include "SM/VoxelTerrainManager.hpp"
#include "SM/LuaManager.hpp"
#include "SM/RenderStateManager.hpp"
#include "SM/Bounds.hpp"
#include "SM/LuaVM.hpp"
#include "SM/Console.hpp"

using namespace DLL;
using namespace SM;
using namespace VoxelConstants;

void wrap_VoxelTerrain::Register(lua_State* L) {
	SM_LOG("Registering sm.voxelTerrain");

	lua_getglobal(L, "unsafe_env");
	SM_ASSERT(lua_istable(L, -1));

	lua_pushstring(L, "sm");
	lua_rawget(L, -2);
	SM_ASSERT(lua_istable(L, -1));

	lua_pushstring(L, "voxelTerrain");
	lua_newtable(L);

	lua_pushstring(L, "getClosestVoxel");
	lua_pushcfunction(L, getClosestVoxel);
	lua_rawset(L, -3);

	lua_pushstring(L, "getVoxelsAt");
	lua_pushcfunction(L, getVoxelsAt);
	lua_rawset(L, -3);

	lua_pushstring(L, "getVoxelsInSphere");
	lua_pushcfunction(L, getVoxelsInSphere);
	lua_rawset(L, -3);

	lua_pushstring(L, "createSphereAt");
	lua_pushcfunction(L, createSphereAt);
	lua_rawset(L, -3);

	lua_pushstring(L, "createBoxAt");
	lua_pushcfunction(L, createBoxAt);
	lua_rawset(L, -3);

	lua_pushstring(L, "createCylinderAt");
	lua_pushcfunction(L, createCylinderAt);
	lua_rawset(L, -3);

	lua_pushstring(L, "createConeAt");
	lua_pushcfunction(L, createConeAt);
	lua_rawset(L, -3);

	lua_pushstring(L, "createCapsuleAt");
	lua_pushcfunction(L, createCapsuleAt);
	lua_rawset(L, -3);

	lua_pushstring(L, "createTorusAt");
	lua_pushcfunction(L, createTorusAt);
	lua_rawset(L, -3);

	lua_pushstring(L, "iterateVoxels");
	lua_pushcfunction(L, iterateVoxels);
	lua_rawset(L, -3);

	lua_pushstring(L, "eraseVoxelsInSphere");
	lua_pushcfunction(L, eraseVoxelsInSphere);
	lua_rawset(L, -3);

	lua_pushstring(L, "reduceVoxelsInSphere");
	lua_pushcfunction(L, reduceVoxelsInSphere);
	lua_rawset(L, -3);

	lua_pushstring(L, "importMesh");
	lua_pushcfunction(L, importMesh);
	lua_rawset(L, -3);

	lua_pushstring(L, "reloadMesh");
	lua_pushcfunction(L, reloadMesh);
	lua_rawset(L, -3);

	lua_pushstring(L, "reloadMeshTexture");
	lua_pushcfunction(L, reloadMeshTexture);
	lua_rawset(L, -3);

	lua_pushstring(L, "getWorldBounds");
	lua_pushcfunction(L, getWorldBounds);
	lua_rawset(L, -3);

	lua_pushstring(L, "getWorldChunkBounds");
	lua_pushcfunction(L, getWorldChunkBounds);
	lua_rawset(L, -3);

	lua_pushstring(L, "exportChunk");
	lua_pushcfunction(L, exportChunk);
	lua_rawset(L, -3);

	lua_pushstring(L, "importChunk");
	lua_pushcfunction(L, importChunk);
	lua_rawset(L, -3);

	lua_pushstring(L, "exportSerializedChunk");
	lua_pushcfunction(L, exportSerializedChunk);
	lua_rawset(L, -3);

	lua_pushstring(L, "importSerializedChunk");
	lua_pushcfunction(L, importSerializedChunk);
	lua_rawset(L, -3);

	lua_pushstring(L, "isChunkEmpty");
	lua_pushcfunction(L, isChunkEmpty);
	lua_rawset(L, -3);


	lua_pushstring(L, "applyMode");
	lua_newtable(L);

	lua_pushstring(L, "applyDensity");
	lua_pushinteger(L, ApplySDFMode::ApplyDensity);
	lua_rawset(L, -3);

	lua_pushstring(L, "applyMaterial");
	lua_pushinteger(L, ApplySDFMode::ApplyMaterial);
	lua_rawset(L, -3);

	lua_pushstring(L, "removeAny");
	lua_pushinteger(L, ApplySDFMode::Erase);
	lua_rawset(L, -3);

	lua_pushstring(L, "removeMaterial");
	lua_pushinteger(L, ApplySDFMode::EraseMaterial);
	lua_rawset(L, -3);

	lua_pushstring(L, "default");
	lua_pushinteger(L, ApplySDFMode::Default);
	lua_rawset(L, -3);

	lua_rawset(L, -3);

	lua_pushstring(L, "versionString");
	lua_pushstring(L, ModVersionString);
	lua_rawset(L, -3);

	lua_pushstring(L, "versionInt");
	lua_pushinteger(L, ModVersionInt);
	lua_rawset(L, -3);

	wrap_VoxelShared::Register(L);
	wrap_RestrictionArea::Register(L);

	lua_rawset(L, -3);

	lua_pop(L, 2);

	SM_LOG("sm.voxelTerrain registered");
}

int wrap_VoxelTerrain::getClosestVoxel(lua_State* L) {
	CheckArgsMinMax(L, 1, 2);

	const Vec3 vPosition = CheckVec3(L, 1);
	const VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 2, false);

	i32Vec3 vChunkIndex;
	i32Vec3 vVoxelIndex;
	uint16 vox = pVoxelWorld->getClosestVoxel(vPosition, &vChunkIndex, &vVoxelIndex);

	if ( vox != 0xFFFF ) {
		PushVec3(L, Vec3(vChunkIndex) * float(MetersPerChunkAxis) + Vec3(vVoxelIndex));
		lua_pushinteger(L, uint64((vox >> 6) & 0b00000011));
		lua_pushinteger(L, uint64(vox & 0b00111111));
		return 3;
	}
	return 0;
}

int wrap_VoxelTerrain::getVoxelsAt(lua_State* L) {
	CheckArgsMinMax(L, 1, 2);

	const Vec3 vPosition = CheckVec3(L, 1);
	const VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 2, false);

	const i32Vec3 vBaseVoxelIndex = WorldPositionToClosestVoxelIndex(vPosition);

	SM_ASSERT(
		vBaseVoxelIndex.x < VoxelsPerChunkAxis &&
		vBaseVoxelIndex.y < VoxelsPerChunkAxis &&
		vBaseVoxelIndex.z < VoxelsPerChunkAxis
	);

	const i32Vec3 vChunkIndex = i32Vec3(glm::floor(vPosition / float(MetersPerChunkAxis)));
	const uint32 uChunkIndex = pVoxelWorld->getChunkIndex(vChunkIndex);
	if ( uChunkIndex == InvalidChunkIndex )
		return 0;

	const VoxelTerrainChunk* pChunk = pVoxelWorld->getChunk(uChunkIndex);
	if ( pChunk == nullptr )
		return 0;

	constexpr i32Vec3 arrVoxelIndexOffsets[8] = {
		{0, 0, 0},
		{1, 0, 0},
		{0, 1, 0},
		{1, 1, 0},
		{0, 0, 1},
		{1, 0, 1},
		{0, 1, 1},
		{1, 1, 1}
	};

	for ( const i32Vec3& vIndexOffset : arrVoxelIndexOffsets ) {
		const i32Vec3 vVoxelIndex = vBaseVoxelIndex + vIndexOffset;
		const uint8 vox = pChunk->voxels[VoxelIndex3To1(vVoxelIndex)];
		
		lua_createtable(L, 3, 0);
		PushVec3(L, Vec3(vChunkIndex) * float(MetersPerChunkAxis) + Vec3(vVoxelIndex));
		lua_rawseti(L, -2, 1);

		lua_pushinteger(L, uint64((vox >> 6) & 0b00000011));
		lua_rawseti(L, -2, 2);

		lua_pushinteger(L, vox & 0b00111111);
		lua_rawseti(L, -2, 3);
	}

	return 8;
}

int wrap_VoxelTerrain::getVoxelsInSphere(lua_State* L) {
	CheckArgsMinMax(L, 2, 3);

	const Vec3 vSpherePosition = CheckVec3(L, 1);
	const float radius = CheckNumberInRange(L, 2, 0.5f, INFINITY);
	const VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 3, false);

	const IntBounds chunkBounds(
		i32Vec3(glm::floor((vSpherePosition - radius) / float(MetersPerChunkAxis))),
		i32Vec3(glm::floor((vSpherePosition + radius) / float(MetersPerChunkAxis)))
	);

	lua_createtable(
		L, std::max(
			int((glm::length(Vec3(chunkBounds.max - chunkBounds.min)) * sizeof(VoxelTerrainChunk::voxels)) * 0.9f),
			int(sizeof(VoxelTerrainChunk::voxels) * 0.9f)
		), 0
	);

	int voxelIndex = 0;
	ITERATE_BOUNDS_BEGIN(chunkBounds, cx, cy, cz);
		const i32Vec3 vChunkIndex = {cx, cy, cz};
		const uint32 uChunkIndex = pVoxelWorld->getChunkIndex(vChunkIndex);
		if ( uChunkIndex == InvalidChunkIndex )
			continue;

		VoxelTerrainChunk* pChunk = pVoxelWorld->getChunk(uChunkIndex);
		if ( pChunk == nullptr )
			continue;

		for ( uint8 vx = 0; vx < VoxelsPerChunkAxis; ++vx ) {
			for ( uint8 vy = 0; vy < VoxelsPerChunkAxis; ++vy ) {
				for ( uint8 vz = 0; vz < VoxelsPerChunkAxis; ++vz ) {
					const i32Vec3 vVoxelIndex = {vx, vy, vz};

					const Vec3 vVoxelPosition = (Vec3(vChunkIndex) * float(MetersPerChunkAxis)) + Vec3(vVoxelIndex);
					const float distance = glm::length2(vVoxelPosition - vSpherePosition);
					if ( distance <= (radius * radius) ) {
						const uint8 vox = pChunk->voxels[VoxelIndex3To1(vVoxelIndex)];

						lua_createtable(L, 4, 0);

						PushVec3(L, Vec3(vChunkIndex) * float(MetersPerChunkAxis) + Vec3(vVoxelIndex));
						lua_rawseti(L, -2, 1);

						lua_pushnumber(L, distance);
						lua_rawseti(L, -2, 2);

						lua_pushinteger(L, uint64((vox >> 6) & 0b00000011));
						lua_rawseti(L, -2, 3);

						lua_pushinteger(L, vox & 0b00111111);
						lua_rawseti(L, -2, 4);

						lua_rawseti(L, -2, ++voxelIndex);
					}
				}
			}
		}
	ITERATE_BOUNDS_END;

	return 1;
}



int wrap_VoxelTerrain::createSphereAt(lua_State* L) {
	CheckArgsMinMax(L, 2, 6);
	LuaManager::Get()->checkServerMode(L, true);

	const Vec3 vSpherePosition = CheckVec3(L, 1);
	const Vec3 vRadius = CheckVec3InRange(L, 2, Vec3(0.5f), Vec3(INFINITY));
	const Quat rot = OptQuat(L, 3);
	const uint8 material = OptMaterial(L, 4);
	const ApplySDFMode applyMode = OptApplyMode(L, 5);
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 6, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	if ( !gModDataManager->checkRestrictionsAt(pVoxelWorld->getId(), vSpherePosition, ApplyModeToBuildFlags(applyMode)) )
		return 0;

	const IntBounds chunkBounds = CalculateShapeChunkBounds(vSpherePosition, rot, vRadius);
	const float surfaceFalloff = CalculateSurfaceFalloff(vRadius);
	const Quat rotInv = glm::inverse(rot);

	const ApplySDF_T applySDF = GetApplySDFMethod(applyMode);

	pVoxelWorld->createAndIterateChunksAndVoxels(
		chunkBounds, true, ((applyMode & ApplySDFMode::ApplyDensity) != 0),
		[applySDF, &rotInv, &vSpherePosition, &vRadius, surfaceFalloff, material]
		(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex) {
			const Vec3 vVoxelLocal = rotInv * (vVoxelPosition - vSpherePosition);

			const float distance = (glm::length(vVoxelLocal / vRadius) - 1.0f) * glm::length(vRadius);

			applySDF(pChunk, uVoxelIndex, distance, surfaceFalloff, material);
		}
	);
	return 0;
}

int wrap_VoxelTerrain::createBoxAt(lua_State* L) {
	CheckArgsMinMax(L, 2, 6);
	LuaManager::Get()->checkServerMode(L, true);

	const Vec3 vCenterPosition = CheckVec3(L, 1);
	const Vec3 vHalfExtents = CheckVec3InRange(L, 2, Vec3(0.5f), Vec3(INFINITY));
	const Quat rot = OptQuat(L, 3);
	const uint8 material = OptMaterial(L, 4);
	const ApplySDFMode applyMode = OptApplyMode(L, 5);
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 6, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	if ( !gModDataManager->checkRestrictionsAt(pVoxelWorld->getId(), vCenterPosition, ApplyModeToBuildFlags(applyMode)) )
		return 0;

	const IntBounds chunkBounds = CalculateShapeChunkBounds(vCenterPosition, rot, vHalfExtents);
	const float surfaceFalloff = CalculateSurfaceFalloff(vHalfExtents);
	const Quat rotInv = glm::inverse(rot);

	const ApplySDF_T applySDF = GetApplySDFMethod(applyMode);

	pVoxelWorld->createAndIterateChunksAndVoxels(
		chunkBounds, true, ((applyMode & ApplySDFMode::ApplyDensity) != 0),
		[applySDF, &rotInv, &vCenterPosition, &vHalfExtents, surfaceFalloff, material]
		(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex) {
			const Vec3 vVoxelLocal = rotInv * (vVoxelPosition - vCenterPosition);

			const Vec3 vDistance = glm::abs(vVoxelLocal) - vHalfExtents;
			const float outside = glm::length(glm::max(vDistance, 0.0f));
			const float inside = glm::min(glm::max(vDistance.x, glm::max(vDistance.y, vDistance.z)), 0.0f);
			const float distance = outside + inside;

			applySDF(pChunk, uVoxelIndex, distance, surfaceFalloff, material);
		}
	);
	return 0;
}

int wrap_VoxelTerrain::createCylinderAt(lua_State* L) {
	CheckArgsMinMax(L, 3, 7);
	LuaManager::Get()->checkServerMode(L, true);

	const Vec3 vCenterPosition = CheckVec3(L, 1);
	const float radius = CheckNumberInRange(L, 2, 0.5f, INFINITY);
	const float halfHeight = CheckNumberInRange(L, 3, 0.5f, INFINITY);
	const Quat rot = OptQuat(L, 4);
	const uint8 material = OptMaterial(L, 5);
	const ApplySDFMode applyMode = OptApplyMode(L, 6);
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 7, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	if ( !gModDataManager->checkRestrictionsAt(pVoxelWorld->getId(), vCenterPosition, ApplyModeToBuildFlags(applyMode)) )
		return 0;

	const Vec3 vHalfExtents = Vec3(radius, radius, halfHeight);
	const IntBounds chunkBounds = CalculateShapeChunkBounds(vCenterPosition, rot, vHalfExtents);
	const float surfaceFalloff = CalculateSurfaceFalloff(vHalfExtents);
	const Quat rotInv = glm::inverse(rot);

	const ApplySDF_T applySDF = GetApplySDFMethod(applyMode);

	pVoxelWorld->createAndIterateChunksAndVoxels(
		chunkBounds, true, ((applyMode & ApplySDFMode::ApplyDensity) != 0),
		[applySDF, &rotInv, &vCenterPosition, radius, halfHeight, surfaceFalloff, material]
		(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex) {
			const Vec3 vVoxelLocal = rotInv * (vVoxelPosition - vCenterPosition);

			const Vec2 d = glm::abs(Vec2(glm::length(Vec2(vVoxelLocal)), vVoxelLocal.z)) - Vec2(radius, halfHeight);
			const float distance = std::min(std::max(d.x, d.y), 0.0f) + glm::length(glm::max(d, 0.0f));

			applySDF(pChunk, uVoxelIndex, distance, surfaceFalloff, material);
		}
	);
	return 0;
}

int wrap_VoxelTerrain::createConeAt(lua_State* L) {
	CheckArgsMinMax(L, 3, 7);
	LuaManager::Get()->checkServerMode(L, true);

	const Vec3 vApexPosition = CheckVec3(L, 1);
	const float radius = CheckNumberInRange(L, 2, 0.5f, INFINITY);
	const float height = CheckNumberInRange(L, 3, 0.5f, INFINITY);
	const Quat rot = OptQuat(L, 4);
	const uint8 material = OptMaterial(L, 5);
	const ApplySDFMode applyMode = OptApplyMode(L, 6);
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 7, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	const Vec3 vApexDirection = glm::rotate(rot, Vec3(0.0f, 0.0f, 1.0f));
	const Vec3 vBasePosition = vApexPosition - vApexDirection * height;
	const Vec3 vCenter = vApexPosition - vApexDirection * (height / 2.0f);
	const Vec3 vHalfExtents = Vec3(radius, radius, height / 2.0f);

	if ( !gModDataManager->checkRestrictionsAt(pVoxelWorld->getId(), vCenter, ApplyModeToBuildFlags(applyMode)) )
		return 0;

	const IntBounds chunkBounds = CalculateShapeChunkBounds(vCenter, rot, vHalfExtents);
	const float surfaceFalloff = CalculateSurfaceFalloff(vHalfExtents);
	const Quat invRot = glm::inverse(rot);

	const ApplySDF_T applySDF = GetApplySDFMethod(applyMode);

	pVoxelWorld->createAndIterateChunksAndVoxels(
		chunkBounds, true, ((applyMode & ApplySDFMode::ApplyDensity) != 0),
		[applySDF, &invRot, &vBasePosition, radius, height, surfaceFalloff, material]
		(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex) {
			const Vec3 vVoxelLocal = invRot * (vVoxelPosition - vBasePosition);

			if ( vVoxelLocal.z <= height && vVoxelLocal.z >= 0.0f ) {
				const float distanceXY = glm::length(Vec2(vVoxelLocal));
				const float radiusAtZ = radius * ((height - vVoxelLocal.z) / height);
				const float distance = distanceXY - radiusAtZ;
				applySDF(pChunk, uVoxelIndex, distance, surfaceFalloff, material);
			}
		}
	);
	return 0;
}

int wrap_VoxelTerrain::createCapsuleAt(lua_State* L) {
	CheckArgsMinMax(L, 3, 6);
	LuaManager::Get()->checkServerMode(L, true);

	const Vec3 vPositionA = CheckVec3(L, 1);
	const Vec3 vPositionB = CheckVec3(L, 2);
	const float radius = CheckNumberInRange(L, 3, 0.5f, INFINITY);
	const uint8 material = OptMaterial(L, 4);
	const ApplySDFMode applyMode = OptApplyMode(L, 5);
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 6, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	const Vec3 vCenter = vPositionA + (vPositionB - vPositionA) * 0.5f;
	if ( !gModDataManager->checkRestrictionsAt(pVoxelWorld->getId(), vCenter, ApplyModeToBuildFlags(applyMode)) )
		return 0;

	const Vec3 vDirectionRange = vPositionB - vPositionA;
	const IntBounds chunkBounds = {
		i32Vec3(glm::floor((glm::min(vPositionA, vPositionB) - Vec3(radius)) / float(MetersPerChunkAxis))),
		i32Vec3(glm::floor((glm::max(vPositionA, vPositionB) + Vec3(radius)) / float(MetersPerChunkAxis)))
	};

	const float surfaceFalloff = CalculateSurfaceFalloff(Vec3(radius, radius, radius + (glm::length(vPositionA - vPositionB) / 2.0f)));

	const ApplySDF_T applySDF = GetApplySDFMethod(applyMode);

	pVoxelWorld->createAndIterateChunksAndVoxels(
		chunkBounds, true, ((applyMode & ApplySDFMode::ApplyDensity) != 0),
		[applySDF, &vPositionA, &vPositionB, &vDirectionRange, radius, surfaceFalloff, material]
		(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex) {
			const Vec3 vLocalPos = vVoxelPosition - vPositionA;
			const float h = glm::clamp(glm::dot(vLocalPos, vDirectionRange) / glm::dot(vDirectionRange, vDirectionRange), 0.0f, 1.0f);
			const float distance = glm::length(vLocalPos - vDirectionRange * h) - radius;

			applySDF(pChunk, uVoxelIndex, distance, surfaceFalloff, material);
		}
	);
	return 0;
}

int wrap_VoxelTerrain::createTorusAt(lua_State* L) {
	CheckArgsMinMax(L, 3, 7);
	LuaManager::Get()->checkServerMode(L, true);

	const Vec3 vCenterPosition = CheckVec3(L, 1);
	const float majorRadius = CheckNumberInRange(L, 2, 0.5f, INFINITY);
	const float minorRadius = CheckNumberInRange(L, 3, 0.5f, INFINITY);
	const Quat rot = OptQuat(L, 4);
	const uint8 material = OptMaterial(L, 5);
	const ApplySDFMode applyMode = OptApplyMode(L, 6);
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 7, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	if ( !gModDataManager->checkRestrictionsAt(pVoxelWorld->getId(), vCenterPosition, ApplyModeToBuildFlags(applyMode)) )
		return 0;

	const float outerRadius = majorRadius + minorRadius;
	const IntBounds chunkBounds = {
		i32Vec3(glm::floor((vCenterPosition - Vec3(outerRadius)) / float(MetersPerChunkAxis))),
		i32Vec3(glm::floor((vCenterPosition + Vec3(outerRadius)) / float(MetersPerChunkAxis)))
	};
	const Quat invRot = glm::inverse(rot);

	const float surfaceFalloff = CalculateSurfaceFalloff(Vec3(outerRadius));

	const ApplySDF_T applySDF = GetApplySDFMethod(applyMode);

	pVoxelWorld->createAndIterateChunksAndVoxels(
		chunkBounds, true, ((applyMode & ApplySDFMode::ApplyDensity) != 0),
		[applySDF, &vCenterPosition, &invRot, majorRadius, minorRadius, surfaceFalloff, material]
		(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex) {
			const Vec3 vVoxelLocal = invRot * (vVoxelPosition - vCenterPosition);

			const Vec2 q = Vec2(glm::length(Vec2(vVoxelLocal)) - majorRadius, vVoxelLocal.z);
			const float distance = glm::length(q) - minorRadius;

			applySDF(pChunk, uVoxelIndex, distance, surfaceFalloff, material);
		}
	);
	return 0;
}

int wrap_VoxelTerrain::iterateVoxels(lua_State* L) {
	CheckArgsMinMax(L, 3, 7);
	LuaManager::Get()->checkServerMode(L, true);

	if ( !(lua_isfunction(L, 1) && !lua_iscfunction(L, 1)) )
		luaL_argerror(L, 1, "Lua function expected");

	const Vec3 vCenter = CheckVec3(L, 2);
	const Vec3 vHalfExtents = CheckVec3InRange(L, 3, Vec3(0.5f), Vec3(INFINITY));
	const Quat rot = OptQuat(L, 4);
	const bool createChunks = OptBoolean(L, 5, true);
	const bool clearEdgeVoxels = OptBoolean(L, 6, true);
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 7, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	const Bounds bounds = {
		vCenter - vHalfExtents,
		vCenter + vHalfExtents
	};
	const IntBounds chunkBounds = CalculateShapeChunkBounds(vCenter, rot, vHalfExtents);
	const Quat invRot = glm::inverse(rot);

	LuaVM* pVM = LuaVM::Get(L);
	pVM->pushCallback("<Voxel Callback>");

	pVoxelWorld->createAndIterateChunksAndVoxels(
		chunkBounds, createChunks, clearEdgeVoxels,
		[&vCenter, &invRot, &bounds, L]
		(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex) {
			const Vec3 vLocal = invRot * (vVoxelPosition - vCenter);
			const uint8 vox = pChunk->voxels[uVoxelIndex];
			
			lua_pushvalue(L, 1);
			PushVec3(L, vVoxelPosition);
			PushVec3(L, vLocal);
			lua_pushinteger(L, (vox >> 6) & 0b00000011);
			lua_pushinteger(L, vox & 0b00111111);
			lua_call(L, 4, 2);

			if ( !lua_isnumber(L, -2) )
				luaL_error(L, "callback error: return value #1 expected integer, got %s", lua_typename(L, lua_type(L, -2)));
			if ( !lua_isnumber(L, -1) )
				luaL_error(L, "callback error: return value #2 expected integer, got %s", lua_typename(L, lua_type(L, -1)));

			const uint8 material = uint8(lua_tointeger(L, -2));
			const uint8 density = uint8(lua_tointeger(L, -1));
			pChunk->voxels[uVoxelIndex] = ((material & 0b00000011) << 6) | (density & 0b00111111);

			lua_pop(L, 2);
		}
	);
	
	pVM->popCallback();

	return 0;
}

int wrap_VoxelTerrain::eraseVoxelsInSphere(lua_State* L) {
	CheckArgsMinMax(L, 3, 4);
	LuaManager::Get()->checkServerMode(L, true);

	const Vec3 vSpherePosition = CheckVec3(L, 1);
	const float radius = CheckNumberInRange(L, 2, 0.5f, INFINITY);
	const float strength = CheckNumberInRange(L, 3, 0.0f, 1.0f);
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 4, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	if ( !gModDataManager->checkRestrictionsAt(pVoxelWorld->getId(), vSpherePosition, RestrictionFlags::VoxelRemoval) )
		return 0;

	const IntBounds chunkBounds = {
		i32Vec3(glm::floor((vSpherePosition - Vec3(radius)) / float(MetersPerChunkAxis))),
		i32Vec3(glm::floor((vSpherePosition + Vec3(radius)) / float(MetersPerChunkAxis)))
	};

	pVoxelWorld->createAndIterateChunksAndVoxels(
		chunkBounds, false, false,
		[&vSpherePosition, radius, strength]
		(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex) {
			float distance = glm::distance(vVoxelPosition, vSpherePosition);
			if ( distance <= radius ) {
				const uint8 vox = pChunk->voxels[uVoxelIndex];
				const float fDensity = float(vox & 0b00111111);

				const float distanceNorm = distance / radius;
				const float k = glm::mix(0.063f, 16.0f, strength);
				const float falloff = glm::clamp(1.0f - glm::pow(distanceNorm, k), 0.0f, 0.9f);

				const uint8 newDensity = glm::clamp(uint8(glm::mix(fDensity, 0.0f, falloff)), uint8(0), MaxVoxelDensity);

				pChunk->voxels[uVoxelIndex] = (vox & 0b11000000) | newDensity;
			}
		}
	);
	return 0;
}

int wrap_VoxelTerrain::reduceVoxelsInSphere(lua_State* L) {
	CheckArgsMinMax(L, 2, 4);
	LuaManager::Get()->checkServerMode(L, true);

	const Vec3 vSpherePosition = CheckVec3(L, 1);
	const float radius = CheckNumberInRange(L, 2, 0.5f, INFINITY);
	const float strength = CheckNumberInRange(L, 3, 0.0f, 1.0f);
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 4, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	if ( !gModDataManager->checkRestrictionsAt(pVoxelWorld->getId(), vSpherePosition, RestrictionFlags::VoxelRemoval) )
		return 0;

	const IntBounds chunkBounds = {
		i32Vec3(glm::floor((vSpherePosition - Vec3(radius)) / float(MetersPerChunkAxis))),
		i32Vec3(glm::floor((vSpherePosition + Vec3(radius)) / float(MetersPerChunkAxis)))
	};

	pVoxelWorld->createAndIterateChunksAndVoxels(
		chunkBounds, false, false,
		[&vSpherePosition, radius, strength]
		(const Vec3& vVoxelPosition, VoxelTerrainChunk* pChunk, uint32 uVoxelIndex) {
			// Core logic extracted from the game's executable
			const Vec3 vDistanceToCenter = vSpherePosition - vVoxelPosition;
			const float fDistance = (radius - glm::length(vDistanceToCenter)) + 0.5f;
			if ( fDistance > 0.0f ) {
				const uint8 vox = pChunk->voxels[uVoxelIndex];
				float fDensity = float(vox & 0b00111111) * 0.015873017f;
				if ( fDensity > 0.0f ) {
					fDensity -= fDistance * strength;
					fDensity = glm::clamp(fDensity, 0.0f, 1.0f);
					pChunk->voxels[uVoxelIndex] = (vox & 0b11000000) | (uint8(fDensity * MaxVoxelDensity) & 0b00111111);
				}
			}
		}
	);
	return 0;
}

int wrap_VoxelTerrain::importMesh(lua_State* L) {
	CheckArgsMinMax(L, 3, 9);
	LuaManager::Get()->checkServerMode(L, true);

	std::string meshPath = CheckString(L, 1);
	const Vec3 vCenter = CheckVec3(L, 2);
	const Vec3 vHalfSize = CheckVec3InRange(L, 3, Vec3(1.0f), Vec3(FLT_MAX));
	const Quat rotation = OptQuat(L, 4);
	std::string texPath = OptString(L, 5);
	const uint8 material = OptMaterial(L, 6);
	const bool bSubtractive = OptBoolean(L, 7, false);
	const float smoothingRadius = float(luaL_optnumber(L, 8, 1.0f));
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 9, true);
	if ( pVoxelWorld == nullptr )
		return 0;

	gVoxelizer->importMesh(pVoxelWorld->getId(), meshPath, vCenter, rotation, vHalfSize, texPath, material, bSubtractive, smoothingRadius);
	return 0;
}

int wrap_VoxelTerrain::reloadMesh(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	LuaManager::Get()->checkServerMode(L, true);
	const std::string_view meshPath = CheckStringView(L, 1);
	gVoxelizer->reloadMesh(meshPath);
	return 0;
}

int wrap_VoxelTerrain::reloadMeshTexture(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	LuaManager::Get()->checkServerMode(L, true);
	const std::string texPath = CheckString(L, 1);
	gVoxelizer->reloadTexture(texPath);
	return 0;
}

int wrap_VoxelTerrain::getWorldBounds(lua_State* L) {
	CheckArgsMinMax(L, 0, 1);
	const VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 1, false);
	const Bounds bounds = pVoxelWorld->getBounds();
	PushVec3(L, bounds.min);
	PushVec3(L, bounds.max);
	return 2;
}

int wrap_VoxelTerrain::getWorldChunkBounds(lua_State* L) {
	CheckArgsMinMax(L, 0, 1);
	const VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 1, false);
	const IntBounds bounds = pVoxelWorld->getChunkBounds();
	PushVec3(L, bounds.min);
	PushVec3(L, bounds.max);
	return 2;
}

int wrap_VoxelTerrain::exportChunk(lua_State* L) {
	CheckArgsMinMax(L, 1, 2);
	const i32Vec3 vChunkIndex = i32Vec3(CheckVec3(L, 1));
	const VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 2, false);

	const IntBounds worldBounds = pVoxelWorld->getChunkBounds();
	if ( glm::any(glm::lessThan(vChunkIndex, worldBounds.min)) || glm::any(glm::greaterThan(vChunkIndex, worldBounds.max)) )
		return 0;

	const uint32 uChunkIndex = pVoxelWorld->getChunkIndex(vChunkIndex);
	const VoxelTerrainChunk* pChunk = pVoxelWorld->getChunk(uChunkIndex);
	if ( pChunk == nullptr )
		return 0;

	ChunkToLuaTable(L, *pChunk);

	return 1;
}

int wrap_VoxelTerrain::importChunk(lua_State* L) {
	CheckArgsMinMax(L, 2, 6);
	LuaManager::Get()->checkServerMode(L, true);

	const i32Vec3 vChunkIndex = i32Vec3(CheckVec3(L, 1));
	const bool merge = OptBoolean(L, 3, false);
	const uint8 rotID = glm::clamp(uint8(luaL_optinteger(L, 4, 0)), uint8(0), uint8(3));
	const uint8 axisID = glm::clamp(uint8(luaL_optinteger(L, 5, 0)), uint8(0), uint8(2));
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 6, true);
	luaL_checktype(L, 2, LUA_TTABLE);

	if ( pVoxelWorld == nullptr )
		return 0;

	const IntBounds worldBounds = pVoxelWorld->getChunkBounds();
	if ( glm::any(glm::lessThan(vChunkIndex, worldBounds.min)) || glm::any(glm::greaterThan(vChunkIndex, worldBounds.max)) )
		luaL_error(L,
			"Chunk index is out of bounds! index: {%i, %i, %i}, min: {%i, %i, %i}, max: {%i, %i, %i}",
			vChunkIndex.x, vChunkIndex.y, vChunkIndex.z, worldBounds.min.x, worldBounds.min.y, worldBounds.min.z,
			worldBounds.max.x, worldBounds.max.y, worldBounds.max.z
		);

	VoxelTerrainChunk* pChunk = pVoxelWorld->getOrCreateChunk(vChunkIndex, false);
	if ( pChunk == nullptr )
		return luaL_error(L, "Failed to get chunk");

	VoxelTerrainChunk newChunk;

	LuaTableToChunk(L, 2, newChunk);

	if ( rotID != 0 )
		CopyChunkRotated(&newChunk, pChunk, axisID, rotID, merge);
	else {
		if ( merge ) {
			for ( uint16 i = 0; i < sizeof(VoxelTerrainChunk::voxels); ++i ) {
				const uint8 oldVox = pChunk->voxels[i];
				const uint8 newVox = newChunk.voxels[i];
				if ( (newVox & 0b00111111) >= (oldVox & 0b00111111) )
					pChunk->voxels[i] = newVox;
			}
		} else
			memcpy(pChunk->voxels, newChunk.voxels, sizeof(VoxelTerrainChunk::voxels));
	}

	const glm::bvec3 vMinEdges = {
		vChunkIndex.x == worldBounds.min.x,
		vChunkIndex.y == worldBounds.min.y,
		vChunkIndex.z == worldBounds.min.z
	};
	const glm::bvec3 vMaxEdges = {
		vChunkIndex.x == worldBounds.max.x,
		vChunkIndex.y == worldBounds.max.y,
		vChunkIndex.z == worldBounds.max.z
	};
	ClearChunkEdgeVoxels(*pChunk, vMinEdges, vMaxEdges);

	pVoxelWorld->updateChunk(vChunkIndex);

	return 0;
}

int wrap_VoxelTerrain::exportSerializedChunk(lua_State* L) {
	CheckArgsMinMax(L, 1, 2);
	const i32Vec3 vChunkIndex = i32Vec3(CheckVec3(L, 1));
	const VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 2, false);

	const IntBounds worldBounds = pVoxelWorld->getChunkBounds();
	if ( glm::any(glm::lessThan(vChunkIndex, worldBounds.min)) || glm::any(glm::greaterThan(vChunkIndex, worldBounds.max)) )
		return 0;

	const uint32 uChunkIndex = pVoxelWorld->getChunkIndex(vChunkIndex);
	const VoxelTerrainChunk* pChunk = pVoxelWorld->getChunk(uChunkIndex);
	if ( pChunk == nullptr )
		return 0;

	std::string data;

	if ( !SerializeChunk(*pChunk, data) )
		luaL_error(L, "failed to serialize chunk");

	lua_pushlstring(L, data.data(), data.size());

	return 1;
}

int wrap_VoxelTerrain::importSerializedChunk(lua_State* L) {
	CheckArgsMinMax(L, 2, 6);
	LuaManager::Get()->checkServerMode(L, true);

	const i32Vec3 vChunkIndex = i32Vec3(CheckVec3(L, 1));
	const bool merge = OptBoolean(L, 3, false);
	const uint8 rotID = glm::clamp(uint8(luaL_optinteger(L, 4, 0)), uint8(0), uint8(3));
	const uint8 axisID = glm::clamp(uint8(luaL_optinteger(L, 5, 0)), uint8(0), uint8(2));
	VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 6, true);
	std::string data = CheckString(L, 2);
	if ( pVoxelWorld == nullptr )
		return 0;

	const IntBounds worldBounds = pVoxelWorld->getChunkBounds();
	if ( glm::any(glm::lessThan(vChunkIndex, worldBounds.min)) || glm::any(glm::greaterThan(vChunkIndex, worldBounds.max)) ) {
		// Lua doesn't call the destructor
		data.~data();
		luaL_error(L,
			"Chunk index is out of bounds! index: {%i, %i, %i}, min: {%i, %i, %i}, max: {%i, %i, %i}",
			vChunkIndex.x, vChunkIndex.y, vChunkIndex.z, worldBounds.min.x, worldBounds.min.y, worldBounds.min.z,
			worldBounds.max.x, worldBounds.max.y, worldBounds.max.z
		);
	}

	VoxelTerrainChunk* pWorldChunk = pVoxelWorld->getOrCreateChunk(vChunkIndex, false);
	if ( pWorldChunk == nullptr )
		return 0;

	VoxelTerrainChunk newChunk;
	if ( !DeserializeChunk(data, newChunk) ) {
		// Lua doesn't call the destructor
		data.~data();
		luaL_error(L, "failed to deserialize chunk");
	}

	if ( rotID != 0 )
		CopyChunkRotated(&newChunk, pWorldChunk, axisID, rotID, merge);
	else {
		if ( merge ) {
			for ( uint16 i = 0; i < sizeof(VoxelTerrainChunk::voxels); ++i ) {
				const uint8 oldVox = pWorldChunk->voxels[i];
				const uint8 newVox = newChunk.voxels[i];
				if ( (newVox & 0b00111111) >= (oldVox & 0b00111111) )
					pWorldChunk->voxels[i] = newVox;
			}
		} else
			memcpy(pWorldChunk->voxels, newChunk.voxels, sizeof(VoxelTerrainChunk::voxels));
	}

	const glm::bvec3 vMinEdges = {
		vChunkIndex.x == worldBounds.min.x,
		vChunkIndex.y == worldBounds.min.y,
		vChunkIndex.z == worldBounds.min.z
	};
	const glm::bvec3 vMaxEdges = {
		vChunkIndex.x == worldBounds.max.x,
		vChunkIndex.y == worldBounds.max.y,
		vChunkIndex.z == worldBounds.max.z
	};
	ClearChunkEdgeVoxels(*pWorldChunk, vMinEdges, vMaxEdges);

	pVoxelWorld->updateChunk(vChunkIndex);

	return 0;
}

int wrap_VoxelTerrain::isChunkEmpty(lua_State* L) {
	CheckArgsMinMax(L, 1, 2);
	const i32Vec3 vChunkIndex = i32Vec3(CheckVec3(L, 1));
	const VoxelTerrainWorld* pVoxelWorld = GetVoxelWorld(L, 2, false);

	const uint32 uChunkIndex = pVoxelWorld->getChunkIndex(vChunkIndex);
	const VoxelTerrainPhysicsProxy* pProxy = pVoxelWorld->getChunkProxy(uChunkIndex);

	lua_pushboolean(L, pProxy == nullptr);
	return 1;
}



void wrap_VoxelTerrain::VoxelTerrain_print(void* self, std::ostream* out) {
	*out << "{<VoxelTerrain>, materialId = " << *(uint32*)self << "}";
}

int wrap_VoxelTerrain::VoxelTerrain_index(lua_State* L) {
	CheckArgsMinMax(L, 2, 2);
	uint64 len = 0;
	const char* key = luaL_checklstring(L, 2, &len);
	if ( std::string_view(key, len) == "materialId" ) {
		lua_pushinteger(L, *(uint32*)luaL_checkudata(L, 1, "VoxelTerrain"));
		return 1;
	}
	lua_getmetatable(L, 1);
	lua_getfield(L, -1, key);
	const int t = lua_type(L, -1);
	if ( t < LUA_TBOOLEAN )
		return luaL_error(L, "Unknown member \'%s\' in userdata", key);
	return 1;
}

int wrap_VoxelTerrain::VoxelTerrain_getMaterialId(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	lua_pushinteger(L, *(uint32*)luaL_checkudata(L, 1, "VoxelTerrain"));
	return 1;
}
