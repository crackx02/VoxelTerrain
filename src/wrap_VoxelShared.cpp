
#include "wrap_VoxelShared.hpp"
#include "LuaUtils.hpp"
#include "VoxelUtils.hpp"
#include "SM/VoxelTerrainConstants.hpp"
#include "SM/Console.hpp"

using namespace DLL;
using namespace SM;
using namespace VoxelConstants;

void wrap_VoxelShared::Register(lua_State* L) {
	SM_ASSERT(lua_istable(L, -1));

	lua_pushstring(L, "constants");
	lua_newtable(L);

	lua_pushstring(L, "worldBoundsMinZ");
	lua_pushnumber(L, WorldBoundsMinZ);
	lua_rawset(L, -3);

	lua_pushstring(L, "worldBoundsMaxZ");
	lua_pushnumber(L, WorldBoundsMaxZ);
	lua_rawset(L, -3);

	lua_pushstring(L, "chunkIndexMinZ");
	lua_pushinteger(L, ChunkIndexMinZ);
	lua_rawset(L, -3);

	lua_pushstring(L, "chunkIndexMaxZ");
	lua_pushinteger(L, ChunkIndexMaxZ);
	lua_rawset(L, -3);

	lua_pushstring(L, "metersPerChunkAxis");
	lua_pushinteger(L, MetersPerChunkAxis);
	lua_rawset(L, -3);

	lua_pushstring(L, "voxelsPerChunkAxis");
	lua_pushinteger(L, VoxelsPerChunkAxis);
	lua_rawset(L, -3);

	lua_pushstring(L, "chunksPerCellXY");
	lua_pushnumber(L, ChunksPerCellXY);
	lua_rawset(L, -3);

	lua_pushstring(L, "maxVoxelDensity");
	lua_pushnumber(L, MaxVoxelDensity);
	lua_rawset(L, -3);

	lua_rawset(L, -3);

	lua_pushstring(L, "createVoxelArray");
	lua_pushcfunction(L, createVoxelArray);
	lua_rawset(L, -3);

	lua_pushstring(L, "serializeChunkData");
	lua_pushcfunction(L, serializeChunkData);
	lua_rawset(L, -3);

	lua_pushstring(L, "deserializeChunkData");
	lua_pushcfunction(L, deserializeChunkData);
	lua_rawset(L, -3);
}



int wrap_VoxelShared::createVoxelArray(lua_State* L) {
	CheckArgsMinMax(L, 0, 2);
	uint8 material = uint8(luaL_optinteger(L, 1, 0));
	uint8 density = uint8(luaL_optinteger(L, 2, 0));
	lua_createtable(L, VoxelTableSize, 0);
	for ( int i = 1; i <= VoxelTableSize; i += 2 ) {
		lua_pushinteger(L, material);
		lua_rawseti(L, -2, i);
		lua_pushinteger(L, density);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

int wrap_VoxelShared::serializeChunkData(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);

	VoxelTerrainChunk chunk;
	LuaTableToChunk(L, 1, chunk);

	std::string data;
	if ( !SerializeChunk(chunk, data) )
		luaL_error(L, "failed to serialize chunk");

	lua_pushlstring(L, data.data(), data.size());

	return 1;
}

int wrap_VoxelShared::deserializeChunkData(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);

	VoxelTerrainChunk chunk;
	if ( !DeserializeChunk(CheckStringView(L, 1), chunk) )
		luaL_error(L, "failed to deserialize chunk");

	ChunkToLuaTable(L, chunk);
	return 1;
}
