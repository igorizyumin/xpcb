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

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	QString name() const {return mName;}
	void addPin( PartPin * pin);
	void removePin( PartPin * pin);
	QSet<PartPin*> getPins() const { return mPins; }
	Padstack* getViaPs() const { return mViaPS; }
	bool visible() const { return mIsVisible; }
	void setVisible(bool v) { mIsVisible = v; }

	static Net* newFromXML(QXmlStreamReader &reader, PCBDoc *doc,
						   const QHash<int, Padstack*> &padstacks);
	void toXML(QXmlStreamWriter &writer) const;
private:
	class NetState : public PCBObjStateInternal
	{
	public:
		virtual ~NetState() {}
	private:
		friend class Net;
		NetState(const Net &p)
			: vis(p.visible()), name(p.name()), pins(p.getPins())
		{}

		bool vis;
		QString name;
		QSet<PartPin*> pins;
	};

	bool mIsVisible;
	PCBDoc * mDoc;	// PCB document
	QString mName;		// net name
	QSet<PartPin*> mPins;	// pointers to part pins that are in this net
	Padstack* mViaPS; // via padstack for this net
};

#endif // NET_H
