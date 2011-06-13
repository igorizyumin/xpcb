#ifndef TRACE_H
#define TRACE_H

#include <QList>
#include <QSet>
#include "PCBObject.h"

class QXmlStreamReader;
class QXmlStreamWriter;
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

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }
	virtual PCBObjState getState() const { return PCBObjState(NULL); }
	virtual bool loadState(PCBObjState& /*state*/) { return false; }

	QPoint pos() const {return mPos;}

	/// Adds a connected segment to the vertex's segment set.
	void addSegment(Segment* seg);
	/// Removes a segment from the set of connected segments.
	void removeSegment(Segment* seg);
	/// Returns a reference to the segment set.
	const QSet<Segment*> & segments() const { return mSegs; }
	/// Returns true if a vertex is present on the given layer
	bool onLayer(const Layer& layer) const;
	/// Returns true if the vertex is a via (exists on multiple layers)
	bool isVia() const;

	bool isForcedVia() const {return mForceVia; }

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
	Segment(TraceList* parent, Vertex* v1, Vertex* v2, const Layer& layer, int w = 0);
	~Segment();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }
	virtual PCBObjState getState() const { return PCBObjState(NULL); }
	virtual bool loadState(PCBObjState& /*state*/) { return false; }

	int width() const {return mWidth;}
	void setWidth(int w) {mWidth = w;}
	const Layer& layer() const {return mLayer;}
	void setLayer(const Layer& layer) {mLayer = layer;}

	const Vertex* v1() const { return mV1; }
	const Vertex* v2() const { return mV2; }
	Vertex* otherVertex(Vertex* v) const {return (v == mV1 ? mV2 : mV1);}
private:
	Layer mLayer;
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
	void toXML(QXmlStreamWriter &writer) const;
private:
	void clear();
	void rebuildConnectionList();

	QSet<Segment*> mySeg;		// set of segments
	QSet<Vertex*> myVtx;		// set of vertices

	/// Master list of connections
	QList< QSet<Vertex*> > mConnections;
};



#endif // TRACE_H
