#pragma once

#include "lua.hpp"

namespace SM {
	namespace wrap_VoxelTerrainGrid {
		void Register(lua_State* L);

		int createChunk(lua_State* L);
		int createSerializedChunk(lua_State* L);
		int getChunkIndexBounds(lua_State* L);
	}
}
