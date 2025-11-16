
#include "lua.hpp"

#include "AssemblyPatches.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "Memory.hpp"
#include "VoxelUtils.hpp"
#include "SM/LuaCallData.hpp"
#include "SM/LuaManager.hpp"
#include "SM/VoxelTerrainManager.hpp"
#include "SM/VoxelTerrainWorld.hpp"
#include "SM/VoxelTerrainConstants.hpp"

using namespace DLL;
using namespace AssemblyPatches;
using namespace SM;
using namespace VoxelConstants;

constexpr uintptr Offset_TerrainBodyCheck = 0x0670AC9;
constexpr uintptr Offset_Patch_WorldOnMelee = 0x0253C33;
constexpr uintptr Offset_Patch_WorldOnProjectile = 0x0253477;
constexpr uintptr Offset_Patch_ReadRenderMode = 0x0704a9f;
constexpr uintptr Offset_Patch_ApplyRenderMode = 0x02ab30c;



static void PushClosestVoxelOrNil(lua_State* L, const Vec3& vPosition) {
	uint16 worldID = LuaManager::Get()->getCurrentWorld(L);
	VoxelTerrainWorld* pVoxelWorld = VoxelTerrainManager::Get()->getWorld(worldID);
	SM_ASSERT(pVoxelWorld && "Melee callback without a world?");

	if ( uint16 vox = pVoxelWorld->getClosestVoxel(vPosition); vox != 0xFFFF ) {
		uint32* pVoxelTerrain = (uint32*)lua_newuserdata(L, sizeof(uint32));
		*pVoxelTerrain = ((uint8(vox) >> 6) & 0b00000011);
		luaL_setmetatable(L, "VoxelTerrain");
	} else
		lua_pushnil(L);
}

static void P_WorldOnMelee_PushVoxelTerrain(lua_State* L, LuaCallData::WorldOnMelee* pCallData) {
	PushClosestVoxelOrNil(L, pCallData->vHitPosition);
}

static void P_WorldOnProjectile_PushVoxelTerrain(lua_State* L, LuaCallData::WorldOnProjectile* pCallData) {
	PushClosestVoxelOrNil(L, pCallData->vHitPosition);
}

static void P_World_ReadRenderMode(std::string* psRenderMode, World* self) {
	if ( *psRenderMode == "warehouse" )
		self->setRenderMode(World::RenderMode::Warehouse);
	else if ( *psRenderMode == "underground" )
		self->setRenderMode(World::RenderMode::Underground);
}



