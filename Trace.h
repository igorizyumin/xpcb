#ifndef TRACE_H
#define TRACE_H

#include <QList>
#include <QSet>
#include "PCBObject.h"

class Area;
class Polygon;

/// A trace vertex.

/// Trace vertices are endpoints of trace segments.  They form
/// connections to part pins and areas, and act as interlayer vias.
class Vertex : public PCBObject
{
public:
	Vertex(TraceList* parent, QPoint pos = QPoint(0, 0));

	QPoint pos() {return myPos;}

	/// Adds a connected segment to the vertex's segment set.
	void addSegment(Segment* seg);
	/// Removes a segment from the set of connected segments.
	void removeSegment(Segment* seg);
	/// Returns a reference to the segment set.
	const QSet<Segment*> & segments() { return mySegs; }
	/// Returns true if a vertex is present on the given layer
	bool onLayer(PCBLAYER layer);
	/// Returns true if the vertex is a via (exists on multiple layers)
	bool isVia();

private:
	TraceList * mParent;
	QPoint mPos;
	QSet<Segment*> mSegs;
};

/// Trace segment

/// A trace segment is a straight line between two trace vertices
class Segment : public PCBObject
{
public:
	Segment(TraceList* parent, Vertex* v1, Vertex* v2, PCBLAYER l = LAY_RAT_LINE, int w = 0);

	int width() {return myWidth;}
	void setWidth(int w) {myWidth = w;}
	PCBLAYER layer() {return myLayer;}
	void setLayer(PCBLAYER layer) {myLayer = layer;}

	Vertex* otherVertex(Vertex* v) {return (v == mV1 ? mV2 : mV1);}
	void draw(QPainter *painter, PCBLAYER layer);
private:
	PCBLAYER mLayer;
	TraceList *mParent;
	Vertex *mV1;
	Vertex *mV2;
	int mWidth;
};


/// The container for all copper traces and areas on the board.

/// A trace consists of a set of connected segments and
/// vertices, which form a graph structure.
class TraceList
{
public:
	TraceList();

	QSet<Vertex*> getConnectedVertices(Vertex* vtx);
	QSet<Vertex*> getVerticesInPoly(Polygon* poly);

private:

	void rebuildConnectionList();

	QSet<Segment*> mySeg;		// set of segments
	QSet<Vertex*> myVtx;		// set of vertices

	/// Master list of connections
	QList< QSet<Vertex*> > mConnections;
};



#endif // TRACE_H
