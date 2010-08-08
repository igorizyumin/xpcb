#ifndef NET_H
#define NET_H

#include <QString>
#include <QList>
#include <QSet>
#include <QXmlStreamReader>
#include "global.h"
#include "Part.h"
#include "Trace.h"

// Basic types

class PCBDoc;

class Net : public PCBObject
{
public:
	Net( PCBDoc *doc, const QString &name);
	~Net();

	QString name() {return name;}
	void AddPin( PartPin * pin, bool set_areas=true );
	PartPin* TestHitOnPin( QPoint pt, PCBLAYER layer);
	QSet<PartPin*> getPins() { return mPins; }

	static Net* newFromXML(QXmlStreamReader &reader);
private:
	PCBDoc * mDoc;	// PCB document
	QString mName;		// net name
	QSet<PartPin*> mPins;	// pointers to part pins that are in this net
	Padstack* mViaPS; // via padstack for this net
};

#endif // NET_H
