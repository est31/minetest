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

#define AST_SMALLER_EQ_AS(p, q) ((p.X <= q.X) && (p.Y < q.Y) && (p.Z < q.Z))

#define AST_OVERLAPS_IN_DIMENSION(a, b, d) (                               \
	((a->minedge.d >= b->minedge.d) && (a->minedge.d <= b->maxedge.d))     \
	|| ((a->maxedge.d >= b->minedge.d) && (a->maxedge.d <= b->maxedge.d)))

#define AST_CONTAINS_PT(a, p) (AST_SMALLER_EQ_AS(a->minedge, q) && \
	AST_SMALLER_EQ_AS(p, a->maxedge))

#define AST_CONTAINS_AREA(a, b) (AST_SMALLER_EQ_AS(a->minedge, b->minedge) \
	&& VAST_SMALLER_EQ_AS(b->maxedge, a->maxedge))

#define AST_AREAS_OVERLAP(a, b) (AST_OVERLAPS_IN_DIMENSION(a, b, X) && \
	AST_OVERLAPS_IN_DIMENSION(a, b, Y) && AST_OVERLAPS_IN_DIMENSION(a, b, Z))

void VectorAreaStore::insertArea(Area *a)
{
	m_areas.push_back(a);
}

void VectorAreaStore::removeArea(u64 id)
{
	std::vector<Area *>::size_t msiz = m_areas.size();
	for (std::vector<Area *>::size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (b->id == id) {
			m_areas.erase(m_areas.begin() + i);
			break;
		}
	}
}

void VectorAreaStore::getAreasForPos(std::vector<Area *> &result, v3s16 pos)
{
	std::vector<Area *>::size_t msiz = m_areas.size();
	for (std::vector<Area *>::size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (AST_CONTAINS_PT(b, pos)) {
			result.push_back(b);
		}
	}
}

void VectorAreaStore::getAreasinArea(std::vector<Area *> &result, Area *a, bool accept_overlap)
{
	std::vector<Area *>::size_t msiz = m_areas.size();
	for (std::vector<Area *>::size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (AST_CONTAINS_AREA(b, b) || (accept_overlap && AST_AREAS_OVERLAP(a, b))) {
			result.push_back(b);
		}
	}
}

void VectorAreaStore::~VectorAreaStore()
{
	std::vector<Area *>::size_t msiz = m_areas.size();
	for (std::vector<Area *>::size_t i = 0; i < msiz; i++) {
		delete *m_areas[i];
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
		std::map<u64, Area*>::size_type s = areas_stored.size();
		if (s > 64) {
			// remove area, and add it below
			// take the area with the best score
		} else if (s < 32) {
			// add area from below, or if no "below" exists, be happy :)

			// take the area in the lower "layer" with the lowest score difference
			char sst = -1;
			char sst_score
			for (char i = 0; i < 8; i++) {
			}
		}
	}

	//every area is stored in exactly one areas_stored map,
	//but can be inside multiple areas_inside maps.
	//TODO make this std::unordered_map when we can
	std::map<u64, Area*> areas_stored;
} AreaStruct;

void OctreeAreaStore::insertArea(Area *a)
{
	m_root_struct.insert(a);
}

void OctreeAreaStore::removeArea(u64 id)
{
	//TODO implement
	// 1. traverse tree to find area (using unique id marking)
	// 2. call remove() on the containing area
	std::vector<Area *>::size_t msiz = m_areas.size();
	for (std::vector<Area *>::size_t i = 0; i < msiz; i++) {
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
	std::vector<Area *>::size_t msiz = m_areas.size();
	for (std::vector<Area *>::size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (AST_CONTAINS_PT(b, pos)) {
			result.push_back(b);
		}
	}
}

void OctreeAreaStore::getAreasinArea(std::vector<Area *> &result, Area *a, bool accept_overlap)
{
	//TODO implement
	// traverse tree down, and check areas on the way
	std::vector<Area *>::size_t msiz = m_areas.size();
	for (std::vector<Area *>::size_t i = 0; i < msiz; i++) {
		Area * b = m_areas[i];
		if (AST_CONTAINS_AREA(b, b) || (accept_overlap && AST_AREAS_OVERLAP(a, b))) {
			result.push_back(b);
		}
	}
}

void OctreeAreaStore::~OctreeAreaStore()
{
	//TODO implement
	std::vector<Area *>::size_t msiz = m_areas.size();
	for (std::vector<Area *>::size_t i = 0; i < msiz; i++) {
		delete *m_areas[i];
	}
}

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