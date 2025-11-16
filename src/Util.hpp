#pragma once

#define ASSERT_SIZE(type, sz) static_assert(sizeof(type) == sz, #type " has wrong size! Expected " #sz " bytes" )

#define ResolveClassOffset(obj) obj::_selfPtr = decltype(obj::_selfPtr)(uintptr_t(GetModuleHandle(0)) + uintptr_t(obj::_selfPtr))
#define ResolveOffset(off) (uintptr_t(GetModuleHandle(0)) + off)
#define ResolveGlobal(name) if (!g_##name) g_##name = (decltype(g_##name))(ResolveOffset(Offset_##name))
#define MakeHook(name) MH_CreateHook((LPVOID)ResolveOffset(Offset_##name), (LPVOID)&H_##name, (LPVOID*)&O_##name)
#define EnableHook(name) MH_EnableHook((LPVOID)ResolveOffset(Offset_##name))
#define ITERATE_BOUNDS_BEGIN(bounds, nx, ny, nz) \
for ( int nx = bounds.min.x; nx <= bounds.max.x; ++nx ) { \
	for ( int ny = bounds.min.y; ny <= bounds.max.y; ++ny ) { \
		for ( int nz = bounds.min.z; nz <= bounds.max.z; ++nz ) {
#define ITERATE_BOUNDS_END \
		} \
	} \
}
