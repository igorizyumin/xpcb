#ifndef AREA_H
#define AREA_H

#include "PolyLine.h"

class PartPin;
class Vertex;
class Net;
class TraceList;
class PCBDoc;

/// A copper area.

/// Area describes a copper polygon.  Areas can either be unconnected,
/// or they can be assigned to a net.  In the latter case, the area will
/// automatically connect to any vertices, vias, or pins that are within its
/// boundaries.
class Area : public PolyLine
{
public:
	Area();
	~Area();

private:
	/// Parent container
	PCBDoc* mDoc;

	/// Net assigned to this area
	Net* mNet;

	/// Whether to connect SMT pads to this area
	bool mConnectSMT;

	/// List of connected pins
	QList<PartPin*> mConnPins;
	/// List of connected vias and vertices
	QList<Vertex*> mConnVtx;
};

#endif // AREA_H
