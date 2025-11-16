#pragma once

#include "lua.hpp"

namespace DLL {
	namespace wrap_VoxelShared {
		void Register(lua_State* L);

		int createVoxelArray(lua_State* L);
		int serializeChunkData(lua_State* L);
		int deserializeChunkData(lua_State* L);
	}
}
