
#include <thread>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "MinHook.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"

#include "Util.hpp"
#include "Types.hpp"
#include "PhysicsUtils.hpp"
#include "AssemblyPatches.hpp"
#include "Memory.hpp"
#include "ModDataManager.hpp"
#include "MeshVoxelizer.hpp"
#include "wrap_VoxelTerrain.hpp"
#include "wrap_RestrictionArea.hpp"
#include "SM/Console.hpp"
#include "SM/VoxelTerrainManager.hpp"
#include "SM/ResourceManager.hpp"
#include "SM/LuaVM.hpp"
#include "SM/LuaManager.hpp"
#include "SM/wrap_VoxelTerrainGrid.hpp"
#include "SM/RaycastResult.hpp"
#include "SM/MyPlayer.hpp"
#include "SM/PhysicsBase.hpp"
#include "SM/PhysicsProxy.hpp"
#include "SM/RenderStateManager.hpp"
#include "SM/ClientWorld.hpp"
#include "SM/Bounds.hpp"
#include "SM/LuaUserdata.hpp"
#include "SM/CreationManager.hpp"
#include "SM/GameScript.hpp"
#include "SM/ThreadContext.hpp"
#include "SM/DirectoryManager.hpp"
#include "SM/VoxelTerrainNetChunk.hpp"

using namespace DLL;
using namespace PhysicsUtils;
using namespace SM;
using namespace VoxelConstants;

constexpr uintptr Offset_InitializeConsole = 0x02d7a80;
constexpr uintptr Offset_C_VoxelTerrainManager = 0x070f930;
constexpr uintptr Offset_LuaVM_SetupEnv = 0x054a7f0;
constexpr uintptr Offset_ReadClassMemberString = 0x08a8050;
constexpr uintptr Offset_MyPlayerRaycast = 0x042b290;
constexpr uintptr Offset_BodyTerrainIntersectionTest = 0x0752900;
constexpr uintptr Offset_RegisterUserdata = 0x054e1a0;
constexpr uintptr Offset_InitWorld = 0x0705860;
constexpr uintptr Offset_VoxelTerrainManager_RemoveWorld = 0x07106e0;
constexpr uintptr Offset_PlayStateLoad = 0x042c290;
constexpr uintptr Offset_PlayStateCleanup = 0x042dab0;
constexpr uintptr Offset_ContraptionRender = 0x02d9180;
constexpr uintptr Offset_VoxelNodeTree = 0x0a31c10;
constexpr uintptr Offset_Tick1 = 0x126763c;
constexpr uintptr Offset_Tick2 = 0x1267638;
constexpr uintptr Offset_Tick3 = 0x1267640;
constexpr uintptr Offset_VoxelTerrainNetChunk_Update = 0x0fd7e00;



// State //

static struct {
	bool bInitializingFromDLL = false;
	bool bMhInitialized = false;
	bool bManagersInitialized = false;
	uint32 lastTick = 0;
} g_State;

static uint32* g_Tick1 = nullptr;
static uint32* g_Tick2 = nullptr;
static uint32* g_Tick3 = nullptr;

static uint32 GetCurrentTick() {
	return (*g_Tick1 - *g_Tick2) + *g_Tick3 + 1;
}



// Hooks //

static void(*O_VoxelTerrainNetChunk_Update)(VoxelTerrainNetChunk*) = nullptr;
static void H_VoxelTerrainNetChunk_Update(VoxelTerrainNetChunk* self) {
	uint32 prevRevision = self->getRevision();
	O_VoxelTerrainNetChunk_Update(self);
	if ( self->getRevision() > prevRevision )
		gModDataManager->onChunkUpdated(self->getWorld(), self->getChunkIndex());
}

