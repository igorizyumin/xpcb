#ifndef TRACE_H
#define TRACE_H

#include <QList>
#include <QSet>
#include "PCBObject.h"

class QXmlStreamReader;
class Area;
class Polygon;
class PartPin;
class Padstack;
class TraceList;
class Segment;

/// A trace vertex.

/// Trace vertices are endpoints of trace segments.  They form
/// connections to part pins and areas, and act as interlayer vias.
class Vertex : public PCBObject
{
public:
	Vertex(TraceList* parent, QPoint pos = QPoint(0, 0), bool forcevia = false);

	virtual void draw(QPainter *painter, PCBLAYER layer);
	virtual QRect bbox() const;

	QPoint pos() const {return mPos;}

	/// Adds a connected segment to the vertex's segment set.
	void addSegment(Segment* seg);
	/// Removes a segment from the set of connected segments.
	void removeSegment(Segment* seg);
	/// Returns a reference to the segment set.
	const QSet<Segment*> & segments() const { return mSegs; }
	/// Returns true if a vertex is present on the given layer
	bool onLayer(PCBLAYER layer) const;
	/// Returns true if the vertex is a via (exists on multiple layers)
	bool isVia() const;

private:
	TraceList * mParent;
	QPoint mPos;
	QSet<Segment*> mSegs;
	PartPin* mPartPin;
	Padstack* mPadstack;
	bool mForceVia;

};

/// Trace segment

/// A trace segment is a straight line between two trace vertices
class Segment : public PCBObject
{
public:
	Segment(TraceList* parent, Vertex* v1, Vertex* v2, PCBLAYER l = LAY_RAT_LINE, int w = 0);
	~Segment();

	virtual void draw(QPainter *painter, PCBLAYER layer);
	virtual QRect bbox() const;

	int width() const {return mWidth;}
	void setWidth(int w) {mWidth = w;}
	PCBLAYER layer() const {return mLayer;}
	void setLayer(PCBLAYER layer) {mLayer = layer;}

	Vertex* otherVertex(Vertex* v) const {return (v == mV1 ? mV2 : mV1);}
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
	TraceList() {}

	QSet<Vertex*> getConnectedVertices(Vertex* vtx) const;
	QSet<Vertex*> getVerticesInArea(const Area& poly) const;

	QSet<Segment*> segments() const {return mySeg;}
	QSet<Vertex*> vertices() const {return myVtx;}
	void loadFromXml(QXmlStreamReader &reader);
private:
	void clear();
	void rebuildConnectionList();

	QSet<Segment*> mySeg;		// set of segments
	QSet<Vertex*> myVtx;		// set of vertices

	/// Master list of connections
	QList< QSet<Vertex*> > mConnections;
};



#endif // TRACE_H
