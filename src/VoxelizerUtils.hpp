#pragma once

#include <vector>

#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

#include "Types.hpp"

namespace DLL {
	namespace VoxelizerUtils {
		struct RaycastHitCountCallback : public btTriangleCallback {
			btVec3 vRayStart;
			btVec3 vRayEnd;
			uint32 numHits = 0;

			inline RaycastHitCountCallback(const Vec3& from, const Vec3& to): vRayStart(ToBT(from)), vRayEnd(ToBT(to)) {};
			void processTriangle(btVec3* triangle, int partId, int triangleIndex) override {
				const btVec3& v0 = triangle[0];
				const btVec3& v1 = triangle[1];
				const btVec3& v2 = triangle[2];

				btVec3 vRayDir = vRayEnd - vRayStart;

				btVec3 vEdge1 = v1 - v0;
				btVec3 vEdge2 = v2 - v0;
				btVec3 h = vRayDir.cross(vEdge2);
				float a = vEdge1.dot(h);

				if ( glm::abs(a) < FLT_EPSILON )
					return;

				float f = 1.0f / a;
				btVec3 s = vRayStart - v0;
				float u = f * s.dot(h);

				if ( u < -FLT_EPSILON || u > 1.0f + FLT_EPSILON )
					return;

				btVec3 q = s.cross(vEdge1);
				float v = f * vRayDir.dot(q);

				if ( v < -FLT_EPSILON || u + v > 1.0f + FLT_EPSILON )
					return;

				float t = f * vEdge2.dot(q);
				if ( t > FLT_EPSILON )
					++numHits;
			}
		};

		struct IndexedTriangle {
			Vec3 v0;
			Vec3 v1;
			Vec3 v2;
			int index;
		};

		struct TriangleFetchCallback : public btTriangleCallback {
			std::vector<IndexedTriangle>& vTriangles;

			TriangleFetchCallback(std::vector<IndexedTriangle>& vTris): vTriangles(vTris) {};
			void processTriangle(btVec3* triangle, int partId, int triangleIndex) override {
				vTriangles.emplace_back(ToGLM(triangle[0]), ToGLM(triangle[1]), ToGLM(triangle[2]), triangleIndex);
			}
		};

		Vec3 FindClosestPointOnTriangle(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c) {
			const Vec3 ab = b - a;
			const Vec3 ac = c - a;
			const Vec3 ap = p - a;

			const float d1 = glm::dot(ab, ap);
			const float d2 = glm::dot(ac, ap);
			if ( d1 <= 0.0f && d2 <= 0.0f )
				return a;

			const Vec3 bp = p - b;
			const float d3 = glm::dot(ab, bp);
			const float d4 = glm::dot(ac, bp);
			if ( d3 >= 0.0f && d4 <= d3 )
				return b;

			const Vec3 cp = p - c;
			const float d5 = glm::dot(ab, cp);
			const float d6 = glm::dot(ac, cp);
			if ( d6 >= 0.0f && d5 <= d6 )
				return c;

			const float vc = d1 * d4 - d3 * d2;
			if ( vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f )
				return a + (d1 / (d1 - d3)) * ab;

			const float vb = d5 * d2 - d1 * d6;
			if ( vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f )
				return a + (d2 / (d2 - d6)) * ac;

			const float va = d3 * d6 - d5 * d4;
			if ( va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f )
				return b + ((d4 - d3) / ((d4 - d3) + (d5 - d6))) * (c - b);

			const float denom = 1.0f / (va + vb + vc);
			const float v = vb * denom;
			const float w = vc * denom;
			return a + v * ab + w * ac;
		}
	}
}
