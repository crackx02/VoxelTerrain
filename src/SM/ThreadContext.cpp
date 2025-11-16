
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include "ThreadContext.hpp"

using namespace SM;

static ThreadContext* GetContextPtr() {
	struct ThreadContextStorage {
		char _pad[0x130];
		ThreadContext context;
	};
	struct ThreadLocalStorage {
		ThreadContextStorage* pContextStorage;
	};
	struct TEB {
		char _pad[0x58];
		ThreadLocalStorage* tlsPtr;
	};
	TEB* pTEB = (TEB*)NtCurrentTeb();
	if ( pTEB != nullptr && pTEB->tlsPtr != nullptr )
		return &pTEB->tlsPtr->pContextStorage->context;
	return nullptr;
}

ThreadContext::_Enum ThreadContext::Get() {
	ThreadContext* p = GetContextPtr();
	return p ? p->value : None;
}

void ThreadContext::Set(ThreadContext ctx) {
	ThreadContext* p = GetContextPtr();
	p->value = ctx.value;
}
