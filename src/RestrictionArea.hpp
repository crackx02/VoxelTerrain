#pragma once

#include "Types.hpp"
#include "SM/Bounds.hpp"
#include "SM/VoxelTerrainConstants.hpp"

namespace DLL {
	struct RestrictionFlags {
		enum _Enum : uint8 {
			None = 0,
			VoxelPlacement = 1 << 0,
			VoxelRemoval = 1 << 1,
			MaterialChange = 1 << 2,
			ShapeConstruction = 1 << 3,
			Override = 1 << 7,
			All = VoxelPlacement | VoxelRemoval | MaterialChange | ShapeConstruction
		} value = None;

		constexpr inline RestrictionFlags() {};
		constexpr inline RestrictionFlags(_Enum e) {value = e;};
		constexpr inline RestrictionFlags(int i) {value = _Enum(i);};
		constexpr inline operator _Enum() const {return value;};
		constexpr inline RestrictionFlags operator|=(const RestrictionFlags& other) {return value = _Enum(value | other.value);};
	};

	class RestrictionArea {
		public:
			RestrictionArea(uint32 id, uint16 world, const SM::Bounds& bounds, RestrictionFlags flags):
				m_id(id), m_world(world), m_bounds(bounds), m_flags(flags) {};

			inline SM::IntBounds getChunkBounds() const {
				return SM::IntBounds(
					i32Vec3(glm::floor(m_bounds.min / float(SM::VoxelConstants::MetersPerChunkAxis))),
					i32Vec3(glm::floor(m_bounds.max / float(SM::VoxelConstants::MetersPerChunkAxis)))
				);
			};

			inline const SM::Bounds& getBounds() const {return m_bounds;};
			inline uint32 getId() const {return m_id;};
			inline uint16 getWorld() const {return m_world;};
			inline RestrictionFlags getFlags() const {return m_flags;};
			inline bool isActive() const {return m_bActive;};

			void setBounds(const SM::Bounds& bounds);
			inline void setFlags(RestrictionFlags flags) {m_flags = flags;};
			inline void setActive(bool active) {m_bActive = active;};

		private:
			SM::Bounds m_bounds;
			uint32 m_id = 0;
			uint16 m_world = 0;
			RestrictionFlags m_flags;
			bool m_bActive = false;
	};
}
