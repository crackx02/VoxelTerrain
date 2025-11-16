
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include "CreationManager.hpp"

using namespace SM;

constexpr uintptr Offset_QueueDetachShapesTask = 0x066ed40;

using QueueDetachShapesTaskFunc = std::shared_ptr<RigidBody>*(*)(CreationManager*, std::shared_ptr<RigidBody>*, uint8);
QueueDetachShapesTaskFunc g_QueueDetachShapesTask = nullptr;



CreationManager** CreationManager::_selfPtr = (CreationManager**)0x1267740;

void CreationManager::queueDetachDisconnectedShapesTask(std::shared_ptr<RigidBody> pBody) {
	ResolveGlobal(QueueDetachShapesTask);
	g_QueueDetachShapesTask(this, &pBody, 1);
};
