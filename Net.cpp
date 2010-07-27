#include "Net.h"
#include "Part.h"

Connection::Connection(PartPin* start, PartPin* end)
{
	locked = 0;
	utility = 0;
	start_pin = start;
	end_pin = end;

	vtx.append(Vertex(start->getPos(), start->getLayer()));
	if (end_pin) // if null, this is a stub -- no segment
	{
		vtx.append(Vertex(end->getPos(), end->getLayer()));
		seg.append(Segment(LAY_RAT_LINE));
	}
}

void Connection::cleanup()
{
	for( int is=seg.size()-1; is>=0; is-- )
	{
		// check for zero-length segment
		if( vtx[is].pos() == vtx[is+1].pos())
		{
			// yes, analyze segment
			enum { UNDEF=0, THRU_PIN, SMT_PIN, VIA, TEE, SEGMENT, END_STUB };
			int pre_type = UNDEF;	// type of preceding item
			int pre_layer = UNDEF;	// layer if SEGMENT or SMT_PIN
			int post_type = UNDEF;	// type of following item
			int post_layer = UNDEF;	// layer if SEGMENT or SMT_PIN
			int layer = seg[is].layer;
			// analyze start of segment
			if( is == 0 )
			{
				// first segment
				pre_layer = vtx[0].pad_layer;
				if( pre_layer == LAY_PAD_THRU )
					pre_type = THRU_PIN;	// starts on a thru pin
				else
					pre_type = SMT_PIN;		// starts on a SMT pin
			}
			else
			{
				// not first segment
				pre_layer = seg[is-1].layer;	// preceding layer
				if( vtx[is].tee_ID )
					pre_type = TEE;				// starts on a tee-vertex
				else if( vtx[is].via_w )
					pre_type = VIA;				// starts on a via
				else
					pre_type = SEGMENT;			// starts on a segment
			}
			// analyze end of segment
			if( is == seg.size()-1 && end_pin == NULL )
			{
				// last segment of stub trace
				if( vtx[is+1].isTee )
					post_type = TEE;			// ends on a tee-vertex
				else if( vtx[is+1].via_w )
					post_type = VIA;			// ends on a via
				else
					post_type = END_STUB;		// ends a stub (no via or tee)
			}
			else if( is == seg.size()-1 )
			{
				// last segment of regular trace
				post_layer = vtx[is+1].pad_layer;
				if( post_layer == LAY_PAD_THRU )
					post_type = THRU_PIN;		// ends on a thru pin
				else
					post_type = SMT_PIN;		// ends on a SMT pin
			}
			else
			{
				// not last segment
				post_layer = seg[is+1].layer;
				if( vtx[is+1].isTee )
					post_type = TEE;				// ends on a tee-vertex
				else if( vtx[is+1].via_w )
					post_type = VIA;				// ends on a via
				else
					post_type = SEGMENT;			// ends on a segment
			}
			// OK, now see if we can remove the zero-length segment by removing
			// the starting vertex
			bool bRemove = false;
			if( pre_type == SEGMENT && pre_layer == layer
				|| pre_type == SEGMENT && layer == LAY_RAT_LINE
				|| pre_type == VIA && post_type == VIA
				|| pre_type == VIA && post_type == THRU_PIN
				|| post_type == END_STUB )
			{
				// remove starting vertex
				vtx.removeAt(is);
				bRemove = true;
			}
			else if( post_type == SEGMENT && post_layer == layer
				|| post_type == SEGMENT && layer == LAY_RAT_LINE
				|| post_type == VIA && pre_type == THRU_PIN )
			{
				// remove following vertex
				vtx.removeAt(is+1);
				bRemove = true;
			}
			if( bRemove )
			{
				seg.removeAt(is);
#if 0
				if( logstr )
				{
					CString str;
					if( c->end_pin == cconnect::NO_END )
					{
						str.Format( "net %s: stub trace from %s.%s: removing zero-length segment\r\n",
							net->name,
							net->pin[c->start_pin].ref_des, net->pin[c->start_pin].pin_name );
					}
					else
					{
						str.Format( "net %s: trace %s.%s to %s.%s: removing zero-length segment\r\n",
							net->name,
							net->pin[c->start_pin].ref_des, net->pin[c->start_pin].pin_name,
							net->pin[c->end_pin].ref_des, net->pin[c->end_pin].pin_name );
					}
					*logstr += str;
				}
#endif
			}
		}
	}
#if 0
	// see if there are any segments left
	if( seg.size() == 0 )
	{
		// no, remove connection
		bConnectionRemoved = true;
		net->connect.RemoveAt(ic);
		net->nconnects--;
	}
#endif
	if (seg.size())
	{
		// look for segments on same layer, with same width,
		// not separated by a tee or via
		for( int is=seg.size()-2; is>=0; is-- )
		{
			if( seg[is].layer == seg[is+1].layer
				&& seg[is].width == seg[is+1].width
				&& vtx[is+1].via_w == 0
				&& vtx[is+1].isTee == false )
			{
				// see if colinear
				double dx1 = vtx[is+1].x - vtx[is].x;
				double dy1 = vtx[is+1].y - vtx[is].y;
				double dx2 = vtx[is+2].x - vtx[is+1].x;
				double dy2 = vtx[is+2].y - vtx[is+1].y;
				if( dy1*dx2 == dy2*dx1 && (dx1*dx2>0.0 || dy1*dy2>0.0) )
				{
					// yes, combine these segments
#if 0
					if( logstr )
					{
						CString str;
						if( c->end_pin == cconnect::NO_END )
						{
							str.Format( "net %s: stub trace from %s.%s: combining colinear segments\r\n",
								net->name,
								net->pin[c->start_pin].ref_des, net->pin[c->start_pin].pin_name );
						}
						else
						{
							str.Format( "net %s: trace %s.%s to %s.%s: combining colinear segments\r\n",
								net->name,
								net->pin[c->start_pin].ref_des, net->pin[c->start_pin].pin_name,
								net->pin[c->end_pin].ref_des, net->pin[c->end_pin].pin_name );
						}
						*logstr += str;
					}
#endif
					vtx.removeAt(is+1);
					seg.removeAt(is+1);
				}
			}
		}
		// now check for non-branch stubs with a single unrouted segment and no end-via
		if( end_pin == NULL && seg.size() == 1 )
		{
			Vertex * end_v = &vtx[seg.size()];
			Segment * end_s = &seg[seg.size()-1];
			if( !end_v->isTee && end_v->via_w == 0 && end_s->layer == LAY_RAT_LINE )
			{
				// remove segment and vertices, connect will be deleted automatically
				seg.clear();
				vtx.clear();
#if 0
				if( logstr )
				{
					CString str;
					str.Format( "net %s: stub trace from %s.%s: single unrouted segment and no end via, removed\r\n",
						net->name,
						net->pin[c->start_pin].ref_des, net->pin[c->start_pin].pin_name );
					*logstr += str;
				}

				bConnectionRemoved = true;
				net->connect.RemoveAt(ic);
				net->nconnects--;
#endif
			}
		}
#if 0
		if( !bConnectionRemoved && m_dlist )
			DrawConnection( net, ic );
#endif
	}
}

// Add new net to netlist
//
Net::Net( QString name, int def_w, int def_via_w, int def_via_hole_w )
{
	// set default trace width
	this->def_w = def_w;
	this->def_via_w = def_via_w;
	this->def_via_hole_w = def_via_hole_w;

	// create id and set name
	id id( ID_NET, 0 );
	this->id = id;
	this->name = name;

	// visible by default
	this->visible = true;
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
		Connection * c =  &net->connect[ic];
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
	Connection c(start, end);
	connect.append(c);
}

// Remove connection from net
// Does not remove any orphaned branches that result
// Leave pins in pin list for net
//
void Net::RemoveConnect( int ic, bool set_areas )
{
	Connection * c = &connect[ic];
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
