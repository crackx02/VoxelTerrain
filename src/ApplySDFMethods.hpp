#pragma once

#include "SM/VoxelTerrainChunk.hpp"
#include "VoxelUtils.hpp"
#include "Types.hpp"
#include "RestrictionArea.hpp"

namespace DLL {
	struct ApplySDFMode {
		enum _Enum : uint8 {
			ApplyDensity = 0x1,	// Applies density only, material is unchanged
			ApplyMaterial = 0x2,	// Applies material only, density is unchanged
			Erase = 0x4,			// Erases whatever is there
			EraseMaterial = 0x8,	// Erases only voxels which match the given material
			Default = ApplyDensity | ApplyMaterial
		} _v;
		inline ApplySDFMode(uint8 i) {_v = _Enum(i);};
		inline operator _Enum() const {return _v;};
	};

	using ApplySDF_T = void(*)(SM::VoxelTerrainChunk* pChunk, uint32 uVoxelIndex, float distance, float surfaceFalloff, uint8 material);

	static void ApplySDF_ApplyDensity(SM::VoxelTerrainChunk* pChunk, uint32 uVoxelIndex, float distance, float surfaceFalloff, uint8 material) {
		if ( distance <= 0.0f ) {
			const uint8 vox = pChunk->voxels[uVoxelIndex];
			pChunk->voxels[uVoxelIndex] = (vox & 0b11000000) | CalculateDensityWithFalloff(vox & 0b00111111, distance, surfaceFalloff);
		}
	}

	static void ApplySDF_ApplyMaterial(SM::VoxelTerrainChunk* pChunk, uint32 uVoxelIndex, float distance, float surfaceFalloff, uint8 material) {
		if ( distance <= 0.0f ) {
			const uint8 vox = pChunk->voxels[uVoxelIndex];
			pChunk->voxels[uVoxelIndex] = (vox & 0b00111111) | material;
		}
	}

	static void ApplySDF_ApplyDensityAndMaterial(SM::VoxelTerrainChunk* pChunk, uint32 uVoxelIndex, float distance, float surfaceFalloff, uint8 material) {
		if ( distance <= 0.0f ) {
			const uint8 vox = pChunk->voxels[uVoxelIndex];
			pChunk->voxels[uVoxelIndex] = material | CalculateDensityWithFalloff(vox & 0b00111111, distance, surfaceFalloff);
		}
	}

	static void ApplySDF_Erase(SM::VoxelTerrainChunk* pChunk, uint32 uVoxelIndex, float distance, float surfaceFalloff, uint8 material) {
		constexpr uint8 MaxVoxelDensity = SM::VoxelConstants::MaxVoxelDensity;
		if ( distance <= 0.0f ) {
			const uint8 vox = pChunk->voxels[uVoxelIndex];
			pChunk->voxels[uVoxelIndex] = (vox & 0b11000000) | CalculateDensityWithFalloffInv(vox & 0b00111111, distance, surfaceFalloff);
		}
	}

	static void ApplySDF_EraseMaterial(SM::VoxelTerrainChunk* pChunk, uint32 uVoxelIndex, float distance, float surfaceFalloff, uint8 material) {
		constexpr uint8 MaxVoxelDensity = SM::VoxelConstants::MaxVoxelDensity;
		if ( distance <= 0.0f ) {
			distance = glm::abs(distance);
			const uint8 vox = pChunk->voxels[uVoxelIndex];

			if ( (vox & 0b11000000) != material )
				return;

			pChunk->voxels[uVoxelIndex] = (vox & 0b11000000) | CalculateDensityWithFalloffInv(vox & 0b00111111, distance, surfaceFalloff);
		}
	}

	inline static ApplySDF_T GetApplySDFMethod(ApplySDFMode::_Enum mode) {
		switch ( mode ) {
			case ApplySDFMode::ApplyDensity:
				return &ApplySDF_ApplyDensity;
			case ApplySDFMode::ApplyMaterial:
				return &ApplySDF_ApplyMaterial;
			case ApplySDFMode::ApplyDensity | ApplySDFMode::ApplyMaterial:
				return &ApplySDF_ApplyDensityAndMaterial;
			case ApplySDFMode::Erase:
				return &ApplySDF_Erase;
			case ApplySDFMode::EraseMaterial:
				return &ApplySDF_EraseMaterial;
			default:
				return nullptr;
		}
	}

	inline static RestrictionFlags ApplyModeToBuildFlags(ApplySDFMode mode) {
		switch ( mode ) {
			case ApplySDFMode::ApplyDensity:
				return RestrictionFlags::VoxelPlacement;
			case ApplySDFMode::ApplyMaterial:
				return RestrictionFlags::MaterialChange;
			case ApplySDFMode::ApplyDensity | ApplySDFMode::ApplyMaterial:
				return RestrictionFlags::VoxelPlacement | RestrictionFlags::MaterialChange;
			case ApplySDFMode::Erase:
			case ApplySDFMode::EraseMaterial:
				return RestrictionFlags::VoxelRemoval;
			default:
				return RestrictionFlags::None;
		}
	}

	inline static ApplySDFMode OptApplyMode(lua_State* L, int index) {
		return ApplySDFMode::_Enum(uint8(luaL_optinteger(L, index, ApplySDFMode::Default)));
	}
}
