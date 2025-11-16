
#include "wrap_RestrictionArea.hpp"
#include "LuaUtils.hpp"
#include "ModDataManager.hpp"
#include "VoxelUtils.hpp"
#include "SM/VoxelTerrainWorld.hpp"
#include "SM/Console.hpp"

using namespace DLL;
using namespace wrap_RestrictionArea;
using namespace SM;
using namespace VoxelConstants;

static const char* UdataName = "RestrictionArea";

static LuaUserdata RestrictionAreaUserdata = {
	"RestrictionArea",
	80000, 0,
	&RestrictionArea_print,
	&RestrictionArea_exists,
	nullptr, nullptr
};

static luaL_Reg Metamethods[] = {
	{"__index", RestrictionArea_index},
	{"__newindex", RestrictionArea_newindex},
	{"__tostring", RestrictionArea_tostring},
	{"__gc", RestrictionArea_gc},
	{nullptr, nullptr}
};

static luaL_Reg MemberMethods[] = {
	{"getBounds", RestrictionArea_getBounds},
	{"getWorld", RestrictionArea_getWorld},
	{"getFlags", RestrictionArea_getFlags},
	{"getActive", RestrictionArea_getActive},
	{"setBounds", RestrictionArea_setBounds},
	{"setFlags", RestrictionArea_setFlags},
	{"setActive", RestrictionArea_setActive},
	{"destroy", RestrictionArea_destroy},
	{nullptr, nullptr}
};

static RestrictionArea* CheckRestrictionArea(lua_State* L, int index, bool optional) {
	if ( optional && lua_type(L, index) <= LUA_TNIL )
		return nullptr;
	const uint32 id = *(uint32*)luaL_checkudata(L, index, UdataName);
	RestrictionArea* pArea = gModDataManager->getRestrictionArea(id);
	if ( pArea == nullptr )
		luaL_error(L, "%s does not exist", UdataName);
	return pArea;
}

static void PushRestrictionArea(lua_State* L, const RestrictionArea* pArea) {
	uint32* pId = (uint32*)lua_newuserdata(L, sizeof(uint32));
	luaL_setmetatable(L, UdataName);
	*pId = pArea->getId();
}



void wrap_RestrictionArea::Register(lua_State* L) {
	SM_ASSERT(lua_istable(L, -1));

	lua_pushstring(L, "setGlobalRestrictions");
	lua_pushcfunction(L, setGlobalRestrictions);
	lua_rawset(L, -3);

	lua_pushstring(L, "createRestrictionArea");
	lua_pushcfunction(L, createRestrictionArea);
	lua_rawset(L, -3);

	lua_pushstring(L, "getRestrictionsAt");
	lua_pushcfunction(L, getRestrictionsAt);
	lua_rawset(L, -3);

	lua_pushstring(L, "getRestrictionAreasAt");
	lua_pushcfunction(L, getRestrictionAreasAt);
	lua_rawset(L, -3);

	lua_pushstring(L, "getRestrictionAreasInBounds");
	lua_pushcfunction(L, getRestrictionAreasInBounds);
	lua_rawset(L, -3);


	lua_pushstring(L, "restrictionFlags");
	lua_newtable(L);

	lua_pushstring(L, "terrainPlacement");
	lua_pushinteger(L, RestrictionFlags::VoxelPlacement);
	lua_rawset(L, -3);

	lua_pushstring(L, "terrainRemoval");
	lua_pushinteger(L, RestrictionFlags::VoxelRemoval);
	lua_rawset(L, -3);

	lua_pushstring(L, "voxelMaterialChange");
	lua_pushinteger(L, RestrictionFlags::MaterialChange);
	lua_rawset(L, -3);

	lua_pushstring(L, "shapeConstruction");
	lua_pushinteger(L, RestrictionFlags::ShapeConstruction);
	lua_rawset(L, -3);

	lua_pushstring(L, "override");
	lua_pushinteger(L, RestrictionFlags::Override);
	lua_rawset(L, -3);

	lua_pushstring(L, "all");
	lua_pushinteger(L, RestrictionFlags::All);
	lua_rawset(L, -3);

	lua_rawset(L, -3);
}

SM::LuaUserdata* wrap_RestrictionArea::GetUserdata() {
	return &RestrictionAreaUserdata;
}

luaL_Reg* wrap_RestrictionArea::GetMetatable() {
	return Metamethods;
}

luaL_Reg* wrap_RestrictionArea::GetMethodsTable() {
	return MemberMethods;
}



