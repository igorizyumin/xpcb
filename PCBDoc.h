#ifndef PCBDOC_H
#define PCBDOC_H

#include <QList>
#include <QFile>
#include <QUndoStack>
#include "Trace.h"
#include "Part.h"
#include "Net.h"
#include "Text.h"

class PCBDoc : public QObject
{
	Q_OBJECT

	friend class XmlLoadTest;

public:
    PCBDoc();
	virtual ~PCBDoc();

	TraceList& traceList() const {return *mTraceList;}

	bool saveToFile(const QString & file);
	bool loadFromFile(const QString & file);
	bool loadFromFile(QFile & file);
	bool loadFromXml(QXmlStreamReader &reader);

	bool saveToXML(const QString &file);
	bool saveToXML(QXmlStreamWriter &writer);

	bool isModified();

	Part* getPart(const QString & refdes);
	Footprint* getFootprint(const QString &name);
	Net* getNet(const QString &name) const;

	QString name() { return mName; }
	UNIT units() { return mUnits; }

	/// Returns a list of all objects that are hit
	QList<PCBObject*> findObjs(QPoint &pt);
	/// Returns list of all objects touching rect
	QList<PCBObject*> findObjs(QRect &rect);

	void addText(Text* t);
	void removeText(Text* t);

	void doCommand(QUndoCommand *cmd);

signals:
	void changed();
	void cleanChanged(bool clean);
	void canUndoChanged(bool e);
	void canRedoChanged(bool e);

public slots:
	void undo();
	void redo();

private:
	void clearDoc();

	QUndoStack *mUndoStack;

	/// Project name
	QString mName;
	/// Default units
	UNIT mUnits;

	TraceList* mTraceList;
	QList<Net*> mNets;
	QList<Part*> mParts;
	QList<Text*> mTexts;
	QList<Area*> mAreas;
	QList<Footprint*> mFootprints;
	QList<Padstack*> mPadstacks;
	Polygon * mBoardOutline;

	Padstack* mDefaultPadstack;

};

#endif // PCBDOC_H
