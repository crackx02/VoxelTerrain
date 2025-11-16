#pragma once

#include <cstdint>

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "LinearMath/btVector3.h"

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using i32Vec3 = glm::i32vec3;
using i32Vec2 = glm::i32vec2;
using u32Vec3 = glm::u32vec3;
using Quat = glm::quat;
using Mat33 = glm::mat3x3;
using Mat44 = glm::mat4x4;

using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using int32 = int32_t;
using uint32 = uint32_t;
using int64= int64_t;
using uint64 = uint64_t;
using uintptr = uintptr_t;

using btVec3 = btVector3;

inline btVector3 ToBT(const Vec3& v) {
	return btVector3(v.x, v.y, v.z);
}

inline Vec3 ToGLM(const btVector3& v) {
	return Vec3(v.x(), v.y(), v.z());
}
