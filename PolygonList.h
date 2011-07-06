/*
	Copyright (C) 2010-2011 Igor Izyumin	
	
	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef POLYGONLIST_H
#define POLYGONLIST_H

#include <QSet>
#include "polybool.h"

class Polygon;

class PolygonList : public QSet<Polygon*>
{
public:
	PolygonList();
	PolygonList(const Polygon& from);
	PolygonList(const POLYBOOLEAN::PAREA* from);
	PolygonList(const PolygonList& from);
	~PolygonList();

	// Operator overloads
	/// Assignment operator
	const PolygonList & operator=(const PolygonList& rhs);

	/// Union operator -- performs boolean OR with rhs polygon.
	PolygonList& operator|=(const Polygon& rhs);
	PolygonList& operator|(const Polygon& rhs) const;
	PolygonList& operator|=(const PolygonList& rhs);
	PolygonList& operator|(const PolygonList& rhs) const;
	/// Intersection operator -- performs boolean AND with rhs polygon.
	PolygonList& operator&=(const Polygon& rhs);
	PolygonList& operator&(const Polygon& rhs) const;
	PolygonList& operator&=(const PolygonList& rhs);
	PolygonList& operator&(const PolygonList& rhs) const;
	/// Subtract operator -- subtracts rhs polygon from this one.
	PolygonList& operator-=(const Polygon& rhs);
	PolygonList& operator-(const Polygon& rhs) const;
	PolygonList& operator-=(const PolygonList& rhs);
	PolygonList& operator-(const PolygonList& rhs) const;

private:
	/// Deallocates memory, then clears.
	void removeAll();
	void removeElement(Polygon* p);
	POLYBOOLEAN::PAREA* toPareaList() const;
	void rebuildFromParea(const POLYBOOLEAN::PAREA* a);
	void doBoolean(const PolygonList &rhs, POLYBOOLEAN::PAREA::PBOPCODE op);
};

#endif // POLYGONLIST_H
