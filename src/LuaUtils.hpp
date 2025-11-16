#pragma once

#include <string>

#include "lua.hpp"

#include "Types.hpp"

namespace DLL {
	static std::string CheckString(lua_State* L, int idx) {
		uint64 len = 0;
		const char* psMeshPath = luaL_checklstring(L, idx, &len);
		return {psMeshPath, len};
	}

	static std::string_view CheckStringView(lua_State* L, int idx) {
		uint64 len = 0;
		const char* psMeshPath = luaL_checklstring(L, idx, &len);
		return {psMeshPath, len};
	}

	static std::string OptString(lua_State* L, int idx) {
		if ( lua_type(L, idx) == LUA_TSTRING ) {
			uint64 len = 0;
			const char* psMeshPath = luaL_checklstring(L, idx, &len);
			return {psMeshPath, len};
		}
		return {};
	}

	static float CheckNumberInRange(lua_State* L, int idx, float min, float max) {
		return glm::clamp(float(luaL_checknumber(L, idx)), min, max);
	}

	inline static Vec3 CheckVec3(lua_State* L, int idx) {
		return *(Vec3*)luaL_checkudata(L, idx, "Vec3");
	}

	inline static Vec3 CheckVec3InRange(lua_State* L, int idx, const Vec3& min, const Vec3& max) {
		return glm::clamp(*(Vec3*)luaL_checkudata(L, idx, "Vec3"), min, max);
	}

	static Vec3 OptVec3InRange(lua_State* L, int idx, const Vec3& min, const Vec3& max) {
		int t = lua_type(L, idx);
		if ( t <= LUA_TNIL )
			return Vec3(0.0f, 0.0f, 0.0f);
		return glm::clamp(*(Vec3*)luaL_checkudata(L, idx, "Vec3"), min, max);
	}

	static Quat OptQuat(lua_State* L, int idx) {
		int t = lua_type(L, idx);
		if ( t <= LUA_TNIL )
			return Quat(1.0f, 0.0f, 0.0f, 0.0f);
		return *(Quat*)luaL_checkudata(L, idx, "Quat");
	}

	static bool CheckBoolean(lua_State* L, int idx) {
		int t = lua_type(L, idx);
		if ( t != LUA_TBOOLEAN )
			luaL_typerror(L, idx, "boolean");
		return lua_toboolean(L, idx);
	}

	static bool OptBoolean(lua_State* L, int idx, bool def) {
		int t = lua_type(L, idx);
		if ( t <= LUA_TNIL )
			return def;
		else if ( t != LUA_TBOOLEAN )
			luaL_typerror(L, idx, "boolean");
		return lua_toboolean(L, idx);
	}

	static void PushVec3(lua_State* L, const Vec3& vec) {
		Vec3* pVec = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
		luaL_setmetatable(L, "Vec3");
		*pVec = vec;
	}

	static void PushWorld(lua_State* L, uint16 worldID) {
		uint32* pID = (uint32*)lua_newuserdata(L, 4);
		luaL_setmetatable(L, "World");
		*pID = worldID;
	}

	static void CheckArgsMinMax(lua_State* L, int min, int max) {
		int top = lua_gettop(L);
		if ( top < min )
			luaL_error(L, "expected at least %i arguments, got %i", min, top);
		if ( top > max )
			luaL_error(L, "expected at most %i arguments, got %i", max, top);
	}
}
