/*
	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PART_H
#define PART_H

#include "Footprint.h"
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
	PartPin(Part* parent, QSharedPointer<Pin> pin) : mPin(pin), mPart(parent) {}
	~PartPin();

	Pad getPadOnLayer(const Layer& layer) const;

	void setNet(QSharedPointer<Net> newnet) { mNet = newnet.toWeakRef(); }
	QSharedPointer<Net> net() const { return mNet.toStrongRef(); }

	void setVertex(QSharedPointer<Vertex> vertex) { mVertex = vertex.toWeakRef(); }
	QSharedPointer<Vertex> vertex() const { return mVertex; }
	Part* part() const { return mPart; }
	QString name() const {return mPin->name(); }
	QSharedPointer<Pin> fpPin() const { return mPin; }

	QPoint pos() const;
	bool isSmt() const;

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	bool testHit(const QPoint &pt, const Layer& layer) const;

private:
	class PartPinState : public PCBObjStateInternal
	{
	public:
		virtual ~PartPinState() {}
	private:
		friend class PartPin;
		PartPinState(const PartPin &p)
			: pin(p.fpPin()), part(p.part()), net(p.net()),
			vertex(p.vertex())
		{}

		QSharedPointer<Pin> pin;
		Part* part;
		QWeakPointer<Net> net;
		QWeakPointer<Vertex> vertex;
	};

	/// Maps a PCB layer to a pin layer (i.e. top copper -> start for parts on top side)
	Layer mapLayer(const Layer& layer) const;

	/// Pointer to footprint pin (contains position, etc.)
	QSharedPointer<Pin> mPin;
	/// Pointer to parent part.
	Part * mPart;
	/// Pointer to assigned net.  NULL if no assigned net.
	QWeakPointer<Net> mNet;
	/// Pointer to attached vertex. May be NULL.
	QWeakPointer<Vertex> mVertex;
};

/// A part is an instance of a Footprint on a printed circuit board.  Parts
/// have an associated position and rotation.  Child elements include part pins,
/// and reference/value texts.
class Part : public PCBObject
{
public:
	enum SIDE { SIDE_TOP = 0, SIDE_BOTTOM };

	Part(PCBDoc* doc);
	~Part();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	QString refdes() const { return mRefdes->text(); }
	QString value() const { return mValue->text(); }
	QSharedPointer<Text> refdesText() const { return mRefdes; }
	bool refVisible() const { return mRefVisible; }
	QSharedPointer<Text> valueText() const { return mValue; }
	bool valueVisible() const { return mValueVisible; }
	QPoint pos() const { return mPos; }
	int angle() const { return mAngle; }
	SIDE side() const { return mSide; }
	bool locked() const { return mLocked; }

	void setPos(QPoint pos) { mPos = pos; updateTransform(); }
	void setAngle(int angle) { mAngle = angle; updateTransform(); }
	void setSide(SIDE side);
	void setLocked(bool locked) { mLocked = locked; }
	void setRefVisible(bool vis) { mRefVisible = vis; }
	void setValueVisible(bool vis) { mValueVisible = vis; }

	QSharedPointer<Footprint> footprint() const { return mFp;}
	const QUuid& fpUuid() const { return mFpUuid; }
	void setFootprint(QUuid uuid);

	QList<QSharedPointer<PartPin> > pins() const { return mPins; }
	QSharedPointer<PartPin> pin(const QString &name);

	static QSharedPointer<Part> newFromXML(QXmlStreamReader &reader, PCBDoc* doc);
	void toXML(QXmlStreamWriter &writer) const;

	QTransform transform() const { return mTransform; }
	bool testHit(QPoint pt, const Layer& /*layer*/) const { return bbox().contains(pt); }

	PCBDoc* doc() const {return mDoc; }
private:
	class PartState : public PCBObjStateInternal
	{
	public:
		virtual ~PartState() {}
	private:
		friend class Part;
		PartState(const Part &p)
			: transform(p.transform()), pos(p.pos()), angle(p.angle()),
			side(p.side()), locked(p.locked()), refVis(p.refVisible()),
			valVis(p.valueVisible()), uuid(p.fpUuid()), fp(p.footprint()), pins(p.pins()),
			doc(p.doc())
		{}

		QTransform transform;
		QPoint pos;
		int angle;
		SIDE side;
		bool locked;
		bool refVis;
		bool valVis;
		QUuid uuid;
		QSharedPointer<Footprint> fp;
		QList<QSharedPointer<PartPin> > pins;
		PCBDoc* doc;
	};

	void resetFp();
	void updateFp();

	void updateTransform();

	/// Coordinate transform from part to world coords
	QTransform mTransform;

	/// Part position on board.  This is the origin of the part's coordinate system.
	QPoint mPos;
	/// Rotation angle (clockwise).
	int mAngle;
	/// PCB side on which the part is located.
	SIDE mSide;
	/// Locked parts cannot be moved.
	bool mLocked;
	/// Reference designator text
	QSharedPointer<Text> mRefdes;
	bool mRefVisible;
	/// Value text
	QSharedPointer<Text> mValue;
	bool mValueVisible;
	/// Pointer to the footprint of the part
	QSharedPointer<Footprint> mFp;
	/// UUID of footprint
	QUuid mFpUuid;
	/// List of part pins.
	QList<QSharedPointer<PartPin> > mPins;
	/// Parent document
	PCBDoc* mDoc;
};

#endif // PART_H
