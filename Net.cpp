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
	Net* n = new Net(doc, attr.value("name"));
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
		QString refdes = attr.value("partref");
		QString pinname = attr.value("pinname");
		PartPin* pin = doc->getPart(refdes)->GetPin(pinname);
		Q_ASSERT(pin != NULL);
		n->mPins.insert(pin);
	}
	Q_ASSERT(reader.isEndElement() && reader.name() == "net");
	return n;
}

Net::~Net()
{
	// remove pointers to net from pins on part
	for( int ip=0; ip<pin.size(); ip++ )
	{
		Part * part = pin.at(ip).part;
		if( part )
			part->GetPin(pin.at(ip).pin_name).setNet(NULL);
	}
}

void Net::moveOrigin(QPoint delta)
{
	// TODO move this to appropriate classes
	for( int ic=0; ic<connect.size(); ic++ )
	{
		Trace * c =  &net->connect[ic];
		UndrawConnection( net, ic );
		for( int iv=0; iv<=c->nsegs; iv++ )
		{
			cvertex * v = &c->vtx[iv];
			v->x += x_off;
			v->y += y_off;
		}
		DrawConnection( net, ic );
	}
	for( int ia=0; ia<net->area.GetSize(); ia++ )
	{
		carea * a = &net->area[ia];
		a->poly->MoveOrigin( x_off, y_off );
		SetAreaConnections( net, ia );
	}
}

// Add new pin to net
//
void Net::AddPin( PartPin* newpin, bool set_areas )
{
	pin.append(newpin);
	newpin->setNet(this);

	// adjust connections to areas
	if( net->nareas && set_areas )
		SetAreaConnections();
}

void Net::RemovePin(PartPin* p, bool set_areas)
{
	// Remove all connections to this pin
	for(int i = 0; i < connect.size(); i++)
	{
		if (connect[i].hasPin(p))
		{
			RemoveConnect(i, false);
			i--;
		}
	}
	// Disconnect from net
	p->setNet(NULL);
	// Remove from net
	pin.remove(p);

	// adjust connections to areas
	if( net->nareas && set_areas )
		SetAreaConnections();
}


// Add new connection to net, consisting of one unrouted segment
// start/end are part pins
// if end is null, a stub is added
void Net::AddNetConnect(PartPin* start, PartPin* end )
{
	Trace c(start, end);
	connect.append(c);
}

// Remove connection from net
// Does not remove any orphaned branches that result
// Leave pins in pin list for net
//
void Net::RemoveConnect( int ic, bool set_areas )
{
	Trace * c = &connect[ic];
	if( c->endPin() == NULL )
	{
		// stub
		if( c->vtx[c->nsegs].tee_ID )
		{
			// stub trace ending on tee, remove tee
			DisconnectBranch( net, ic );
		}
	}

	// see if contains tee-vertices
	for( int iv=1; iv<c->nsegs; iv++ )
	{
		int id = c->vtx[iv].tee_ID;
		if( id )
			RemoveTee( net, id );	// yes, remove it
	}

	// remove connection
	net->connect.RemoveAt( ic );
	net->nconnects = net->connect.GetSize();
	RenumberConnections( net );
	// adjust connections to areas
	if( net->nareas && set_areas )
		SetAreaConnections( net );
	return 0;
}

// test for hit on any pad on this net
// returns NULL if not found, otherwise returns pin
//
PartPin* Net::TestHitOnPin( QPoint pt, PCBLAYER layer)
{
	foreach(PartPin* p, pin)
		if (p->TestHit(pt, layer))
			return p;
	return NULL;
}

// Clean up connections by removing connections with no segments,
// removing zero-length segments and combining segments
//
void Net::CleanUpConnections()
{
	for( int ic=connect.size()-1; ic>=0; ic-- )
	{
		connect[ic].cleanup();
		if (connect[ic].isVoid())
			RemoveConnect(ic, false);
	}
}
