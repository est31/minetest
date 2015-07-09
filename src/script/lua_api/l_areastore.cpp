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
#include "cpp_api/s_security.h"
#include "areastore.h"
#include "filesys.h"
#include <fstream>

static inline void get_data_and_border_flags(lua_State *L, u8 start_i,
		bool *borders, bool *data)
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

static inline void push_area(lua_State *L, const Area *a, bool data)
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

static inline void push_areas(lua_State *L, const std::vector<Area *> &areas,
		bool borders, bool data)
{
	lua_newtable(L);
	size_t cnt = areas.size();
	for (size_t i = 0; i < cnt; i++) {
		if (borders) {
			lua_pushnumber(L, areas[i]->id);
			push_area(L, areas[i], data);
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

	bool data = false;
	if (lua_isboolean(L, 3)) {
		data = lua_toboolean(L, 3);
	}

	const Area *res;
	
	res = ast->getArea(id);
	push_area(L, res, data);

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
	get_data_and_border_flags(L, 3, &borders, &data);

	std::vector<Area *> res;
	
	ast->getAreasForPos(&res, pos);
	push_areas(L, res, borders, data);

	return 1;
}

// l_get_areas_in_area(area, borders, data)
int LuaAreaStore::l_get_areas_in_area(lua_State *L)
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
		get_data_and_border_flags(L, 4, &borders, &data);
	}
	std::vector<Area *> res;
	
	ast->getAreasInArea(&res, minedge, maxedge, accept_overlap);
	push_areas(L, res, borders, data);

	return 1;
}

// insert_area(area)
int LuaAreaStore::l_insert_area(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	AreaStore *ast = o->as;

	Area a;

	luaL_checktype(L, 2, LUA_TTABLE);
	lua_getfield(L, 2, "min");
	a.minedge = check_v3s16(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, 2, "max");
	a.maxedge = check_v3s16(L, -1);
	lua_pop(L, 1);

	a.extremifyEdges();
	a.id = ast->getFreeId(a.minedge, a.maxedge);

	if (a.id != 0) {
		lua_getfield(L, 2, "data");
		size_t d_len;
		const char *data = luaL_checklstring(L,-1, &d_len);
		lua_pop(L, 1);

		a.data = new char[d_len];
		a.datalen = d_len;

		memcpy(a.data, data, d_len);

		ast->insertArea(a);

		lua_pushboolean(L, true);
	} else {
		// couldn't get free id
		lua_pushboolean(L, false);
	}
	return 1;
}

// remove_area(id)
int LuaAreaStore::l_remove_area(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	AreaStore *ast = o->as;

	u32 id = lua_tonumber(L, 2);
	bool success = ast->removeArea(id);

	lua_pushboolean(L, success);
	return 1;
}

// to_string()
int LuaAreaStore::l_to_string(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	AreaStore *ast = o->as;

	std::ostringstream os(std::ios_base::binary);
	ast->serialize(os);
	std::string str = os.str();

	lua_pushlstring(L, str.c_str(), str.length());
	return 1;
}

// to_file(filename)
int LuaAreaStore::l_to_file(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	AreaStore *ast = o->as;

	const char *filename = luaL_checkstring(L, 2);
	CHECK_SECURE_PATH_OPTIONAL(L, filename);

	std::ostringstream os(std::ios_base::binary);
	ast->serialize(os);

	lua_pushboolean(L, fs::safeWriteToFile(filename, os.str()));
	return 1;
}

// from_string(str)
int LuaAreaStore::l_from_string(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	AreaStore *ast = o->as;

	size_t len;
	const char *str = luaL_checklstring(L, 2, &len);

	std::istringstream is(std::string(str, len), std::ios::binary);
	bool success = ast->deserialize(is);

	lua_pushboolean(L, success);
	return 1;
}

// from_file(filename)
int LuaAreaStore::l_from_file(lua_State *L)
{
	LuaAreaStore *o = checkobject(L, 1);
	AreaStore *ast = o->as;

	const char *filename = luaL_checkstring(L, 2);
	CHECK_SECURE_PATH_OPTIONAL(L, filename);

	std::ifstream is(filename, std::ios::binary);
	bool success = ast->deserialize(is);

	lua_pushboolean(L, success);
	return 1;
}

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
