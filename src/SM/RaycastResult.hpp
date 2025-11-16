#pragma once

#include "Types.hpp"
#include "Util.hpp"

namespace SM {
	enum class RaycastHitType : uint8 {
		None = 0,
		Limiter = 1,
		TerrainSurface = 2,
		TerrainAsset = 3,
		Body = 4,
		Joint1 = 5,
		Lift = 6,
		Character = 7,
		Joint2 = 8,
		Harvestable = 9,
		Vision = 10,
		AreaTrigger = 11,
		Ragdoll = 12,
		VoxelTerrain = 13,
		TunnelCatcher = 14
	};

	struct RaycastResult {
		bool bValid;
		char _pad0[0x3];
		Vec3 vOriginWorld;
		Vec3 vDirectionWorld;
		char _pad1[0x82];
		Vec3 normalWorld;
		Vec3 normalLocal;
		Vec3 pointWorld;
		Vec3 pointLocal;
		RaycastHitType type;
		char _pad2[0x7];
		int32 hitObjectID;
		int32 hitChildShapeIndex;
		char _pad3[0x8];
		float fraction;
	};
	ASSERT_SIZE(RaycastResult, 0xEC);
}
