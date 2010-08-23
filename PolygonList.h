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
