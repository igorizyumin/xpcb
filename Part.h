#ifndef PART_H
#define PART_H

#include "Shape.h"
#include "PCBObject.h"
#include "Text.h"

// forward declarations
class Net;
class Vertex;
class PCBDoc;
class Part;

/// A PartPin represents an instance of a footprint Pin
/// that is associated with a specific part.  PartPins store
/// net attachment information.  Drawing/editing is delegated to the
/// footprint pin.
class PartPin : public PCBObject
{
public:
	PartPin(Part* parent, const Pin* pin) : mPin(pin), mPart(parent), mNet(NULL), mVertex(NULL) {}
	~PartPin();

	Pad getPadOnLayer(PCBLAYER layer) const;

	void setNet(Net* newnet);
	Net* getNet() {return mNet; }

	void setVertex(Vertex* vertex);
	Part* getPart() { return mPart; }
	QString getName() {return mPin->name(); }
	const Pin* fpPin() { return mPin; }

	QPoint pos() const;
	bool isSmt() const;

	virtual void draw(QPainter *painter, PCBLAYER layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	bool testHit(const QPoint &pt, PCBLAYER layer) const;

private:
	/// Maps a PCB layer to a pin layer (i.e. top copper -> start for parts on top side)
	Padstack::PSLAYER mapLayer(PCBLAYER layer) const;

	/// Pointer to footprint pin (contains position, etc.)
	const Pin * mPin;
	/// Pointer to parent part.
	Part * mPart;
	/// Pointer to assigned net.  NULL if no assigned net.
	Net * mNet;
	/// Pointer to attached vertex. May be NULL.
	Vertex* mVertex;
};

/// A part is an instance of a Footprint on a printed circuit board.  Parts
/// have an associated position and rotation.  Child elements include part pins,
/// and reference/value texts.
class Part : public PCBObject
{
public:
	Part(PCBDoc* doc);
	~Part();

	virtual void draw(QPainter *painter, PCBLAYER layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	QString refdes() const { return mRefdes->text(); }
	QString value() const { return mValue->text(); }
	Text* refdesText() const { return mRefdes; }
	bool refVisible() const { return mRefVisible; }
	Text* valueText() const { return mValue; }
	bool valueVisible() const { return mValueVisible; }
	QPoint pos() const { return mPos; }
	int angle() { return mAngle; }
	PCBSIDE side() { return mSide; }
	bool locked() { return mLocked; }

	void setPos(QPoint pos) { mPos = pos; updateTransform(); }
	void setAngle(int angle) { mAngle = angle; updateTransform(); }
	void setSide(PCBSIDE side);
	void setLocked(bool locked) { mLocked = locked; }
	bool setRefVisible(bool vis) { mRefVisible = vis; }
	bool setValueVisible(bool vis) { mValueVisible = vis; }

	Footprint* footprint() { return mFp;}
	void setFootprint( Footprint * fp );

	PartPin* getPin(const QString &name);

	static Part* newFromXML(QXmlStreamReader &reader, PCBDoc* doc);
	void toXML(QXmlStreamWriter &writer) const;

	QTransform transform() const { return mTransform; }
	bool testHit(QPoint pt, PCBLAYER l) const { return bbox().contains(pt); }

private:
	void resetFp();
	void updateTransform();

	/// Coordinate transform from part to world coords
	QTransform mTransform;

	/// Part position on board.  This is the origin of the part's coordinate system.
	QPoint mPos;
	/// Rotation angle (clockwise).
	int mAngle;
	/// PCB side on which the part is located.
	PCBSIDE mSide;
	/// Locked parts cannot be moved.
	bool mLocked;
	/// Reference designator text
	Text* mRefdes;
	bool mRefVisible;
	/// Value text
	Text* mValue;
	bool mValueVisible;
	/// Pointer to the footprint of the part, may be NULL
	Footprint * mFp;
	/// List of part pins.
	QList<PartPin*> mPins;
	/// Parent document
	PCBDoc* mDoc;
};

#endif // PART_H
