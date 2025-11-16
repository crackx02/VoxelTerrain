#pragma once

#include "glm/gtx/norm.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "lz4hc.h"
#include "base64.hpp"

#include "SM/VoxelTerrainWorld.hpp"
#include "SM/LuaManager.hpp"
#include "SM/VoxelTerrainManager.hpp"
#include "SM/VoxelTerrainConstants.hpp"
#include "SM/RenderStateManager.hpp"
#include "SM/Bounds.hpp"
#include "SM/Console.hpp"
#include "ModDataManager.hpp"
#include "Types.hpp"

namespace DLL {
	// Size of the Lua voxel chunk table
	constexpr static int VoxelTableSize = int(sizeof(SM::VoxelTerrainChunk::voxels)) * 2;

	static uint32 GetWorldOrScriptWorld(lua_State* L, int idx) {
		uint16 worldID = 0;
		if ( lua_type(L, idx) <= LUA_TNIL )
			worldID = SM::LuaManager::Get()->getCurrentWorld(L);
		else
			worldID = *(uint16*)luaL_checkudata(L, idx, "World");
		if ( worldID == SM::NoWorldID )
			luaL_error(L, "World does not exist");
		return worldID;
	}

	static SM::VoxelTerrainWorld* GetVoxelWorld(lua_State* L, int idx, bool requireEnabledTerrain) {
		uint32 worldID = GetWorldOrScriptWorld(L, idx);

		SM::VoxelTerrainWorld* pVoxelWorld = SM::VoxelTerrainManager::Get()->getWorld(worldID);
		if ( pVoxelWorld == nullptr )
			luaL_argerror(L, idx, "Voxel World does not exist");

		if ( requireEnabledTerrain && !DLL::gModDataManager->isVoxelTerrainEnabled(worldID) )
			return nullptr;

		return pVoxelWorld;
	}

	inline static uint8 OptMaterial(lua_State* L, int idx) {
		return (uint8(luaL_optinteger(L, idx, 0)) << 6) & 0b11000000;
	}

	inline static uint8 CheckMaterial(lua_State* L, int idx) {
		return (uint8(luaL_checkinteger(L, idx)) << 6) & 0b11000000;
	}

	static uint32 VoxelIndex3To1(const i32Vec3& vIdx) {
		constexpr int VoxelsPerAxis = SM::VoxelConstants::VoxelsPerChunkAxis;
		return (vIdx.z + (VoxelsPerAxis * vIdx.y) + ((VoxelsPerAxis * VoxelsPerAxis) * vIdx.x));
	}

	static SM::IntBounds CalculateShapeChunkBounds(const Vec3& vCenterPosition, const Quat& rot, const Vec3& vHalfExtents) {
		const Mat33 mRot = glm::mat3_cast(rot);
		const Vec3 x = mRot[0] * vHalfExtents.x;
		const Vec3 y = mRot[1] * vHalfExtents.y;
		const Vec3 z = mRot[2] * vHalfExtents.z;
		const Vec3 halfBounds = glm::abs(x) + glm::abs(y) + glm::abs(z);
		const SM::IntBounds chunkBounds = {
			i32Vec3(glm::floor((vCenterPosition - halfBounds) / float(SM::VoxelConstants::MetersPerChunkAxis))),
			i32Vec3(glm::floor((vCenterPosition + halfBounds) / float(SM::VoxelConstants::MetersPerChunkAxis)))
		};
		return chunkBounds;
	}

	inline static float CalculateSurfaceFalloff(const Vec3& vHalfExtents) {
		return glm::min(vHalfExtents.x * 0.75f, glm::min(vHalfExtents.y * 0.75f, glm::min(vHalfExtents.z * 0.75f, 2.0f)));
	}

	inline static uint8 CalculateDensityWithFalloff(uint8 previousDensity, float distance, float surfaceFalloff) {
		distance = glm::abs(distance);
		if ( distance <= surfaceFalloff ) {
			float distanceNorm = distance / surfaceFalloff;
			distanceNorm = glm::clamp(distanceNorm, 0.0f, 1.0f);
			return glm::max(previousDensity, uint8(distanceNorm * float(SM::VoxelConstants::MaxVoxelDensity))) & 0b00111111;
		}
		return SM::VoxelConstants::MaxVoxelDensity;
	}

