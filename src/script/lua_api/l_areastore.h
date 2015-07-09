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

#ifndef L_AREASTORE_H_
#define L_AREASTORE_H_

#include "lua_api/l_base.h"
#include "irr_v3d.h"
#include <map>
#include "areastore.h"

class Map;
class MapBlock;
class MMVManip;

/*
  AreaStore
 */
class LuaAreaStore : public ModApiBase {
private:

	static const char className[];
	static const luaL_reg methods[];

	static int gc_object(lua_State *L);

	static int l_get_area(lua_State *L);

	static int l_get_areas_for_pos(lua_State *L);
	static int l_get_areas_in_area(lua_State *L);
	static int l_insert_area(lua_State *L);
	static int l_remove_area(lua_State *L);

	static int l_to_string(lua_State *L);
	static int l_to_file(lua_State *L);

	static int l_from_string(lua_State *L);
	static int l_from_file(lua_State *L);
	/*static int l_write_to_map(lua_State *L);

	static int l_get_node_at(lua_State *L);
	static int l_set_node_at(lua_State *L);

	static int l_update_map(lua_State *L);
	static int l_update_liquids(lua_State *L);

	static int l_calc_lighting(lua_State *L);
	static int l_set_lighting(lua_State *L);
	static int l_get_light_data(lua_State *L);
	static int l_set_light_data(lua_State *L);

	static int l_get_param2_data(lua_State *L);
	static int l_set_param2_data(lua_State *L);

	static int l_was_modified(lua_State *L);
	static int l_get_emerged_area(lua_State *L);*/

public:
	AreaStore *as;

	LuaAreaStore();
	LuaAreaStore(const std::string &type);
	~LuaAreaStore();

	// AreaStore()
	// Creates a AreaStore and leaves it on top of stack
	static int create_object(lua_State *L);

	static LuaAreaStore *checkobject(lua_State *L, int narg);

	static void Register(lua_State *L);
};

#endif /* L_AREASTORE_H_ */
