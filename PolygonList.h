#ifndef POLYGONLIST_H
#define POLYGONLIST_H

#include <QSet>

class Polygon;
class PAREA;
enum PAREA::PBOPCODE;

class PolygonList : public QSet<Polygon>
{
public:
	PolygonList();
	PolygonList(const Polygon& from);
	PolygonList(const PAREA* from);
	~PolygonList();

	// Operator overloads
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
	PolygonList& operator&=(const PolygonList& rhs);
	PolygonList& operator&(const PolygonList& rhs) const;

private:
	PAREA* toPareaList();
	void rebuildFromParea(PAREA* a);
	void doBoolean(const PolygonList &rhs, PAREA::PBOPCODE op);
};

#endif // POLYGONLIST_H
