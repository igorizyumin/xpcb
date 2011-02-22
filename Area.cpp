#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "Area.h"
#include "Net.h"
#include "Part.h"
#include "global.h"
#include "PCBDoc.h"
#include "Polygon.h"

Area::Area(const PCBDoc *doc) :
		mDoc(doc), mNet(NULL), mConnectSMT(true),
		mPoly(NULL), mHatchStyle(NO_HATCH)
{
}

Area::~Area()
{
	delete mPoly;
}

void Area::draw(QPainter *painter, const Layer& layer) const
{
	// XXX TODO draw via if needed
}

QRect Area::bbox() const
{
	// XXX TODO do something useful here
	return QRect();
}

void Area::findConnections()
{
	if (!this->mNet || !this->mPoly || !this->mDoc)
		return;

	this->mConnPins.clear();

	Layer layer = this->layer();
	QSet<PartPin*> pins = mNet->getPins();

	foreach(PartPin* pin, pins)
	{
		// see if pin is on the right layer
		Pad pad = pin->getPadOnLayer(layer);

		if (pad.isNull())
			continue; // no pad on this layer

		if( pad.connFlag() == Pad::PAD_CONNECT_NEVER )
			continue;	// pad never allowed to connect

		if( pad.connFlag() == Pad::PAD_CONNECT_DEFAULT
			&& pin->isSmt() && !mConnectSMT )
			continue;	// SMT pad, not allowed to connect to this area

		// add to list if pad is inside copper area
		if( mPoly->testPointInside(pin->pos()) )
			this->mConnPins.insert(pin);
	}

	// find all vertices within copper area
	TraceList &tl = this->mDoc->traceList();
	mConnVtx = tl.getVerticesInArea(*this);
}

bool Area::pointInside(const QPoint &p) const
{
	if (!mPoly) return false;
	return mPoly->testPointInside(p);
}

Area* Area::newFromXML(QXmlStreamReader &reader, const PCBDoc &doc)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "area");

	QXmlStreamAttributes attr = reader.attributes();
	Area* a = new Area(&doc);
	a->mNet = doc.getNet(attr.value("net").toString());
	a->mLayer = Layer(attr.value("layer").toString().toInt());
	QStringRef t = attr.value("hatch");
	if (t == "none")
		a->mHatchStyle = Area::NO_HATCH;
	else if (t == "full")
		a->mHatchStyle = Area::DIAGONAL_FULL;
	else if (t == "edge")
		a->mHatchStyle = Area::DIAGONAL_EDGE;
	a->mConnectSMT = (attr.value("connectSmt") == "1");
	reader.readNextStartElement();
	a->mPoly = Polygon::newFromXML(reader);

	return a;
}

void Area::toXML(QXmlStreamWriter &writer)
{
	writer.writeStartElement("area");
	writer.writeAttribute("net", mNet->name());
	writer.writeAttribute("layer", QString::number(mLayer.toInt()));
	switch(mHatchStyle)
	{
	case Area::NO_HATCH:
		writer.writeAttribute("hatch", "none");
		break;
	case Area::DIAGONAL_FULL:
		writer.writeAttribute("hatch", "full");
		break;
	case Area::DIAGONAL_EDGE:
		writer.writeAttribute("hatch", "edge");
		break;
	}
	writer.writeAttribute("connectSmt", mConnectSMT ? "1" : "0");
	mPoly->toXML(writer);
	writer.writeEndElement();
}
