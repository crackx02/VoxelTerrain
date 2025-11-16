#pragma once

#include "lua.hpp"

namespace DLL {
	namespace wrap_VoxelTerrain {
		void Register(lua_State* L);

		int getClosestVoxel(lua_State* L);
		int getVoxelsAt(lua_State* L);
		int getVoxelsInSphere(lua_State* L);

		int createSphereAt(lua_State* L);
		int createBoxAt(lua_State* L);
		int createCylinderAt(lua_State* L);
		int createConeAt(lua_State* L);
		int createCapsuleAt(lua_State* L);
		int createTorusAt(lua_State* L);
		int iterateVoxels(lua_State* L);
		int eraseVoxelsInSphere(lua_State* L);
		int reduceVoxelsInSphere(lua_State* L);
		int importMesh(lua_State* L);
		int reloadMesh(lua_State* L);
		int reloadMeshTexture(lua_State* L);
		int getWorldBounds(lua_State* L);
		int getWorldChunkBounds(lua_State* L);
		int exportChunk(lua_State* L);
		int importChunk(lua_State* L);
		int exportSerializedChunk(lua_State* L);
		int importSerializedChunk(lua_State* L);
		int isChunkEmpty(lua_State* L);

		void VoxelTerrain_print(void* self, std::ostream* out);
		int VoxelTerrain_index(lua_State* L);
		int VoxelTerrain_getMaterialId(lua_State* L);
	}
}
