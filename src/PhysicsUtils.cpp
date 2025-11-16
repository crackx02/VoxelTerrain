
#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"

#include "PhysicsUtils.hpp"
#include "SM/PhysicsProxy.hpp"
#include "SM/Console.hpp"

using namespace DLL;
using namespace PhysicsUtils;
using namespace SM;

bool FindStaticBodiesCallback::process(const btBroadphaseProxy* pBtProxy) {
	btCollisionObject* pColObj = (btCollisionObject*)pBtProxy->m_clientObject;
	if ( pColObj == nullptr )
		return true;
	if ( !pColObj->isStaticObject() )
		return true;
	PhysicsProxy* pProxy = (PhysicsProxy*)pColObj->getUserPointer();
	if ( pProxy == nullptr )
		return true;

	if ( pProxy->type() == PhysicsProxy::Type::Body ) {
		int userIndex = pColObj->getUserIndex();
		if ( userIndex == -1 )
			return true;

		auto pBody = pCreationManager->getBody(uint32(userIndex));
		if ( !pBody ) {
			SM_ERROR("Failed to find body {}", userIndex);
			return true;
		}
		if ( !pBody->isConvertibleToDynamic() )
			return true;
		if ( pBody->getRevision() > 0 )
			vecBodies.emplace_back(pBody);
	}
	return true;
}

bool FindDynamicBodiesCallback::process(const btBroadphaseProxy* pBtProxy) {
	btCollisionObject* pColObj = (btCollisionObject*)pBtProxy->m_clientObject;
	if ( pColObj == nullptr )
		return true;
	if ( pColObj->isStaticObject() )
		return true;
	PhysicsProxy* pProxy = (PhysicsProxy*)pColObj->getUserPointer();
	if ( pProxy == nullptr )
		return true;

	if ( pProxy->type() == PhysicsProxy::Type::Body ) {
		int userIndex = pColObj->getUserIndex();
		if ( userIndex == -1 )
			return true;
		auto pBody = pCreationManager->getBody(uint32(userIndex));
		if ( !pBody ) {
			SM_ERROR("Failed to find body {}", userIndex);
			return true;
		}
		if ( pBody->getRevision() > 0 )
			vecBodies.emplace_back(pBody);
	}

	return true;
}
