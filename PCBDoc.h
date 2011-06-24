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
	virtual QList<Layer> layerList(LayerOrder order = ListOrder) = 0;

	virtual QString name() const { return mName; }
	virtual XPcb::UNIT units() const { return mUnits; }

	/// Returns a list of all objects that are hit
	virtual QList<PCBObject*> findObjs(QPoint &pt) = 0;
	/// Returns list of all objects touching rect
	virtual QList<PCBObject*> findObjs(QRect &rect) = 0;

	/// Add text object to document.
	virtual void addText(Text* t) = 0;
	/// Remove text object from document.
	virtual void removeText(Text* t) = 0;


	/// Returns a list of all padstacks used in the document.
	virtual QList<Padstack*> padstacks() = 0;
	/// Adds the provided padstack to the document.
	virtual void addPadstack(Padstack* ps) = 0;
	/// Removes the provided padstack from the document.
	virtual void removePadstack(Padstack* ps) = 0;
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
	QUndoStack *mUndoStack;
};

class PCBDoc : public Document
{
	Q_OBJECT

	friend class XmlLoadTest;
public:
    PCBDoc();
	virtual ~PCBDoc();

	// overrides
	using Document::saveToFile;
	using Document::loadFromFile;
	virtual bool saveToXml(QXmlStreamWriter &writer);
	virtual bool loadFromFile(QFile & file);
	virtual bool loadFromXml(QXmlStreamReader &reader);
	virtual QList<Layer> layerList(LayerOrder order = ListOrder);
	virtual QList<PCBObject*> findObjs(QPoint &pt);
	virtual QList<PCBObject*> findObjs(QRect &rect);

	TraceList& traceList() const {return *mTraceList;}
	Part* getPart(const QString & refdes);
	QSharedPointer<Footprint> getFootprint(QUuid uuid);
	Net* getNet(const QString &name) const;

	void addText(Text* t);
	void removeText(Text* t);

	void addPart(Part* p);
	void removePart(Part* p);

	int numLayers() const { return mNumLayers; }

	virtual QList<Padstack*> padstacks() { return mPadstacks; }
	virtual void addPadstack(Padstack* ps);
	virtual void removePadstack(Padstack* ps);

private:
	void clearDoc();

	/// Number of copper layers
	int mNumLayers;
	TraceList* mTraceList;
	QList<Net*> mNets;
	QList<Part*> mParts;
	QList<Text*> mTexts;
	QList<Area*> mAreas;
	QHash<QUuid, QSharedPointer<Footprint> > mFootprints;
	QList<Padstack*> mPadstacks;
	Polygon mBoardOutline;
	Padstack* mDefaultPadstack;
};

class FPDoc : public Document
{
	Q_OBJECT

public:
	FPDoc();
	virtual ~FPDoc();

	using Document::saveToFile;
	using Document::loadFromFile;
	virtual bool saveToXml(QXmlStreamWriter &writer);
	virtual bool loadFromFile(QFile & file);
	virtual bool loadFromXml(QXmlStreamReader &reader);

	virtual QList<Layer> layerList(LayerOrder order = ListOrder);

	virtual QList<PCBObject*> findObjs(QPoint &pt);
	virtual QList<PCBObject*> findObjs(QRect &rect);

	virtual void addText(Text* t) { mFp->addText(t); }
	virtual void removeText(Text* t) { mFp->removeText(t); }

	virtual QList<Padstack*> padstacks() { return mFp->padstacks(); }
	virtual void addPadstack(Padstack* ps) { return mFp->addPadstack(ps); }
	virtual void removePadstack(Padstack* ps) { return mFp->removePadstack(ps); }

	void addPin(Pin* p);
	void removePin(Pin* p);

	void addLine(Line* l);
	void removeLine(Line* l);

	Footprint* footprint() { return mFp; }

private:
	void clearFP();

	Footprint *mFp;
};

#endif // PCBDOC_H
