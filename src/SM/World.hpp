#pragma once

#include <vector>

#include "Sphere.hpp"
#include "ClientWorld.hpp"
#include "LuaWorldScript.hpp"
#include "Types.hpp"

namespace SM {
	constexpr uint16 NoWorldID = 65534;
	class World {
		public:
			static constexpr float MetersPerCell = 64.0f;
			enum class RenderMode : uint32 {
				Outdoor,
				Challenge,
				Warehouse,
				Underground
			};
			struct CellBounds {
				int minX;
				int maxX;
				int minY;
				int maxY;
			};

			inline uint16 getID() const {return m_id;};

			inline void addDirtySphere(const Vec3& position, float radius) {
				m_vecDirtySpheres.emplace_back(position, radius);
			};

			inline bool isRenderWorld() const {
				return m_id == ClientWorld::Get()->getWorldID();
			};

			inline void setRenderMode(RenderMode mode) {m_renderMode = mode;}

			inline LuaWorldScript& getScript() {return m_script;};

			inline const CellBounds& getCellBounds() const {return m_cellBounds;};

		private:
			virtual void func() {};
			uint16 m_id;
			char _pad0[0x68];
			RenderMode m_renderMode;
			CellBounds m_cellBounds;
			char _pad1[0x398];
			LuaWorldScript m_script;
			char _pad2[0x18];
			std::vector<Sphere> m_vecDirtySpheres;
			char _pad3[0x18];
	};
	ASSERT_SIZE(World, 0x518);
}
