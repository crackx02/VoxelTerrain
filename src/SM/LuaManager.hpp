#pragma once

#include "lua.hpp"

#include "Types.hpp"

namespace SM {
	class LuaManager {
		public:
			static LuaManager** _selfPtr;
			inline static LuaManager* Get() {return *_selfPtr;};

			uint16 getCurrentWorld(lua_State* L);

			inline void checkServerMode(lua_State* L, bool mode) const {
				if ( mode == m_bServerMode )
					return;
				luaL_error(L, "%s function called from %s callback", mode ? "server" : "client", m_bServerMode ? "server" : "client");
			};

		private:
			char _pad0[0x5A];
			bool m_bServerMode;
	};
}
