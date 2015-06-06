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

#include "irr_v3d.h"
#include <map>
#include <list>
#include <vector>

typedef struct Area {
	Area()
	{
		data = 0;
		datalen = 0;
	}
	~Area()
	{
		delete[] data;
	}

	u64 id;
	v3s16 minedge;
	v3s16 maxedge;
	void *data;
	size_t datalen;
} Area;

class AreaStore {
public:
	virtual void insertArea(Area *a) = 0;
	virtual void removeArea(u64 id) = 0;
	virtual void getAreasForPos(std::vector<Area *> &result, v3s16 pos) = 0;
	virtual void getAreasinArea(std::vector<Area *> &result, Area *a, bool accept_overlap) = 0;
	virtual u64 getFreeId(Area *a);
};


class VectorAreaStore : AreaStore {
public:
	virtual void insertArea(Area *a);
	virtual void removeArea(u64 id);
	virtual void getAreasForPos(std::vector<Area *> &result, v3s16 pos);
	virtual void getAreasinArea(std::vector<Area *> &result, Area *a, bool accept_overlap);
	~VectorAreaStore();
private:
	std::vector<Area *> m_areas;
};

typedef struct AreaStruct;

class OctreeAreaStore : AreaStore {
public:
	virtual void insertArea(Area *a);
	virtual void removeArea(u64 id);
	virtual void getAreasForPos(std::vector<Area *> &result, v3s16 pos);
	virtual void getAreasinArea(std::vector<Area *> &result, Area *a, bool accept_overlap);
	~OctreeAreaStore();
private:
	AreaStruct m_root_struct;
};

/*//TODO make static
//TODO find better name
typedef struct AreaStruct {
	AreaStruct() {
		for (int i = 0; i < 7; i++)
			substructs[i] = 0;
	}
	AreaStruct *substructs[8];

	//every area is stored in exactly one areas_stored map,
	//but can be inside multiple areas_inside maps.
	//TODO make this std::unordered_map when we can
	std::map<u64, Area*> areas_stored;
	std::list<Area *> areas_inside;
} AreaStruct;

class AreaStore {
public:
	// id will be overwritten
	void insertArea(Area * a);
	void removeArea(u64 id);
	std::vector<Area *> getAreasForPos(v3s16 pos);
	std::vector<Area *> getAreasinArea(Area * a, bool accept_overlap);
private:
	AreaStruct m_as;
};*/