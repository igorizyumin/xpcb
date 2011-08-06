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

#ifndef PCBDOC_H
#define PCBDOC_H

#include <QList>
#include <QFile>
#include <QUndoStack>
#include "Trace.h"
#include "Part.h"
#include "Net.h"
#include "Text.h"
#include "Polygon.h"
#include "Area.h"

class Document : public QObject
{
	Q_OBJECT

public:
	/// Specifies which layer ordering is desired when getting a list of layers.
	enum LayerOrder
	{
		ListOrder,			/// The order in which the layers appear in the layer list.
		DrawPriorityOrder	/// The order in which the layers should be drawn.
	};

	enum LayerType
	{
		Copper = 0x01,
		Silk = 0x02,
		Mask = 0x04,
		Glue = 0x08,
		Paste = 0x10,
		Hole = 0x20,
		Physical = 0xFF,
		Display = 0x100,
		All = 0xFFFF
	};

	Q_DECLARE_FLAGS(LayerMask, LayerType)


	Document();
	virtual ~Document();

	/// Saves the document to the provided path.
	/// \param file The file to be written.
	/// \returns true if write was successful; false if there was an error.
	virtual bool saveToFile(const QString & file);

	/// Saves the document to the provided XML stream.
	/// \param file The XML stream writer to write to.
	/// \returns true if write was successful; false if there was an error.
	virtual bool saveToXml(QXmlStreamWriter &writer) = 0;

	/// Loads the document from the provided path.
	/// \param file The path to be read.
	/// \returns true if load was successful; false if there was an error.
	virtual bool loadFromFile(const QString & file);

	/// Loads the document from the provided file handle.
	/// \param file The file handle to read from.
	/// \returns true if load was successful; false if there was an error.
	virtual bool loadFromFile(QFile & file) = 0;

	/// Loads the document from the provided XML stream.
	/// \param file The XML stream reader to read from.
	/// \returns true if load was successful; false if there was an error.
	virtual bool loadFromXml(QXmlStreamReader &reader) = 0;

	/// Returns true if the undo stack is not in a clean state.
	virtual bool isModified();

	/// Pushes the provided command to the undo stack. The command's
	/// redo() method gets executed when this occurs.  The document
	/// takes ownership of the command.
	virtual void doCommand(QUndoCommand *cmd);

	/// Returns the list of layers in the document, ordered appropriately.
	/// \param order The order in which layers will be arranged.
	virtual QList<Layer> layerList(LayerOrder order = ListOrder, LayerMask mask = All) = 0;

	virtual QString name() const { return mName; }
	virtual XPcb::UNIT units() const { return mUnits; }

	/// Returns a list of all objects that are hit
	virtual QList<QSharedPointer<PCBObject> > findObjs(QPoint &pt, int dist = 1) = 0;
	/// Returns list of all objects touching rect
	virtual QList<QSharedPointer<PCBObject> > findObjs(QRect &rect) = 0;

	/// Add text object to document.
	virtual void addText(QSharedPointer<Text> t) = 0;
	/// Remove text object from document.
	virtual void removeText(QSharedPointer<Text> t) = 0;


	/// Returns a list of all padstacks used in the document.
	virtual QList<QSharedPointer<Padstack> > padstacks() = 0;
	/// Adds the provided padstack to the document.
	virtual void addPadstack(QSharedPointer<Padstack>) = 0;
	/// Removes the provided padstack from the document.
	virtual void removePadstack(QSharedPointer<Padstack>) = 0;
	/// Returns a padstack for the given UUID (or null if not present).
	virtual QSharedPointer<Padstack> padstack(QUuid uuid) = 0;
signals:
	void changed();
	void cleanChanged(bool clean);
	void canUndoChanged(bool e);
	void canRedoChanged(bool e);

public slots:
	virtual void undo();
	virtual void redo();

protected:
	/// Project name
	QString mName;
	/// Default units
	XPcb::UNIT mUnits;
	/// Undo stack
	QUndoStack mUndoStack;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Document::LayerMask)

