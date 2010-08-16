#ifndef POLYGONLIST_H
#define POLYGONLIST_H

#include <QList>

class Polygon;

class PolygonList : public QList<Polygon>
{
	PolygonList();
	PolygonList(const Polygon& from);
	PolygonList(const QList<Polygon>& from);

	// Operator overloads
	/// Union operator -- performs boolean OR with rhs polygon.
	PolygonList& operator|=(const Polygon& rhs);
	PolygonList& operator|(const Polygon& rhs);
	PolygonList& operator|=(const PolygonList& rhs);
	PolygonList& operator|(const PolygonList& rhs);
	/// Intersection operator -- performs boolean AND with rhs polygon.
	PolygonList& operator&=(const Polygon& rhs);
	PolygonList& operator&(const Polygon& rhs);
	PolygonList& operator&=(const PolygonList& rhs);
	PolygonList& operator&(const PolygonList& rhs);
	/// Subtract operator -- subtracts rhs polygon from this one.
	PolygonList& operator-=(const Polygon& rhs);
	PolygonList& operator-(const Polygon& rhs);
	PolygonList& operator&=(const PolygonList& rhs);
	PolygonList& operator&(const PolygonList& rhs);
};

#endif // POLYGONLIST_H
