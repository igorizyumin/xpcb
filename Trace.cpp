#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "Trace.h"
#include "Area.h"

static void vertexDFS(QSet<Vertex*> &toVisit, QSet<Vertex*> &currSet, Vertex *currVtx);


Vertex::Vertex(QPoint pos, bool forcevia)
	: mPos(pos), mPadstack(NULL), mForceVia(forcevia)
{

}

void Vertex::draw(QPainter */*painter*/, const Layer& /*layer*/) const
{
	// XXX TODO draw via if needed
}

QRect Vertex::bbox() const
{
	// XXX TODO do something useful here
	return QRect();
}

void Vertex::addSegment(Segment* seg)
{
	mSegs.insert(seg);
}

void Vertex::removeSegment(Segment* seg)
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
	foreach(Vertex* vtx, myVtx)
	{
		if (vtx->onLayer(layer)  && a.pointInside(vtx->pos()))
			set.insert(vtx);
	}
	return set;
}

void TraceList::addSegment(Segment *s, Vertex *v1, Vertex *v2)
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

void TraceList::removeSegment(Segment *s)
{
	if (!mySeg.remove(s))
		return;

	s->v1()->removeSegment(s);
	s->v2()->removeSegment(s);

	if (s->v1()->segments().count() == 0)
	{
		myVtx.remove(s->v1());
		delete s->v1();
	}
	if (s->v2()->segments().count() == 0)
	{
		myVtx.remove(s->v2());
		delete s->v2();
	}
	delete s;
	mIsDirty = true;
}

