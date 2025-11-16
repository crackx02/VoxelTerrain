#pragma once

#include "NetObj.hpp"
#include "GameScript.hpp"
#include "PhysicsProxy.hpp"

namespace SM {
	class RigidBody : public NetObj {
		public:
			struct Flags {
				enum _Enum : uint8 {
					ForceStatic = 0x80
				} value;

				constexpr inline Flags(_Enum e) {value = e;};
				constexpr inline Flags(int i) {value = _Enum(i);};
				constexpr inline operator _Enum() const {return value;};
			};

			virtual void func27() = 0;
			virtual void func28() = 0;
			virtual void func29() = 0;
			virtual void func30() = 0;
			virtual void func31() = 0;
			virtual void createPhysicsProxy(std::shared_ptr<PhysicsProxy>* pProxy, uint8 flags) {};
			virtual void func33() = 0;
			virtual void func34() = 0;

			inline bool isConvertibleToDynamic() const {
				return GameScript::Get()->getEnableRestrictions() ? ((m_flags & Flags::ForceStatic) == 0) : true;
			};

		private:
			char _pad0[0x50];
			Flags m_flags;
	};
}
