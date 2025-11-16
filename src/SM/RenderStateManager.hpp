#pragma once

#include <vector>

#include "Sphere.hpp"

namespace SM {
	class RenderStateManager {
		public:
			static RenderStateManager** _selfPtr;
			inline static RenderStateManager* Get() {return *_selfPtr;}
			
			inline void addDirtySphere(const Vec3& position, float radius) {
				m_vecDirtySpheres.emplace_back(position, radius);
			};

		private:
			char _pad[0x96538];
			std::vector<Sphere> m_vecDirtySpheres;
	};
}
