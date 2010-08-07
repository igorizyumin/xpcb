#ifndef PCBDOC_H
#define PCBDOC_H

#include <QList>
#include "Trace.h"
#include "PartList.h"
#include "Net.h"

class PCBDoc
{
public:
    PCBDoc();

	TraceList& traceList() {return mTraceList;}
	PartList& partList() {return mPartList;}

	bool loadFile(const QString & file);
	bool saveFile(const QString & file);
private:
	TraceList mTraceList;
	QList<Net*> mNets;
	QList<Part*> mParts;
	QList<Footprint*> mFootprints;
	PolyLine * mBoardOutline;

};

#endif // PCBDOC_H
