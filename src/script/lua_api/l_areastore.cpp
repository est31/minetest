/*
Minetest
Copyright (C) 2015 est31 <mtest31@outlook.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "lua_api/l_areastore.h"
#include "lua_api/l_internal.h"
#include "common/c_converter.h"
#include "areastore.h"

static inline void get_data_and_border_flags(u8 start_i, bool *borders, bool *data)
{
	if (!lua_isboolean(L, start_i))
		return;
	*borders = lua_toboolean(L, start_i);
	lua_pop(L, 1);
	if (!lua_isboolean(L, start_i + 1))
		return;
	*data = lua_toboolean(L, start_i + 1);
	lua_pop(L, 1);
}

static inline void push_area(const Area *a, bool data)
{
	lua_newtable(L);
	lua_pushstring(L, "min");
	push_v3s16(L, a->minedge);
	lua_settable(L,-3);
	lua_pushstring(L, "max");
	push_v3s16(L, a->maxedge);
	lua_settable(L,-3);
	if (data) {
		lua_pushstring(L, "data");
		lua_pushlstring(L, a->data, a->datalen);
		lua_settable(L,-3);
	}
}

static inline void push_areas(const std::vector<Area *> &areas, bool borders, bool data)
{
	lua_newtable(L);
	size_t cnt = areas.size();
	for (size_t i = 0; i < cnt; i++) {
		if (borders) {
			lua_pushnumber(L, areas[i]->id);
			push_area(areas[i], data);
			lua_settable(L,-3);
		} else {
			lua_pushnumber(L, areas[i]->id);
			lua_pushboolean(L, true);
			lua_settable(L,-3);
		}
	}
}

// garbage collector
int LuaAreaStore::gc_object(lua_State *L)
{
	LuaAreaStore *o = *(LuaAreaStore **)(lua_touserdata(L, 1));
	delete o;
	return 0;
}

// get_area(id, borders, data)
int LuaAreaStore::l_get_area(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	AreaStore *ast = o->as;

	u32 id = lua_tonumber(L, 2);

	bool borders = true;
	bool data = false;
	get_data_and_border_flags(3, borders, data);

	Area *res;
	
	ast->getArea(&res, id);
	push_area(res, borders, data);

	return 1;
}

// get_areas_for_pos(pos, borders, data)
int LuaAreaStore::l_get_areas_for_pos(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	AreaStore *ast = o->as;

	v3s16 pos = check_v3s16(L, 2);

	bool borders = true;
	bool data = false;
	get_data_and_border_flags(3, borders, data);

	std::vector<Area *> res;
	
	ast->getAreasForPos(&res, pos);
	push_areas(res, borders, data);

	return 1;
}

// get_areas_for_area(area, borders, data)
int LuaAreaStore::l_get_areas_for_area(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	AreaStore *ast = o->as;

	luaL_checktype(L, 2, LUA_TTABLE);
	lua_getfield(L, 2, "min");
	v3s16 minedge = check_v3s16(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, 2, "max");
	v3s16 maxedge = check_v3s16(L, -1);
	lua_pop(L, 1);

	bool borders = true;
	bool data = false;
	bool accept_overlap = false;
	if (lua_isboolean(L, 3)) {
		accept_overlap = lua_toboolean(L, 3);
		get_data_and_border_flags(4, borders, data);
	}
	std::vector<Area *> res;
	
	ast->getAreasInArea(&res, minedge, maxedge, accept_overlap);
	push_areas(res, borders, data);

	return 1;
}

// insert_area(area)
int LuaAreaStore::l_insert_area(lua_State *L)
{
	// TODO:
	// 1. find free id
	// 2. insert area
	// 3. return true (for success)
	return 1;
}

// remove_area(id)
int LuaAreaStore::l_remove_area(lua_State *L)
{
	// TODO:
	// 1. remove area with id
	// 3. return success boolean value
	return 1;
}

// to_string()
int LuaAreaStore::l_to_string(lua_State *L)
{
	// TODO: serialize to string
	return 1;
}

// to_file(filename)
int LuaAreaStore::l_to_string(lua_State *L)
{
	// TODO: serialize to file
	return 1;
}

// from_string(str)
int LuaAreaStore::l_from_string(lua_State *L)
{
	// TODO: deserialize from string
	return 1;
}

// from_file(filename)
int LuaAreaStore::l_from_file(lua_State *L)
{
	// TODO: deserialize from file
	return 1;
}

/*int LuaAreaStore::l_get_data(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = checkobject(L, 1);
	bool use_buffer  = lua_istable(L, 2);

	MMVManip *vm = o->vm;

	u32 volume = vm->m_area.getVolume();

	if (use_buffer)
		lua_pushvalue(L, 2);
	else
		lua_newtable(L);

	for (u32 i = 0; i != volume; i++) {
		lua_Integer cid = vm->m_data[i].getContent();
		lua_pushinteger(L, cid);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int LuaAreaStore::l_set_data(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = checkobject(L, 1);
	MMVManip *vm = o->vm;

	if (!lua_istable(L, 2))
		return 0;

	u32 volume = vm->m_area.getVolume();
	for (u32 i = 0; i != volume; i++) {
		lua_rawgeti(L, 2, i + 1);
		content_t c = lua_tointeger(L, -1);

		vm->m_data[i].setContent(c);

		lua_pop(L, 1);
	}

	return 0;
}

int LuaAreaStore::l_write_to_map(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	MMVManip *vm = o->vm;

	vm->blitBackAll(&o->modified_blocks);

	return 0;
}

int LuaAreaStore::l_get_node_at(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	GET_ENV_PTR;

	LuaAreaStore *o = checkobject(L, 1);
	v3s16 pos        = check_v3s16(L, 2);

	pushnode(L, o->vm->getNodeNoExNoEmerge(pos), env->getGameDef()->ndef());
	return 1;
}

int LuaAreaStore::l_set_node_at(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	GET_ENV_PTR;

	LuaAreaStore *o = checkobject(L, 1);
	v3s16 pos        = check_v3s16(L, 2);
	MapNode n        = readnode(L, 3, env->getGameDef()->ndef());

	o->vm->setNodeNoEmerge(pos, n);

	return 0;
}

int LuaAreaStore::l_update_liquids(lua_State *L)
{
	GET_ENV_PTR;

	LuaAreaStore *o = checkobject(L, 1);

	Map *map = &(env->getMap());
	INodeDefManager *ndef = getServer(L)->getNodeDefManager();
	MMVManip *vm = o->vm;

	Mapgen mg;
	mg.vm   = vm;
	mg.ndef = ndef;

	mg.updateLiquid(&map->m_transforming_liquid,
			vm->m_area.MinEdge, vm->m_area.MaxEdge);

	return 0;
}

int LuaAreaStore::l_calc_lighting(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = checkobject(L, 1);
	if (!o->is_mapgen_vm)
		return 0;

	INodeDefManager *ndef = getServer(L)->getNodeDefManager();
	EmergeManager *emerge = getServer(L)->getEmergeManager();
	MMVManip *vm = o->vm;

	v3s16 yblock = v3s16(0, 1, 0) * MAP_BLOCKSIZE;
	v3s16 fpmin  = vm->m_area.MinEdge;
	v3s16 fpmax  = vm->m_area.MaxEdge;
	v3s16 pmin   = lua_istable(L, 2) ? check_v3s16(L, 2) : fpmin + yblock;
	v3s16 pmax   = lua_istable(L, 3) ? check_v3s16(L, 3) : fpmax - yblock;

	sortBoxVerticies(pmin, pmax);
	if (!vm->m_area.contains(VoxelArea(pmin, pmax)))
		throw LuaError("Specified voxel area out of VoxelManipulator bounds");

	Mapgen mg;
	mg.vm          = vm;
	mg.ndef        = ndef;
	mg.water_level = emerge->params.water_level;

	mg.calcLighting(pmin, pmax, fpmin, fpmax);

	return 0;
}

int LuaAreaStore::l_set_lighting(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = checkobject(L, 1);
	if (!o->is_mapgen_vm)
		return 0;

	if (!lua_istable(L, 2))
		return 0;

	u8 light;
	light  = (getintfield_default(L, 2, "day",   0) & 0x0F);
	light |= (getintfield_default(L, 2, "night", 0) & 0x0F) << 4;

	MMVManip *vm = o->vm;

	v3s16 yblock = v3s16(0, 1, 0) * MAP_BLOCKSIZE;
	v3s16 pmin = lua_istable(L, 3) ? check_v3s16(L, 3) : vm->m_area.MinEdge + yblock;
	v3s16 pmax = lua_istable(L, 4) ? check_v3s16(L, 4) : vm->m_area.MaxEdge - yblock;

	sortBoxVerticies(pmin, pmax);
	if (!vm->m_area.contains(VoxelArea(pmin, pmax)))
		throw LuaError("Specified voxel area out of VoxelManipulator bounds");

	Mapgen mg;
	mg.vm = vm;

	mg.setLighting(light, pmin, pmax);

	return 0;
}

int LuaAreaStore::l_get_light_data(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = checkobject(L, 1);
	MMVManip *vm = o->vm;

	u32 volume = vm->m_area.getVolume();

	lua_newtable(L);
	for (u32 i = 0; i != volume; i++) {
		lua_Integer light = vm->m_data[i].param1;
		lua_pushinteger(L, light);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int LuaAreaStore::l_set_light_data(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = checkobject(L, 1);
	MMVManip *vm = o->vm;

	if (!lua_istable(L, 2))
		return 0;

	u32 volume = vm->m_area.getVolume();
	for (u32 i = 0; i != volume; i++) {
		lua_rawgeti(L, 2, i + 1);
		u8 light = lua_tointeger(L, -1);

		vm->m_data[i].param1 = light;

		lua_pop(L, 1);
	}

	return 0;
}

int LuaAreaStore::l_get_param2_data(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = checkobject(L, 1);
	MMVManip *vm = o->vm;

	u32 volume = vm->m_area.getVolume();

	lua_newtable(L);
	for (u32 i = 0; i != volume; i++) {
		lua_Integer param2 = vm->m_data[i].param2;
		lua_pushinteger(L, param2);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int LuaAreaStore::l_set_param2_data(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = checkobject(L, 1);
	MMVManip *vm = o->vm;

	if (!lua_istable(L, 2))
		return 0;

	u32 volume = vm->m_area.getVolume();
	for (u32 i = 0; i != volume; i++) {
		lua_rawgeti(L, 2, i + 1);
		u8 param2 = lua_tointeger(L, -1);

		vm->m_data[i].param2 = param2;

		lua_pop(L, 1);
	}

	return 0;
}

int LuaAreaStore::l_update_map(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	if (o->is_mapgen_vm)
		return 0;

	Environment *env = getEnv(L);
	if (!env)
		return 0;

	Map *map = &(env->getMap());

	// TODO: Optimize this by using Mapgen::calcLighting() instead
	std::map<v3s16, MapBlock *> lighting_mblocks;
	std::map<v3s16, MapBlock *> *mblocks = &o->modified_blocks;

	lighting_mblocks.insert(mblocks->begin(), mblocks->end());

	map->updateLighting(lighting_mblocks, *mblocks);

	MapEditEvent event;
	event.type = MEET_OTHER;
	for (std::map<v3s16, MapBlock *>::iterator
		it = mblocks->begin();
		it != mblocks->end(); ++it)
		event.modified_blocks.insert(it->first);

	map->dispatchEvent(&event);

	mblocks->clear();

	return 0;
}

int LuaAreaStore::l_was_modified(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = checkobject(L, 1);
	MMVManip *vm = o->vm;

	lua_pushboolean(L, vm->m_is_dirty);

	return 1;
}

int LuaAreaStore::l_get_emerged_area(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);

	push_v3s16(L, o->vm->m_area.MinEdge);
	push_v3s16(L, o->vm->m_area.MaxEdge);

	return 2;
}*/

