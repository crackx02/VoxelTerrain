#pragma once

#include "Util.hpp"

namespace SM {
	class GameScript {
		public:
			static GameScript** _selfPtr;
			inline static GameScript* Get() {return *_selfPtr;};

			inline bool getEnableRestrictions() const {return m_bEnableRestrictions;};

		private:
			char _pad0[0xD9];
			bool m_bEnableRestrictions;
			char _pad1[0x7E];
	};
	ASSERT_SIZE(GameScript, 0x158);
}
