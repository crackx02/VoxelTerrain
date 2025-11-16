
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include "VoxelTerrainManager.hpp"
#include "ResourceManager.hpp"
#include "Console.hpp"
#include "ModConstants.hpp"

using namespace SM;
using namespace VoxelConstants;

constexpr uintptr Offset_LoadVoxelMaterialSet = 0x07119a0;
constexpr TextureResourceLayout DifTextureLayout = TextureResourceLayout::BC3_R5G6B5A8_UNorm;
constexpr TextureResourceLayout AsgTextureLayout = TextureResourceLayout::BC3_R5G6B5A8_UNorm;
constexpr TextureResourceLayout NorTextureLayout = TextureResourceLayout::BC5_R8G8_UNorm;

using LoadVoxelMaterialSetFunc = bool(*)(TerrainGroundMaterialSet*, std::string*);
LoadVoxelMaterialSetFunc g_LoadVoxelMaterialSet = nullptr;



VoxelTerrainManager** VoxelTerrainManager::_selfPtr = (VoxelTerrainManager**)0x12676f8;

VoxelTerrainChunk* VoxelTerrainManager::AllocateVoxelChunk() {
	// Use malloc to match the game's allocator
	VoxelTerrainChunk* pChunk = (VoxelTerrainChunk*)malloc(sizeof(VoxelTerrainChunk));
	SM_ASSERT(pChunk);
	memset(pChunk, 0x0, sizeof(VoxelTerrainChunk));
	return pChunk;
}

void VoxelTerrainManager::loadStandardMaterialSet() {
	ResolveGlobal(LoadVoxelMaterialSet);
	std::string sPath = std::format("$CONTENT_{}{}", DLL::VoxelHelperModUuid, StandardMaterialSetPath);
	if ( !g_LoadVoxelMaterialSet(&m_standardVoxelMaterialSet, &sPath) ) {
		SM_ERROR("Failed to load standard voxel material set, fallback to error material");
		m_standardVoxelMaterialSet.hash = 0;
		m_standardVoxelMaterialSet.vecMaterialNames = {"Error", "Error", "Error"};
		m_standardVoxelMaterialSet.sFilepath = "";
		std::vector<std::string> vecTextureFiles = {
			"$GAME_DATA/Textures/error.tga",
			"$GAME_DATA/Textures/error.tga",
			"$GAME_DATA/Textures/error.tga"
		};
		ResourceManager* pResManager = ResourceManager::Get();
		m_standardVoxelMaterialSet.pDifTexArray = pResManager->createTextureArrayResource(vecTextureFiles, 256, 256, DifTextureLayout);
		m_standardVoxelMaterialSet.pAsgTexArray = pResManager->createTextureArrayResource(vecTextureFiles, 256, 256, AsgTextureLayout);
		m_standardVoxelMaterialSet.pNorTexArray = pResManager->createTextureArrayResource(vecTextureFiles, 256, 256, NorTextureLayout);
		return;
	}
	SM_LOG("Loaded standard voxel material set: {}", sPath);
}
