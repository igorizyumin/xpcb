#ifndef PCBDOC_H
#define PCBDOC_H

#include <QList>
#include <QFile>
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

	bool isModified() { return mModified; }

	Part* getPart(const QString & refdes);
	Footprint* getFootprint(const QString &name);
	Net* getNet(const QString &name) const;

	QString name() { return mName; }
	UNIT units() { return mUnits; }

	/// Draw the document using the provided painter
	/// \param rect the bounding rectangle (in pcb coordinates)
	void draw(QPainter* painter, QRect rect, PCBLAYER layer);

signals:
	void changed();

private:
	/// Document has been modified since last load/save
	bool mModified;

	/// Project name
	QString mName;
	/// Default units
	UNIT mUnits;

	TraceList* mTraceList;
	QList<Net*> mNets;
	QList<Part*> mParts;
	QList<Text> mTexts;
	QList<Area*> mAreas;
	QList<Footprint*> mFootprints;
	QList<Padstack*> mPadstacks;
	Polygon * mBoardOutline;

};

#endif // PCBDOC_H
