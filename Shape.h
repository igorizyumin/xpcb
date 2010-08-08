// Shape.h : interface for the CShape class
//
#pragma once
#include <QHash>
#include <QXmlStreamReader>
#include "PolyLine.h"
#include "PCBObject.h"
#include "Text.h"

class Text;

// pad shapes
typedef enum {
	PAD_NONE = 0,
	PAD_ROUND,
	PAD_SQUARE,
	PAD_RECT,
	PAD_OBROUND,
	PAD_OCTAGON,
	PAD_DEFAULT = 99
} PADSHAPE;

// pad area connect flags
enum {
	PAD_CONNECT_DEFAULT = 0,
	PAD_CONNECT_NEVER,
	PAD_CONNECT_THERMAL,
	PAD_CONNECT_NOTHERMAL
};

/// A pad is a padstack component; it describes the
/// shape of a pad on a given layer of the padstack.
class Pad
{
public:
	Pad();
	bool operator==(const Pad &p) const;

	static Pad newFromXML(QXmlStreamReader &reader);

private:
	PADSHAPE shape;	// see enum above
	int width, height;
};

/// A padstack is a collection of pads and a hole.
/// Padstacks are used for component pins as well as vias.
/// A padstack is shared between all physically identical pins.
class Padstack
{
public:
	Padstack();
	bool operator==(Padstack p);

	static Padstack* newFromXML(QXmlStreamReader &reader);
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
class Pin : public PCBObject
{
public:
	Pin(Footprint* fp) : mAngle(0), mPadstack(NULL), mFootprint(fp) {}

	int getAngle() const { return mAngle; }
	QPoint getPos() const { return mPos; }
	Padstack* getPadstack() { return mPadstack; }

	static Pin newFromXML(QXmlStreamReader &reader, QHash<int, Padstack*> &padstacks, Footprint* fp);
private:
	/// Pin name (i.e. "1", "B2", "GATE")
	QString mName;
	/// Position relative to parent footprint
	QPoint mPos;
	/// Rotation angle (CW)
	int mAngle;
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
	void Clear();

	static Footprint* newFromXML(QXmlStreamReader &reader);

	int numPins();
	Pin* getPin( QString name );

	QRect getBounds( bool bIncludeLineWidths=true );
	QRect getCornerBounds();
	QRect getPadBounds( int i );
	QRect getPadRowBounds( int i, int num );
	QRect getAllPadBounds();

	int Copy( Footprint * shape );	// copy all data from shape
	bool Compare( Footprint * shape );	// compare shapes, return true if same

	static Footprint* newFromXML(QXmlStreamReader &reader, QHash<int, Padstack*> &padstacks);
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
	QList<PolyLine*> mOutline;
	/// Silkscreen text
	QList<Text*> mTexts;
};

