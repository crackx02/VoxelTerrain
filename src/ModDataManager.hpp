#pragma once

#include <vector>

#include "SM/XXHash.hpp"
#include "RestrictionArea.hpp"
#include "PhysicsUtils.hpp"

namespace DLL {
	class ModDataManager {
		friend class RestrictionArea;
		public:
			~ModDataManager();

			inline void setCreationManager(SM::CreationManager* pCreationManager) {
				m_findStaticBodiesCallback.pCreationManager = pCreationManager;
				m_findDynamicBodiesCallback.pCreationManager = pCreationManager;
			};

			void update();
			void initialize();

			void addWorld(uint16 world, bool bVoxelTerrainEnabled);
			inline bool hasWorld(uint16 world) const {
				auto it = m_mapWorldVoxelTerrainState.find(world);
				return it != m_mapWorldVoxelTerrainState.end();
			}
			void removeWorld(uint16 world);

			void setGlobalRestrictions(uint16 world, RestrictionFlags flags);
			RestrictionFlags getGlobalRestrictions(uint16 world);
			RestrictionArea* createRestrictionArea(uint16 world, const SM::Bounds& bounds, RestrictionFlags flags);
			void destroyRestrictionArea(uint32 id);

			inline RestrictionArea* getRestrictionArea(uint32 id) {
				auto it = m_mapRestrictionAreas.find(id);
				if ( it != m_mapRestrictionAreas.end() )
					return &it->second;
				return nullptr;
			};
			
			RestrictionFlags getRestrictionsAt(uint16 world, const Vec3& vPosition) const;
			inline bool checkRestrictionsAt(uint16 world, const Vec3& vPosition, RestrictionFlags flags) const {
				return (getRestrictionsAt(world, vPosition) & flags) == RestrictionFlags::None;
			}

			const std::vector<RestrictionArea*>* getRestrictionAreasInChunk(uint16 world, const i32Vec3& vChunkIndex) const;

			bool isVoxelTerrainEnabled(uint16 world);

			void onChunkUpdated(uint16 world, const i32Vec3& vChunkIndex);

			//void queueDetachDisconnectedBodiesInChunks(uint16 world, const SM::IntBounds& chunkBounds);

		private:
			struct WorldRestrictions {
				RestrictionFlags globalRestrictions = RestrictionFlags::None;
				SM::XXHashMap<i32Vec3, std::vector<RestrictionArea*>> mapChunkRestrictionAreas;
			};
			uint32 m_restrictionAreaCount = 0;
			SM::XXHashMap<uint32, RestrictionArea> m_mapRestrictionAreas;
			SM::XXHashMap<uint16, WorldRestrictions> m_mapWorldRestrictions;
			SM::XXHashMap<uint16, SM::XXHashSet<i32Vec3>> m_mapSetUpdatedWorldChunks;
			//SM::XXHashMap<uint16, SM::XXHashSet<SM::IntBounds>> m_mapSetWorldBodyDetachChunkBounds;
			SM::XXHashMap<uint16, bool> m_mapWorldVoxelTerrainState;
			PhysicsUtils::FindStaticBodiesCallback m_findStaticBodiesCallback;
			PhysicsUtils::FindDynamicBodiesCallback m_findDynamicBodiesCallback;

			// Called from RestrictionArea::setBounds();
			void updateRestrictionAreaChunkBounds(RestrictionArea* pArea, const SM::Bounds& newBounds);
	};
	extern ModDataManager* gModDataManager;
}