LuaAreaStore::LuaAreaStore()
{
	this->as = new VectorAreaStore();
}

LuaAreaStore::LuaAreaStore(const std::string &type)
{
	if (type == "Octree") {
		//this->as = new OctreeAreaStore();
	} else if (type == "LibSpatial-R*") {
		//this->as = new LibSpatialAreaStore();
	} else {
		this->as = new VectorAreaStore();
	}
}

LuaAreaStore::LuaAreaStore(const std::string &filename)
{
	this->vm = new MMVManip(map);
	this->is_mapgen_vm = false;

	v3s16 bp1 = getNodeBlockPos(p1);
	v3s16 bp2 = getNodeBlockPos(p2);
	sortBoxVerticies(bp1, bp2);
	vm->initialEmerge(bp1, bp2);
}

LuaAreaStore::~LuaAreaStore()
{
	delete as;
}

// LuaAreaStore()
// Creates an LuaAreaStore and leaves it on top of stack
int LuaAreaStore::create_object(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	LuaAreaStore *o = (lua_isstring(L, 1)) ?
		new LuaAreaStore(lua_tostring(L, 1)) :
		new LuaAreaStore();

	*(void **)(lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
	return 1;
}

LuaAreaStore *LuaAreaStore::checkobject(lua_State *L, int narg)
{
	NO_MAP_LOCK_REQUIRED;

	luaL_checktype(L, narg, LUA_TUSERDATA);

	void *ud = luaL_checkudata(L, narg, className);
	if (!ud)
		luaL_typerror(L, narg, className);

	return *(LuaAreaStore **)ud;  // unbox pointer
}

void LuaAreaStore::Register(lua_State *L)
{
	lua_newtable(L);
	int methodtable = lua_gettop(L);
	luaL_newmetatable(L, className);
	int metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);  // hide metatable from Lua getmetatable()

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, gc_object);
	lua_settable(L, metatable);

	lua_pop(L, 1);  // drop metatable

	luaL_openlib(L, 0, methods, 0);  // fill methodtable
	lua_pop(L, 1);  // drop methodtable

	// Can be created from Lua (VoxelManip())
	lua_register(L, className, create_object);
}

const char LuaAreaStore::className[] = "AreaStore";
const luaL_reg LuaAreaStore::methods[] = {
	luamethod(LuaAreaStore, get_area),
	luamethod(LuaAreaStore, get_areas_for_pos),
	luamethod(LuaAreaStore, get_areas_in_area),
	luamethod(LuaAreaStore, insert_area),
	luamethod(LuaAreaStore, remove_area),
	luamethod(LuaAreaStore, to_string),
	luamethod(LuaAreaStore, to_file),
	luamethod(LuaAreaStore, from_string),
	luamethod(LuaAreaStore, from_file),
	{0,0}
};