static void*(*O_VoxelNodeTree)(void*, Vec3*, Vec3*, uint32) = nullptr;
static void* H_VoxelNodeTree(void* self, Vec3* pvBoundsMin, Vec3* pvBoundsMax, uint32 materialSet) {
	pvBoundsMin->z = WorldBoundsMinZ;
	pvBoundsMax->z = WorldBoundsMaxZ;
	return O_VoxelNodeTree(self, pvBoundsMin, pvBoundsMax, materialSet);
}

static void(*O_ContraptionRender)(void*, float) = nullptr;
static void H_ContraptionRender(void* self, float dt) {
	O_ContraptionRender(self, dt);
	uint32 currentTick = GetCurrentTick();
	if ( currentTick > g_State.lastTick ) {
		g_State.lastTick = currentTick;
		ThreadContext::Set(ThreadContext::Synchronized);
		gModDataManager->update();
		gVoxelizer->update();
	}
}

static void(*O_PlayStateCleanup)(void*) = nullptr;
static void H_PlayStateCleanup(void* self) {
	if ( g_State.bManagersInitialized ) {
		gVoxelizer->initialize(false);
		gModDataManager->initialize();
		g_State.bManagersInitialized = false;
	}
	O_PlayStateCleanup(self);
}

static void(*O_PlayStateLoad)(void*, void*) = nullptr;
static void H_PlayStateLoad(void* self, void* p2) {
	if ( !g_State.bManagersInitialized ) {
		gModDataManager->initialize();
		gVoxelizer->initialize(true);
		g_State.bManagersInitialized = true;
		g_State.lastTick = 0;
	}
	return O_PlayStateLoad(self, p2);
}

static void(*O_VoxelTerrainManager_RemoveWorld)(VoxelTerrainManager*, uint16) = nullptr;
static void H_VoxelTerrainManager_RemoveWorld(VoxelTerrainManager* self, uint16 world) {
	SM_LOG("Removing world {} from VoxelTerrainManager", world);
	O_VoxelTerrainManager_RemoveWorld(self, world);
	gModDataManager->removeWorld(world);
}

static void(*O_InitWorld)(World*) = nullptr;
static void H_InitWorld(World* self) {
	gModDataManager->addWorld(self->getID(), self->getScript().bEnableVoxelTerrain);
	O_InitWorld(self);
}

static void(*O_RegisterUserdata)(LuaVM*, LuaUserdata*, const luaL_Reg*, const luaL_Reg*) = nullptr;
static void H_RegisterUserdata(LuaVM* self, LuaUserdata* pUd, const luaL_Reg* pArrMetamethods, const luaL_Reg* pArrMemberMethods) {
	SM_LOG("Registering userdata {}", pUd->name);
	if ( pUd->typeID == 0x273A ) {
		const luaL_Reg arrMetamethods[] = {
			pArrMetamethods[0],
			{"__index", wrap_VoxelTerrain::VoxelTerrain_index},
			{nullptr, nullptr}
		};
		const luaL_Reg arrMemberMethods[] = {
			{"getMaterialId", wrap_VoxelTerrain::VoxelTerrain_getMaterialId}
		};
		pUd->pfPrint = &wrap_VoxelTerrain::VoxelTerrain_print;
		SM_LOG("VoxelTerrain userdata overridden");
		O_RegisterUserdata(
			self,
			wrap_RestrictionArea::GetUserdata(),
			wrap_RestrictionArea::GetMetatable(),
			wrap_RestrictionArea::GetMethodsTable()
		);
		SM_LOG("RestrictionArea userdata registered");
		return O_RegisterUserdata(self, pUd, arrMetamethods, arrMemberMethods);
	}
	return O_RegisterUserdata(self, pUd, pArrMetamethods, pArrMemberMethods);
}

