#include "Line.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

Line::Line()
	: mWidth(0), mLayer(LAY_UNKNOWN)
{
}

Line Line::newFromXml(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "line");

	QXmlStreamAttributes attr = reader.attributes();

	Line l;
	l.mWidth = attr.value("width").toString().toInt();
	l.mLayer = (PCBLAYER) attr.value("layer").toString().toInt();
	l.mStart = QPoint(
			attr.value("x1").toString().toInt(),
			attr.value("y1").toString().toInt());
	l.mEnd = QPoint(
			attr.value("x2").toString().toInt(),
			attr.value("y2").toString().toInt());

	do
			reader.readNext();
	while(!reader.isEndElement());

	return l;
}

void Line::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("line");
	writer.writeAttribute("width", QString::number(mWidth));
	writer.writeAttribute("layer", QString::number(mLayer));
	writer.writeAttribute("x1", QString::number(mStart.x()));
	writer.writeAttribute("y1", QString::number(mStart.y()));
	writer.writeAttribute("x2", QString::number(mEnd.x()));
	writer.writeAttribute("y2", QString::number(mEnd.y()));
	writer.writeEndElement();

}

void Line::draw(QPainter *painter, PCBLAYER layer) const
{
	if (layer != mLayer && layer != LAY_SELECTION)
		return;

	QPen pen = painter->pen();
	pen.setWidth(mWidth);
	painter->setPen(pen);

	painter->drawLine(mStart, mEnd);
}

QRect Line::bbox() const
{
	return QRect(mStart, mEnd).adjusted(-mWidth, -mWidth, mWidth, mWidth);
}

///////////////////////// ARC /////////////////////////////////////////

Arc::Arc()
	: mIsCw(false), mWidth(0), mLayer(LAY_UNKNOWN)
{
}

Arc Arc::newFromXml(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "arc");

	QXmlStreamAttributes attr = reader.attributes();

	Arc a;
	a.mWidth = attr.value("width").toString().toInt();
	a.mLayer = (PCBLAYER) attr.value("layer").toString().toInt();
	a.mStart = QPoint(
			attr.value("x1").toString().toInt(),
			attr.value("y1").toString().toInt());
	a.mEnd = QPoint(
			attr.value("x2").toString().toInt(),
			attr.value("y2").toString().toInt());
	a.mIsCw = attr.value("dir") == "cw";

	do
			reader.readNext();
	while(!reader.isEndElement());

	return a;
}

void Arc::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("arc");
	writer.writeAttribute("width", QString::number(mWidth));
	writer.writeAttribute("layer", QString::number(mLayer));
	writer.writeAttribute("x1", QString::number(mStart.x()));
	writer.writeAttribute("y1", QString::number(mStart.y()));
	writer.writeAttribute("x2", QString::number(mEnd.x()));
	writer.writeAttribute("y2", QString::number(mEnd.y()));
	writer.writeAttribute("dir", mIsCw ? "cw" : "ccw");
	writer.writeEndElement();
}

void Arc::draw(QPainter *painter, PCBLAYER layer) const
{
	if (layer != mLayer && layer != LAY_SELECTION) return;
	int x1, x2, y1, y2;
	// make x/y always be clockwise values
	if (mIsCw)
	{
		x1 = mStart.x();
		y1 = mStart.y();
		x2 = mEnd.x();
		y2 = mEnd.y();
	}
	else
	{
		x2 = mStart.x();
		y2 = mStart.y();
		x1 = mEnd.x();
		y1 = mEnd.y();
	}
	QRect r;
	int startAngle;

	// figure out quadrant
	if (x1 < x2 && y1 > y2)
	{
		// quadrant 1
		r = QRect(2*x1-x2, 2*y2-y1, 2*(x2-x1), 2*(y1-y2));
		startAngle = 270*16;
	}
	else if (x1 < x2 && y1 < y2)
	{
		// quadrant 2
		r = QRect(x1, 2*y1-y2, 2*(x2-x1), 2*(y2-y1));
		startAngle = 180*16;
	}
	else if (x1 > x2 && y1 < y2)
	{
		// quadrant 3
		r = QRect(x2, y1, 2*(x1-x2), 2*(y2-y1));
		startAngle = 90*16;
	}
	else // x1 > x2, y1 > y2
	{
		// quadrant 4
		r = QRect(2*x2-x1, y2, 2*(x1-x2), 2*(y1-y2));
		startAngle = 0;
	}
	QPen pen = painter->pen();
	pen.setWidth(mWidth);
	painter->setPen(pen);
	painter->drawArc(r, startAngle, 90*16);
}

QRect Arc::bbox() const
{
	return QRect(mStart, mEnd).adjusted(-mWidth, -mWidth, mWidth, mWidth);
}
