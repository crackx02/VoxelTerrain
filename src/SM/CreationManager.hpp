#pragma once

#include <unordered_map>
#include <memory>

#include "Types.hpp"
#include "RigidBody.hpp"

namespace SM {
	struct BodyState {
		std::shared_ptr<RigidBody> pBody;
		std::shared_ptr<PhysicsProxy> pProxy;
		char _pad[0x8];
	};
	ASSERT_SIZE(BodyState, 0x28);

	class CreationManager {
		public:
			static CreationManager** _selfPtr;
			inline static CreationManager* Get() {return *_selfPtr;};

			inline std::shared_ptr<RigidBody> getBody(uint32 id) const {
				auto it = m_mapBodyState.find(id);
				if ( it != m_mapBodyState.end() )
					return it->second.pBody;
				return {};
			};

			inline void updateBody(uint32 id) const {
				auto it = m_mapBodyState.find(id);
				if ( it != m_mapBodyState.end() ) {
					RigidBody* pBody = it->second.pBody.get();
					if ( pBody->getRevision() > 0 )
						pBody->update();
				}
			};

			void queueDetachDisconnectedShapesTask(std::shared_ptr<RigidBody> pBody);

		private:
			virtual ~CreationManager() {};
			std::unordered_map<uint32, BodyState> m_mapBodyState;
			char _pad[0x218];
	};
	ASSERT_SIZE(CreationManager, 0x260);
}
