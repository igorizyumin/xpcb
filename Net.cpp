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

#include "Net.h"
#include "Part.h"
#include "Document.h"
#include "Log.h"

#if 0
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

#endif

/////////////////////////////////////////////////////////////////////

void NLPart::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("part");
	writer.writeAttribute("refdes", mRef);
	if (!mValue.isEmpty())
		writer.writeAttribute("value", mValue);
	writer.writeAttribute("footprint", mFp);
	writer.writeEndElement();
}

NLPart NLPart::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "part");
	QXmlStreamAttributes attr = reader.attributes();
	NLPart p(attr.value("refdes").toString(),
			 attr.value("footprint").toString());

	if (attr.hasAttribute("value"))
		p.mValue = attr.value("value").toString();

	do
			reader.readNext();
	while(!reader.isEndElement());

	Q_ASSERT(reader.isEndElement() && reader.name() == "part");
	return p;
}

/////////////////////////////////////////////////////////////////////

void NLNet::toXML(QXmlStreamWriter &writer) const
{
	// don't write out empty nets
	if (mPins.isEmpty()) return;
	writer.writeStartElement("net");
	writer.writeAttribute("name", mName);
	// TODO visible
	// TODO padstack
	// workaround for foreach
	typedef QPair<QString, QString> StringPair;
	foreach(StringPair pin, mPins)
	{
		writer.writeStartElement("pinRef");
		writer.writeAttribute("partref", pin.first);
		writer.writeAttribute("pinname", pin.second);
		writer.writeEndElement();
	}
}

NLNet NLNet::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "net");

	QXmlStreamAttributes attr = reader.attributes();
	NLNet n(attr.value("name").toString());
	// TODO visible/padstack
	// read pins
	while(reader.readNextStartElement())
	{
		Q_ASSERT(reader.isStartElement() && reader.name() == "pinRef");
		attr = reader.attributes();
		n.addPin(attr.value("partref").toString(),
				 attr.value("pinname").toString());
		do
				reader.readNext();
		while(!reader.isEndElement());
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "net");
	return n;
}

/////////////////////////////////////////////////////////////////////

QSharedPointer<Netlist> Netlist::loadFromFile(QString path)
{
	QFile f(path);
	if (!f.open(QFile::ReadOnly | QFile::Text))
		return QSharedPointer<Netlist>();

	QSharedPointer<Netlist> n;
	n = loadFromFile(f);
	f.close();
	if (n)
		return n;
	else
		return QSharedPointer<Netlist>();
}

QSharedPointer<Netlist> Netlist::loadFromFile(QFile &file)
{
	enum State { Start, Header, PartSection, NetSection,
				 InSignal, Done, Invalid };
	QSharedPointer<Netlist> netlist(new Netlist());
	file.reset();
	int lineCnt = 0;
	State state = Start;
	NLNet currNet;
	QList<QByteArray> s;

	while(!file.atEnd())
	{
		QByteArray line = file.readLine().trimmed().simplified();
		if (line.isEmpty() || line.startsWith("//"))
			continue;
		lineCnt++;
		switch(state)
		{
		case Start:
			if (line.startsWith("*PADS-PCB*") ||
					line.startsWith("*PADS2000*"))
				state = Header;
			else if (lineCnt >= 3)
			{
				Log::error("Did not find PADS header while parsing netlist");
				state = Invalid;
			}
			break;
		case Header:
			if (line.startsWith("*PART*"))
				state = PartSection;
			else
			{
				Log::error("Did not find part section while parsing netlist");
				state = Invalid;
			}
			break;
		case PartSection:
			s = line.split(' ');
			// XXX TODO values are not supported yet
			if (!line.startsWith("*") && s.length() == 2)
				netlist->addPart(NLPart(s[0], s[1]));
			else if (line.startsWith("*NET*"))
				state = NetSection;
			else
			{
				Log::error("Unexpected data encountered while parsing "
						   "part section of netlist");
				state = Invalid;
			}
			break;
		case NetSection:
		case InSignal:
			s = line.split(' ');
			if (line.startsWith("*"))
			{
				if ((line.startsWith("*SIGNAL") ||
						line.startsWith("*END*")) && currNet.pins().size() > 0)
				{
					// add current net
					netlist->addNet(currNet);
				}
				if (line.startsWith("*SIGNAL*") && s.size() == 2)
				{
					currNet = NLNet(s[1]);
					state = InSignal;
				}
				else if (line.startsWith("*END*"))
				{
					state = Done;
				}
				else
				{
					Log::error("Unexpected data encountered while parsing "
							   "net section of netlist");
					state = Invalid;
				}
			}
			else if (state == InSignal)
			{
				// get the list of pins (like: R1.2 U1.A13 ...)
				s = line.split(' ');
				foreach(QByteArray a, s)
				{
					// split each pin into the refdes and pin name
					QList<QByteArray> p = a.split('.');
					if (p.size() != 2)
					{
						Log::error("Found invalid part pin reference while "
								   "parsing netlist");
						state = Invalid;
						break;
					}
					// add the pin to the current net
					currNet.addPin(p[0], p[1]);
				}
			}
			else
			{
				Log::error("Unexpected data encountered while parsing "
						   "net section of netlist");
				state = Invalid;
			}
			break;
		default:
			break;
		} // switch(state)
		if (state == Invalid)
			return QSharedPointer<Netlist>();
	} // while loop

	if (state != Done)
	{
		Log::error("Unexpected end of file while parsing netlist");
		return QSharedPointer<Netlist>();
	}

	return netlist;
}

void Netlist::loadFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "netlist");

	mParts.clear();
	mNets.clear();

	while(reader.readNextStartElement())
	{
		if (reader.name() == "part")
			addPart(NLPart::newFromXML(reader));
		else if (reader.name() == "net")
			addNet(NLNet::newFromXML(reader));
	}

	Q_ASSERT(reader.isEndElement() && reader.name() == "netlist");
}

void Netlist::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("netlist");
	foreach(const NLPart& p, mParts.values())
	{
		p.toXML(writer);
	}

	foreach(const NLNet& n, mNets.values())
	{
		n.toXML(writer);
	}
	writer.writeEndElement();
}
