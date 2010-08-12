#ifndef PART_H
#define PART_H

#include "Shape.h"
#include "PCBObject.h"
#include "Text.h"

// forward declarations
class Net;
class PartList;
class Vertex;
class PCBDoc;

/// A PartPin represents an instance of a footprint Pin
/// that is associated with a specific part.  PartPins store
/// net attachment information.  Drawing/editing is delegated to the
/// footprint pin.
class PartPin : public PCBObject
{
	PartPin(Part* parent, Pin* pin) : mPart(parent), mPin(pin) {}
	~PartPin();
	void setNet(Net* newnet);
	PCBLAYER getLayer();

	Net* getNet() {return net; }
	bool testHit( QPoint pt, PCBLAYER layer );
	void setVertex(Vertex* vertex);
	Part* getPart() { return part; }
	QString getName() {return mPin->getName(); }
	bool getPadOnLayer(PCBLAYER layer, Pad &pad);

private:
	/// Pointer to footprint pin (contains position, etc.)
	Pin * mPin;
	/// Pointer to assigned net.  NULL if no assigned net.
	Net * mNet;
	/// Pointer to parent part.
	Part * mPart;
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

	Footprint* getFootprint() { return mFp;}

	void setFootprint( Footprint * fp );

	PCBSIDE getSide() { return mSide; }

	QPoint getCentroidPoint() { return mPos; }
	int getAngle() { return mAngle; }

	static Part* newFromXML(QXmlStreamReader &reader, PCBDoc* doc);

private:
	void resetFp();
	QPoint partToWorld(QPoint pt);

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
	/// Value text
	Text* mValue;
	/// Pointer to the footprint of the part, may be NULL
	Footprint * mFp;
	/// List of part pins.
	QList<PartPin*> mPins;
	/// Parent document
	PCBDoc* mDoc;

};

#endif // PART_H
