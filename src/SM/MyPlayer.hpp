#pragma once

#include "RaycastResult.hpp"

namespace SM {
	class MyPlayer {
		public:
			static MyPlayer** _selfPtr;
			inline static MyPlayer* Get() {return *_selfPtr;};

			inline const RaycastResult& getRaycast() {return m_raycast;};

		private:
			char _pad1[0x210];
			RaycastResult m_raycast;
			char _pad2[0x74];
	};
	ASSERT_SIZE(MyPlayer, 0x370);
}
