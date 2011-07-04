
class PartPin : PCBObject
{
%TypeHeaderCode
#include "Part.h"
#include "Net.h"
#include "Trace.h"
%End
public:
	PartPin(Part* parent, QSharedPointer<const Pin> pin);
	~PartPin();

	Pad getPadOnLayer(const Layer& layer) const;

	void setNet(QSharedPointer<Net> newnet);
	QSharedPointer<Net> net() const;

	void setVertex(QSharedPointer<Vertex> vertex);
	QSharedPointer<Vertex> vertex() const;
	Part* part() const;
	QString name() const;
	QSharedPointer<Pin> fpPin() const;

	QPoint pos() const;
	bool isSmt() const;

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	bool testHit(const QPoint &pt, const Layer& layer) const;
};

class Part : PCBObject
{
%TypeHeaderCode
#include "Part.h"
#include "Net.h"
#include "Trace.h"
%End
public:
	enum SIDE { SIDE_TOP = 0, SIDE_BOTTOM };

	Part(PCBDoc* doc);
	~Part();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	QString refdes() const; 
	QString value() const;
	QSharedPointer<Text> refdesText() const;
	bool refVisible() const; 
	QSharedPointer<Text> valueText() const;
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

	QList<QSharedPointer<PartPin> > pins() const;
	QSharedPointer<PartPin> pin(const QString &name);

	static QSharedPointer<Part> newFromXML(QXmlStreamReader &reader, PCBDoc* doc);
	void toXML(QXmlStreamWriter &writer) const;

	QTransform transform() const; 
	bool testHit(QPoint pt, const Layer& /*layer*/) const;

	PCBDoc* doc() const;
};

