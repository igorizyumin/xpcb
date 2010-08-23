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

	virtual void draw(QPainter *painter, PCBLAYER layer);
	virtual QRect bbox() const;

	QString name() {return mName;}
	void addPin( PartPin * pin);
	void removePin( PartPin * pin);
	QSet<PartPin*> getPins() { return mPins; }

	static Net* newFromXML(QXmlStreamReader &reader, PCBDoc *doc,
						   const QHash<int, Padstack*> &padstacks);
private:
	PCBDoc * mDoc;	// PCB document
	QString mName;		// net name
	QSet<PartPin*> mPins;	// pointers to part pins that are in this net
	Padstack* mViaPS; // via padstack for this net
};

#endif // NET_H
