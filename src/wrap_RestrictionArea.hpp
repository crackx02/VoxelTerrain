#pragma once

#include <ostream>

#include "lua.hpp"

#include "SM/LuaUserdata.hpp"

namespace DLL {
	namespace wrap_RestrictionArea {
		void Register(lua_State* L);
		SM::LuaUserdata* GetUserdata();
		luaL_Reg* GetMetatable();
		luaL_Reg* GetMethodsTable();

		int setGlobalRestrictions(lua_State* L);
		int getGlobalRestrictions(lua_State* L);
		int createRestrictionArea(lua_State* L);
		int getRestrictionsAt(lua_State* L);
		int getRestrictionAreasAt(lua_State* L);
		int getRestrictionAreasInBounds(lua_State* L);
		int checkRestrictionsAt(lua_State* L);

		int RestrictionArea_getBounds(lua_State* L);
		int RestrictionArea_getWorld(lua_State* L);
		int RestrictionArea_getFlags(lua_State* L);
		int RestrictionArea_getActive(lua_State* L);

		int RestrictionArea_setBounds(lua_State* L);
		int RestrictionArea_setFlags(lua_State* L);
		int RestrictionArea_setActive(lua_State* L);

		int RestrictionArea_destroy(lua_State* L);

		void RestrictionArea_print(void* self, std::ostream* out);
		bool RestrictionArea_exists(lua_State* L);
		int RestrictionArea_index(lua_State* L);
		int RestrictionArea_newindex(lua_State* L);
		int RestrictionArea_tostring(lua_State* L);
		int RestrictionArea_gc(lua_State* L);
	}
}
