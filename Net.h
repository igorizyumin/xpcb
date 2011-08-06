/*
	Copyright (C) 2010-2011 Igor Izyumin

	This file is part of xpcb.

	xpcb is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xpcb is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xpcb.  If not, see <http://www.gnu.org/licenses/>.
*/

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

// to be removed soon
#if 0
// why the hell is a net a PCB object?
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
#endif

class NLPart
{
public:
	NLPart(QString ref = QString(), QString footprint = QString(),
		   QString value = QString())
		: mRef(ref), mValue(value), mFp(footprint)
	{
	}

	QString refdes() const { return mRef; }
	QString value() const { return mValue; }
	QString footprint() const { return mFp; }

	void toXML(QXmlStreamWriter &writer) const;
	static NLPart newFromXML(QXmlStreamReader &reader);
private:
	QString mRef;
	QString mValue;
	QString mFp;
};

class NLNet
{
public:
	NLNet(QString name = QString(), QSet<QPair<QString, QString> > pins =
		  QSet<QPair<QString, QString> >())
		: mName(name), mPins(pins) {}

	QString name() const { return mName; }
	QSet<QPair<QString, QString> > pins() const { return mPins; }

	void addPin(QString partref, QString pinname)
	{
		mPins.insert(QPair<QString, QString>(partref, pinname));
	}

	void removePin(QString partref, QString pinname)
	{
		mPins.remove(QPair<QString, QString>(partref, pinname));
	}

	bool hasPin(QString partref, QString pinname) const
	{
		return mPins.contains(QPair<QString, QString>(partref, pinname));
	}

	void toXML(QXmlStreamWriter &writer) const;
	static NLNet newFromXML(QXmlStreamReader &reader);
private:
	QString mName;
	QSet<QPair<QString, QString> > mPins;
};

class Netlist
{
public:
	void addNet(const NLNet& net) { mNets.insert(net.name(), net); }
	void removeNet(QString name) { mNets.remove(name); }
	NLNet net(QString &name) const { return mNets.value(name); }

	QList<NLPart> parts() const { return mParts.values(); }
	NLPart part(QString ref) const { return mParts.value(ref); }

	void addPart(const NLPart &part) { mParts.insert(part.refdes(), part); }

	static QSharedPointer<Netlist> loadFromFile(QString path);

	void toXML(QXmlStreamWriter &writer) const;
	void loadFromXML(QXmlStreamReader &reader);
protected:
	static QSharedPointer<Netlist> loadFromFile(QFile &file);

	/// List of nets in this netlist (keyed by net name).
	QHash<QString, NLNet> mNets;
	/// List of parts (keyed by refdes).
	QHash<QString, NLPart> mParts;
};

#endif // NET_H
