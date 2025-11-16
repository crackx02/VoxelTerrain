#pragma once

#include "lua.hpp"

#include "Types.hpp"

namespace SM {
	struct LuaUserdata {
		const char* name;
		uint32 typeID;
		uint32 domain;
		void(*pfPrint)(void* self, std::ostream* out);
		bool(*pfExists)(lua_State* L);
		void(*pfSerialize)(void* self, class BitStream* pBs);
		void(*pfDeserializeAndPush)(lua_State* L, class BitStream* pBs, const char* metatableName);
	};
}
