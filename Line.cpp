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
	// XXX TODO do something useful
}

QRect Line::bbox() const
{
	// XXX TODO do something useful here
	return QRect();
}

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
	a.mCtr = QPoint(
			attr.value("ctrX").toString().toInt(),
			attr.value("ctrY").toString().toInt());
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
	writer.writeAttribute("ctrX", QString::number(mCtr.x()));
	writer.writeAttribute("ctrY", QString::number(mCtr.y()));
	writer.writeAttribute("dir", mIsCw ? "cw" : "ccw");
	writer.writeEndElement();

}

void Arc::draw(QPainter *painter, PCBLAYER layer) const
{
	// XXX TODO draw via if needed
}

QRect Arc::bbox() const
{
	// XXX TODO do something useful here
	return QRect();
}
