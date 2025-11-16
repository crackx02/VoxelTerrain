#pragma once

#include <unordered_set>

#include "SM/TickDynamicsWorld.hpp"
#include "SM/World.hpp"
#include "Util.hpp"

namespace SM {
	class PhysicsBase {
		public:
			inline TickDynamicsWorld* getTickWorld() const {return m_pTickWorld;};
			inline World* getWorld() const {return m_pWorld;}

			inline btBroadphaseInterface* getTickWorldBroadphase() const { return m_pTickWorldBroadphase; };
			inline btBroadphaseInterface* getCollisionWorldBroadphase() const {return m_pCollisionWorldBroadphase;};

		private:
			virtual void func() = 0;
			char _pad0[0x8];
			World* m_pWorld;
			char _pad1[0x60];
			btBroadphaseInterface* m_pTickWorldBroadphase;
			char _pad2[0x8];
			TickDynamicsWorld* m_pTickWorld;
			char _pad3[0xD0];
			btBroadphaseInterface* m_pCollisionWorldBroadphase;
			char _pad5[0x208];
	};
	ASSERT_SIZE(PhysicsBase, 0x370);
}
