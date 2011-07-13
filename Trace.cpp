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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "Trace.h"
#include "Area.h"

static void vertexDFS(QSet<QSharedPointer<Vertex> > &toVisit,
					  QSet<Vertex* > &currSet,
					  QSharedPointer<Vertex> currVtx);


Vertex::Vertex(QPoint pos, bool forcevia)
	: mPos(pos), mForceVia(forcevia)
{

}

void Vertex::draw(QPainter */*painter*/, const Layer& /*layer*/) const
{
	// XXX TODO draw via if needed
}

QRect Vertex::bbox() const
{
	// XXX TODO do something useful here
	return QRect(mPos.x(), mPos.y(), 1, 1);
}

bool Vertex::testHit(QPoint p, int dist, const Layer &l) const
{
	// XXX TODO check bbox
	if (QRect(p.x()-dist/2, p.y()-dist/2, dist, dist).contains(mPos))
	{
		// test each of the connected segments
		foreach(QSharedPointer<Segment> s, mSegs)
			if (s->layer() == l)
				return true;
	}
	return false;
}


void Vertex::addSegment(QSharedPointer<Segment> seg)
{
	mSegs.insert(seg);
}

void Vertex::removeSegment(QSharedPointer<Segment> seg)
{
	mSegs.remove(seg);
}

QSet<Vertex*> TraceList::getConnectedVertices(Vertex* vtx) const
{
	update();
	foreach(QSet<Vertex*> set, mConnections)
	{
		if (set.contains(vtx))
			return set;
	}
	return QSet<Vertex*>();
}

QSet<Vertex*> TraceList::getVerticesInArea(const Area& a) const
{
	Layer layer = a.layer();
	QSet<Vertex*> set;
	foreach(QSharedPointer<Vertex> vtx, myVtx)
	{
		if (vtx->onLayer(layer)  && a.pointInside(vtx->pos()))
			set.insert(vtx.data());
	}
	return set;
}

void TraceList::addSegment(QSharedPointer<Segment> s,
						   QSharedPointer<Vertex> v1,
						   QSharedPointer<Vertex> v2)
{
	// ensure vertices are added to the list
	myVtx.insert(v1);
	myVtx.insert(v2);
	mySeg.insert(s);
	s->setV1(v1);
	s->setV2(v2);
	v1->addSegment(s);
	v2->addSegment(s);
	mIsDirty = true;
}

void TraceList::removeSegment(QSharedPointer<Segment> s)
{
	if (!mySeg.remove(s))
		return;

	s->v1()->removeSegment(s);
	s->v2()->removeSegment(s);

	if (s->v1()->segments().count() == 0)
	{
		myVtx.remove(s->v1());
	}

	if (s->v2()->segments().count() == 0)
	{
		myVtx.remove(s->v2());
	}

	mIsDirty = true;
}

void TraceList::swapVtx(QSharedPointer<Segment> s,
						QSharedPointer<Vertex> vOld,
						QSharedPointer<Vertex> vNew)
{
	Q_ASSERT(s && vOld && vNew && (s->v1() == vOld || s->v2() == vOld));
	vOld->removeSegment(s);
	if (vOld->segments().count() == 0)
	{
		myVtx.remove(vOld);
	}
	if (s->v1() == vOld)
		s->setV1(vNew);
	else
		s->setV2(vNew);
	myVtx.insert(vNew);
	vNew->addSegment(s);
}

void TraceList::update() const
{
	if (!mIsDirty) return;
	// rebuild connection list
	QSet<QSharedPointer<Vertex> > toVisit(myVtx);
	QSet<Vertex* > currSet;
	this->mConnections.clear();
	while(!toVisit.empty())
	{
		currSet.clear();
		QSharedPointer<Vertex> vtx = *toVisit.begin();
		toVisit.remove(vtx);
		vertexDFS(toVisit, currSet, vtx);
		mConnections.append(currSet);
	}
}

void vertexDFS(QSet<QSharedPointer<Vertex> > &toVisit,
			   QSet<Vertex* > &currSet,
			   QSharedPointer<Vertex> currVtx)
{
	foreach(QSharedPointer<Segment> seg, currVtx->segments())
	{
		QSharedPointer<Vertex> vtx = seg->otherVertex(currVtx);
		if (toVisit.contains(vtx))
		{
			toVisit.remove(vtx);
			currSet.insert(vtx.data());
			vertexDFS(toVisit, currSet, vtx);
		}

	}
}

void TraceList::clear()
{
	// clear all internal refs to prevent leaks due to cycles
	foreach(QSharedPointer<Segment> seg, mySeg)
	{
		seg->clear();
	}
	mySeg.clear();
	foreach(QSharedPointer<Vertex> vtx, myVtx)
	{
		vtx->clear();
	}
	myVtx.clear();
}

