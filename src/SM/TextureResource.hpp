#pragma once

#include "Types.hpp"

namespace SM {
	enum class TextureResourceLayout : uint32 {
		BC1_R5G6B5A1_UNorm,
		BC2_R5G6B5A4_UNorm,
		BC3_R5G6B5A8_UNorm,
		BC4_R8_UNorm,
		BC5_R8G8_UNorm,
		R11G11B10_Float,
		R8G8B8A8_UNorm,
		R16G16_UNorm,
		R16,
		R32,
		R32G8X24,
		R24G8,
		R8_UNorm
	};
	class TextureResource;
}
