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
#include "Document.h"
#include <QDebug>

Via::Via(QPoint pos, QSharedPointer<Padstack> ps)
	: mPos(pos), mPadstack(ps)
{
}

void Via::draw(QPainter *painter, const Layer &layer) const
{
	if (!layer.isCopper())
		return;

	painter->save();
	painter->translate(pos());

	if (layer == Layer::LAY_TOP_COPPER)
		padstack()->startPad().draw(painter);
	else if (layer == Layer::LAY_BOTTOM_COPPER)
		padstack()->endPad().draw(painter);
	else
		padstack()->innerPad().draw(painter);

	painter->restore();

}

QRect Via::bbox() const
{
	return padstack()->bbox().translated(pos());
}

bool Via::testHit(QPoint p, int dist, const Layer &l) const
{
	if (!l.isCopper())
		return false;

	QPoint delta( p - pos() );
	if (XPcb::norm(delta) < (padstack()->holeSize()/2+dist) )
		return true;
	if (l == Layer::LAY_TOP_COPPER)
		return padstack()->startPad().testHit(delta, dist);
	else if (l == Layer::LAY_BOTTOM_COPPER)
		return padstack()->endPad().testHit(delta, dist);
	else
		return padstack()->innerPad().testHit(delta, dist);
}

bool Via::loadState(PCBObjState &state)
{
	// convert to via state
	if (state.ptr().isNull())
		return false;
	QSharedPointer<ViaState> vs = state.ptr().dynamicCast<ViaState>();
	if (vs.isNull())
		return false;
	// restore state
	mPos = vs->mPos;
	mPadstack = vs->mPadstack;
	return true;
}

void Via::attach(const Vertex* vtx) const
{
	if (mVtxs.contains(vtx)) return;
	mVtxs.insert(vtx);
	vtx->attach(this);
}

void Via::detach(const Vertex *vtx) const
{
	if (!mVtxs.contains(vtx)) return;
	mVtxs.remove(vtx);
	vtx->detachVia();
}

void Via::detachAll() const
{
	foreach(const Vertex* vtx, mVtxs)
	{
		vtx->detachVia();
	}
	mVtxs.clear();
	mPartPins.clear();
}

bool Via::onLayer(const Layer &layer) const
{
	if (!layer.isCopper()) return false;
	if (layer == Layer::LAY_TOP_COPPER)
		return !padstack()->startPad().isNull();
	else if (layer == Layer::LAY_BOTTOM_COPPER)
		return !padstack()->endPad().isNull();
	else
		return !padstack()->innerPad().isNull();
}

/////////////////////// VERTEX ///////////////////////

Vertex::Vertex(QPoint pos)
	: mPos(pos), mPartPin(NULL), mVia(NULL)
{

}

void Vertex::draw(QPainter */*painter*/, const Layer& /*layer*/) const
{
	// XXX TODO draw appropriate things
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
	return true;
}

void Vertex::attach(const PartPin *pin) const
{
	if (mPartPin == pin) return;
	if (mPartPin) detachPin();
	mPartPin = pin;
	mPartPin->attach(this);
}

void Vertex::attach(const Via *via) const
{
	if (mVia == via) return;
	if (mVia) detachVia();
	mVia = via;
	via->attach(this);
}

void Vertex::detachPin() const
{
	if (!mPartPin) return;
	mPartPin->detach(this);
	mPartPin = NULL;
}

void Vertex::detachVia() const
{
	if (!mVia) return;
	mVia->detach(this);
	mVia = NULL;
}

void Vertex::detachAll() const
{
	detachPin();
	detachVia();
}

