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
 
#include "areastore.h"
#include "util/serialize.h"

#define AST_SMALLER_EQ_AS(p, q) ((p.X <= q.X) && (p.Y < q.Y) && (p.Z < q.Z))

#define AST_OVERLAPS_IN_DIMENSION(amine, amaxe, b, d) (              \
	((amine.d >= b->minedge.d) && (amine.d <= b->maxedge.d))     \
	|| ((amaxe.d >= b->minedge.d) && (amaxe.d <= b->maxedge.d)))

#define AST_CONTAINS_PT(a, p) (AST_SMALLER_EQ_AS(a->minedge, p) && \
	AST_SMALLER_EQ_AS(p, a->maxedge))

#define AST_CONTAINS_AREA(amine, amaxe, b)         \
	(AST_SMALLER_EQ_AS(amine, b->minedge) \
	&& AST_SMALLER_EQ_AS(b->maxedge, amaxe))

#define AST_AREAS_OVERLAP(amine, amaxe, b)                \
	(AST_OVERLAPS_IN_DIMENSION(amine, amaxe, b, X) && \
	AST_OVERLAPS_IN_DIMENSION(amine, amaxe, b, Y) &&  \
	AST_OVERLAPS_IN_DIMENSION(amine, amaxe, b, Z))


u16 AreaStore::size() const
{
	return count;
}

Area *AreaStore::getArea(u32 id) const
{
	Area *res = NULL;
	std::map<u32, Area *>::const_iterator itr = areas_map.find(id);
	if (itr != areas_map.end()) {
		res = itr->second;
	}
	return res;
}

bool AreaStore::deserialize(std::istream &is)
{
	u8 ver = readU8(is);
	if (ver != 1)
		return false;
	u16 count_areas = readU16(is);
	for (u16 i = 0; i < count_areas; i++) {
		// deserialize an area
		Area *a = new Area();
		a->id = readU32(is);
		a->minedge = readV3S16(is);
		a->maxedge = readV3S16(is);
		a->datalen = readU16(is);
		a->data = new char[a->datalen];
		is.read((char *) a->data, a->datalen);
		insertArea(a);
	}
	return true;
}


static bool serialize_area(void *ostr, Area *a)
{
	std::ostream &os = *((std::ostream *) ostr);
	writeU32(os, a->id);
	writeV3S16(os, a->minedge);
	writeV3S16(os, a->maxedge);
	writeU16(os, a->datalen);
	os.write(a->data, a->datalen);

	return false;
}


void AreaStore::serialize(std::ostream &os) const
{
	// write initial data
	writeU8(os, 1); // serialisation version
	writeU16(os, count);
	forEach(&serialize_area, &os);
}

void VectorAreaStore::insertArea(Area *a)
{
	areas_map[a->id] = a;
	count++;
	m_areas.push_back(a);
}

void VectorAreaStore::removeArea(u32 id)
{
	areas_map.erase(id);
	size_t msiz = m_areas.size();
	for (size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (b->id == id) {
			m_areas.erase(m_areas.begin() + i);
			count--;
			break;
		}
	}
}

void VectorAreaStore::getAreasForPos(std::vector<Area *> *result, v3s16 pos)
{
	size_t msiz = m_areas.size();
	for (size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (AST_CONTAINS_PT(b, pos)) {
			result->push_back(b);
		}
	}
}

void VectorAreaStore::getAreasInArea(std::vector<Area *> *result,
		v3s16 minedge, v3s16 maxedge, bool accept_overlap)
{
	size_t msiz = m_areas.size();
	for (size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (AST_CONTAINS_AREA(minedge, maxedge, b) || (accept_overlap &&
				AST_AREAS_OVERLAP(minedge, maxedge, b))) {
			result->push_back(b);
		}
	}
}

bool VectorAreaStore::forEach(bool (*callback)(void *args, Area *a), void *args) const
{
	size_t msiz = m_areas.size();
	for (size_t i = 0; i < msiz; i++) {
		if (callback(args, m_areas[i])) {
			return true;
		}
	}
	return false;
}

VectorAreaStore::~VectorAreaStore()
{
	size_t msiz = m_areas.size();
	for (size_t i = 0; i < msiz; i++) {
		delete m_areas[i];
	}
}

