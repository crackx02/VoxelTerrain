#pragma once

#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "Util.hpp"
#include "Types.hpp"

namespace SM {
	class PhysicsProxy {
		public:

			struct Type {
				enum _Enum : uint8 {
					Null = 0x0,
					Limiter = 0x1,
					TerrainSurface = 0x2,
					TerrainAsset = 0x3,
					Body = 0x4,
					BodyJoint = 0x5,
					Lift = 0x6,
					Character = 0x7,
					Joint = 0x8,
					Harvestable = 0x9,
					Vision = 0xA,
					AreaTrigger = 0xB,	// Also WaterAreaTrigger
					Ragdoll = 0xC,
					VoxelTerrain = 0xD,
					TunnelCatcher = 0xE
				} value;

				constexpr inline Type(_Enum e) {value = e;};
				constexpr inline Type(int i) {value = _Enum(i);};
				constexpr inline operator _Enum() const {return value;};
			};

			virtual Type::_Enum type() {return Type::Null;};

			inline btRigidBody* getDynamicsWorldBody() const {return m_pDynamicsWorldRigidBody;};
			inline uint16 getWorld() const {return m_worldID;};

		private:
			char _pad0[0x8];
			btRigidBody* m_pDynamicsWorldRigidBody;
			btCollisionObject* m_pTickRaycastCollisionObject;
			btCollisionObject* m_pInterpolatedRaycastCollisionObject;
			uint16 m_worldID;
	};
	ASSERT_SIZE(PhysicsProxy, 0x30);
}