static bool(*O_BodyTerrainIntersectionTest)(PhysicsBase*, std::shared_ptr<PhysicsProxy>*) = nullptr;
static bool H_BodyTerrainIntersectionTest(PhysicsBase* self, std::shared_ptr<PhysicsProxy>* ppBodyProxy) {
	const std::shared_ptr<PhysicsProxy>& pBodyProxy = *ppBodyProxy;
	btRigidBody* pThisBody = pBodyProxy->getDynamicsWorldBody();

	btVector3 bodyMin, bodyMax;
	pThisBody->getCollisionShape()->getAabb(pThisBody->getWorldTransform(), bodyMin, bodyMax);

	const IntBounds chunkBounds = {
		i32Vec3(glm::floor(Vec3(bodyMin.x(), bodyMin.y(), bodyMin.z()) / float(MetersPerChunkAxis))),
		i32Vec3(glm::floor(Vec3(bodyMax.x(), bodyMax.y(), bodyMax.z()) / float(MetersPerChunkAxis))),
	};

	const uint16 worldID = self->getWorld()->getID();
	if ( !gModDataManager->isVoxelTerrainEnabled(worldID) )
		return O_BodyTerrainIntersectionTest(self, ppBodyProxy);

	const VoxelTerrainWorld* pVoxelWorld = VoxelTerrainManager::Get()->getWorld(worldID);
	SM_ASSERT(pVoxelWorld);

	btRigidBody* arrChunkBodies[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	constexpr uint8 bodyIdxMax = uint8(std::size(arrChunkBodies) - 1);
	uint8 bodyIdx = 0;

	auto processChunks = [self, &bodyIdx, &arrChunkBodies, &bodyMin, &bodyMax, pThisBody]() {
		for ( uint8 i = 0; i < bodyIdx; ++i ) {
			btRigidBody* pChunkBody = arrChunkBodies[i];

			OverlapTestCallback cb;
			self->getTickWorld()->contactPairTest(pThisBody, pChunkBody, cb);
			if ( cb.hasContact )
				return true;
		}
		return false;
	};

	ITERATE_BOUNDS_BEGIN(chunkBounds, cx, cy, cz);
		uint32 uChunkIdx = pVoxelWorld->getChunkIndex({cx, cy, cz});
		VoxelTerrainPhysicsProxy* pChunkProxy = pVoxelWorld->getChunkProxy(uChunkIdx);
		if ( pChunkProxy ) {
			btRigidBody* pChunkBody = pChunkProxy->getDynamicsWorldBody();
			arrChunkBodies[bodyIdx++] = pChunkBody;
			if ( bodyIdx == bodyIdxMax ) {
				if ( processChunks() )
					return true;
				bodyIdx = 0;
			}
		}
	ITERATE_BOUNDS_END;

	if ( processChunks() )
		return true;

	return O_BodyTerrainIntersectionTest(self, ppBodyProxy);
}

static bool(*O_MyPlayerRaycast)(PhysicsBase**, void*, void*, void*, void*, RaycastResult*) = nullptr;
static bool H_MyPlayerRaycast(PhysicsBase** pPhysicsPhysicsBase, void* p2, void* p3, void* p4, void* p5, RaycastResult* pResult) {
	bool res = O_MyPlayerRaycast(pPhysicsPhysicsBase, p2, p3, p4, p5, pResult);
	if ( res )
		if ( pResult->type == RaycastHitType::VoxelTerrain && pResult == &MyPlayer::Get()->getRaycast() ) {
			const uint16 world = (*pPhysicsPhysicsBase)->getWorld()->getID();
			if ( gModDataManager->checkRestrictionsAt(world, pResult->pointWorld, RestrictionFlags::ShapeConstruction) )
				pResult->type = RaycastHitType::TerrainSurface;
		}
	return res;
}

static void(*O_ReadClassMemberString)(lua_State*, std::string*, char*, void*) = nullptr;
static void H_ReadClassMemberString(lua_State* L, std::string* psValue, char* sKey, void* ptr) {
	O_ReadClassMemberString(L, psValue, sKey, ptr);
	if ( strcmp(sKey, "renderMode") == 0 ) {
		World* pWorld = (World*)(uintptr(psValue) - (3 * sizeof(std::string)) - 0x420);
		LuaWorldScript& script = pWorld->getScript();

		SM_ASSERT(lua_istable(L, -1));
		const int top = lua_gettop(L);

		lua_getfield(L, -1, "enableVoxelTerrain");
		const int t = lua_type(L, -1);
		if ( t != LUA_TBOOLEAN ) {
			if ( t != LUA_TNIL )
				SM_ERROR("WorldClass member 'enableVoxelTerrain' expected boolean or nil, got {}. Defaulting to false.", lua_typename(L, t));
			script.bEnableVoxelTerrain = true;
		} else
			script.bEnableVoxelTerrain = lua_toboolean(L, -1);

		lua_settop(L, top);

		if ( script.bEnableVoxelTerrain ) {
			// Static terrain refuses to generate voxel terrain collisions
			if ( script.bStatic ) {
				SM_ERROR("WorldClass member 'enableVoxelTerrain' conflicts with 'isStatic' (voxel terrain is not supported in static worlds).");
				//script.bEnableVoxelTerrain = false;
				return;
			}
			// Indoors worlds fail to allocate chunk slots for some reason
			if ( script.bIndoors ) {
				SM_ERROR("WorldClass member 'enableVoxelTerrain' conflicts with 'isIndoor' (voxel terrain is not supported in indoor worlds).");
				//script.bEnableVoxelTerrain = false;
				return;
			}

			lua_getfield(L, -1, "voxelMaterialSet");
			int t = lua_type(L, -1);
			if ( t != LUA_TNIL && t != LUA_TSTRING )
				SM_ERROR("WorldClass member 'voxelMaterialSet' expected string or nil, got {}", lua_typename(L, t));
			else {
				size_t sz = 0;
				const char* str = lua_tolstring(L, -1, &sz);
				script.sVoxelMaterialSet = std::string(str, sz);
			}
			lua_settop(L, top);
		}
	}
}

static void(*O_LuaVM_SetupEnv)(LuaVM*, void*, LuaEnvironment) = nullptr;
static void H_LuaVM_SetupEnv(LuaVM* self, void* ptr, LuaEnvironment env) {
	O_LuaVM_SetupEnv(self, ptr, env);
	if ( env == LuaEnvironment::Game )
		wrap_VoxelTerrain::Register(self->getLua());
	else if ( env == LuaEnvironment::Terrain )
		wrap_VoxelTerrainGrid::Register(self->getLua());
}

static VoxelTerrainManager* (*O_C_VoxelTerrainManager)(VoxelTerrainManager*) = nullptr;
static VoxelTerrainManager* H_C_VoxelTerrainManager(VoxelTerrainManager* self) {
	SM_LOG("Initializing VoxelTerrainManager");
	O_C_VoxelTerrainManager(self);
	self->loadStandardMaterialSet();
	gModDataManager->initialize();
	return self;
}

static void(*O_InitializeConsole)(void*, void*) = nullptr;
static void H_InitializeConsole(void* pContraption, void* ptr) {
	if ( pContraption != nullptr )
		O_InitializeConsole(pContraption, ptr);
	
	SM_LOG("Initializing");
	ResolveClassOffset(VoxelTerrainManager);
	ResolveClassOffset(ResourceManager);
	ResolveClassOffset(MyPlayer);
	ResolveClassOffset(LuaManager);
	ResolveClassOffset(RenderStateManager);
	ResolveClassOffset(ClientWorld);
	ResolveClassOffset(CreationManager);
	ResolveClassOffset(GameScript);
	ResolveClassOffset(DirectoryManager);
	ResolveGlobal(Tick1);
	ResolveGlobal(Tick2);
	ResolveGlobal(Tick3);

	gModDataManager = new ModDataManager();
	gVoxelizer = new MeshVoxelizer();

	void* pf_VoxelTerrainNetChunk_Update = &H_VoxelTerrainNetChunk_Update;
	if ( !Memory::Read((void*)ResolveOffset(Offset_VoxelTerrainNetChunk_Update), &O_VoxelTerrainNetChunk_Update, sizeof(void*)) )
		SM_ERROR("Failed to read VoxelTerrainNetChunk::update pointer");
	if ( !Memory::Write((void*)ResolveOffset(Offset_VoxelTerrainNetChunk_Update), &pf_VoxelTerrainNetChunk_Update, sizeof(void*)) )
		SM_ERROR("Failed to write VoxelTerrainNetChunk::update pointer");

	if ( !AssemblyPatches::Apply() )
		SM_ERROR("Failed to apply assembly patches, things may not work properly");

	if ( MakeHook(VoxelNodeTree) != MH_OK ) {
		SM_ERROR("Failed to hook VoxelNodeTree!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(ContraptionRender) != MH_OK ) {
		SM_ERROR("Failed to hook Contraption::Render!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(PlayStateCleanup) != MH_OK ) {
		SM_ERROR("Failed to hook PlayState::Cleanup!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(PlayStateLoad) != MH_OK ) {
		SM_ERROR("Failed to hook PlayState::Load!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(VoxelTerrainManager_RemoveWorld) != MH_OK ) {
		SM_ERROR("Failed to hook VoxelTerrainManager::RemoveWorld!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(InitWorld) != MH_OK ) {
		SM_ERROR("Failed to hook InitWorld!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(RegisterUserdata) != MH_OK ) {
		SM_ERROR("Failed to hook RegisterUserdata!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(BodyTerrainIntersectionTest) != MH_OK ) {
		SM_ERROR("Failed to hook BodyTerrainIntersectionTest!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(C_VoxelTerrainManager) != MH_OK ) {
		SM_ERROR("Failed to hook VoxelTerrainManager!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(LuaVM_SetupEnv) != MH_OK ) {
		SM_ERROR("Failed to hook LuaVM!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(ReadClassMemberString) != MH_OK ) {
		SM_ERROR("Failed to hook ReadClassMemberString!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MakeHook(MyPlayerRaycast) != MH_OK ) {
		SM_ERROR("Failed to hook MyPlayerRaycast!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	if ( MH_EnableHook(MH_ALL_HOOKS) != MH_OK ) {
		SM_ERROR("Failed to enable hooks!");
		if ( g_State.bInitializingFromDLL )
			*(bool*)ptr = false;
		return;
	}

	SM_LOG("Initialized");
}



// DLL Process //

static bool Attach() {
	if ( MH_Initialize() != MH_OK ) {
		MessageBoxA(nullptr, "Failed to initialize MinHook", "VoxelTerrain ERROR", MB_OK);
		return false;
	}
	g_State.bMhInitialized = true;
	
	ResolveClassOffset(Console);

	// Initialize only once the console exists, that way we can properly log stuff
	if ( Console::Get() == nullptr ) {
		if ( MakeHook(InitializeConsole) != MH_OK || EnableHook(InitializeConsole) != MH_OK ) {
			MessageBoxA(nullptr, "Failed to hook InitializeConsole", "VoxelTerrain ERROR", MB_OK);
			return false;
		}
	} else {
		SM_WARN("VoxelTerrain mod was injected after game startup! This is experimental and may not work properly.");
		g_State.bInitializingFromDLL = true;
		bool res = true;
		H_InitializeConsole(nullptr, &res);
		return res;
	}

	return true;
}

static void Detach(bool processShutdown) {
	if ( g_State.bMhInitialized ) {
		g_State.bMhInitialized = false;
		if ( processShutdown )
			gVoxelizer->onProcessShutdown();
		delete gVoxelizer;
		delete gModDataManager;
		MH_Uninitialize();
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: {
			return Attach();
			break;
		}
		case DLL_PROCESS_DETACH:
			Detach(lpReserved != nullptr);
	}
	return TRUE;
}
