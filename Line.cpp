#include "Line.h"
#include <QXmlStreamReader>

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

	return l;
}

void Line::draw(QPainter *painter, PCBLAYER layer)
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
			attr.value("x2").toString().toInt(),
			attr.value("y2").toString().toInt());
	a.mIsCw = attr.value("dir") == "cw";

	return a;
}

void Arc::draw(QPainter *painter, PCBLAYER layer)
{
	// XXX TODO draw via if needed
}

QRect Arc::bbox() const
{
	// XXX TODO do something useful here
	return QRect();
}
