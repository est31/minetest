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

#ifndef AREASTORE_H_
#define AREASTORE_H_

#include "irr_v3d.h"
#include "noise.h" // for PcgRandom
#include <map>
#include <list>
#include <vector>
#include <istream>
#include "util/numeric.h"

typedef struct Area {
	Area()
	{
		data = NULL;
		datalen = 0;
	}

	~Area()
	{
		delete[] data; // TODO perhaps overthink this...
	}

	void extremifyEdges()
	{
		v3s16 nminedge;
		v3s16 nmaxedge;

		nminedge.X = MYMIN(minedge.X, maxedge.X);
		nminedge.Y = MYMIN(minedge.Y, maxedge.Y);
		nminedge.Z = MYMIN(minedge.Z, maxedge.Z);
		nmaxedge.X = MYMAX(minedge.X, maxedge.X);
		nmaxedge.Y = MYMAX(minedge.Y, maxedge.Y);
		nmaxedge.Z = MYMAX(minedge.Z, maxedge.Z);

		maxedge = nmaxedge;
		minedge = nminedge;
	}

	u32 id;
	v3s16 minedge;
	v3s16 maxedge;
	char *data;
	u16 datalen;
} Area;

class AreaStore {
protected:
	// TODO change to unordered_map when we can
	std::map<u32, Area> areas_map;
	u16 count;
	PcgRandom random;
public:
	virtual void insertArea(const Area &a) = 0;
	virtual bool removeArea(u32 id) = 0;
	virtual void getAreasForPos(std::vector<Area *> *result, v3s16 pos) = 0;
	virtual void getAreasInArea(std::vector<Area *> *result,
		v3s16 minedge, v3s16 maxedge, bool accept_overlap) = 0;

	// calls a passed function for every stored area, until the
	// callback returns true. If that happens, it returns true,
	// if the search is exhausted, it returns false
	virtual bool forEach(bool (*callback)(void *args, Area *a), void *args) const = 0;

	virtual ~AreaStore()
	{}

	u32 getFreeId(v3s16 minedge, v3s16 maxedge);
	const Area *getArea(u32 id) const;
	u16 size() const;
	bool deserialize(std::istream &is);
	void serialize(std::ostream &is) const;
};


class VectorAreaStore : public AreaStore {
public:
	virtual void insertArea(const Area &a);
	virtual bool removeArea(u32 id);
	virtual void getAreasForPos(std::vector<Area *> *result, v3s16 pos);
	virtual void getAreasInArea(std::vector<Area *> *result,
		v3s16 minedge, v3s16 maxedge, bool accept_overlap);
	virtual bool forEach(bool (*callback)(void *args, Area *a), void *args) const;
private:
	std::vector<Area *> m_areas;
};

typedef struct AreaStruct AreaStruct;

class OctreeAreaStore : public AreaStore {
public:
	virtual void insertArea(const Area &a);
	virtual bool removeArea(u32 id);
	virtual void getAreasForPos(std::vector<Area *> &result, v3s16 pos);
	virtual void getAreasInArea(std::vector<Area *> &result,
		v3s16 minedge, v3s16 maxedge, bool accept_overlap);
	~OctreeAreaStore();
private:
	AreaStruct *m_root_struct;
	std::map<u32, Area*> areas_stored;
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

#endif /* AREASTORE_H_ */
