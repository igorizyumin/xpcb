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
	Vertex(QPoint pos = QPoint(0, 0), bool forcevia = false);

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }
	virtual PCBObjState getState() const { return PCBObjState(new VtxState(*this)); }
	virtual bool loadState(PCBObjState& state);

	QPoint pos() const {return mPos;}
	void setPos(QPoint pos) { mPos = pos; }

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
	/// Returns the number of connected segments
	int numSegments() const { return mSegs.count(); }

	/// Returns the via padstack
	Padstack* padstack() const { return mPadstack; }

	bool isForcedVia() const {return mForceVia; }

private:
	class VtxState : public PCBObjStateInternal
	{
	public:
		virtual ~VtxState() {}
	private:
		friend class Vertex;
		VtxState(const Vertex &v)
			: mPos(v.pos()), mSegs(v.segments()),
			  mPadstack(v.padstack()), mForceVia(v.isForcedVia())
		{}

		QPoint mPos;
		QSet<Segment*> mSegs;
		Padstack* mPadstack;
		bool mForceVia;
	};
	QPoint mPos;
	QSet<Segment*> mSegs;
	Padstack* mPadstack;
	bool mForceVia;

};

/// Trace segment

/// A trace segment is a straight line between two trace vertices
class Segment : public PCBObject
{
public:
	Segment(Vertex* v1, Vertex* v2, const Layer& layer, int w = 0);
	~Segment();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual bool testHit(QPoint p, const Layer &l) const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }
	virtual PCBObjState getState() const { return PCBObjState(NULL); }
	virtual bool loadState(PCBObjState& /*state*/) { return false; }

	int width() const {return mWidth;}
	void setWidth(int w) {mWidth = w;}
	const Layer& layer() const {return mLayer;}
	void setLayer(const Layer& layer) {mLayer = layer;}

	Vertex* v1() { return mV1; }
	const Vertex* v1() const { return mV1; }
	Vertex* v2() { return mV2; }
	const Vertex* v2() const { return mV2; }
	Vertex* otherVertex(Vertex* v) const {return (v == mV1 ? mV2 : mV1);}

	// workaround so that python can compare object pointers correctly
	bool operator==(const Segment& other) const { return this == &other; }
private:
	Layer mLayer;
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

	void addSegment(Segment* s);
	void removeSegment(Segment* s);

	QSet<Segment*> segments() const {return mySeg;}
	QSet<Vertex*> vertices() const {return myVtx;}
	void loadFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
private:
	void clear();
	void update() const;

	QSet<Segment*> mySeg;		// set of segments
	QSet<Vertex*> myVtx;		// set of vertices

	bool mIsDirty;
	/// Master list of connections
	mutable QList< QSet<Vertex*> > mConnections;
};



#endif // TRACE_H
