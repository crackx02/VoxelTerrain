#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"

#include "Types.hpp"
#include "Util.hpp"
#include "SM/Console.hpp"

namespace DLL {
	namespace Memory {
		static bool Read(void* pSource, void* pTarget, uint64 size) {
			DWORD oldProtect;
			if ( !VirtualProtect(pSource, size, PAGE_READWRITE, &oldProtect) ) {
				SM_ERROR("Failed to change memory protection for READ at address {}, size {} ({})", pSource, size, GetLastError());
				return false;
			}
			std::memcpy(pTarget, pSource, size);
			if ( !VirtualProtect(pSource, size, oldProtect, &oldProtect) )
				SM_WARN("Failed to restore memory protection of READ at address {}, size {} ({})", pSource, size, GetLastError());
			return true;
		}
		static bool Write(void* pTarget, const void* pSource, uint64 size) {
			DWORD oldProtect;
			if ( !VirtualProtect(pTarget, size, PAGE_READWRITE, &oldProtect) ) {
				SM_ERROR("Failed to change memory protection for WRITE at address {}, size {} ({})", pSource, size, GetLastError());
				return false;
			}
			std::memcpy(pTarget, pSource, size);
			if ( !VirtualProtect(pTarget, size, oldProtect, &oldProtect) )
				SM_WARN("Failed to restore memory protection of WRITE at address {}, size {} ({})", pSource, size, GetLastError());
			if ( !FlushInstructionCache(GetModuleHandle(0), pTarget, size) )
				SM_WARN("Failed to flush instruction cache for address {}, size {}", pSource, size);
			return true;
		}
		template<uint64 N>
		static bool VerifyAndPatchBytes(void* pTarget, uint8(&arrOldBytes)[N], uint8(&arrNewBytes)[N], const char* name = nullptr) {
			uint8 arrTempBytes[N] = {};
			std::memset(arrTempBytes, 0x0, N);
			if ( !Memory::Read(pTarget, arrTempBytes, N) ) {
				SM_ERROR("Failed to read patch memory for '{}'", name ? name : "");
				return false;
			}

			if ( std::memcmp(arrOldBytes, arrTempBytes, N) != 0 ) {
				SM_ERROR("Patch memory data mismatch for '{}'", name ? name : "");
				return false;
			}

			if ( !Memory::Write(pTarget, arrNewBytes, N) ) {
				SM_ERROR("Failed to write patch memory for '{}'", name ? name : "");
				return false;
			}
			return true;
		}
	}
}
