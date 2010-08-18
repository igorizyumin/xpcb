#ifndef PCBDOC_H
#define PCBDOC_H

#include <QList>
#include "Trace.h"
#include "Part.h"
#include "Net.h"
#include "Text.h"

class PCBDoc
{
public:
    PCBDoc();

	TraceList& traceList() {return mTraceList;}
	PartList& partList() {return mPartList;}

	bool saveToFile(const QString & file);
	bool loadFromFile(const QString & file, PCBDoc *newdoc);

	Part* getPart(const QString & refdes);
	Footprint* getFootprint(const QString &name);
	Net* getNet(const QString &name);

private:


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

};

#endif // PCBDOC_H
