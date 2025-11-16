#pragma once

#include <string>
#include <vector>

#include "Types.hpp"

namespace DLL {
	class Texture {
		public:
			inline Texture() {};
			inline Texture(Texture&& other) noexcept {*this = std::move(other);};
			Texture(const std::string& path);

			const Vec3& getCoord(Vec2 coord) const {
				coord *= m_vConversionRatio;
				i32Vec2 iCoord = i32Vec2(coord * Vec2(m_vSize));
				iCoord = glm::clamp(iCoord, i32Vec2(0), m_vBounds);
				return m_vecPixels.at(iCoord.x + (iCoord.y * m_vSize.x));
			};

			inline Texture& operator=(Texture&& other) noexcept {
				m_vSize = other.m_vSize;
				m_vBounds = other.m_vBounds;
				m_vConversionRatio = other.m_vConversionRatio;
				m_vecPixels = std::move(other.m_vecPixels);
				m_bValid = other.m_bValid;
				other.m_bValid = false;
				return *this;
			};

			inline bool valid() const {return m_bValid;};

		private:
			std::vector<Vec3> m_vecPixels;
			i32Vec2 m_vSize = {0, 0};
			i32Vec2 m_vBounds = {0, 0};
			Vec2 m_vConversionRatio = {1.0f, 1.0f};
			bool m_bValid = false;
	};
}
