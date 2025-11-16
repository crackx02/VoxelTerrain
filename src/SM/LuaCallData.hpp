#pragma once

#include "Types.hpp"
#include "Util.hpp"

namespace SM {
	namespace LuaCallData {
		enum class ObjectType : uint8 {
			Null,
			Shape,
			Harvestable,
			Character,
			Player,
			Unit,
			Lift,
			VoxelTerrain
		};

		struct WorldOnMelee {
			ObjectType attackerType;
			uint32 attackerID;
			Vec3 vHitPosition;
			Vec3 vHitNormal;
			Vec3 vHitDirection;
			int damage;
			float power;
			ObjectType targetType;
			uint32 targetID;
		};
		ASSERT_SIZE(WorldOnMelee, 0x3C);

		struct WorldOnProjectile {
			Vec3 vHitPosition;
			Vec3 vHitNormal;
			float airTime;
			Vec3 vVelocity;
			const char* pProjectileName;
			ObjectType shooterType;
			uint32 shooterID;
			int damage;
			void* pCustomData;
			int customDataSize;
			ObjectType targetType;
			uint32 targetID;
			char projectileUuid[16];
		};
		ASSERT_SIZE(WorldOnProjectile, 0x68);
	}
}
