#ifndef NET_H
#define NET_H

#include <QString>
#include <QFile>
#include <QList>
#include <QSet>
#include <QHash>
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
//	Net(const Net& other);
	~Net();

	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	QString name() const {return mName;}
	void addPin( QSharedPointer<PartPin> pin);
	void removePin( QSharedPointer<PartPin> pin);
	QList<QSharedPointer<PartPin> > pins() const;
	QSharedPointer<Padstack> viaPs() const { return mViaPS; }
	bool visible() const { return mIsVisible; }
	void setVisible(bool v) { mIsVisible = v; }

	static QSharedPointer<Net> newFromXML(QXmlStreamReader &reader, PCBDoc *doc,
						   const QHash<int, QSharedPointer<Padstack> > &padstacks);
	void toXML(QXmlStreamWriter &writer) const;
private:
	class NetState : public PCBObjStateInternal
	{
	public:
		virtual ~NetState() {}
	private:
		friend class Net;
		NetState(const Net &p)
			: vis(p.visible()), name(p.name())
		{
			foreach(QSharedPointer<PartPin> pp, p.pins())
			{
				pins.append(pp.toWeakRef());
			}
		}

		bool vis;
		QString name;
		QList<QWeakPointer<PartPin> > pins;
	};

	bool mIsVisible;
	PCBDoc * mDoc;	// PCB document
	QString mName;		// net name
	QList<QWeakPointer<PartPin> > mPins;	// pointers to part pins that are in this net
	QSharedPointer<Padstack> mViaPS; // via padstack for this net
};


class Netlist
{
public:
	void addNet(QString name, QList<QPair<QString, QString> > net) { mNets.insert(name, net); }
	void removeNet(QString name) { mNets.remove(name); }
	QList<QPair<QString, QString> > net(QString &name) const { return mNets.value(name); }

	/// Returns a list of part reference designators.
	QList<QString> parts() const { return mParts.keys(); }
	/// Returns the footprint name for a part.
	QString partFpName(QString refdes) const { return mParts.value(refdes); }

	void addPart(QString refdes, QString fp) { mParts.insert(refdes, fp); }

protected:
	/// List of nets in this netlist.
	QHash<QString, QList<QPair<QString, QString> > > mNets;
	/// Associates parts with footprint names.
	QHash<QString, QString> mParts;
};

class NetlistLoader
{
public:
	static Netlist* loadFile(QString path);
	static void registerLoader(NetlistLoader* loader) { mLoaders.append(loader); }

protected:
	virtual Netlist* loadFromFile(QFile &file) = 0;
	static QList<NetlistLoader*> mLoaders;
};

#endif // NET_H
