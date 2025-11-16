#pragma once

#include "Types.hpp"

namespace SM {
	struct Bounds {
		Vec3 min;
		Vec3 max;

		inline bool testPoint(const Vec3& p) const {
			return (glm::all(glm::greaterThan(p, min)) && glm::all(glm::lessThan(p, max)));
		}
		inline bool testOverlap(const Bounds& other) const {
			return (glm::all(glm::lessThan(other.min, max)) && glm::all(glm::greaterThan(other.max, min)));
		}
		inline Bounds getOverlap(const Bounds& other) const {
			return Bounds(
				glm::max(min, other.min),
				glm::min(max, other.max)
			);
		}
	};
	struct IntBounds {
		i32Vec3 min;
		i32Vec3 max;

		inline bool testPoint(const i32Vec3& p) const {
			return (glm::all(glm::greaterThanEqual(p, min)) && glm::all(glm::lessThanEqual(p, max)));
		}
		inline bool testOverlap(const IntBounds& other) const {
			return (glm::all(glm::lessThan(other.min, max)) && glm::all(glm::greaterThan(other.max, min)));
		}
		inline IntBounds getOverlap(const IntBounds& other) const {
			return IntBounds(
				glm::max(min, other.min),
				glm::min(max, other.max)
			);
		}
		inline bool operator==(const IntBounds& other) const {
			return min == other.min && max == other.max;
		}
	};
}
