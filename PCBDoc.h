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

private:
	TraceList mTraceList;
	QList<Net*> mNets;
	PartList mPartList;


};

#endif // PCBDOC_H
