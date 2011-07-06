/*
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

#include "Net.h"
#include "Part.h"
#include "Document.h"
#include "Log.h"

Net::Net( PCBDoc *doc, const QString &name ) :
		mDoc(doc), mName(name), mViaPS(NULL)
{
}

QSharedPointer<Net> Net::newFromXML(QXmlStreamReader &reader, PCBDoc *doc,
					 const QHash<int, QSharedPointer<Padstack> > &padstacks)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "net");

	QXmlStreamAttributes attr = reader.attributes();
	QSharedPointer<Net> n( new Net(doc, attr.value("name").toString()) );
	if (attr.hasAttribute("visible"))
	{
		bool visible = attr.value("visible").toString() == "1";
		n->setVisible(visible);
	}
	if (attr.hasAttribute("defViaPadstack"))
	{
		int ips = attr.value("defViaPadstack").toString().toInt();
		if (padstacks.contains(ips))
		{
			n->mViaPS = padstacks.value(ips);
		}
		else
		{
			Log::instance().error("Got invalid via padstack reference when creating net");
		}
	}
	// read pins, find them in the document, and insert into net
	while(reader.readNextStartElement())
	{
		Q_ASSERT(reader.isStartElement() && reader.name() == "pinRef");
		attr = reader.attributes();
		QString refdes = attr.value("partref").toString();
		QString pinname = attr.value("pinname").toString();
		QSharedPointer<Part> part = doc->part(refdes);
		Q_ASSERT(!part.isNull());
		QSharedPointer<PartPin> pin = part->pin(pinname);
		Q_ASSERT(!pin.isNull());
		n->mPins.append(pin);

		do
				reader.readNext();
		while(!reader.isEndElement());
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "net");
	return n;
}

void Net::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("net");
	writer.writeAttribute("name", mName);
	writer.writeAttribute("visible", mIsVisible ? "1" : "0");
	writer.writeAttribute("defViaPadstack", QString::number(mViaPS->getid()));
	foreach(QWeakPointer<PartPin> pw, mPins)
	{
		QSharedPointer<PartPin> p = pw.toStrongRef();
		if (p.isNull()) continue;
		writer.writeStartElement("pinRef");
		writer.writeAttribute("partref", p->part()->refdes());
		writer.writeAttribute("pinname", p->name());
		writer.writeEndElement();
	}
	writer.writeEndElement();
}

Net::~Net()
{
}

// Add new pin to net
//
void Net::addPin( QSharedPointer<PartPin> newpin )
{
	mPins.append(newpin.toWeakRef());
}

void Net::removePin(QSharedPointer<PartPin> p)
{
	mPins.removeOne(p.toWeakRef());
}

QList<QSharedPointer<PartPin> > Net::pins() const
{
	QList<QSharedPointer<PartPin> > out;
	foreach(QWeakPointer<PartPin> pp, mPins)
	{
		if (pp.toStrongRef().isNull())
			continue;
		out.append(pp.toStrongRef());
	}
	return out;
}

void Net::draw(QPainter */*painter*/, const Layer& /*layer*/) const
{

}

QRect Net::bbox() const
{
	return QRect();
}

PCBObjState Net::getState() const
{
	return PCBObjState(new NetState(*this));
}

bool Net::loadState(PCBObjState &state)
{
	// convert to part state
	QSharedPointer<NetState> s = state.ptr().dynamicCast<NetState>();
	if (s.isNull()) return false;
	mIsVisible = s->vis;
	mName = s->name;
	mPins = s->pins;
	return true;
}


/////////////////////////////////////////////////////////////////////

QList<NetlistLoader*> NetlistLoader::mLoaders;

Netlist* NetlistLoader::loadFile(QString path)
{
	QFile f(path);
	if (!f.open(QFile::ReadOnly | QFile::Text))
		return NULL;

	Netlist* n;
	foreach(NetlistLoader* l, mLoaders)
	{
		n = l->loadFromFile(f);
		if (n)
		{
			f.close();
			return n;
		}
	}
	f.close();
	return NULL;
}
