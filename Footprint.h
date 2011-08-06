/*
	Copyright (C) 2010-2011 Igor Izyumin	
	
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

#ifndef FOOTPRINT_H
#define FOOTPRINT_H

#include <QHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QUuid>
#include "PCBObject.h"
#include "Text.h"
#include "Line.h"

class Footprint;

/// A pad is a padstack component; it describes the
/// shape of a pad on a given layer of the padstack.
class Pad
{
public:
	// pad shapes
	enum PADSHAPE {
		PAD_NONE = 0,
		PAD_DEFAULT,
		PAD_ROUND,
		PAD_SQUARE,
		PAD_RECT,
		PAD_RRECT,
		PAD_OBROUND,
		PAD_OCTAGON,
	};

	/// Describes how to connect a pad to copper areas.
	enum PADCONNTYPE {
		CONN_DEFAULT = 0,	///< use global setting
		CONN_NEVER,			///< never connect pad to area
		CONN_THERMAL,		///< connect pad using a thermal structure
		CONN_NOTHERMAL		///< flood pad with copper
	};

	Pad(PADSHAPE shape = PAD_NONE, int width = 0, int length = 0, int radius = 0, PADCONNTYPE connType = CONN_DEFAULT);
	bool operator==(const Pad &p) const;
	bool isNull() const { return mShape == PAD_NONE; }
	bool isDefault() const { return mShape == PAD_DEFAULT; }

	static Pad newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;


	PADSHAPE shape() {return mShape;}
	int width() {return mWidth;}
	int length() {return mLength;}
	int radius() {return mRadius;}
	PADCONNTYPE connFlag() {return mConnFlag;}
	void setConnFlag(PADCONNTYPE flag) { mConnFlag = flag; }

	bool testHit( const QPoint & pt, int dist );

	QRect bbox() const;
	void draw(QPainter *painter) const;

private:
	PADSHAPE mShape;
	int mWidth, mLength, mRadius;
	PADCONNTYPE mConnFlag;
};

/// A padstack is a collection of pads and a hole.
/// Padstacks are used for component pins as well as vias.
/// A padstack is shared between all physically identical pins.
class Padstack
{
public:
	Padstack();
//	bool operator==(const Padstack &p) const;

	static QSharedPointer<Padstack> newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	QString name() const {return mName; }
	void setName(QString name) { mName = name; }

	int holeSize() const {return hole_size;}
	void setHoleSize(int size) { hole_size = size; }

	Pad& startPad() {return start;}
	Pad& endPad() {return end;}
	Pad& innerPad() {return inner;}
	Pad& startMask() {return start_mask;}
	Pad& endMask() {return end_mask; }
	Pad& startPaste() {return start_paste;}
	Pad& endPaste() {return end_paste;}
	bool isSmt() const {return hole_size == 0;}
	QRect bbox() const;
	void draw(QPainter *painter, const Layer& layer) const;

	QUuid uuid() const { return mUuid; }
private:
	/// Padstack name; optional; only used for library padstacks
	/// (i.e. VIA_15MIL)
	QString mName;
	int hole_size;		// 0 = no hole (i.e SMT)
	Pad start, start_mask, start_paste;
	Pad end, end_mask, end_paste;
	Pad inner;
	QUuid mUuid;
};

/// A pin is an instance of a padstack associated with a footprint.
/// It stores the name, coordinates, rotation, and the associated padstack
/// of each pin within a footprint.
class Pin : public PCBObject
{
public:

	Pin(Footprint* fp) : mAngle(0), mIsDirty(true), mFootprint(fp) {}

	int angle() const { return mAngle; }
	QPoint pos() const { return mPos; }
	QSharedPointer<Padstack> padstack() const { return mPadstack; }
	QString name() const { return mName; }

	void setName(QString name) { mName = name; }
	void setPos(QPoint pos) { mPos = pos; markDirty(); }
	void setAngle(int angle) { mAngle = angle; markDirty(); }
	void setPadstack(QSharedPointer<Padstack> ps) { mPadstack = ps; }

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	virtual bool testHit(QPoint pt, int dist, const Layer& layer) const;

	static QSharedPointer<Pin> newFromXML(QXmlStreamReader &reader, const QHash<int, QSharedPointer<Padstack> > &padstacks, Footprint* fp);
	void toXML(QXmlStreamWriter &writer) const;

	Pad getPadOnLayer(const Layer& layer) const;

	virtual QTransform transform() const { return mFpTransform; }

private:
	class PinState : public PCBObjStateInternal
	{
	public:
		virtual ~PinState() {}
	private:
		friend class Pin;
		PinState(const Pin &p)
			: name(p.name()), pos(p.pos()), angle(p.angle()),
			ps(p.padstack())
		{}

		QString name;
		QPoint pos;
		int angle;
		QSharedPointer<Padstack> ps;
	};

	// disabled copy constructor
	Pin(const Pin& other);
	// disabled assignment operator
	Pin& operator=(const Pin& other);

	void updateTransform() const;
	void markDirty() const { mIsDirty = true; }
	/// Pin name (i.e. "1", "B2", "GATE")
	QString mName;
	/// Position relative to parent footprint
	QPoint mPos;
	/// Rotation angle (CW)
	int mAngle;
	/// Transform to part coordinates
	mutable QTransform mFpTransform;
	mutable bool mIsDirty;
	/// Padstack used for this pin
	QSharedPointer<Padstack> mPadstack;
	/// Parent footprint
	Footprint* mFootprint;
};

class Footprint
{

public:
	Footprint();
	Footprint(const Footprint& other);

	enum FP_DRAW_LAYER { LAY_START, LAY_INNER, LAY_END };

	void draw(QPainter *painter, FP_DRAW_LAYER layer) const;
	QRect bbox() const;

	QString name() const { return mName; }
	void setName(QString name) { mName = name; }

	QString author() const { return mAuthor; }
	void setAuthor(QString author) { mAuthor = author; }

	QString source() const { return mSource; }
	void setSource(QString src) { mSource = src; }

	QString desc() const { return mDesc; }
	void setDesc(QString desc) { mDesc = desc; }


	QList<QSharedPointer<Padstack> > padstacks() { return mPadstacks; }
	void addPadstack(QSharedPointer<Padstack> ps) { mPadstacks.append(ps); }
	void removePadstack(QSharedPointer<Padstack> ps);
	QSharedPointer<Padstack> padstack(QUuid uuid) const;

	int numPins() const;
	QSharedPointer<Pin> pin(const QString & pin) const;
	QSharedPointer<Pin> pin(int i) {return mPins.at(i);}
	const QList<QSharedPointer<Pin> > pins() { return mPins; }
	void addPin(QSharedPointer<Pin> p) { mPins.append(p); }
	void removePin(QSharedPointer<Pin> p) { mPins.removeOne(p); }

	const QList<QSharedPointer<Text> > texts() { return mTexts; }
	void addText(QSharedPointer<Text> t) { mTexts.append(t); }
	void removeText(QSharedPointer<Text> t) { mTexts.removeOne(t); }

	const QList<QSharedPointer<Line> > lines() { return mOutlineLines; }

	void addLine(QSharedPointer<Line> l) { mOutlineLines.append(l); }
	void removeLine(QSharedPointer<Line> l) { mOutlineLines.removeOne(l); }

	QRect getPinBounds() const;

	QSharedPointer<Text> refText() {return mRefText;}
	QSharedPointer<Text> valueText() {return mValueText;}

	QPoint centroid() {return mCentroid;}
	bool isCustomCentroid() {return mCustomCentroid;}
	XPcb::UNIT units() {return mUnits; }

	static QSharedPointer<Footprint> newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	const QUuid& uuid() const { return mUuid; }

private:
	/// Assignment operator (disabled)
	Footprint& operator=(Footprint& other);

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
	XPcb::UNIT mUnits;
	/// Reference designator text
	QSharedPointer<Text> mRefText;
	/// Value text
	QSharedPointer<Text> mValueText;
	/// Centroid point; can be automatic or user-defined
	QPoint mCentroid;
	/// If false, centroid is automatically set to the center of all pins
	/// If true, centroid is user-defined
	bool mCustomCentroid;
	/// Footprint padstacks
	QList<QSharedPointer<Padstack> > mPadstacks;
	/// Footprint pins
	QList<QSharedPointer<Pin> > mPins;
	/// Silkscreen lines (used for part outline)
	QList<QSharedPointer<Line> > mOutlineLines;
	/// Silkscreen text
	QList<QSharedPointer<Text> > mTexts;
	/// UUID for this footprint
	QUuid mUuid;
};

class FPDBFile;
class FPDBFolder;

/// The FPDatabase class is a singleton that is responsible for maintaining a database of available footprints.
/// It searches the configured footprint directories for footprint files and adds them to a tree structure.
class FPDatabase
{
public:
	static FPDatabase& instance();

	QList<QSharedPointer<FPDBFolder> > rootFolders() const { return mRootFolders; }
	QSharedPointer<const FPDBFile> getByUuid(QUuid uuid) const { return mUuidHash.value(uuid); }

private:
	FPDatabase();
	static FPDatabase* mInst;
	QList<QSharedPointer<FPDBFolder> > mRootFolders;
	QHash<QUuid, QSharedPointer<FPDBFile> > mUuidHash;

	QSharedPointer<FPDBFolder> createFolder(QString path, bool fullName = false);
	QSharedPointer<FPDBFile> createFile(QString path);
};

class FPDBFile
{
public:
	FPDBFile(QString path, QString name, QString author, QString source,
			 QString desc, QUuid uuid);

	QString path() const { return mFpPath; }
	QString name() const { return mName; }
	QString author() const { return mAuthor; }
	QString source() const { return mSource; }
	QString desc() const { return mDesc; }
	QUuid uuid() const { return mUuid; }

	QSharedPointer<Footprint> loadFootprint() const;

	void setParent(FPDBFolder* parent) { mParent = parent; }
	FPDBFolder* parent() const { return mParent; }

private:
	/// Absolute path to footprint file
	QString mFpPath;
	/// Footprint name
	QString mName;
	QString mAuthor;
	QString mSource;
	QString mDesc;
	QUuid mUuid;
	/// Parent folder
	FPDBFolder* mParent;
};

class FPDBFolder
{
public:
	FPDBFolder(QString name, QList<QSharedPointer<FPDBFolder> > folders,
			   QList<QSharedPointer<FPDBFile> > files);

	void setParent(FPDBFolder* parent) { mParent = parent; }
	FPDBFolder* parent() const { return mParent; }

	QString name() const { return mName; }
	QList<QSharedPointer<FPDBFolder> > folders() const { return mFolders; }
	QList<QSharedPointer<FPDBFile> > items() const { return mFiles; }
private:
	QString mName;
	QList<QSharedPointer<FPDBFolder> > mFolders;
	QList<QSharedPointer<FPDBFile> > mFiles;
	FPDBFolder* mParent;
};

#endif