bool AssemblyPatches::Apply() {
	{
		/*
			Patches a check in the terrain/creation intersection check code to force the code
			to run even if no normal terrain bodies are present.
			This is needed to stop the game from not-calling our TerrainBodyIntersectionCheck
			hook if no normal (non-voxel) terrain is present in the world.
		*/
		constexpr uint64 PatchSize = 0x6;
		uint8 arrOldBytes[PatchSize] = {0x0F, 0x85, 0xB6, 0x00, 0x00, 0x00};
		uint8 arrNewBytes[PatchSize] = {0xE9, 0xB7, 0x00, 0x00, 0x00, 0x90};

		if ( !Memory::VerifyAndPatchBytes((void*)ResolveOffset(Offset_TerrainBodyCheck), arrOldBytes, arrNewBytes, "TerrainBodyCheck") )
			return false;
	}

	{
		/*
			Patch World:onMelee to call into the DLL instead of pushing a VoxelTerrain userdata.
			Axolot's code just sets the userdata's material ID to 1 regardless of the actual material.
			The DLL's function gets the material and pushes a VoxelTerrain with the proper material ID.
		*/
		constexpr uint64 PatchSize = 0x39;
		uint8 arrOldBytes[PatchSize] = {
			0xBA, 0x04, 0x00, 0x00, 0x00,				// mov edx, 0x4
			0x48, 0x8B, 0x0B,							// mov rcx, [rbx]
			0xFF, 0x15, 0x5F, 0x77, 0xC8, 0x00,			// call qword ptr [lua_newuserdata]
			0xC7, 0x00, 0x01, 0x00, 0x00, 0x00,			// mov [rax], 0x1
			0x4C, 0x8D, 0x05, 0x52, 0x75, 0xD1, 0x00,	// lea r8, ["VoxelTerrain"]
			0xBA, 0xF0, 0xD8, 0xFF, 0xFF,				// mov edx, LUA_REGISTRYINDEX
			0x48, 0x8B, 0x0B,							// mov rcx, [rbx]
			0xFF, 0x15, 0xBC, 0x77, 0xC8, 0x00,			// call qword ptr [lua_getfield]
			0xBA, 0xFE, 0xFF, 0xFF, 0xFF,				// mov edx, -2
			0x48, 0x8B, 0x0B,							// mov rcx, [rbx]
			0xFF, 0x15, 0x3E, 0x77, 0xC8, 0x00,			// call qword ptr [lua_setmetatable]
			0xEB, 0x0D									// jmp ScrapMechanic.exe + 0x0253C79
		};

		uint8 arrNewBytes[PatchSize] = {
			0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// mov r10, nullptr
			0x48, 0x8B, 0x0B,											// mov rcx, [rbx]
			0x48, 0x8B, 0xD7,											// mov rdx, rdi
			0x41, 0xFF, 0xD2,											// call r10
			0xEB, 0x31,													// jmp ScrapMechanic.exe + 0x0253C79
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90
		};

		// Replace the R10 pointer so the code calls the hook function
		*(void**)(&arrNewBytes[2]) = &P_WorldOnMelee_PushVoxelTerrain;

		if ( !Memory::VerifyAndPatchBytes((void*)ResolveOffset(Offset_Patch_WorldOnMelee), arrOldBytes, arrNewBytes, "World:onMelee") )
			return false;
	}

	{
		// Patch World:onProjectile in the same way as World:onMelee as it has the same issue
		constexpr uint64 PatchSize = 0x39;
		uint8 arrOldBytes[PatchSize] = {
			0xBA, 0x04, 0x00, 0x00, 0x00,				// mov edx, 0x4
			0x48, 0x8B, 0x0B,							// mov rcx, [rbx]
			0xFF, 0x15, 0x1B, 0x7F, 0xC8, 0x00,			// call qword ptr [lua_newuserdata]
			0xC7, 0x00, 0x01, 0x00, 0x00, 0x00,			// mov [rax], 0x1
			0x4C, 0x8D, 0x05, 0x0E, 0x7D, 0xD1, 0x00,	// lea r8, ["VoxelTerrain"]
			0xBA, 0xF0, 0xD8, 0xFF, 0xFF,				// mov edx, LUA_REGISTRYINDEX
			0x48, 0x8B, 0x0B,							// mov rcx, [rbx]
			0xFF, 0x15, 0x78, 0x7F, 0xC8, 0x00,			// call qword ptr [lua_getfield]
			0xBA, 0xFE, 0xFF, 0xFF, 0xFF,				// mov edx, -2
			0x48, 0x8B, 0x0B,							// mov rcx, [rbx]
			0xFF, 0x15, 0xFA, 0x7E, 0xC8, 0x00,			// call qword ptr [lua_setmetatable]
			0xEB, 0x0E,									// jmp ScrapMechanic.exe + 0x02534EB
		};

		uint8 arrNewBytes[PatchSize] = {
			0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// mov r10, nullptr
			0x48, 0x8B, 0x0B,											// mov rcx, [rbx]
			0x48, 0x8B, 0xD7,											// mov rdx, rdi
			0x41, 0xFF, 0xD2,											// call r10
			0xEB, 0x37,													// jmp ScrapMechanic.exe + 0x02534BE
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90
		};

		// Replace the R10 pointer so the code calls the hook function
		*(void**)(&arrNewBytes[2]) = &P_WorldOnProjectile_PushVoxelTerrain;

		if ( !Memory::VerifyAndPatchBytes((void*)ResolveOffset(Offset_Patch_WorldOnProjectile), arrOldBytes, arrNewBytes, "World:onProjectile") )
			return false;
	}

	{
		// We hook the code that parses the render mode name so we can enable the hidden 4th render mode
		constexpr uint64 PatchSize = 0x30;
		uint8 arrOldBytes[PatchSize] = {
			0x4C, 0x8B, 0x41, 0x10,						// mov r8, [rcx + 0x10]
			0x48, 0x83, 0x79, 0x18, 0x0F,				// cmp qword ptr [rcx + 0x18], 0xF
			0x76, 0x03,									// jna ScrapMechanic.exe + 0x0704AAD
			0x48, 0x8B, 0x09,							// mov rcx, [rcx]
			0x49, 0x83, 0xF8, 0x09,						// cmp r8, 0x9
			0x75, 0x1C,									// jne ScrapMechanic.exe + 0x0704ACF
			0x48, 0x8D, 0x15, 0x06, 0x5F, 0x8A, 0x00,	// lea rdx, ["warehouse"]
			0xE8, 0x61, 0xDD, 0x63, 0x00,				// call memcmp
			0x85, 0xC0,									// test eax, eax,
			0x0F, 0x94, 0xc0,							// sete al
			0x84, 0xC0,									// test al, al
			0x74, 0x07,									// je ScrapMechanic.exe + 0x0704ACF
			0xC7, 0x46, 0x74, 0x02, 0x00, 0x00, 0x00	// mov [rsi + 0x74], 0x2
		};

		uint8 arrNewBytes[PatchSize] = {
			0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// mov r10, nullptr
			0x48, 0x8B, 0xD6,											// mov rdx, rsi
			0x41, 0xFF, 0xD2,											// call r10
			0xEB, 0x1E,													// jmp ScrapMechanic.exe + 0x0704ACF
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
		};

		// Replace the R10 pointer so the code calls the hook function
		*(void**)(&arrNewBytes[2]) = &P_World_ReadRenderMode;

		if ( !Memory::VerifyAndPatchBytes((void*)ResolveOffset(Offset_Patch_ReadRenderMode), arrOldBytes, arrNewBytes, "ReadRenderMode") )
			return false;
	}

	{
		// We need to patch another check to make the render mode work
		constexpr uint64 PatchSize = 0x7;
		uint8 arrOldBytes[PatchSize] = {
			0x75, 0x15,						// jne ScrapMechanic.exe + 0x02AB323
			0xBA, 0x02, 0x00, 0x00, 0x00	// mov edx, 0x2
		};

		uint8 arrNewBytes[PatchSize] = {
			0x90, 0x90,
			0x8B, 0xD1,			// mov edx, ecx
			0x90, 0x90, 0x90
		};

		if ( !Memory::VerifyAndPatchBytes((void*)ResolveOffset(Offset_Patch_ApplyRenderMode), arrOldBytes, arrNewBytes, "ApplyRenderMode") )
			return false;
	}
	return true;
}
