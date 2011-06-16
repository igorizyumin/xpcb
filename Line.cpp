#include "Line.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

Line::Line()
	: mWidth(0), mType(LINE)
{
}

Line* Line::newFromXml(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && (reader.name() == "line" || reader.name() == "arc"));

	QXmlStreamAttributes attr = reader.attributes();

	Line* l = new Line();
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
		drawArc(painter, mStart, mEnd, mType);
}

QRect Line::bbox() const
{
	return QRect(mStart, mEnd).normalized().adjusted(-mWidth/2, -mWidth/2, mWidth/2, mWidth/2);
}

bool Line::testHit(QPoint p, const Layer &l) const
{
	return (l == mLayer) && bbox().contains(p);
}

void Line::drawArc(QPainter* painter, QPoint start, QPoint end, LineType type)
{
	int x1, x2, y1, y2;
	Q_ASSERT(type != LINE);

	// make x/y always be clockwise values
	if (type == ARC_CW)
	{
		x1 = start.x();
		y1 = start.y();
		x2 = end.x();
		y2 = end.y();
	}
	else
	{
		x2 = start.x();
		y2 = start.y();
		x1 = end.x();
		y1 = end.y();
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

	painter->drawArc(r, startAngle, 90*16);
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
