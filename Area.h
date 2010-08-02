#ifndef AREA_H
#define AREA_H

#include "PCBObject.h"

/// A copper area.

/// Area describes a copper polygon.  Areas can either be unconnected,
/// or they can be assigned to a net.  In the latter case, the area will
/// automatically connect to any vertices, vias, or pins that are within its
/// boundaries.
class Area : public PCBObject
{
public:
	Area();
	~Area();						// destructor

private:
	QPolygon * poly;	// outline
	QList<PartPin*> pin;	// array of thru-hole pins
	QList<int> vcon;	// via connections (wtf is this???)
	QList<int> vtx;	// vertices (what is this???)
};

#endif // AREA_H
