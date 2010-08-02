#ifndef TRACE_H
#define TRACE_H

#include "PCBObject.h"
#include "Part.h"

/// A copper trace.

/// A trace consists of a set of connected segments and
/// vertices, which form a graph structure.
class Trace : public PCBObject
{
public:
	Trace();

	bool isVoid() {return mySeg.size() == 0 || myVtx.size() == 0;}

private:
	QSet<Segment> mySeg;		// set of segments
	QList<Vertex> myVtx;		// set of vertices
};

/// A trace vertex.

/// Trace vertices are endpoints of trace segments.  They form
/// connections to part pins and areas, and act as interlayer vias.
class Vertex : public PCBObject
{
public:
	Vertex(Trace* parent, QPoint pos = QPoint(0, 0));

	QPoint pos() {return myPos;}

	/// Adds a connected segment to the vertex's segment set.
	void addSegment(Segment* seg);
	/// Removes a segment from the set of connected segments.
	void removeSegment(Segment* seg);
	/// Returns a reference to the segment set.
	const QSet<Segment*> & segments() { return mySegs; }

private:
	Trace * myParent;
	QPoint myPos;
	QSet<Segment*> mySegs;
};

/// Trace segment

/// A trace segment is a straight line between two trace vertices
class Segment : public PCBObject
{
public:
	Segment(Trace* parent, Vertex* v1, Vertex* v2, PCBLAYER l = LAY_RAT_LINE, int w = 0);

	int width() {return myWidth;}
	void setWidth(int w) {myWidth = w;}
	PCBLAYER layer() {return myLayer;}
	void setLayer(PCBLAYER layer) {myLayer = layer;}

	void draw(QPainter *painter, PCBLAYER layer);
private:
	PCBLAYER myLayer;
	Trace *myParent;
	Vertex *myV1;
	Vertex *myV2;
	int myWidth;
};

#endif // TRACE_H
