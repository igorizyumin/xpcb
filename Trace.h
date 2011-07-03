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
			: mPos(v.pos()),
			  mPadstack(v.padstack()), mForceVia(v.isForcedVia())
		{}

		QPoint mPos;
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
	Segment(const Layer& layer, int w = 0);

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual bool testHit(QPoint p, const Layer &l) const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }
	virtual PCBObjState getState() const { return PCBObjState(new SegState(*this)); }
	virtual bool loadState(PCBObjState& state);

	int width() const {return mWidth;}
	void setWidth(int w) {mWidth = w;}
	const Layer& layer() const {return mLayer;}
	void setLayer(const Layer& layer) {mLayer = layer;}

	Vertex* otherVertex(Vertex* v) const {if (v != mV1 && v != mV2) return NULL; else return (v == mV1 ? mV2 : mV1);}

	Vertex* v1() const { return mV1; }
	Vertex* v2() const { return mV2; }

	void setV1(Vertex* v) { mV1 = v; }
	void setV2(Vertex* v) { mV2 = v; }

	// workaround so that python can compare object pointers correctly
	bool operator==(const Segment& other) const { return this == &other; }
private:
	class SegState : public PCBObjStateInternal
	{
	public:
		virtual ~SegState() {}
	private:
		friend class Segment;
		SegState(const Segment &v)
			: mLayer(v.layer()), mWidth(v.width())
		{}

		Layer mLayer;
		int mWidth;
	};
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
	TraceList() : mIsDirty(false) {}
	~TraceList() { clear(); }

	QSet<Vertex*> getConnectedVertices(Vertex* vtx) const;
	QSet<Vertex*> getVerticesInArea(const Area& poly) const;

	/// Takes ownership of all args
	void addSegment(Segment* s, Vertex* v1, Vertex* v2);
	QUndoCommand* addSegmentCmd(Segment* s, Vertex* v1, Vertex* v2, QUndoCommand* parent = NULL) { return new AddSegCmd(parent, this, s, v1, v2); }

	/// Also removes and deletes any attached vertices.
	void removeSegment(Segment* s);
	QUndoCommand* removeSegmentCmd(Segment* s, QUndoCommand* parent = NULL) { return new DelSegCmd(parent, this, s); }
	/// Replaces vOld with vNew for the given segment.  Takes ownership of vNew.
	void swapVtx(Segment* s, Vertex* vOld, Vertex* vNew);
	QUndoCommand* swapVtxCmd(Segment* s, Vertex* vOld, Vertex* vNew, QUndoCommand* parent = NULL) { return new SwapVtxCmd(parent, this, s, vOld, vNew); }

	Segment* segment(Vertex* v1, Vertex* v2) const;

	QSet<Segment*> segments() const {return mySeg;}
	QSet<Vertex*> vertices() const {return myVtx;}
	void loadFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
private:
	class AddSegCmd : public QUndoCommand
	{
	public:
		AddSegCmd(QUndoCommand* parent, TraceList *tl,
					  Segment* s, Vertex* v1, Vertex* v2);
		virtual ~AddSegCmd();

		virtual void undo();
		virtual void redo();
	private:
		TraceList *mTl;
		Segment *mSeg;
		Vertex *mV1, *mV2;
		bool mUndone;
	};

	class DelSegCmd : public QUndoCommand
	{
	public:
		DelSegCmd(QUndoCommand* parent, TraceList *tl,
					  Segment* s);
		virtual ~DelSegCmd();

		virtual void undo();
		virtual void redo();
	private:
		TraceList *mTl;
		Segment *mSeg;
		Vertex *mV1, *mV2;
		bool mUndone;
	};

	class SwapVtxCmd : public QUndoCommand
	{
	public:
		SwapVtxCmd(QUndoCommand* parent, TraceList *tl,
					  Segment* s, Vertex* vOld, Vertex* vNew);
		virtual ~SwapVtxCmd();

		virtual void undo();
		virtual void redo();
	private:
		TraceList *mTl;
		Segment *mSeg;
		Vertex *mVOld, *mVNew;
		bool mNewRemoved, mOldRemoved;
	};

	friend class AddSegCmd;
	friend class DelSegCmd;
	friend class SwapVtxCmd;
	void clear();
	void update() const;

	QSet<Segment*> mySeg;		// set of segments
	QSet<Vertex*> myVtx;		// set of vertices

	bool mIsDirty;
	/// Master list of connections
	mutable QList< QSet<Vertex*> > mConnections;
};



#endif // TRACE_H
