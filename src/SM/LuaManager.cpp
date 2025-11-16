
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include "LuaManager.hpp"
#include "Util.hpp"

using namespace SM;

constexpr uintptr Offset_GetCurrentWorld = 0x044b2c0;

using GetCurrentWorldFunc = uint16(*)(LuaManager*, lua_State*, void*, bool);
GetCurrentWorldFunc g_GetCurrentWorld = nullptr;


LuaManager** LuaManager::_selfPtr = (LuaManager**)0x1267620;

uint16 LuaManager::getCurrentWorld(lua_State* L) {
	ResolveGlobal(GetCurrentWorld);
	return g_GetCurrentWorld(this, L, nullptr, false);
}
