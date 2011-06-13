#ifndef LINE_H
#define LINE_H

#include "PCBObject.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class LineState;

/// The Line class represents silkscreen line segments and arcs.
/// Lines are defined by a start and an end point.  Arcs are elliptical sections
/// that always have a 90 degree sweep.  An arc is defined by its start and
/// end points and an orientation flag (clockwise/counterclockwise).
class Line : public PCBObject
{
public:
	enum LineType { LINE, ARC_CW, ARC_CCW };

    Line();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);
	virtual bool testHit(QPoint p, const Layer &l) const { return (l == mLayer) && bbox().contains(p); }

	QPoint start() const { return mStart; }
	void setStart(QPoint p) { mStart = p; }
	QPoint end() const { return mEnd; }
	void setEnd(QPoint p) { mEnd = p; }
	int width() const { return mWidth; }
	void setWidth(int width) { mWidth = width; }
	const Layer& layer() const { return mLayer; }
	void setLayer(const Layer& l) { mLayer = l; }
	LineType type() const { return mType; }
	void setType(LineType t) { mType = t; }

	static Line* newFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	static void drawArc(QPainter* painter, QPoint start, QPoint end, LineType type);

private:
	class LineState : public PCBObjStateInternal
	{
	private:
		friend class Line;
		LineState(const Line &l)
			: start(l.start()), end(l.end()), width(l.width()),
			layer(l.layer()), type(l.type())
		{}

		QPoint start, end;
		int width;
		Layer layer;
		Line::LineType type;
	};

	QPoint mStart;
	QPoint mEnd;
	int mWidth;
	Layer mLayer;
	LineType mType;
};

#endif // LINE_H
