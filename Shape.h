// Shape.h : interface for the CShape class
//
#pragma once
#include <QHash>
#include <QXmlStreamReader>
#include "PCBObject.h"
#include "Text.h"

class Line;
class Arc;
class Footprint;





/// A pad is a padstack component; it describes the
/// shape of a pad on a given layer of the padstack.
class Pad
{
public:
	// pad shapes
	enum PADSHAPE {
		PAD_NONE = 0,
		PAD_ROUND,
		PAD_SQUARE,
		PAD_RECT,
		PAD_OBROUND,
		PAD_OCTAGON,
	};

	/// Describes how to connect a pad to copper areas.
	enum PADCONNTYPE {
		PAD_CONNECT_DEFAULT = 0,	///< use global setting
		PAD_CONNECT_NEVER,			///< never connect pad to area
		PAD_CONNECT_THERMAL,		///< connect pad using a thermal structure
		PAD_CONNECT_NOTHERMAL		///< flood pad with copper
	};

	Pad();
	bool operator==(const Pad &p) const;
	bool isNull() { return mShape == PAD_NONE; }

	static Pad newFromXML(QXmlStreamReader &reader);

	PADSHAPE shape() {return mShape;}
	int width() {return mWidth;}
	int height() {return mHeight;}
	PADCONNTYPE connFlag() {return mConnFlag;}

	bool testHit( const QPoint & pt );

	QRect bbox() const;

private:
	PADSHAPE mShape;
	int mWidth, mHeight;
	PADCONNTYPE mConnFlag;
};

/// A padstack is a collection of pads and a hole.
/// Padstacks are used for component pins as well as vias.
/// A padstack is shared between all physically identical pins.
class Padstack
{
public:
	Padstack();
	bool operator==(const Padstack &p) const;

	static Padstack* newFromXML(QXmlStreamReader &reader);

	QString getName() const {return name; }
	int getHole() const {return hole_size;}
	Pad getStartPad() const {return start;}
	Pad getEndPad() const {return end;}
	Pad getInnerPad() const {return inner;}
	Pad getStartMask() const {return start_mask;}
	Pad getEndMask() const {return end_mask; }
	Pad getStartPaste() const {return start_paste;}
	Pad getEndPaste() const {return end_paste;}
	bool isSmt() const {return hole_size == 0;}
	QRect bbox() const;
private:
	/// Padstack name; optional; only used for library padstacks
	/// (i.e. VIA_15MIL)
	QString name;
	int hole_size;		// 0 = no hole (i.e SMT)
	Pad start, start_mask, start_paste;
	Pad end, end_mask, end_paste;
	Pad inner;
};

/// A pin is an instance of a padstack associated with a footprint.
/// It stores the name, coordinates, rotation, and the associated padstack
/// of each pin within a footprint.
class Pin
{
public:
	enum PINLAYER { LAY_START, LAY_INNER, LAY_END, LAY_UNKNOWN };

	Pin(Footprint* fp) : mAngle(0), mPadstack(NULL), mFootprint(fp) {}

	int angle() const { return mAngle; }
	QPoint pos() const { return mPos; }
	Padstack* padstack() const { return mPadstack; }
	QString name() const { return mName; }

	void draw(QPainter *painter, PINLAYER layer);
	QRect bbox() const;

	bool testHit(const QPoint &pt, PINLAYER layer) const;

	static Pin newFromXML(QXmlStreamReader &reader, const QHash<int, Padstack*> &padstacks, Footprint* fp);

	Pad getPadOnLayer(PINLAYER layer) const;

private:
	void updateTransform();
	/// Pin name (i.e. "1", "B2", "GATE")
	QString mName;
	/// Position relative to parent footprint
	QPoint mPos;
	/// Rotation angle (CW)
	int mAngle;
	/// Transform to part coordinates
	QTransform mPartTransform;
	/// Padstack used for this pin
	Padstack* mPadstack;
	/// Parent footprint
	Footprint* mFootprint;
};

class Footprint : public PCBObject
{

public:
	Footprint();
	~Footprint();

	virtual void draw(QPainter *painter, PCBLAYER layer);
	virtual QRect bbox() const;

	QString name() const { return mName; }
	QString author() const { return mAuthor; }
	QString source() const { return mSource; }
	QString desc() const { return mDesc; }


	int numPins() const;
	const Pin* getPin(const QString & pin) const;
	const Pin* getPin(int i) {return &mPins.at(i);}

	QRect getPinBounds() const;

	Text getRefText() {return mRefText;}
	Text getValueText() {return mValueText;}

	static Footprint* newFromXML(QXmlStreamReader &reader, const QHash<int, Padstack*> &padstacks);
private:
	/// Computes the default centroid (center of all pins)
	QPoint getDefaultCentroid();

	/// Footprint name (i.e. DIP20)
	QString mName;
	/// Footprint author
	QString mAuthor;
	/// Source document used to create footprint (i.e. mfg datasheet)
	QString mSource;
	/// Description of footprint
	QString mDesc;
	/// Units used to draw the footprint
	UNIT mUnits;
	/// Reference designator text
	Text mRefText;
	/// Value text
	Text mValueText;
	/// Centroid point; can be automatic or user-defined
	QPoint mCentroid;
	/// If false, centroid is automatically set to the center of all pins
	/// If true, centroid is user-defined
	bool mCustomCentroid;
	/// Footprint pins
	QList<Pin> mPins;
	/// Silkscreen lines (used for part outline)
	QList<Line> mOutlineLines;
	/// Silkscreen lines (used for part outline)
	QList<Arc> mOutlineArcs;
	/// Silkscreen text
	QList<Text> mTexts;
};