int wrap_RestrictionArea::setGlobalRestrictions(lua_State* L) {
	CheckArgsMinMax(L, 1, 2);
	RestrictionFlags flags = RestrictionFlags(luaL_checkinteger(L, 1));
	gModDataManager->setGlobalRestrictions(GetWorldOrScriptWorld(L, 2), flags);
	return 0;
}

int wrap_RestrictionArea::getGlobalRestrictions(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	lua_pushinteger(L, int(gModDataManager->getGlobalRestrictions(GetWorldOrScriptWorld(L, 1))));
	return 1;
}

int wrap_RestrictionArea::createRestrictionArea(lua_State* L) {
	CheckArgsMinMax(L, 3, 4);
	const Vec3 vMin = CheckVec3(L, 1);
	const Vec3 vMax = CheckVec3(L, 2);
	const RestrictionFlags flags = RestrictionFlags(luaL_checkinteger(L, 3));

	RestrictionArea* pArea = gModDataManager->createRestrictionArea(GetWorldOrScriptWorld(L, 4), Bounds(vMin, vMax), flags);
	PushRestrictionArea(L, pArea);
	return 1;
}

int wrap_RestrictionArea::getRestrictionsAt(lua_State* L) {
	CheckArgsMinMax(L, 1, 2);
	const Vec3 vPosition = CheckVec3(L, 1);

	RestrictionFlags flags = gModDataManager->getRestrictionsAt(GetWorldOrScriptWorld(L, 2), vPosition);
	lua_pushinteger(L, int(flags));
	return 1;
}

int wrap_RestrictionArea::getRestrictionAreasAt(lua_State* L) {
	CheckArgsMinMax(L, 1, 2);
	const Vec3 vPosition = CheckVec3(L, 1);

	const i32Vec3 vChunkIndex = i32Vec3(glm::floor(vPosition / float(MetersPerChunkAxis)));

	const auto* pvAreas = gModDataManager->getRestrictionAreasInChunk(GetWorldOrScriptWorld(L, 2), vChunkIndex);
	lua_createtable(L, int(pvAreas->size()), 0);
	if ( pvAreas != nullptr ) {
		for ( uint64 i = 0; i < pvAreas->size(); ++i ) {
			const RestrictionArea* pArea = pvAreas->at(i);
			if ( pArea->getBounds().testPoint(vPosition) ) {
				PushRestrictionArea(L, pArea);
				lua_rawseti(L, -2, int(i + 1));
			}
		}
	}
	return 1;
}

int wrap_RestrictionArea::getRestrictionAreasInBounds(lua_State* L) {
	CheckArgsMinMax(L, 2, 3);
	const Vec3 vMin = CheckVec3(L, 1);
	const Vec3 vMax = CheckVec3(L, 2);

	const Bounds bounds(vMin, vMax);
	const IntBounds chunkBounds(
		i32Vec3(glm::floor(vMin / float(MetersPerChunkAxis))),
		i32Vec3(glm::floor(vMax / float(MetersPerChunkAxis)))
	);

	lua_newtable(L);
	ITERATE_BOUNDS_BEGIN(chunkBounds, cx, cy, cz);
		const auto* pvAreas = gModDataManager->getRestrictionAreasInChunk(GetWorldOrScriptWorld(L, 3), {cx, cy, cz});
		if ( pvAreas != nullptr ) {
			// This isn't great but Lua hashes userdata by its pointer so we cannot use the vec3 directly as then you can't index it
			PushVec3(L, Vec3(cx, cy, cz));
			SM_ASSERT(luaL_callmeta(L, -1, "__tostring") && "Vec3 __tostring call failed");
			lua_remove(L, -2);

			lua_createtable(L, int(pvAreas->size()), 0);
			for ( uint64 i = 0; i < pvAreas->size(); ++i ) {
				const RestrictionArea* pArea = pvAreas->at(i);
				if ( pArea->getBounds().testOverlap(bounds) ) {
					PushRestrictionArea(L, pArea);
					lua_rawseti(L, -2, int(i + 1));
				}
			}
			lua_rawset(L, -3);
		}
	ITERATE_BOUNDS_END;
	return 1;
}

int wrap_RestrictionArea::checkRestrictionsAt(lua_State* L) {
	CheckArgsMinMax(L, 2, 3);
	const Vec3 vPosition = CheckVec3(L, 1);
	const RestrictionFlags flags = RestrictionFlags(luaL_checkinteger(L, 2));
	lua_pushboolean(L, gModDataManager->checkRestrictionsAt(GetWorldOrScriptWorld(L, 3), vPosition, flags));
	return 1;
}



int wrap_RestrictionArea::RestrictionArea_getBounds(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	const RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);
	const Bounds& bounds = pArea->getBounds();
	PushVec3(L, bounds.min);
	PushVec3(L, bounds.max);
	return 2;
}