void TraceList::loadFromXml(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "traces");
	clear();

	// read vertices
	reader.readNextStartElement();
	Q_ASSERT(reader.isStartElement() && reader.name() == "vertices");
	QHash<int, QSharedPointer<Vertex> > vmap;
	while(reader.readNextStartElement())
	{
		Q_ASSERT(reader.name() == "vertex");
		QXmlStreamAttributes attr = reader.attributes();
		int id = attr.value("id").toString().toInt();
		QPoint pt = QPoint(
				attr.value("x").toString().toInt(),
				attr.value("y").toString().toInt());
		bool forcevia = attr.value("forcevia") == "1";

		// XXX TODO find padstack / pin reference

		QSharedPointer<Vertex> v(new Vertex(pt, forcevia));
		this->myVtx.insert(v);
		vmap.insert(id, v);
		do
				reader.readNext();
		while(!reader.isEndElement());
	};

	// read segments
	reader.readNextStartElement();
	Q_ASSERT(reader.isStartElement() && reader.name() == "segments");
	while(reader.readNextStartElement())
	{
		Q_ASSERT(reader.name() == "segment");
		QXmlStreamAttributes attr = reader.attributes();
		int start = attr.value("start").toString().toInt();
		int end = attr.value("end").toString().toInt();
		Layer layer(static_cast<Layer::Type>(attr.value("layer").toString().toInt()));
		int width = attr.value("width").toString().toInt();

		QSharedPointer<Segment> s(new Segment(layer, width));
		addSegment(s,
				   vmap.value(start, QSharedPointer<Vertex>()),
				   vmap.value(end, QSharedPointer<Vertex>()));
		do
				reader.readNext();
		while(!reader.isEndElement());
	}
}

void TraceList::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("traces");
	writer.writeStartElement("vertices");
	foreach(QSharedPointer<Vertex> v, myVtx)
	{
		writer.writeStartElement("vertex");
		writer.writeAttribute("id", QString::number(v->getid()));
		writer.writeAttribute("x", QString::number(v->pos().x()));
		writer.writeAttribute("y", QString::number(v->pos().y()));
		writer.writeAttribute("forcevia", v->isForcedVia() ? "1" : "0");
		writer.writeEndElement();
	}
	writer.writeEndElement();

	writer.writeStartElement("segments");
	foreach(QSharedPointer<Segment> s, mySeg)
	{
		writer.writeStartElement("segment");
		writer.writeAttribute("start", QString::number(s->v1()->getid()));
		writer.writeAttribute("end", QString::number(s->v2()->getid()));
		writer.writeAttribute("layer", QString::number(s->layer().toInt()));
		writer.writeAttribute("width", QString::number(s->width()));
		writer.writeEndElement();
	}
	writer.writeEndElement();

	writer.writeEndElement();
}

QSharedPointer<Segment> TraceList::segment(QSharedPointer<Vertex> v1,
										   QSharedPointer<Vertex> v2) const
{
	foreach(QSharedPointer<Segment> s, v1->segments())
	{
		if (s->otherVertex(v1) == v2)
			return s;
	}
	return QSharedPointer<Segment>();
}

/////////////////////// SEGMENT ///////////////////////

Segment::Segment(const Layer& l, int w)
	: mLayer(l), mWidth(w)
{
}

Segment::Segment(const Segment &other)
	: mLayer(other.mLayer), mWidth(other.mWidth)
{
}

void Segment::draw(QPainter *painter, const Layer& layer) const
{
	if (mV1.isNull() || mV2.isNull()) return;
	if (layer != mLayer && layer != Layer::LAY_SELECTION)
		return;

	QPen pen = painter->pen();
	pen.setWidth(mWidth);
	painter->setPen(pen);

	painter->drawLine(mV1->pos(), mV2->pos());
}

QRect Segment::bbox() const
{
	if (mV1.isNull() || mV2.isNull()) return QRect();
	return QRect(mV1->pos(), mV2->pos()).normalized().adjusted(-mWidth/2, -mWidth/2, mWidth/2, mWidth/2);
}

bool Segment::testHit(QPoint p, int dist, const Layer &l) const
{
	return (l == layer()) &&
			(XPcb::distPtToSegment(p, mV1->pos(), mV2->pos())
			 <= (dist + mWidth));
}

bool Segment::loadState(PCBObjState& state)
{
	// convert to vertex state
	if (state.ptr().isNull())
		return false;
	QSharedPointer<SegState> vs = state.ptr().dynamicCast<SegState>();
	if (vs.isNull())
		return false;
	// restore state
	mLayer = vs->mLayer;
	mWidth = vs->mWidth;
	return true;
}

/////////////////////// VERTEX ///////////////////////

bool Vertex::isVia() const
{
	Layer layer;
	bool first = true;
	foreach(QSharedPointer<Segment> seg, this->mSegs)
	{
		if (!first && seg->layer() != layer)
			return true; // consecutive segments on different layers
		layer = seg->layer();
		first = false;
	}
	// all segments on the same layer
	return false;
}


bool Vertex::onLayer(const Layer& layer) const
{
	// vias are present on all layers
	if (isVia())
		return true;
	else
	{
		// not a via, all segments must be on same layer
		QSharedPointer<Segment> first = *this->mSegs.begin();
		if (layer == first->layer())
			return true;
		else
			return false;
	}
}

