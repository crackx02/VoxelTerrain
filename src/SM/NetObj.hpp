#pragma once

#include <memory>

#include "Aligned16Base.hpp"

#include "Types.hpp"
#include "Util.hpp"

namespace SM {
	class NetObj : public Aligned16Base, std::enable_shared_from_this<NetObj> {
		public:
			virtual ~NetObj() {};
			virtual void func1() {};
			virtual void func2() {};
			virtual void update() {};
			virtual void func4() {};
			virtual void func5() {};
			virtual void func6() {};
			virtual void func7() {};
			virtual bool func8() {};
			virtual void func9() = 0;
			virtual void func10() = 0;
			virtual void func11() = 0;
			virtual void func12() = 0;
			virtual void func13() = 0;
			virtual void func14() = 0;
			virtual void func15() = 0;
			virtual uint8 func16() {};
			virtual uint8 func17() {};
			virtual void func18() = 0;
			virtual void func19() = 0;
			virtual void func20() = 0;
			virtual uint8 func21() {};
			virtual uint8 func22() {};
			virtual void func23() = 0;
			virtual void func24() = 0;
			virtual void func25() = 0;
			virtual void func26() = 0;

			inline int32 getId() const {return m_id;};
			inline int32 getRevision() const {return m_revision;};

		private:
			//std::shared_ptr<NetObj> m_selfPtr;
			int32 m_id;
			int32 m_revision;
	};
	ASSERT_SIZE(NetObj, 0x30);
}