int wrap_RestrictionArea::RestrictionArea_getWorld(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	const RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);
	PushWorld(L, pArea->getWorld());
	return 1;
}

int wrap_RestrictionArea::RestrictionArea_getFlags(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	const RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);
	lua_pushinteger(L, int(pArea->getFlags()));
	return 1;
}

int wrap_RestrictionArea::RestrictionArea_getActive(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	const RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);
	lua_pushboolean(L, pArea->isActive());
	return 1;
}

int wrap_RestrictionArea::RestrictionArea_setBounds(lua_State* L) {
	CheckArgsMinMax(L, 3, 3);
	RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);
	const Vec3 vMin = CheckVec3(L, 2);
	const Vec3 vMax = CheckVec3(L, 3);
	pArea->setBounds(Bounds(vMin, vMax));
	return 0;
}

int wrap_RestrictionArea::RestrictionArea_setFlags(lua_State* L) {
	CheckArgsMinMax(L, 2, 2);
	RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);
	const RestrictionFlags flags = RestrictionFlags(luaL_checkinteger(L, 2));
	pArea->setFlags(flags);
	return 0;
}

int wrap_RestrictionArea::RestrictionArea_setActive(lua_State* L) {
	CheckArgsMinMax(L, 2, 2);
	RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);
	const bool active = CheckBoolean(L, 2);
	pArea->setActive(active);
	return 0;
}

int wrap_RestrictionArea::RestrictionArea_destroy(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	const RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);
	gModDataManager->destroyRestrictionArea(pArea->getId());
	return 0;
}



void wrap_RestrictionArea::RestrictionArea_print(void* self, std::ostream* out) {
	const uint32 id = *(uint32*)self;
	*out << "{<" << UdataName << ", id = " << id << ">}";
}

bool wrap_RestrictionArea::RestrictionArea_exists(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	return gModDataManager->getRestrictionArea(*(uint32*)luaL_checkudata(L, 1, UdataName)) != nullptr;
}

int wrap_RestrictionArea::RestrictionArea_index(lua_State* L) {
	CheckArgsMinMax(L, 2, 2);
	const RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);

	uint64 len = 0;
	const char* key = luaL_checklstring(L, 2, &len);
	const std::string_view svKey(key, len);

	if ( svKey == "world" ) {
		PushWorld(L, pArea->getWorld());
		return 1;
	} else if ( svKey == "flags" ) {
		lua_pushinteger(L, int(pArea->getFlags()));
		return 1;
	} else if ( svKey == "active" ) {
		lua_pushboolean(L, pArea->isActive());
		return 1;
	}

	lua_getmetatable(L, 1);
	lua_getfield(L, -1, key);
	const int t = lua_type(L, -1);
	if ( t <= LUA_TNIL )
		return luaL_error(L, "Unknown member \'%s\' in userdata", key);

	return 1;
}

int wrap_RestrictionArea::RestrictionArea_newindex(lua_State* L) {
	CheckArgsMinMax(L, 3, 3);
	RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);

	uint64 len = 0;
	const char* key = luaL_checklstring(L, 2, &len);
	const std::string_view svKey(key, len);

	if ( svKey == "world" )
		return luaL_error(L, "Cannot modify member \'%s\' in userdata", key);
	else if ( svKey == "flags" ) {
		pArea->setFlags(RestrictionFlags(luaL_checkinteger(L, 3)));
		return 0;
	} else if ( svKey == "active" ) {
		pArea->setActive(CheckBoolean(L, 3));
		return 0;
	}

	lua_getmetatable(L, 1);
	lua_getfield(L, -1, key);
	const int t = lua_type(L, -1);
	if ( t <= LUA_TNIL )
		return luaL_error(L, "Unknown member \'%s\' in userdata", key);

	return 0;
}

int wrap_RestrictionArea::RestrictionArea_tostring(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	const RestrictionArea* pArea = CheckRestrictionArea(L, 1, false);
	char buf[64];
	auto res = std::format_to_n(buf, sizeof(buf), "{{<{}>, id = {}}}", UdataName, pArea->getId());
	lua_pushlstring(L, buf, res.size);
	return 1;
}

int wrap_RestrictionArea::RestrictionArea_gc(lua_State* L) {
	CheckArgsMinMax(L, 1, 1);
	const uint32 id = *(uint32*)luaL_checkudata(L, 1, UdataName);
	if ( gModDataManager->getRestrictionArea(id) )
		gModDataManager->destroyRestrictionArea(id);
	return 0;
}
