#ifndef LINE_H
#define LINE_H

#include "PCBObject.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class Line : public PCBObject
{
public:
    Line();

	virtual void draw(QPainter *painter, XPcb::PCBLAYER layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	QPoint start() const { return mStart; }
	QPoint end() const { return mEnd; }
	int width() const { return mWidth; }
	XPcb::PCBLAYER layer() const { return mLayer; }

	static Line newFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
private:
	QPoint mStart;
	QPoint mEnd;
	int mWidth;
	XPcb::PCBLAYER mLayer;
};

/// The Arc class represents silkscreen arcs.  Arcs are elliptical sections
/// that always have a 90 degree sweep.  An arc is defined by its start and
/// end points and an orientation flag (clockwise/counterclockwise).
class Arc : public PCBObject
{
public:
	Arc();

	virtual void draw(QPainter *painter, XPcb::PCBLAYER layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	QPoint start() const { return mStart; }
	QPoint end() const { return mEnd; }
	int width() const { return mWidth; }
	bool isCw() const { return mIsCw; }
	XPcb::PCBLAYER layer() const { return mLayer; }

	static Arc newFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
private:
	QPoint mStart;
	QPoint mEnd;
	bool mIsCw;
	int mWidth;
	XPcb::PCBLAYER mLayer;
};

#endif // LINE_H
