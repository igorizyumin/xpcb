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

void TraceList::addSegment(Segment *s)
{
	mySeg.insert(s);
	myVtx.insert(s->v1());
	myVtx.insert(s->v2());
	mIsDirty = true;
}

void TraceList::removeSegment(Segment *s)
{
	Vertex *v1, *v2;
	if (mySeg.remove(s))
	{
		// Remove vertices if no other segments in the list are referencing them.
		// Note that it is possible for other segments not in the list to still
		// be referencing them, so we check if segments are still present.
		v1 = s->v1();
		v2 = s->v2();
		bool found = false;
		foreach(Segment* s, v1->segments())
		{
			if (mySeg.contains(s))
				found = true;
		}
		if (!found) myVtx.remove(v1);
		found = false;
		foreach(Segment* s, v2->segments())
		{
			if (mySeg.contains(s))
				found = true;
		}
		if (!found) myVtx.remove(v2);
	}
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

		Segment* s = new Segment(vmap.value(start, NULL),
								 vmap.value(end, NULL),
								 layer, width);
		this->mySeg.insert(s);
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

/////////////////////// SEGMENT ///////////////////////

Segment::Segment(Vertex* v1, Vertex* v2, const Layer& l, int w)
	: mLayer(l), mV1(v1), mV2(v2) , mWidth(w)
{
	mV1->addSegment(this);
	mV2->addSegment(this);
}

Segment::~Segment()
{
	mV1->removeSegment(this);
	mV2->removeSegment(this);
	if (mV1->numSegments() == 0)
		delete mV1;
	if (mV2->numSegments() == 0)
		delete mV2;
}

void Segment::draw(QPainter *painter, const Layer& layer) const
{
	if (layer != mLayer && layer != Layer::LAY_SELECTION)
		return;

	QPen pen = painter->pen();
	pen.setWidth(mWidth);
	painter->setPen(pen);

	painter->drawLine(mV1->pos(), mV2->pos());
}

QRect Segment::bbox() const
{
	return QRect(mV1->pos(), mV2->pos()).normalized().adjusted(-mWidth/2, -mWidth/2, mWidth/2, mWidth/2);
}

bool Segment::testHit(QPoint p, const Layer &l) const
{
	return bbox().contains(p) && l == layer();
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
	mSegs = vs->mSegs;
	mPadstack = vs->mPadstack;
	mForceVia = vs->mForceVia;
	return true;
}

