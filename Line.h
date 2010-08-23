#ifndef LINE_H
#define LINE_H

#include "PCBObject.h"

class QXmlStreamReader;

class Line : public PCBObject
{
public:
    Line();

	virtual void draw(QPainter *painter, PCBLAYER layer);
	virtual QRect bbox() const;

	static Line newFromXml(QXmlStreamReader &reader);
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

	virtual void draw(QPainter *painter, PCBLAYER layer);
	virtual QRect bbox() const;

	static Arc newFromXml(QXmlStreamReader &reader);
private:
	QPoint mStart;
	QPoint mEnd;
	QPoint mCtr;
	bool mIsCw;
	int mWidth;
	PCBLAYER mLayer;
};

#endif // LINE_H
