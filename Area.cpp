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
#include "Area.h"
#include "Net.h"
#include "Part.h"
#include "global.h"
#include "Document.h"
#include "Polygon.h"

Area::Area(const PCBDoc *doc) :
		mDoc(doc), mConnectSMT(true),
		mPoly(NULL), mHatchStyle(NO_HATCH)
{
}

Area::~Area()
{
}

void Area::draw(QPainter */*painter*/, const Layer& /*layer*/) const
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
	// XXX TODO fix this
#if 0
	if (!this->mDoc)
		return;

	this->mConnPins.clear();

	Layer layer = this->layer();
	QList<QSharedPointer<PartPin> > pins = net->pins();

	foreach(QSharedPointer<PartPin> pin, pins)
	{
		// see if pin is on the right layer
		Pad pad = pin->getPadOnLayer(layer);

		if (pad.isNull())
			continue; // no pad on this layer

		if( pad.connFlag() == Pad::CONN_NEVER )
			continue;	// pad never allowed to connect

		if( pad.connFlag() == Pad::CONN_DEFAULT
			&& pin->isSmt() && !mConnectSMT )
			continue;	// SMT pad, not allowed to connect to this area

		// add to list if pad is inside copper area
		if( mPoly.testPointInside(pin->pos()) )
			this->mConnPins.append(pin.toWeakRef());
	}

	// find all vertices within copper area
	TraceList &tl = this->mDoc->traceList();
	mConnVtx = tl.getVerticesInArea(*this);
#endif
}

bool Area::pointInside(const QPoint &p) const
{
	return mPoly.testPointInside(p);
}

QSharedPointer<Area> Area::newFromXML(QXmlStreamReader &reader, const PCBDoc &doc)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "area");

	QXmlStreamAttributes attr = reader.attributes();
	QSharedPointer<Area> a(new Area(&doc));
	a->mNet = attr.value("net").toString();
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
	if (!mNet.isEmpty())
		writer.writeAttribute("net", mNet);
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
	mPoly.toXML(writer);
	writer.writeEndElement();
}