void TraceList::swapVtx(Segment *s, Vertex *vOld, Vertex *vNew)
{
	Q_ASSERT(s && vOld && vNew && (s->v1() == vOld || s->v2() == vOld));
	vOld->removeSegment(s);
	if (vOld->segments().count() == 0)
	{
		myVtx.remove(vOld);
		delete vOld;
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
	QSet<Vertex*> toVisit(myVtx);
	QSet<Vertex*> currSet;
	this->mConnections.clear();
	while(!toVisit.empty())
	{
		currSet.clear();
		Vertex* vtx = *toVisit.begin();
		toVisit.remove(vtx);
		vertexDFS(toVisit, currSet, vtx);
		mConnections.append(currSet);
	}
}

void vertexDFS(QSet<Vertex*> &toVisit, QSet<Vertex*> &currSet, Vertex *currVtx)
{
	foreach(Segment* seg, currVtx->segments())
	{
		Vertex* vtx = seg->otherVertex(currVtx);
		if (toVisit.contains(vtx))
		{
			toVisit.remove(vtx);
			currSet.insert(vtx);
			vertexDFS(toVisit, currSet, vtx);
		}

	}
}

void TraceList::clear()
{
	foreach(Segment* seg, mySeg)
	{
		delete seg;
	}
	mySeg.clear();
	foreach(Vertex* vtx, myVtx)
	{
		delete vtx;
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
	QHash<int, Vertex*> vmap;
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

		Vertex* v = new Vertex(pt, forcevia);
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

		Segment* s = new Segment(layer, width);
		addSegment(s, vmap.value(start, NULL), vmap.value(end, NULL));
		do
				reader.readNext();
		while(!reader.isEndElement());
	}
}

void TraceList::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("traces");
	writer.writeStartElement("vertices");
	foreach(Vertex* v, myVtx)
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
	foreach(Segment* s, mySeg)
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

Segment* TraceList::segment(Vertex *v1, Vertex *v2) const
{
	foreach(Segment* s, v1->segments())
	{
		if (s->otherVertex(v1) == v2)
			return s;
	}
	return NULL;
}

/////////////////////// SEGMENT ///////////////////////

Segment::Segment(const Layer& l, int w)
	: mLayer(l), mV1(NULL), mV2(NULL) , mWidth(w)
{
}

void Segment::draw(QPainter *painter, const Layer& layer) const
{
	if (!mV1 || !mV2) return;
	if (layer != mLayer && layer != Layer::LAY_SELECTION)
		return;

	QPen pen = painter->pen();
	pen.setWidth(mWidth);
	painter->setPen(pen);

	painter->drawLine(mV1->pos(), mV2->pos());
}

QRect Segment::bbox() const
{
	if (!mV1 || !mV2) return QRect();
	return QRect(mV1->pos(), mV2->pos()).normalized().adjusted(-mWidth/2, -mWidth/2, mWidth/2, mWidth/2);
}

bool Segment::testHit(QPoint p, const Layer &l) const
{
	return bbox().contains(p) && l == layer();
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
	foreach(Segment* seg, this->mSegs)
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
		Segment* first = *this->mSegs.begin();
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


TraceList::AddSegCmd::AddSegCmd(QUndoCommand *parent, TraceList *tl, Segment *s, Vertex *v1, Vertex *v2)
	: QUndoCommand(parent), mTl(tl), mSeg(s), mV1(v1), mV2(v2), mUndone(true)
{
}

TraceList::AddSegCmd::~AddSegCmd()
{
	if (mUndone)
	{
		// only delete the objects that are not referenced by anything else
		if (mV1->segments().count() == 0)
			delete mV1;
		if (mV2->segments().count() == 0)
			delete mV2;
		delete mSeg;
	}
	// don't delete anything if the command hasn't been undone
}

void TraceList::AddSegCmd::undo()
{
	if (mUndone)
		return;
	mTl->mySeg.remove(mSeg);
	mV1->removeSegment(mSeg);
	mV2->removeSegment(mSeg);

	if (mV1->segments().count() == 0)
		mTl->myVtx.remove(mV1);
	if (mV2->segments().count() == 0)
		mTl->myVtx.remove(mV2);
	mUndone = true;
}

void TraceList::AddSegCmd::redo()
{
	if (!mUndone)
		return;
	mTl->addSegment(mSeg, mV1, mV2);
	mUndone = false;
}


TraceList::DelSegCmd::DelSegCmd(QUndoCommand *parent, TraceList *tl, Segment *s)
	: QUndoCommand(parent), mTl(tl), mSeg(s), mV1(s->v1()), mV2(s->v2()), mUndone(true)
{
}

TraceList::DelSegCmd::~DelSegCmd()
{
	if (!mUndone)
	{
		// only delete the objects that are not referenced by anything else
		if (mV1->segments().count() == 0)
			delete mV1;
		if (mV2->segments().count() == 0)
			delete mV2;
		delete mSeg;
	}
	// don't delete anything if the command has been undone
}

void TraceList::DelSegCmd::undo()
{
	mTl->addSegment(mSeg, mV1, mV2);
	mUndone = true;
}

void TraceList::DelSegCmd::redo()
{
	mTl->mySeg.remove(mSeg);
	mV1->removeSegment(mSeg);
	mV2->removeSegment(mSeg);

	if (mV1->segments().count() == 0)
		mTl->myVtx.remove(mV1);
	if (mV2->segments().count() == 0)
		mTl->myVtx.remove(mV2);
	mUndone = false;
}

/////////////////

TraceList::SwapVtxCmd::SwapVtxCmd(QUndoCommand *parent, TraceList *tl, Segment *s, Vertex* vOld, Vertex* vNew)
	: QUndoCommand(parent), mTl(tl), mSeg(s), mVOld(vOld), mVNew(vNew), mNewRemoved(!tl->myVtx.contains(vNew)), mOldRemoved(!tl->myVtx.contains(vOld))
{
	Q_ASSERT(mVOld != mVNew);
	Q_ASSERT(s->v1() != mVNew && s->v2() != mVNew);
}

TraceList::SwapVtxCmd::~SwapVtxCmd()
{
	if (mNewRemoved)
	{
		Q_ASSERT(mVNew->segments().count() == 0);
		delete mVNew;
	}
//	if (mOldRemoved)
//	{
//		Q_ASSERT(mVOld->segments().count() == 0);
//	/	delete mVOld;
//	}
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
		mNewRemoved = true;
	}
	mTl->myVtx.insert(mVOld);
	mOldRemoved = false;
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
		mOldRemoved = true;
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
	mNewRemoved = false;
	mTl->myVtx.insert(mVNew);

	Q_ASSERT(mSeg->v1()->segments().contains(mSeg));
	Q_ASSERT(mSeg->v2()->segments().contains(mSeg));
	Q_ASSERT((mSeg->v1() == mVNew && mSeg->v2() != mVNew) || (mSeg->v1() != mVNew && mSeg->v2() == mVNew));

}