static u8 get_cost(v3s16 minedge, v3s16 maxedge, unsigned char level) {
	u8 cost = 0;

	while (++level <= 16) {
		unsigned char lev = level - 1;
		cost +=
			((((maxedge.X - minedge.X) >> lev) & 1) ? 1 : 0) +
			((((maxedge.Y - minedge.Y) >> lev) & 1) ? 1 : 0) +
			((((maxedge.Z - minedge.Z) >> lev) & 1) ? 1 : 0);
	}
	return cost;
}

// returns the "cost" of an area given by minedge and maxedge
// intersected with an area given by a coordinate m
static u8 get_cost_for_part(v3s16 minedge, v3s16 maxedge, unsigned char level,
		v3s16 m, unsigned char m_level) {
	u8 cost = 0;

	// todo do the following things to minedge and maxedge:
	// 1. get minedge and maxedge of m and m_level
	// 1a. if both are inside m, be finished :)
	// 2. replace the not inside edge with an edge on m's border plane

	while (++level <= 16) {
		unsigned char lev = level - 1;
		cost +=
			((((maxedge.X - minedge.X) >> lev) & 1) ? 1 : 0) +
			((((maxedge.Y - minedge.Y) >> lev) & 1) ? 1 : 0) +
			((((maxedge.Z - minedge.Z) >> lev) & 1) ? 1 : 0);
	}
	return cost;
}

// http://stackoverflow.com/a/19535699

typedef struct AreaStruct {
	AreaStruct(char n_level) {
		level = n_level;
		for (int i = 0; i < 7; i++)
			substructs[i] = 0;
	}
	AreaStruct *substructs[8];
	char level;

	void insertHere(Area *a)
	{
		areas_stored[a->id] = a;
	}

	void insert(Area *a)
	{
		insertHere(a);
		balance();
	}

	void balance()
	{
		if (level >= 13) // if the level is too deep, we don't have substructs
			return;
		size_t s = areas_stored.size();
		if (s > 64) {
			// remove area, and add it below
			// take the area with the best score
		} else if (s < 32) {
			// add area from below, or if no "below" exists, be happy :)

			// take the area in the lower "layer" with the lowest score difference
			char sst = -1;
			char sst_score;
			for (char i = 0; i < 8; i++) {
			}
		}
	}

	//every area is stored in exactly one areas_stored map,
	//but can be inside multiple areas_inside maps.
	//TODO make this std::unordered_map when we can
	std::map<u64, Area*> areas_stored;
} AreaStruct;

/*void OctreeAreaStore::insertArea(Area *a)
{
	m_root_struct.insert(a);
}

void OctreeAreaStore::removeArea(u32 id)
{
	//TODO implement
	// 1. traverse tree to find area (using unique id marking)
	// 2. call remove() on the containing area
	size_t msiz = m_areas.size();
	for (size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (b->id == id) {
			m_areas.erase(m_areas.begin() + i);
			break;
		}
	}
}

void OctreeAreaStore::getAreasForPos(std::vector<Area *> &result, v3s16 pos)
{
	//TODO implement
	// traverse tree down as far as possible, and check areas on the way
	size_t msiz = m_areas.size();
	for (size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (AST_CONTAINS_PT(b, pos)) {
			result.push_back(b);
		}
	}
}

void OctreeAreaStore::getAreasInArea(std::vector<Area *> *result, Area *a, bool accept_overlap)
{
	//TODO implement
	// traverse tree down, and check areas on the way
	size_t msiz = m_areas.size();
	for (size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (AST_CONTAINS_AREA(b, b) || (accept_overlap && AST_AREAS_OVERLAP(a, b))) {
			result->push_back(b);
		}
	}
}

OctreeAreaStore::OctreeAreaStore()
{
	m_root_struct = new AreaStruct(0);
}

OctreeAreaStore::~OctreeAreaStore()
{
	delete m_root_struct;
	//TODO implement
	size_t msiz = m_areas.size();
	for (size_t i = 0; i < msiz; i++) {
		delete *m_areas[i];
	}
}*/

/*void AreaStore::insertArea(Area * a)
{
	AreaStruct *tostorein = &m_as;
	while (true) {
		tostorein = 
	}
	tostorein->
}
void AreaStore::removeArea(u64 id)
{
	
}
std::vector<Area *> AreaStore::getAreasForPos(v3s16 pos)
{
}

std::vector<Area *> AreaStore::getAreasinArea(Area * a, bool accept_overlap)
{
}
*/