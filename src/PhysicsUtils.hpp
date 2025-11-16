#pragma once

#include <vector>
#include <memory>

#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"

#include "SM/CreationManager.hpp"
#include "SM/RigidBody.hpp"
#include "SM/CreationManager.hpp"

namespace DLL {
	namespace PhysicsUtils {
		struct FindStaticBodiesCallback : public btBroadphaseAabbCallback {
			bool process(const btBroadphaseProxy* pBtProxy) override;

			inline void clear() {vecBodies.clear();};

			std::vector<std::shared_ptr<SM::RigidBody>> vecBodies;
			SM::CreationManager* pCreationManager = nullptr;
		};

		struct FindDynamicBodiesCallback : public btBroadphaseAabbCallback {
			bool process(const btBroadphaseProxy* pBtProxy) override;
			inline void clear() {vecBodies.clear();};
			std::vector<std::shared_ptr<SM::RigidBody>> vecBodies;
			SM::CreationManager* pCreationManager = nullptr;
		};

		struct OverlapTestCallback : public btCollisionWorld::ContactResultCallback {
			virtual btScalar addSingleResult(
				btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0,
				int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1
			) override {
				hasContact = true;
				return 0.0f;
			};
			bool hasContact = false;
		};
	}
}
