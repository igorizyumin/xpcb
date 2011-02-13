#ifndef NET_H
#define NET_H

#include <QString>
#include <QList>
#include <QSet>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
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

	virtual void draw(QPainter *painter, XPcb::PCBLAYER layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	QString name() {return mName;}
	void addPin( PartPin * pin);
	void removePin( PartPin * pin);
	QSet<PartPin*> getPins() { return mPins; }
	Padstack* getViaPs() { return mViaPS; }
	bool visible() { return mIsVisible; }
	void setVisible(bool v) { mIsVisible = v; }

	static Net* newFromXML(QXmlStreamReader &reader, PCBDoc *doc,
						   const QHash<int, Padstack*> &padstacks);
	void toXML(QXmlStreamWriter &writer) const;
private:
	bool mIsVisible;
	PCBDoc * mDoc;	// PCB document
	QString mName;		// net name
	QSet<PartPin*> mPins;	// pointers to part pins that are in this net
	Padstack* mViaPS; // via padstack for this net
};

#endif // NET_H
