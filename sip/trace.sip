class Vertex : PCBObject
{
%TypeHeaderCode
#include "Trace.h"
%End
public:
	Vertex(QPoint pos = QPoint(0, 0), bool forcevia = false);
	~Vertex();
	%MethodCode
		// check if there is an outstanding shared pointer
		if (SharedPtrMgr::hasRef(sipCppV))
		{
			// decrement the reference count and get out
			SharedPtrMgr::decRef(sipCppV);
			return;
		}
		// otherwise let python deal with it
	%End

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState& state);

	QPoint pos() const;
	void setPos(QPoint pos);

	void addSegment(QSharedPointer<Segment> seg);
	void removeSegment(QSharedPointer<Segment> seg);
	const QList<QSharedPointer<Segment> > segments() const;
	bool onLayer(const Layer& layer) const;
	bool isVia() const;

	bool isForcedVia() const;

	QSharedPointer<Padstack> padstack() const;
	void clear();

};

class Segment : PCBObject
{
%TypeHeaderCode
#include "Trace.h"
%End
public:
	Segment(const Layer& layer, int w = 0);
	~Segment();
	%MethodCode
		// check if there is an outstanding shared pointer
		if (SharedPtrMgr::hasRef(sipCppV))
		{
			// decrement the reference count and get out
			SharedPtrMgr::decRef(sipCppV);
			return;
		}
		// otherwise let python deal with it
	%End

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual bool testHit(QPoint p, const Layer &l) const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState& state);

	int width() const;
	void setWidth(int w);
	const Layer& layer();
	void setLayer(const Layer& layer);

	QSharedPointer<Vertex> otherVertex(QSharedPointer<Vertex> v) const;
	QSharedPointer<Vertex> v1() const;
	QSharedPointer<Vertex> v2() const;
	void setV1(QSharedPointer<Vertex> v /Transfer/);
	void setV2(QSharedPointer<Vertex> v /Transfer/);

	void clear();

	bool operator==(const Segment& other) const;
};

class TraceList
{
%TypeHeaderCode
#include "Trace.h"
%End
public:
	TraceList();
	~TraceList();

	void addSegment(QSharedPointer<Segment> s /Transfer/, QSharedPointer<Vertex> v1 /Transfer/, QSharedPointer<Vertex> v2 /Transfer/);
	QUndoCommand* addSegmentCmd(QSharedPointer<Segment> s /Transfer/, QSharedPointer<Vertex> v1 /Transfer/, QSharedPointer<Vertex> v2 /Transfer/, QUndoCommand* parent /TransferThis/ = NULL) /Factory/;

	void removeSegment(QSharedPointer<Segment> s);
	QUndoCommand* removeSegmentCmd(QSharedPointer<Segment> s, QUndoCommand* parent /TransferThis/ = NULL) /Factory/;

	void swapVtx(QSharedPointer<Segment> s, QSharedPointer<Vertex> vOld, QSharedPointer<Vertex> vNew);
	QUndoCommand* swapVtxCmd(QSharedPointer<Segment> s, QSharedPointer<Vertex> vOld, QSharedPointer<Vertex> vNew, QUndoCommand* parent /TransferThis/ = NULL) /Factory/;

	QSharedPointer<Segment> segment(QSharedPointer<Vertex> v1, QSharedPointer<Vertex> v2) const;

	QSet<Vertex*> getConnectedVertices(Vertex* vtx) const;
//	QSet<Vertex*> getVerticesInArea(const Area& poly) const;

	QList<QSharedPointer<Segment> > segments() const;
	QList<QSharedPointer<Vertex> > vertices() const;
	void loadFromXml(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;
};

