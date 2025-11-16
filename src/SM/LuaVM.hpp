#pragma once

#include <vector>
#include <string>

#include "lua.hpp"

#include "Types.hpp"

namespace SM {
	enum class LuaEnvironment : uint32 {
		Game,
		Terrain
	};

	class LuaVM {
		public:
			static LuaVM* Get(lua_State* L);

			inline lua_State* getLua() {return m_L;};

			void* getPtr(const char* name);

			void pushVec3(const Vec3& vec);

			inline void pushCallback(std::string&& name) {m_vecLastMethodStack.emplace_back(std::move(name));};
			inline void popCallback() {m_vecLastMethodStack.pop_back();};
			
		private:
			lua_State* m_L;
			char _pad0[0x80];
			std::vector<std::string> m_vecLastMethodStack;
	};
}
