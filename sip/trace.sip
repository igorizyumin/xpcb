class Vertex : PCBObject
{
%TypeHeaderCode
#include "Trace.h"
%End
public:
	Vertex(QPoint pos = QPoint(0, 0), bool forcevia = false);

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v); 
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState& /*state*/);

	QPoint pos() const;
	void setPos(QPoint pos);

	void addSegment(Segment* seg);
	void removeSegment(Segment* seg);
	const QSet<Segment*> & segments() const;
	bool onLayer(const Layer& layer) const;
	bool isVia() const;

	bool isForcedVia() const;

};

class Segment : PCBObject
{
%TypeHeaderCode
#include "Trace.h"
%End
public:
	Segment(Vertex* v1 /Transfer/, Vertex* v2 /Transfer/, const Layer& layer, int w = 0);
	~Segment();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual bool testHit(QPoint p, const Layer &l) const;
	virtual void accept(PCBObjectVisitor *v); 
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState& /*state*/);

	int width() const;
	void setWidth(int w);
	const Layer& layer();
	void setLayer(const Layer& layer);

	Vertex* v1();
	Vertex* v2();
	Vertex* otherVertex(Vertex* v) const;

	bool operator==(const Segment& other) const;
};

class TraceList
{
%TypeHeaderCode
#include "Trace.h"
%End
public:
	TraceList();

	void addSegment(Segment* s /Transfer/);
	void removeSegment(Segment* s /TransferBack/);

	QSet<Vertex*> getConnectedVertices(Vertex* vtx) const;
//	QSet<Vertex*> getVerticesInArea(const Area& poly) const;

	QSet<Segment*> segments() const;
	QSet<Vertex*> vertices() const;
	void loadFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
};

