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
#ifndef ANDROID
	#include "cmake_config.h"
#endif

#if USE_SPATIAL

#include <SpatialIndex.h>
#include "util/serialize.h"
#endif

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
	virtual void reserve(size_t count) {};
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
	virtual void reserve(size_t count);
	virtual bool removeArea(u32 id);
	virtual void getAreasForPos(std::vector<Area *> *result, v3s16 pos);
	virtual void getAreasInArea(std::vector<Area *> *result,
		v3s16 minedge, v3s16 maxedge, bool accept_overlap);
	virtual bool forEach(bool (*callback)(void *args, Area *a), void *args) const;
private:
	std::vector<Area *> m_areas;
};

#if USE_SPATIAL

//#define SPATIAL_DLEN sizeof(u32)

class SpatialAreaStore : public AreaStore {
public:
	SpatialAreaStore();
	virtual void insertArea(const Area &a);
	virtual bool removeArea(u32 id);
	virtual void getAreasForPos(std::vector<Area *> *result, v3s16 pos);
	virtual void getAreasInArea(std::vector<Area *> *result,
		v3s16 minedge, v3s16 maxedge, bool accept_overlap);
	virtual bool forEach(bool (*callback)(void *args, Area *a), void *args) const;

	virtual ~SpatialAreaStore();
private:
	SpatialIndex::ISpatialIndex *m_tree;
	SpatialIndex::IStorageManager *m_storagemanager;

	class VectorResultVisitor : public SpatialIndex::IVisitor {
	private:
		SpatialAreaStore *m_store;
		std::vector<Area *> *m_result;
	public:
		VectorResultVisitor(std::vector<Area *> *result, SpatialAreaStore *store)
		{
			m_store = store;
			m_result = result;
		}

		virtual void visitNode(const SpatialIndex::INode &in)
		{
		}

		virtual void visitData(const SpatialIndex::IData &in)
		{
			/*uint32_t len;
			uint8_t *result;
			in.getData(len, &result);
			assert(len == SPATIAL_DLEN);
			u32 id = readU32(result);*/
			u32 id = in.getIdentifier();

			std::map<u32, Area>::iterator itr = m_store->areas_map.find(id);
			assert(itr != m_store->areas_map.end());
			m_result->push_back(&itr->second);
		}

		virtual void visitData(std::vector<const SpatialIndex::IData *> &v)
		{
			for (size_t i = 0; i < v.size(); i++)
				visitData(*(v[i]));
		}

		~VectorResultVisitor() {}
	};
};

#endif

#endif /* AREASTORE_H_ */