Layer Vertex::layer() const
{
	if (mSegs.isEmpty()) return Layer();
	else return (*mSegs.begin())->layer();
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
	return QRect(mV1->pos(),
				 mV2->pos()).normalized().adjusted(-mWidth/2,
												   -mWidth/2,
												   mWidth/2,
												   mWidth/2);
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

//////////////////////////    TRACELIST    /////////////////////////////////////

#if 0
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
#endif

QSet<Vertex*> TraceList::getVerticesInArea(const Area& a) const
{
	Layer layer = a.layer();
	QSet<Vertex*> set;
	foreach(QSharedPointer<Vertex> vtx, myVtx)
	{
		if (vtx->layer() == layer  && a.pointInside(vtx->pos()))
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
		vtx->clearSegments();
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

		QSharedPointer<Vertex> v(new Vertex(pt));
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
		Layer layer(static_cast<Layer::Type>(
						attr.value("layer").toString().toInt()));
		int width = attr.value("width").toString().toInt();

		QSharedPointer<Segment> s(new Segment(layer, width));
		addSegment(s,
				   vmap.value(start, QSharedPointer<Vertex>()),
				   vmap.value(end, QSharedPointer<Vertex>()));
		do
				reader.readNext();
		while(!reader.isEndElement());
	}

	// read vias
	reader.readNextStartElement();
	Q_ASSERT(reader.isStartElement() && reader.name() == "vias");
	while(reader.readNextStartElement())
	{
		Q_ASSERT(reader.name() == "via");
		QXmlStreamAttributes attr = reader.attributes();
		QPoint pt = QPoint(
				attr.value("x").toString().toInt(),
				attr.value("y").toString().toInt());
		QUuid psuuid(attr.value("padstack").toString());

		QSharedPointer<Via> via(new Via(pt, mDoc->padstack(psuuid)));
		myVias.insert(via);

		do
				reader.readNext();
		while(!reader.isEndElement());
	}
	mIsDirty = true;
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

	writer.writeStartElement("vias");
	foreach(QSharedPointer<Via> v, myVias)
	{
		writer.writeStartElement("via");
		writer.writeAttribute("x", QString::number(v->pos().x()));
		writer.writeAttribute("y", QString::number(v->pos().y()));
		writer.writeAttribute("padstack", v->padstack() ?
								  v->padstack()->uuid().toString()
								: QUuid().toString());
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
	Q_ASSERT((mSeg->v1() == mVNew && mSeg->v2() != mVNew)
			 || (mSeg->v1() != mVNew && mSeg->v2() == mVNew));

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
	Q_ASSERT((mSeg->v1() == mVOld && mSeg->v2() != mVOld)
			 || (mSeg->v1() != mVOld && mSeg->v2() == mVOld));
}

void TraceList::SwapVtxCmd::redo()
{
	Q_ASSERT((mSeg->v1() == mVOld && mSeg->v2() != mVOld)
			 || (mSeg->v1() != mVOld && mSeg->v2() == mVOld));
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
	Q_ASSERT((mSeg->v1() == mVNew && mSeg->v2() != mVNew)
			 || (mSeg->v1() != mVNew && mSeg->v2() == mVNew));

}

///////////////////////////   LVS   ////////////////////////////////////

TraceList::ConnGroup::ConnGroup(const PartPin *pin)
{
	mPins.insert(pin);
	foreach(const Vertex* v, pin->vertices())
	{
		DFS(v);
	}
	update();
}

void TraceList::ConnGroup::DFS(const Vertex* currVtx)
{
        mVertices.insert(currVtx);
	if (currVtx->partpin())
		mPins.insert(currVtx->partpin());
	if (currVtx->via())
	{
		mVias.insert(currVtx->via());
		mPins.unite(currVtx->via()->partpins());
		foreach(const Vertex* v, currVtx->via()->vertices())
		{
			if (!mVertices.contains(v))
				DFS(v);
		}
	}
	foreach(QSharedPointer<Segment> seg, currVtx->segments())
	{
		const Vertex* vtx = seg->otherVertex(currVtx);
		if (!mVertices.contains(vtx))
			DFS(vtx);
	}
}

void TraceList::ConnGroup::update()
{
	mNet.clear();
	mShortedPins.clear();

	if (mPins.count() == 0)
		return;

	if (mPins.count() == 1)
	{
		mNet = (*mPins.begin())->net();
		return;
	}

	QHash<QString, int> hash;
	foreach(const PartPin* p, mPins)
	{
		// pins not assigned to a net should not be connected to any
		// other pin
		if (p->net().isEmpty())
			mShortedPins.insert(p);
		else
			hash[p->net()]++; // auto-inserted if not present
	}
	if (hash.count() == 1) // only one net
	{
		mNet = (hash.begin().key());
		return;
	}
	// otherwise do majority voting
	QHash<QString, int>::const_iterator i;
	int max = -1;
	for(i = hash.constBegin(); i != hash.constEnd(); i++)
	{
		if (i.value() > max)
			mNet = i.key();
	}
	// insert shorted pins into array
	foreach(const PartPin* p, mPins)
	{
		if (p->net() != mNet)
			mShortedPins.insert(p);
	}
}

void TraceList::rebuildRats() const
{
	mRats.clear();
	foreach(QString net, mConnections.keys())
	{
		if (!net.isEmpty())
			rebuildRatsForNet(net);
	}
}

void TraceList::rebuildRatsForNet(QString netName) const
{
	// Prim's algorithm for building a minimum spanning tree
	// very inefficient, but who cares (for now)
	QList<ConnGroup> net = mConnections[netName];
	QList<ConnGroup> connected;
	connected.append(net.takeLast());
	QList<QPair<const PartPin*, const PartPin*> > rats;
	while(!net.isEmpty())
	{
		// iterate over all pins in the connected set
		int shortest_cg;
		QPair<const PartPin*, const PartPin*> shortest;
		double dist = 1e300; // really large value
		foreach(ConnGroup cg, connected)
		{
			foreach(const PartPin* pin, cg.validPins())
			{
				// now iterate over all pins in the remaining set and find the closest one
				for(int i = 0; i < net.size(); i++)
				{
					ConnGroup curr(net[i]);
					foreach(const PartPin* currPin, curr.validPins())
					{
						double newdist = XPcb::distance(pin->pos(), currPin->pos());
						if (newdist < dist)
						{
							dist = newdist;
							shortest.first = pin;
							shortest.second = currPin;
							shortest_cg = i;
						}
					}
				}
			}
		}
		// done with this cg
		connected.append(net[shortest_cg]);
		net.removeAt(shortest_cg);
		Q_ASSERT(shortest.first && shortest.second);
		rats.append(shortest);
	}

	mRats[netName] = rats;
}

/// Rebuild connection list
void TraceList::update() const
{
	if (!mIsDirty) return;

	rebuildConnectivity();

	// convert to regular pointers
	QSet<const PartPin*> toVisit;
	foreach(QSharedPointer<PartPin> pin, mDoc->partPins())
	{
		toVisit.insert(pin.data());
	}

	this->mConnections.clear();
	while(!toVisit.empty())
	{
		const PartPin* pin = *toVisit.begin();
		ConnGroup cg(pin);
		// remove visited pins
		toVisit.subtract(cg.pins());
		mConnections[cg.net()].append(cg);
	}

	rebuildRats();

//	mIsDirty = false;
}

void TraceList::draw(QPainter *painter, const Layer &layer) const
{
	painter->save();
	QPen pen = painter->pen();
	pen.setWidth(0);
	painter->setPen(pen);
	update();
	if (layer != Layer::LAY_RAT_LINE) return;
	typedef QPair<const PartPin*, const PartPin*> RatPair ;
	foreach(QList<RatPair> l, mRats.values())
	{
		foreach(RatPair rat, l)
		{
			painter->drawLine(rat.first->pos(), rat.second->pos());
		}
	}
	painter->restore();
}

void TraceList::rebuildConnectivity() const
{
	// go through pins, clear everything
	foreach(QSharedPointer<PartPin> pin, mDoc->partPins())
	{
		pin->detachAll();
	}

	// go through vias, check against pins
	QList<Layer> layers = mDoc->layerList(Document::ListOrder, Document::Copper);
	foreach(QSharedPointer<Via> via, myVias)
	{
		via->detachAll();
		foreach(QSharedPointer<PartPin> pin, mDoc->partPins())
		{
			foreach(Layer layer, layers)
			{
				if (via->onLayer(layer) &&
						pin->testHit(via->pos(), 0, layer))
					via->attach(pin.data());
			}
		}
	}
	// go through the vertices
	foreach(QSharedPointer<Vertex> vtx, myVtx)
	{
		// clear any existing connections
		vtx->detachAll();
		QPoint pos = vtx->pos();
		Layer layer = vtx->layer();
		// check each vertex against pins and vias
		foreach(QSharedPointer<PartPin> pin, mDoc->partPins())
		{
			if (pin->testHit(pos, 0, layer))
				vtx->attach(pin.data());
		}
		foreach(QSharedPointer<Via> via, myVias)
		{
			if (via->testHit(pos, 0, layer))
				vtx->attach(via.data());
		}
	}

}