	inline static uint8 CalculateDensityWithFalloffInv(uint8 previousDensity, float distance, float surfaceFalloff) {
		distance = glm::abs(distance);
		if ( distance <= surfaceFalloff ) {
			float distanceNorm = distance / surfaceFalloff;
			distanceNorm = 1.0f - glm::clamp(distanceNorm, 0.0f, 1.0f);
			return glm::min(previousDensity, uint8(distanceNorm * float(SM::VoxelConstants::MaxVoxelDensity))) & 0b00111111;
		}
		return 0;
	}

	static void CopyChunkRotated(SM::VoxelTerrainChunk* pSrcChunk, SM::VoxelTerrainChunk* pDstChunk, uint8 axisID, uint8 rotID, bool merge = false) {
		constexpr Vec3 arrAxes[3] = {
			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f}
		};
		constexpr int VoxelsPerAxis = SM::VoxelConstants::VoxelsPerChunkAxis;
		constexpr Vec3 vCenter = Vec3(float(VoxelsPerAxis - 1) * 0.5f);
		const Vec3 vAxis = arrAxes[axisID];
		const float angle = glm::radians(float(rotID) * -90.0f);
		const Mat44 rMat = glm::rotate(Mat44(1.0f), angle, vAxis);
		for ( uint8 z = 0; z < VoxelsPerAxis; ++z ) {
			for ( uint8 y = 0; y < VoxelsPerAxis; ++y ) {
				for ( uint8 x = 0; x < VoxelsPerAxis; ++x ) {
					const Vec3 vVoxelPos(x, y, z);
					const Vec3 vVoxelCentered = vVoxelPos - vCenter;

					const Vec4 vRotated = rMat * Vec4(vVoxelCentered, 1.0f);
					const Vec3 vNewPos = glm::round(Vec3(vRotated) + vCenter);

					const uint16 uVoxelIndexSrc = VoxelIndex3To1(vVoxelPos);
					const uint16 uVoxelIndexDst = VoxelIndex3To1(vNewPos);

					if ( merge ) {
						const uint8 oldVox = pDstChunk->voxels[uVoxelIndexDst];
						const uint8 newVox = pSrcChunk->voxels[uVoxelIndexSrc];
						if ( (newVox & 0b00111111) >= (oldVox & 0b00111111) )
							pDstChunk->voxels[uVoxelIndexDst] = newVox;
					} else
						pDstChunk->voxels[uVoxelIndexDst] = pSrcChunk->voxels[uVoxelIndexSrc];
				}
			}
		}
	}

	static void ClearChunkEdgeVoxels(SM::VoxelTerrainChunk& chunk, glm::bvec3 vMinEdges, glm::bvec3 vMaxEdges) {
		constexpr uint8 VoxelsPerAxis = SM::VoxelConstants::VoxelsPerChunkAxis;
		if ( vMinEdges.x )
			for ( uint8 vz = 0; vz < VoxelsPerAxis; ++vz )
				for ( uint8 vy = 0; vy < VoxelsPerAxis; ++vy )
					chunk.voxels[VoxelIndex3To1({0, vy, vz})] &= 0b11000000;
		if ( vMinEdges.y )
			for ( uint8 vz = 0; vz < VoxelsPerAxis; ++vz )
				for ( uint8 vx = 0; vx < VoxelsPerAxis; ++vx )
					chunk.voxels[VoxelIndex3To1({vx, 0, vz})] &= 0b11000000;
		if ( vMinEdges.z )
			for ( uint8 vy = 0; vy < VoxelsPerAxis; ++vy )
				for ( uint8 vx = 0; vx < VoxelsPerAxis; ++vx )
					chunk.voxels[VoxelIndex3To1({vx, vy, 0})] &= 0b11000000;

		if ( vMaxEdges.x )
			for ( uint8 vz = 0; vz < VoxelsPerAxis; ++vz )
				for ( uint8 vy = 0; vy < VoxelsPerAxis; ++vy )
					chunk.voxels[VoxelIndex3To1({VoxelsPerAxis - 1, vy, vz})] &= 0b11000000;
		if ( vMaxEdges.y )
			for ( uint8 vz = 0; vz < VoxelsPerAxis; ++vz )
				for ( uint8 vx = 0; vx < VoxelsPerAxis; ++vx )
					chunk.voxels[VoxelIndex3To1({vx, VoxelsPerAxis - 1, vz})] &= 0b11000000;
		if ( vMaxEdges.z )
			for ( uint8 vy = 0; vy < VoxelsPerAxis; ++vy )
				for ( uint8 vx = 0; vx < VoxelsPerAxis; ++vx )
					chunk.voxels[VoxelIndex3To1({vx, vy, VoxelsPerAxis - 1})] &= 0b11000000;
	}

	static i32Vec3 WorldPositionToClosestVoxelIndex(const Vec3& vPosition) {
		Vec3 chunkFraction = glm::fract(vPosition / Vec3(SM::VoxelConstants::MetersPerChunkAxis));
		if ( chunkFraction.x < 0.0f )
			chunkFraction.x = 1.0f + chunkFraction.x;
		if ( chunkFraction.y < 0.0f )
			chunkFraction.y = 1.0f + chunkFraction.y;
		if ( chunkFraction.z < 0.0f )
			chunkFraction.z = 1.0f + chunkFraction.z;
		 
		return glm::round(chunkFraction * Vec3(SM::VoxelConstants::MetersPerChunkAxis));
	}

	static void LuaTableToChunk(lua_State* L, int idx, SM::VoxelTerrainChunk& chunk) {
		luaL_checktype(L, idx, LUA_TTABLE);

		{
			const size_t len = lua_objlen(L, idx);
			const size_t expected = VoxelTableSize;
			if ( len != expected )
				luaL_error(L, "voxel array size %i does not match expected size %i", len, expected);
		}

		int tableIndex = 1;
		int voxelIndex = 0;
		while ( tableIndex <= VoxelTableSize ) {
			lua_rawgeti(L, idx, tableIndex++);
			lua_rawgeti(L, idx, tableIndex++);
			{
				int tm = lua_type(L, -2);
				if ( tm != LUA_TNUMBER )
					luaL_error(L, "voxel array index %i expected integer, was %s", tableIndex - 1, lua_typename(L, tm));
				int td = lua_type(L, -1);
				if ( td != LUA_TNUMBER )
					luaL_error(L, "voxel array index %i expected integer, was %s", tableIndex, lua_typename(L, td));
			}
			uint8 material = uint8(lua_tointeger(L, -2)) << 6;
			uint8 density = uint8(lua_tointeger(L, -1));
			chunk.voxels[voxelIndex++] = (material & 0b11000000) | (density & 0b00111111);
			lua_pop(L, 2);
		}
	}

	static void ChunkToLuaTable(lua_State* L, const SM::VoxelTerrainChunk& chunk) {
		lua_createtable(L, VoxelTableSize, 0);
		uint32 ti = 0;
		for ( uint32 vi = 0; vi < sizeof(chunk.voxels); ++vi ) {
			uint8 vox = chunk.voxels[vi];
			lua_pushinteger(L, (vox & 0b11000000) >> 6);	// Material
			lua_rawseti(L, -2, ++ti);
			lua_pushinteger(L, (vox & 0b00111111));			// Density
			lua_rawseti(L, -2, ++ti);
		}
	}

	static bool SerializeChunk(const SM::VoxelTerrainChunk& chunk, std::string& data) {
		const int bound = LZ4_compressBound(sizeof(chunk.voxels));
		data.resize(bound);
		const int res = LZ4_compress_HC((const char*)chunk.voxels, data.data(), sizeof(chunk.voxels), bound, LZ4HC_CLEVEL_MAX);
		if ( res <= 0 ) {
			SM_ERROR("Failed to compress chunk");
			return false;
		}
		data.resize(res);

		try {
			data = std::move(base64::to_base64(data));
		} catch ( const std::exception& e ) {
			SM_ERROR("Failed to encode chunk: {}", e.what());
			return false;
		}

		return true;
	}

	static bool DeserializeChunk(const std::string_view& data, SM::VoxelTerrainChunk& chunk) {
		std::string decoded;
		try {
			decoded = std::move(base64::from_base64(data));
		} catch ( ... ) {
			SM_ERROR("Failed to decode chunk data");
			return false;
		}

		const int res = LZ4_decompress_safe(decoded.data(), (char*)chunk.voxels, int(decoded.size()), sizeof(chunk.voxels));
		if ( res <= 0 ) {
			SM_ERROR("Failed to decompress chunk");
			return false;
		}

		return true;
	}
}
