#include "Trace.h"
#include "Area.h"

static void vertexDFS(QSet<Vertex> &toVisit, QSet<Vertex> &currSet, Vertex *currVtx);


QSet<Vertex*> TraceList::getConnectedVertices(Vertex* vtx)
{
	foreach(QSet<Vertex*> set, mConnections)
	{
		if (set.contains(vtx))
			return set;
	}
	return QSet<Vertex*>();
}

QSet<Vertex*> TraceList::getVerticesInArea(const Area& a)
{
	PCBLAYER layer = a.layer();
	QSet<Vertex*> set;
	foreach(Vertex* vtx, myVtx)
	{
		if (vtx->onLayer(layer)  && a.pointInside(vtx->pos()))
			set.insert(vtx);
	}
	return set;
}


void TraceList::rebuildConnectionList()
{
	QSet<Vertex*> toVisit(myVtx);
	QSet<Vertex*> currSet;
	this->mConnections.clear();
	while(!toVisit.empty())
	{
		currSet.clear();
		Vertex* vtx = *toVisit.begin();
		toVisit.remove(vtx);
		vertexDFS(toVisit, currSet, vtx);
		mConnections.append(currSet);
	}
}

void vertexDFS(QSet<Vertex> &toVisit, QSet<Vertex> &currSet, Vertex *currVtx)
{
	foreach(Segment* seg, currVtx.segments())
	{
		Vertex* vtx = seg->otherVertex(currVtx);
		if (toVisit.contains(vtx))
		{
			toVisit.remove(vtx);
			currSet.insert(vtx);
			vertexDFS(toVisit, currSet, vtx);
		}

	}
}

Trace::Trace(PartPin* start, PartPin* end)
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

void Trace::cleanup()
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


// Unroute all segments of a connection and merge if possible
// Preserves tees
//
void Trace::Unroute( )
{
	for( int is=0; is<seg.size(); is++ )
		UnrouteSegmentWithoutMerge( net, ic, is );
	MergeUnroutedSegments( net, ic );
	return 0;
}

// Unroute segment
// return id of new segment (since seg[] array may change as a result)
//
// Unroute segment, but don't merge with adjacent unrouted segments
// Assume that there will be an eventual call to MergeUnroutedSegments() to set vias
//
void Trace::UnrouteSegment( int segIndex, bool mergeUnrouted )
{
	// unroute segment
	seg[segIndex].Unroute();

	ReconcileVia( net, ic, is );
	ReconcileVia( net, ic, is+1 );

	if (mergeUnrouted)
	{
		MergeUnroutedSegments();
	}
}


// Reconcile via with preceding and following segments
// if a via is needed, use defaults for adjacent segments
//
int Trace::ReconcileVia( int ivtx )
{
	cconnect * c = &net->connect[ic];
	cvertex * v = &c->vtx[ivtx];
	bool via_needed = false;
	// see if via needed
	if( v->force_via_flag )
	{
		via_needed = 1;
	}
	else
	{
		if( c->end_pin == cconnect::NO_END && ivtx == c->nsegs )
		{
			// end vertex of a stub trace
			if( v->tee_ID )
			{
				// this is a branch, reconcile the main tee
				int tee_ic;
				int tee_iv;
				bool bFound = FindTeeVertexInNet( net, v->tee_ID, &tee_ic, &tee_iv );
				if( bFound )
					ReconcileVia( net, tee_ic, tee_iv );
			}
		}
		else if( ivtx == 0 || ivtx == c->nsegs )
		{
			// first and last vertex are part pads
			return 0;
		}
		else if( v->tee_ID )
		{
			if( TeeViaNeeded( net, v->tee_ID ) )
				via_needed = true;
		}
		else
		{
			c->vtx[ivtx].pad_layer = 0;
			cseg * s1 = &c->seg[ivtx-1];
			cseg * s2 = &c->seg[ivtx];
			if( s1->layer != s2->layer && s1->layer != LAY_RAT_LINE && s2->layer != LAY_RAT_LINE )
			{
				via_needed = true;
			}
		}
	}

	if( via_needed )
	{
		// via needed, make sure it exists or create it
		if( v->via_w == 0 || v->via_hole_w == 0 )
		{
			// via doesn't already exist, set via width and hole width
			int w, via_w, via_hole_w;
			GetWidths( net, &w, &via_w, &via_hole_w );
			// set parameters for via
			v->via_w = via_w;
			v->via_hole_w = via_hole_w;
		}
	}
	else
	{
		// via not needed
		v->via_w = 0;
		v->via_hole_w = 0;
	}
	if( m_dlist )
		DrawVia( net, ic, ivtx );
	return 0;
}

id CNetList::UnrouteSegment( cnet * net, int ic, int is )
{
	id seg_id = id(ID_NET, ID_CONNECT, ic, ID_SEG, is );
	UnrouteSegmentWithoutMerge( is );
	id mid = MergeUnroutedSegments( net, ic );
	if( mid.type == 0 )
		mid = seg_id;
	return mid;
}

/////////////////////////////////////////// SEGMENT //////////////////////////////////////////////////////////////

Segment::Segment(Trace* parent, Vertex* v1, Vertex* v2, PCBLAYER l, int w)
	: myParent(parent), myV1(v1), myV2(v2), myLayer(l), myWidth(w)
{
	myV1->addSegment(this);
	myV2->addSegment(this);
}

Segment::~Segment()
{
	myV1->removeSegment(this);
	myV2->removeSegment(this);
}


bool Vertex::isVia()
{
	PCBLAYER layer = 0;
	foreach(Segment* seg, this->mSegs)
	{
		if (layer && seg->layer() != layer)
			return true; // consecutive segments on different layers
		layer = seg->layer;
	}
	// all segments on the same layer
	return false;
}


bool Vertex::onLayer(PCBLAYER layer)
{
	// vias are present on all layers
	if (isVia())
		return true;
	else
	{
		// not a via, all segments must be on same layer
		Segment* first = *this->mSegs.begin();
		if (layer == first->layer())
			return true;
		else
			return false;
	}
}
