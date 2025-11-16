#pragma once

#include <vector>
#include <string>

#include "TextureResource.hpp"
#include "Types.hpp"

namespace SM {
	class ResourceManager {
		public:
			static ResourceManager** _selfPtr;
			inline static ResourceManager* Get() {return *_selfPtr;};

			TextureResource* createTextureArrayResource(
				const std::vector<std::string>& vecFilePaths,
				int iWidth, int iHeight, TextureResourceLayout eLayout
			);
	};
}
