#pragma once

#define NOMINMAX
#include "Windows.h"

namespace SM {
	class SRWLock {
		public:
			void lock() {
				AcquireSRWLockExclusive(&m_lock);
			}
			void unlock() {
				ReleaseSRWLockExclusive(&m_lock);
			}

		private:
			SRWLOCK m_lock;
	};
}
