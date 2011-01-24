#ifndef LINE_H
#define LINE_H

#include "PCBObject.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class Line : public PCBObject
{
public:
    Line();

	virtual void draw(QPainter *painter, PCBLAYER layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	QPoint start() const { return mStart; }
	QPoint end() const { return mEnd; }
	int width() const { return mWidth; }
	PCBLAYER layer() const { return mLayer; }

	static Line newFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
private:
	QPoint mStart;
	QPoint mEnd;
	int mWidth;
	PCBLAYER mLayer;
};

class Arc : public PCBObject
{
public:
	Arc();

	virtual void draw(QPainter *painter, PCBLAYER layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	QPoint start() const { return mStart; }
	QPoint end() const { return mEnd; }
	QPoint ctr() const { return mCtr; }
	int width() const { return mWidth; }
	bool isCw() const { return mIsCw; }
	PCBLAYER layer() const { return mLayer; }

	static Arc newFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
private:
	QPoint mStart;
	QPoint mEnd;
	QPoint mCtr;
	bool mIsCw;
	int mWidth;
	PCBLAYER mLayer;
};

#endif // LINE_H
