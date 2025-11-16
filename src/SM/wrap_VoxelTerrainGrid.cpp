
#include "wrap_VoxelTerrainGrid.hpp"
#include "wrap_VoxelShared.hpp"
#include "VoxelUtils.hpp"
#include "Console.hpp"
#include "LuaVM.hpp"
#include "VoxelTerrainGrid.hpp"
#include "VoxelTerrainManager.hpp"
#include "VoxelTerrainConstants.hpp"
#include "LuaUtils.hpp"

using namespace DLL;
using namespace SM;
using namespace VoxelConstants;

static VoxelTerrainGrid* CheckGrid(LuaVM* pVM, const char* method) {
	VoxelTerrainGrid* pGrid = (VoxelTerrainGrid*)pVM->getPtr("VoxelTerrainGridPtr");
	if ( pGrid == nullptr )
		luaL_error(pVM->getLua(), "%s is not available in this callback! Only use %s in GetVoxelTerrainForCell.", method, method);
	return pGrid;
}

static void CalculateGridChunkIndexBounds(VoxelTerrainGrid* pGrid, i32Vec3& min, i32Vec3& max) {
	const i32Vec2 worldPosXY = pGrid->boundsMin;
	const i32Vec2 cellIndexXY = worldPosXY / (ChunksPerCellXY * MetersPerChunkAxis);
	min = i32Vec3(cellIndexXY * ChunksPerCellXY, ChunkIndexMinZ);
	max = i32Vec3(i32Vec2(min) + (ChunksPerCellXY - 1), ChunkIndexMaxZ);
}



void wrap_VoxelTerrainGrid::Register(lua_State* L) {
	SM_LOG("Registering sm.voxelTerrainGrid");

	lua_getglobal(L, "unsafe_env");
	SM_ASSERT(lua_istable(L, -1));
	
	lua_pushstring(L, "sm");
	lua_rawget(L, -2);
	SM_ASSERT(lua_istable(L, -1));

	lua_pushstring(L, "voxelTerrainGrid");
	lua_newtable(L);

	lua_pushstring(L, "createChunk");
	lua_pushcfunction(L, createChunk);
	lua_rawset(L, -3);

	lua_pushstring(L, "createSerializedChunk");
	lua_pushcfunction(L, createSerializedChunk);
	lua_rawset(L, -3);

	lua_pushstring(L, "getChunkIndexBounds");
	lua_pushcfunction(L, getChunkIndexBounds);
	lua_rawset(L, -3);

	wrap_VoxelShared::Register(L);

	lua_rawset(L, -3);

	lua_pop(L, 2);

	SM_LOG("sm.voxelTerrainGrid registered");
}



int wrap_VoxelTerrainGrid::createChunk(lua_State* L) {
	CheckArgsMinMax(L, 4, 4);
	VoxelTerrainGrid* pGrid = CheckGrid(LuaVM::Get(L), "createChunk");

	const int x = int(luaL_checkinteger(L, 1));
	const int y = int(luaL_checkinteger(L, 2));
	const int z = int(luaL_checkinteger(L, 3));

	{
		i32Vec3 bMin;
		i32Vec3 bMax;
		CalculateGridChunkIndexBounds(pGrid, bMin, bMax);

		if ( x < bMin.x || x > bMax.x )
			luaL_error(L, "chunk index X is out of bounds! Value: %i, Min: %i, Max: %i", x, bMin.x, bMax.x);

		if ( y < bMin.y || y > bMax.y )
			luaL_error(L, "chunk index Y is out of bounds! Value: %i, Min: %i, Max: %i", y, bMin.y, bMax.y);

		if ( z < bMin.z || z > bMax.z )
			luaL_error(L, "chunk index Z is out of bounds! Value: %i, Min: %i, Max: %i", z, bMin.z, bMax.z);
	}

	VoxelTerrainChunk chunk;

	LuaTableToChunk(L, 4, chunk);

	VoxelChunkPtr& chunkPtr = pGrid->vecChunks.emplace_back();
	chunkPtr.index3D = {x, y, z};
	chunkPtr.pChunk = VoxelTerrainManager::AllocateVoxelChunk();

	memcpy(chunkPtr.pChunk->voxels, chunk.voxels, sizeof(chunk.voxels));

	return 0;
}

int wrap_VoxelTerrainGrid::createSerializedChunk(lua_State* L) {
	CheckArgsMinMax(L, 4, 9);

	VoxelTerrainGrid* pGrid = CheckGrid(LuaVM::Get(L), "createChunk");

	const int x = int(luaL_checkinteger(L, 1));
	const int y = int(luaL_checkinteger(L, 2));
	const int z = int(luaL_checkinteger(L, 3));
	const uint8 rotID = glm::clamp(uint8(luaL_optinteger(L, 5, 0)), uint8(0), uint8(3));
	const uint8 axisID = glm::clamp(uint8(luaL_optinteger(L, 6, 0)), uint8(0), uint8(2));
	const i32Vec3 vClearAxes = glm::clamp(
		i32Vec3(
			int(luaL_optinteger(L, 7, 0)),
			int(luaL_optinteger(L, 8, 0)),
			int(luaL_optinteger(L, 9, 0))
		),
		i32Vec3(-1), i32Vec3(1)
	);
	std::string data = CheckString(L, 4);

	VoxelTerrainChunk chunk;
	if ( !DeserializeChunk(data, chunk) ) {
		data.~data();
		luaL_error(L, "failed to decode chunk data");
	}

	VoxelChunkPtr& chunkPtr = pGrid->vecChunks.emplace_back();
	chunkPtr.index3D = {x, y, z};
	chunkPtr.pChunk = VoxelTerrainManager::AllocateVoxelChunk();

	if ( rotID != 0 )
		CopyChunkRotated(&chunk, chunkPtr.pChunk, axisID, rotID);
	else
		memcpy(chunkPtr.pChunk->voxels, chunk.voxels, sizeof(VoxelTerrainChunk::voxels));

	const glm::bvec3 vMinEdges = {
		vClearAxes.x == -1,
		vClearAxes.y == -1,
		vClearAxes.z == -1
	};
	const glm::bvec3 vMaxEdges = {
		vClearAxes.x == 1,
		vClearAxes.y == 1,
		vClearAxes.z == 1
	};
	ClearChunkEdgeVoxels(*chunkPtr.pChunk, vMinEdges, vMaxEdges);

	return 0;
}

int wrap_VoxelTerrainGrid::getChunkIndexBounds(lua_State* L) {
	luaL_checkstack(L, 0, "expected 0 arguments");
	LuaVM* pVM = LuaVM::Get(L);
	VoxelTerrainGrid* pGrid = CheckGrid(pVM, "getChunkIndexBounds");

	i32Vec3 bMin;
	i32Vec3 bMax;
	CalculateGridChunkIndexBounds(pGrid, bMin, bMax);

	pVM->pushVec3(bMin);
	pVM->pushVec3(bMax);

	return 2;
}
