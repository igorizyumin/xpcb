#ifndef LINE_H
#define LINE_H

#include "PCBObject.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class Line : public PCBObject
{
public:
    Line();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	QPoint start() const { return mStart; }
	QPoint end() const { return mEnd; }
	int width() const { return mWidth; }
	const Layer& layer() const { return mLayer; }

	static Line newFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
private:
	QPoint mStart;
	QPoint mEnd;
	int mWidth;
	Layer mLayer;
};

/// The Arc class represents silkscreen arcs.  Arcs are elliptical sections
/// that always have a 90 degree sweep.  An arc is defined by its start and
/// end points and an orientation flag (clockwise/counterclockwise).
class Arc : public PCBObject
{
public:
	Arc();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	QPoint start() const { return mStart; }
	QPoint end() const { return mEnd; }
	int width() const { return mWidth; }
	bool isCw() const { return mIsCw; }
	Layer layer() const { return mLayer; }

	static Arc newFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
private:
	QPoint mStart;
	QPoint mEnd;
	bool mIsCw;
	int mWidth;
	Layer mLayer;
};

#endif // LINE_H
