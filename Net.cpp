#include "Net.h"
#include "Part.h"
#include "PCBDoc.h"
#include "Log.h"

Net::Net( PCBDoc *doc, const QString &name ) :
		mDoc(doc), mName(name), mViaPS(NULL)
{
}

Net* Net::newFromXML(QXmlStreamReader &reader, PCBDoc *doc,
					 const QHash<int, Padstack*> &padstacks)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "net");

	QXmlStreamAttributes attr = reader.attributes();
	Net* n = new Net(doc, attr.value("name").toString());
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
		Part* part = doc->getPart(refdes);
		Q_ASSERT(part != NULL);
		PartPin* pin = part->getPin(pinname);
		Q_ASSERT(pin != NULL);
		n->mPins.insert(pin);

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
	foreach(PartPin* p, mPins)
	{
		writer.writeStartElement("pinRef");
		writer.writeAttribute("partref", p->part()->refdes());
		writer.writeAttribute("pinname", p->name());
		writer.writeEndElement();
	}
	writer.writeEndElement();
}

Net::~Net()
{
	// disconnect pins from net
	foreach(PartPin* p, mPins)
	{
		p->setNet(NULL);
	}
}

// Add new pin to net
//
void Net::addPin( PartPin* newpin )
{
	mPins.insert(newpin);
	newpin->setNet(this);
}

void Net::removePin(PartPin* p)
{
	// Disconnect from net
	p->setNet(NULL);
	// Remove from net
	mPins.remove(p);
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
