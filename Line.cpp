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

#include "Line.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

Line::Line()
	: mWidth(0), mType(LINE)
{
}

QSharedPointer<Line> Line::newFromXml(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && (reader.name() == "line" || reader.name() == "arc"));

	QXmlStreamAttributes attr = reader.attributes();

	QSharedPointer<Line> l(new Line());
	l->mWidth = attr.value("width").toString().toInt();
	l->mLayer = Layer(attr.value("layer").toString().toInt());
	l->mStart = QPoint(
			attr.value("x1").toString().toInt(),
			attr.value("y1").toString().toInt());
	l->mEnd = QPoint(
			attr.value("x2").toString().toInt(),
			attr.value("y2").toString().toInt());
	if (attr.hasAttribute("dir"))
	{
		l->mType = (attr.value("dir") == "cw") ? ARC_CW : ARC_CCW;
	}
	else
		l->mType = LINE;


	do
			reader.readNext();
	while(!reader.isEndElement());

	return l;
}

void Line::toXML(QXmlStreamWriter &writer) const
{
	if (mType == LINE)
		writer.writeStartElement("line");
	else
		writer.writeStartElement("arc");
	writer.writeAttribute("width", QString::number(mWidth));
	writer.writeAttribute("layer", QString::number(mLayer.toInt()));
	writer.writeAttribute("x1", QString::number(mStart.x()));
	writer.writeAttribute("y1", QString::number(mStart.y()));
	writer.writeAttribute("x2", QString::number(mEnd.x()));
	writer.writeAttribute("y2", QString::number(mEnd.y()));
	if (mType != LINE)
		writer.writeAttribute("dir", mType == ARC_CW ? "cw" : "ccw");
	writer.writeEndElement();

}

void Line::draw(QPainter *painter, const Layer& layer) const
{
	if (layer != mLayer && layer != Layer::LAY_SELECTION)
		return;

	QPen pen = painter->pen();
	pen.setWidth(mWidth);
	painter->setPen(pen);

	if (mType == LINE || mStart.x() == mEnd.x() || mStart.y() == mEnd.y())
		painter->drawLine(mStart, mEnd);
	else
		XPcb::drawArc(painter, mStart, mEnd, mType == ARC_CW);
}

QRect Line::bbox() const
{
	return QRect(mStart, mEnd).normalized().adjusted(-mWidth/2, -mWidth/2, mWidth/2, mWidth/2);
}

bool Line::testHit(QPoint p, int dist, const Layer &l) const
{
	return (l == mLayer) &&
			(XPcb::distPtToSegment(p, mStart, mEnd) <= (dist + mWidth));
}



PCBObjState Line::getState() const
{
	return PCBObjState(new LineState(*this));
}

bool Line::loadState(PCBObjState &state)
{
	// convert to line state
	if (state.ptr().isNull())
		return false;
	QSharedPointer<LineState> ls = state.ptr().dynamicCast<LineState>();
	if (ls.isNull())
		return false;
	// restore state
	mStart = ls->start;
	mEnd = ls->end;
	mWidth = ls->width;
	mLayer = ls->layer;
	mType = ls->type;
	return true;
}
