#pragma once

#include "Types.hpp"
#include "Util.hpp"

namespace SM {
	struct Sphere {
		Vec3 position;
		float radius;
	};
	ASSERT_SIZE(Sphere, 0x10);
}
