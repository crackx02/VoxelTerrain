#pragma once

namespace SM {
	struct ThreadContext {
		enum _Enum : int {
			None = 0,
			Synchronized = 1
		} value = None;

		static _Enum Get();
		static void Set(ThreadContext ctx);

		constexpr inline ThreadContext() {};
		constexpr inline ThreadContext(_Enum e) {value = e;};
		constexpr inline ThreadContext(int i) {value = _Enum(i);};
		constexpr inline operator _Enum() const {return value;};
	};
}
