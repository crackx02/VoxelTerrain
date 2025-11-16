#pragma once

#include <memory>

#include "Util.hpp"
#include "Types.hpp"

namespace SM {
	class ClientWorld {
		public:
			static ClientWorld** _selfPtr;
			inline static ClientWorld* Get() {return *_selfPtr;}

			inline uint16 getWorldID() const {return m_worldID;};

		private:
			char _pad0[0xF8];
			uint16 m_worldID;
			char _pad1[0x36];
	};
	ASSERT_SIZE(ClientWorld, 0x130);
}
