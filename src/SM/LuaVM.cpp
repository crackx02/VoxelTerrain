
#include "SM/LuaVM.hpp"
#include "SM/Console.hpp"

using namespace SM;

LuaVM* LuaVM::Get(lua_State* L) {
	lua_pushstring(L, "VMPtr");
	lua_rawget(L, LUA_REGISTRYINDEX);
	SM_ASSERT(lua_type(L, -1) == LUA_TLIGHTUSERDATA);
	LuaVM* self = (LuaVM*)lua_topointer(L, -1);
	lua_pop(L, 1);
	return self;
}

void* LuaVM::getPtr(const char* name) {
	const int top = lua_gettop(m_L);
	lua_pushstring(m_L, name);
	lua_rawget(m_L, LUA_REGISTRYINDEX);
	void* ptr = (lua_type(m_L, -1) == LUA_TLIGHTUSERDATA) ? (void*)lua_topointer(m_L, -1) : nullptr;
	lua_settop(m_L, top);
	return ptr;
}

void LuaVM::pushVec3(const Vec3& vec) {
	Vec3* pVec3 = (Vec3*)lua_newuserdata(m_L, sizeof(Vec3));
	*pVec3 = vec;
	luaL_setmetatable(m_L, "Vec3");
}
