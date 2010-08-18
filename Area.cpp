#include <QXmlStreamReader>
#include "Area.h"
#include "Net.h"
#include "Part.h"
#include "global.h"
#include "PCBDoc.h"
#include "Polygon.h"

Area::Area(PCBDoc *doc) :
		mDoc(doc), mNet(NULL), mConnectSMT(true),
		mPoly(NULL), mLayer(LAY_UNKNOWN), mHatchStyle(NO_HATCH)
{
}

Area::~Area()
{
	delete mPoly;
}

void Area::findConnections()
{
	if (!this->mNet || !this->mPoly || !this->mDoc)
		return;

	this->mConnPins.clear();

	PCBLAYER layer = this->layer();
	QSet<PartPin*> pins = mNet->getPins();

	foreach(PartPin* pin, pins)
	{
		// see if pin is on the right layer
		PCBLAYER pin_layer = pin->getLayer();
		bool isSMT = (pin_layer == LAY_PAD_THRU);

		if( isSMT && pin_layer != layer )
			continue;	// SMT pad not on our layer

		// get the pad for the appropriate layer
		Pad pad;
		if (!pin->getPadOnLayer(layer, pad))
			continue; // no pad on this layer

		if( pad.connFlag() == PAD_CONNECT_NEVER )
			continue;	// pad never allowed to connect

		if( pad.connFlag() == PAD_CONNECT_DEFAULT
			&& isSMT && !mConnectSMT )
			continue;	// SMT pad, not allowed to connect to this area

		// add to list if pad is inside copper area
		if( mPoly->testPointInside(pin->getPos()) )
			this->mConnPins.append(pin);
	}

	// find all vertices within copper area
	TraceList &tl = this->mDoc->traceList();
	mConnVtx = tl.getVerticesInArea(*this);
}

bool Area::pointInside(const QPoint &p)
{
	if (!mPoly) return false;
	return mPoly->testPointInside(p);
}

Area* Area::newFromXML(QXmlStreamReader &reader, PCBDoc &doc)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "area");

	QXmlStreamAttributes attr = reader.attributes();
	Area* a = new Area(&doc);
	a->mNet = doc.getNet(attr.value("net"));
	a->mLayer = (PCBLAYER) attr.value("layer").toString().toInt();
	switch(attr.value("hatch"))
	{
	case "none":
		a->mHatchStyle = Area::NO_HATCH;
		break;
	case "full":
		a->mHatchStyle = Area::DIAGONAL_FULL;
		break;
	case "edge":
		a->mHatchStyle = Area::DIAGONAL_EDGE;
		break;
	}
	a->mConnectSMT = (attr.value("connectSmt") == "1");
	reader.readNextStartElement();
	a->mPoly = Polygon::newFromXML(reader);

	return a;
}
