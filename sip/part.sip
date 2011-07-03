
class PartPin : PCBObject
{
%TypeHeaderCode
#include "Part.h"
%End
public:
	PartPin(Part* parent, const Pin* pin);
	~PartPin();

	Pad getPadOnLayer(const Layer& layer) const;

	void setNet(Net* newnet);
	Net* net() const;

//	void setVertex(Vertex* vertex);
//	Vertex* vertex() const;
	Part* part() const;
	QString name() const;
	const Pin* fpPin() const;

	QPoint pos() const;
	bool isSmt() const;

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v);
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	bool testHit(const QPoint &pt, const Layer& layer) const;
};

class Part : PCBObject
{
%TypeHeaderCode
#include "Part.h"
%End
public:
	enum SIDE { SIDE_TOP = 0, SIDE_BOTTOM };

	Part(PCBDoc* doc);
	~Part();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v);
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	QString refdes() const; 
	QString value() const;
	Text* refdesText() const; 
	bool refVisible() const; 
	Text* valueText() const; 
	bool valueVisible() const; 
	QPoint pos() const; 
	int angle() const;
	SIDE side() const; 
	bool locked() const; 

	void setPos(QPoint pos);
	void setAngle(int angle);
	void setSide(SIDE side);
	void setLocked(bool locked); 
	void setRefVisible(bool vis); 
	void setValueVisible(bool vis);

	QSharedPointer<Footprint> footprint() const /Factory/;
	const QUuid& fpUuid() const;
	void setFootprint(QUuid uuid);

	QList<PartPin*> pins() const; 
	PartPin* getPin(const QString &name);

	static Part* newFromXML(QXmlStreamReader &reader, PCBDoc* doc);
	void toXML(QXmlStreamWriter &writer) const;

	QTransform transform() const; 
	bool testHit(QPoint pt, const Layer& /*layer*/) const;

	PCBDoc* doc() const;
};