bool Vertex::loadState(PCBObjState& state)
{
	// convert to vertex state
	if (state.ptr().isNull())
		return false;
	QSharedPointer<VtxState> vs = state.ptr().dynamicCast<VtxState>();
	if (vs.isNull())
		return false;
	// restore state
	mPos = vs->mPos;
	mPadstack = vs->mPadstack;
	mForceVia = vs->mForceVia;
	return true;
}

////////////////////// UNDO COMMANDS //////////////////////


TraceList::AddSegCmd::AddSegCmd(QUndoCommand *parent,
								TraceList *tl,
								QSharedPointer<Segment> s,
								QSharedPointer<Vertex> v1,
								QSharedPointer<Vertex> v2)
	: QUndoCommand(parent), mTl(tl), mSeg(s), mV1(v1), mV2(v2)
{
}

void TraceList::AddSegCmd::undo()
{
	mTl->mySeg.remove(mSeg);
	mV1->removeSegment(mSeg);
	mV2->removeSegment(mSeg);

	if (mV1->segments().count() == 0)
		mTl->myVtx.remove(mV1);
	if (mV2->segments().count() == 0)
		mTl->myVtx.remove(mV2);
}

void TraceList::AddSegCmd::redo()
{
	mTl->addSegment(mSeg, mV1, mV2);
}


TraceList::DelSegCmd::DelSegCmd(QUndoCommand *parent,
								TraceList *tl,
								QSharedPointer<Segment> s)
	: QUndoCommand(parent), mTl(tl), mSeg(s)
{
}

void TraceList::DelSegCmd::undo()
{
	mTl->addSegment(mSeg, mV1, mV2);
}

void TraceList::DelSegCmd::redo()
{
	mV1 = mSeg->v1();
	mV2 = mSeg->v2();

	mTl->mySeg.remove(mSeg);
	mV1->removeSegment(mSeg);
	mV2->removeSegment(mSeg);

	if (mV1->segments().count() == 0)
		mTl->myVtx.remove(mV1);
	if (mV2->segments().count() == 0)
		mTl->myVtx.remove(mV2);
}

/////////////////

TraceList::SwapVtxCmd::SwapVtxCmd(QUndoCommand *parent, TraceList *tl,
								  QSharedPointer<Segment> s,
								  QSharedPointer<Vertex> vOld,
								  QSharedPointer<Vertex> vNew)
	: QUndoCommand(parent), mTl(tl), mSeg(s), mVOld(vOld), mVNew(vNew)
{
	Q_ASSERT(mVOld != mVNew);
	Q_ASSERT(s->v1() != mVNew && s->v2() != mVNew);
}

void TraceList::SwapVtxCmd::undo()
{
	Q_ASSERT((mSeg->v1() == mVNew && mSeg->v2() != mVNew) || (mSeg->v1() != mVNew && mSeg->v2() == mVNew));

	Q_ASSERT(mSeg->v1()->segments().contains(mSeg));
	Q_ASSERT(mSeg->v2()->segments().contains(mSeg));


	mVNew->removeSegment(mSeg);
	if (mVNew->segments().count() == 0)
	{
		mTl->myVtx.remove(mVNew);
	}
	mTl->myVtx.insert(mVOld);
	mVOld->addSegment(mSeg);
	if (mSeg->v1() == mVNew)
		mSeg->setV1(mVOld);
	else
		mSeg->setV2(mVOld);

	Q_ASSERT(mSeg->v1()->segments().contains(mSeg));
	Q_ASSERT(mSeg->v2()->segments().contains(mSeg));
	Q_ASSERT((mSeg->v1() == mVOld && mSeg->v2() != mVOld) || (mSeg->v1() != mVOld && mSeg->v2() == mVOld));
}

void TraceList::SwapVtxCmd::redo()
{
	Q_ASSERT((mSeg->v1() == mVOld && mSeg->v2() != mVOld) || (mSeg->v1() != mVOld && mSeg->v2() == mVOld));
	Q_ASSERT(mSeg->v1()->segments().contains(mSeg));
	Q_ASSERT(mSeg->v2()->segments().contains(mSeg));

	mVOld->removeSegment(mSeg);
	if (mVOld->segments().count() == 0)
	{
		mTl->myVtx.remove(mVOld);
	}
	if (mSeg->v1() == mVOld)
	{
		mSeg->setV1(mVNew);
		Q_ASSERT(mSeg->v1() == mVNew);
		Q_ASSERT(mSeg->v2() != mVNew);
	}
	else
	{
		mSeg->setV2(mVNew);
		Q_ASSERT(mSeg->v1() != mVNew);
		Q_ASSERT(mSeg->v2() == mVNew);
	}
	mVNew->addSegment(mSeg);
	mTl->myVtx.insert(mVNew);

	Q_ASSERT(mSeg->v1()->segments().contains(mSeg));
	Q_ASSERT(mSeg->v2()->segments().contains(mSeg));
	Q_ASSERT((mSeg->v1() == mVNew && mSeg->v2() != mVNew) || (mSeg->v1() != mVNew && mSeg->v2() == mVNew));

}