class PCBDoc : public Document
{
	Q_OBJECT

	friend class XmlLoadTest;
public:
    PCBDoc();

	// overrides
	using Document::saveToFile;
	using Document::loadFromFile;
	virtual bool saveToXml(QXmlStreamWriter &writer);
	virtual bool loadFromFile(QFile & file);
	virtual bool loadFromXml(QXmlStreamReader &reader);
	virtual QList<Layer> layerList(LayerOrder order = ListOrder,
								   Document::LayerMask mask = Document::All);
	virtual QList<QSharedPointer<PCBObject> > findObjs(QPoint &pt, int dist = 1);
	virtual QList<QSharedPointer<PCBObject> > findObjs(QRect &rect);

	TraceList& traceList() const {return *mTraceList;}
	Netlist& netlist() const { return *mNetlist; }

	QSharedPointer<Part> part(const QString & refdes);

	QSharedPointer<Footprint> getFootprint(QUuid uuid);
	QList<QSharedPointer<Footprint> > footprints() { return mFootprints.values(); }

	virtual void addText(QSharedPointer<Text> t);
	virtual void removeText(QSharedPointer<Text> t);

	virtual void addPart(QSharedPointer<Part> p);
	virtual void removePart(QSharedPointer<Part> p);

	int numLayers() const { return mNumLayers; }

	virtual QList<QSharedPointer<Padstack> > padstacks() { return mPadstacks.values(); }
	virtual void addPadstack(QSharedPointer<Padstack> ps);
	virtual void removePadstack(QSharedPointer<Padstack> ps);
	virtual QSharedPointer<Padstack> padstack(QUuid uuid);

private:
	void clearDoc();

	/// Number of copper layers
	int mNumLayers;
	QSharedPointer<TraceList> mTraceList;
	QSharedPointer<Netlist> mNetlist;
	QList<QSharedPointer<Part> > mParts;
	QList<QSharedPointer<Text> > mTexts;
	QList<QSharedPointer<Area> > mAreas;
	QHash<QUuid, QSharedPointer<Footprint> > mFootprints;
	QHash<QUuid, QSharedPointer<Padstack> > mPadstacks;
	Polygon mBoardOutline;
	QUuid mDefaultPadstack;
};

class FPDoc : public Document
{
	Q_OBJECT

public:
	FPDoc();

	using Document::saveToFile;
	using Document::loadFromFile;
	virtual bool saveToXml(QXmlStreamWriter &writer);
	virtual bool loadFromFile(QFile & file);
	virtual bool loadFromXml(QXmlStreamReader &reader);

	virtual QList<Layer> layerList(LayerOrder order = ListOrder,
								   Document::LayerMask mask = Document::All);

	virtual QList<QSharedPointer<PCBObject> > findObjs(QPoint &pt, int dist = 1);
	virtual QList<QSharedPointer<PCBObject> > findObjs(QRect &rect);

	virtual void addText(QSharedPointer<Text> t) { mFp->addText(t); }
	virtual void removeText(QSharedPointer<Text> t) { mFp->removeText(t); }

	virtual QList<QSharedPointer<Padstack> > padstacks() { return mFp->padstacks(); }
	virtual void addPadstack(QSharedPointer<Padstack> ps) { return mFp->addPadstack(ps); }
	virtual void removePadstack(QSharedPointer<Padstack> ps) { return mFp->removePadstack(ps); }
	virtual QSharedPointer<Padstack> padstack(QUuid uuid) { return mFp->padstack(uuid); }

	void addPin(QSharedPointer<Pin> p) { mFp->addPin(p); }
	void removePin(QSharedPointer<Pin> p) { mFp->removePin(p); }

	void addLine(QSharedPointer<Line> l) { mFp->addLine(l); }
	void removeLine(QSharedPointer<Line> l) { mFp->removeLine(l); }

	QSharedPointer<Footprint> footprint() { return mFp; }

private:
	QSharedPointer<Footprint> mFp;
};

#endif // PCBDOC_H
