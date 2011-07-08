/*
	Copyright (C) 2010-2011 Igor Izyumin	
	
	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRACE_H
#define TRACE_H

#include <QList>
#include <QSet>
#include "PCBObject.h"
#include "Footprint.h"
#include "Net.h"

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
	virtual PCBObjState getState() const
	{
		return PCBObjState(new VtxState(*this));
	}
	virtual bool loadState(PCBObjState& state);

	QPoint pos() const {return mPos;}
	void setPos(QPoint pos) { mPos = pos; }

	/// Adds a connected segment to the vertex's segment set.
	void addSegment(QSharedPointer<Segment> seg);
	/// Removes a segment from the set of connected segments.
	void removeSegment(QSharedPointer<Segment> seg);
	/// Returns a reference to the segment set.
	const QSet<QSharedPointer<Segment> > segments() const { return mSegs; }
	/// Returns true if a vertex is present on the given layer
	bool onLayer(const Layer& layer) const;
	/// Returns true if the vertex is a via (exists on multiple layers)
	bool isVia() const;

	/// Returns the via padstack
	QSharedPointer<Padstack> padstack() const { return mPadstack; }

	bool isForcedVia() const {return mForceVia; }

	void clear() { mSegs.clear(); }

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
		QSharedPointer<Padstack> mPadstack;
		bool mForceVia;
	};
	QPoint mPos;
	QSet<QSharedPointer<Segment> > mSegs;
	QSharedPointer<Padstack> mPadstack;
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
	virtual PCBObjState getState() const
	{
		return PCBObjState(new SegState(*this));
	}

	virtual bool loadState(PCBObjState& state);

	int width() const {return mWidth;}
	void setWidth(int w) {mWidth = w;}
	const Layer& layer() const {return mLayer;}
	void setLayer(const Layer& layer) {mLayer = layer;}

	bool hasVertex(QSharedPointer<Vertex> v) const
	{
		return (v == mV1 || v == mV2);
	}

	QSharedPointer<Vertex> otherVertex(QSharedPointer<Vertex> v) const
	{
		if (!hasVertex(v))
			return QSharedPointer<Vertex>();
		else
			return (v == mV1 ? mV2 : mV1);
	}

	QSharedPointer<Vertex> commonVertex(QSharedPointer<Segment> s) const
	{
		if (s->hasVertex(mV1))
			return mV1;
		else if (s->hasVertex(mV2))
			return mV2;
		else
			return QSharedPointer<Vertex>();
	}

	QSharedPointer<Vertex> v1() const { return mV1; }
	QSharedPointer<Vertex> v2() const { return mV2; }

	void setV1(QSharedPointer<Vertex> v) { mV1 = v; }
	void setV2(QSharedPointer<Vertex> v) { mV2 = v; }

	void clear() { mV1.clear(); mV2.clear(); }

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
	QSharedPointer<Vertex> mV1;
	QSharedPointer<Vertex> mV2;
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

	void addSegment(QSharedPointer<Segment> s,
					QSharedPointer<Vertex> v1,
					QSharedPointer<Vertex> v2);
	QUndoCommand* addSegmentCmd(QSharedPointer<Segment> s,
								QSharedPointer<Vertex> v1,
								QSharedPointer<Vertex> v2,
								QUndoCommand* parent = NULL)
	{
		return new AddSegCmd(parent, this, s, v1, v2);
	}

	/// Also removes any attached vertices.
	void removeSegment(QSharedPointer<Segment> s);
	QUndoCommand* removeSegmentCmd(QSharedPointer<Segment> s,
								   QUndoCommand* parent = NULL)
	{
		return new DelSegCmd(parent, this, s);
	}

	/// Replaces vOld with vNew for the given segment.
	void swapVtx(QSharedPointer<Segment> s,
				 QSharedPointer<Vertex> vOld,
				 QSharedPointer<Vertex> vNew);
	QUndoCommand* swapVtxCmd(QSharedPointer<Segment> s,
							 QSharedPointer<Vertex> vOld,
							 QSharedPointer<Vertex> vNew,
							 QUndoCommand* parent = NULL)
	{
		return new SwapVtxCmd(parent, this, s, vOld, vNew);
	}

	QSharedPointer<Segment> segment(QSharedPointer<Vertex> v1,
									QSharedPointer<Vertex> v2) const;

	QSet<QSharedPointer<Segment> > segments() const {return mySeg;}
	QSet<QSharedPointer<Vertex> > vertices() const {return myVtx;}
	void loadFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
private:
	class AddSegCmd : public QUndoCommand
	{
	public:
		AddSegCmd(QUndoCommand* parent, TraceList* tl,
				  QSharedPointer<Segment> s,
				  QSharedPointer<Vertex> v1,
				  QSharedPointer<Vertex> v2);

		virtual void undo();
		virtual void redo();
	private:
		TraceList *mTl;
		QSharedPointer<Segment> mSeg;
		QSharedPointer<Vertex> mV1;
		QSharedPointer<Vertex> mV2;
	};

	class DelSegCmd : public QUndoCommand
	{
	public:
		DelSegCmd(QUndoCommand* parent, TraceList *tl,
				  QSharedPointer<Segment> s);

		virtual void undo();
		virtual void redo();
	private:
		TraceList *mTl;
		QSharedPointer<Segment> mSeg;
		QSharedPointer<Vertex> mV1, mV2;
	};

	class SwapVtxCmd : public QUndoCommand
	{
	public:
		SwapVtxCmd(QUndoCommand* parent, TraceList *tl,
				   QSharedPointer<Segment> s,
				   QSharedPointer<Vertex> vOld,
				   QSharedPointer<Vertex> vNew);

		virtual void undo();
		virtual void redo();
	private:
		TraceList *mTl;
		QSharedPointer<Segment> mSeg;
		QSharedPointer<Vertex> mVOld, mVNew;
	};

	friend class AddSegCmd;
	friend class DelSegCmd;
	friend class SwapVtxCmd;
	void clear();
	void update() const;

	QSet<QSharedPointer<Segment> > mySeg;		// set of segments
	QSet<QSharedPointer<Vertex> > myVtx;		// set of vertices

	bool mIsDirty;
	/// Master list of connections
	mutable QList<QSet<Vertex*> > mConnections;
};



#endif // TRACE_H
