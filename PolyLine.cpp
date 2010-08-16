// PolyLine.cpp ... implementation of CPolyLine class

#include <math.h>
#include "PolyLine.h"

#define pi  3.14159265359

// draw hatch lines
//
void Polygon::Hatch()
{
#if 0
	if( m_hatch == NO_HATCH )
	{
		m_nhatch = 0;
		return;
	}

	if( m_dlist && GetClosed() )
	{
		enum {
			MAXPTS = 100,
			MAXLINES = 1000
		};
		dl_hatch.SetSize( MAXLINES, MAXLINES );
		int xx[MAXPTS], yy[MAXPTS];

		// define range for hatch lines
		int min_x = corner[0].x;
		int max_x = corner[0].x;
		int min_y = corner[0].y;
		int max_y = corner[0].y;
		for( int ic=1; ic<m_ncorners; ic++ )
		{
			if( corner[ic].x < min_x )
				min_x = corner[ic].x;
			if( corner[ic].x > max_x )
				max_x = corner[ic].x;
			if( corner[ic].y < min_y )
				min_y = corner[ic].y;
			if( corner[ic].y > max_y )
				max_y = corner[ic].y;
		}
		int slope_flag = 1 - 2*(m_layer%2);	// 1 or -1
		double slope = 0.707106*slope_flag;
		int spacing;
		if( m_hatch == DIAGONAL_EDGE )
			spacing = 10*PCBU_PER_MIL;
		else
			spacing = 50*PCBU_PER_MIL;
		int max_a, min_a;
		if( slope_flag == 1 )
		{
			max_a = (int)(max_y - slope*min_x);
			min_a = (int)(min_y - slope*max_x);
		}
		else
		{
			max_a = (int)(max_y - slope*max_x);
			min_a = (int)(min_y - slope*min_x);
		}
		min_a = (min_a/spacing)*spacing;
		int offset;
		if( m_layer < (LAY_TOP_COPPER+2) )
			offset = 0;
		else if( m_layer < (LAY_TOP_COPPER+4) )
			offset = spacing/2;
		else if( m_layer < (LAY_TOP_COPPER+6) )
			offset = spacing/4;
		else if( m_layer < (LAY_TOP_COPPER+8) )
			offset = 3*spacing/4;
		else if( m_layer < (LAY_TOP_COPPER+10) )
			offset = 1*spacing/8;
		else if( m_layer < (LAY_TOP_COPPER+12) )
			offset = 3*spacing/8;
		else if( m_layer < (LAY_TOP_COPPER+14) )
			offset = 5*spacing/8;
		else if( m_layer < (LAY_TOP_COPPER+16) )
			offset = 7*spacing/8;
		else
			ASSERT(0);
		min_a += offset;

		// now calculate and draw hatch lines
		int nc = m_ncorners;
		int nhatch = 0;
		// loop through hatch lines
		for( int a=min_a; a<max_a; a+=spacing )
		{
			// get intersection points for this hatch line
			int nloops = 0;
			int npts;
			// make this a loop in case my homebrew hatching algorithm screws up
			do
			{
				npts = 0;
				int i_start_contour = 0;
				for( int ic=0; ic<nc; ic++ )
				{
					double x, y, x2, y2;
					int ok;
					if( corner[ic].end_contour )
					{
						ok = FindLineSegmentIntersection( a, slope, 
								corner[ic].x, corner[ic].y,
								corner[i_start_contour].x, corner[i_start_contour].y, 
								side_style[ic],
								&x, &y, &x2, &y2 );
						i_start_contour = ic + 1;
					}
					else
					{
						ok = FindLineSegmentIntersection( a, slope, 
								corner[ic].x, corner[ic].y, 
								corner[ic+1].x, corner[ic+1].y,
								side_style[ic],
								&x, &y, &x2, &y2 );
					}
					if( ok )
					{
						xx[npts] = (int)x;
						yy[npts] = (int)y;
						npts++;
						ASSERT( npts<MAXPTS );	// overflow
					}
					if( ok == 2 )
					{
						xx[npts] = (int)x2;
						yy[npts] = (int)y2;
						npts++;
						ASSERT( npts<MAXPTS );	// overflow
					}
				}
				nloops++;
				a += PCBU_PER_MIL/100;
			} while( npts%2 != 0 && nloops < 3 );
			ASSERT( npts%2==0 );	// odd number of intersection points, error

			// sort points in order of descending x (if more than 2)
			if( npts>2 )
			{
				for( int istart=0; istart<(npts-1); istart++ )
				{
					int max_x = INT_MIN;
					int imax;
					for( int i=istart; i<npts; i++ )
					{
						if( xx[i] > max_x )
						{
							max_x = xx[i];
							imax = i;
						}
					}
					int temp = xx[istart];
					xx[istart] = xx[imax];
					xx[imax] = temp;
					temp = yy[istart];
					yy[istart] = yy[imax];
					yy[imax] = temp;
				}
			}

			// draw lines
			for( int ip=0; ip<npts; ip+=2 )
			{
				id hatch_id = m_id;
				hatch_id.sst = ID_HATCH;
				hatch_id.ii = nhatch;
				double dx = xx[ip+1] - xx[ip];
				if( m_hatch == DIAGONAL_FULL || fabs(dx) < 40*NM_PER_MIL )
				{
					dl_element * dl = m_dlist->Add( hatch_id, 0, m_layer, DL_LINE, 1, 0, 0, 
						xx[ip], yy[ip], xx[ip+1], yy[ip+1], 0, 0 );
					dl_hatch.SetAtGrow(nhatch, dl);
					nhatch++;
				}
				else
				{
					double dy = yy[ip+1] - yy[ip];	
					double slope = dy/dx;
					if( dx > 0 )
						dx = 20*NM_PER_MIL;
					else
						dx = -20*NM_PER_MIL;
					double x1 = xx[ip] + dx;
					double x2 = xx[ip+1] - dx;
					double y1 = yy[ip] + dx*slope;
					double y2 = yy[ip+1] - dx*slope;
					dl_element * dl = m_dlist->Add( hatch_id, 0, m_layer, DL_LINE, 1, 0, 0, 
						xx[ip], yy[ip], x1, y1, 0, 0 );
					dl_hatch.SetAtGrow(nhatch, dl);
					dl = m_dlist->Add( hatch_id, 0, m_layer, DL_LINE, 1, 0, 0, 
						xx[ip+1], yy[ip+1], x2, y2, 0, 0 );
					dl_hatch.SetAtGrow(nhatch+1, dl);
					nhatch += 2;
				}
			}
		} // end for 
		m_nhatch = nhatch;
		dl_hatch.SetSize( m_nhatch );
	}
#endif
}






// Create CPolyLine for a pad
//
Polygon * Polygon::MakePolylineForPad( int type, int x, int y, int w, int l, int r, int angle )
{
	Polygon * poly = new Polygon;
	int dx = l/2;
	int dy = w/2;
	if( angle%180 == 90 )
	{
		dx = w/2;
		dy = l/2;
	}
	if( type == PAD_ROUND )
	{
		poly->Start( 0, 0, 0, x-dx, y, 0, NULL, NULL );
		poly->AppendCorner( x, y+dy, ARC_CW, 0 );
		poly->AppendCorner( x+dx, y, ARC_CW, 0 );
		poly->AppendCorner( x, y-dy, ARC_CW, 0 );
		poly->Close( ARC_CW );
	}
	return poly;
}

// Add cutout for a pad
// Convert arcs to multiple straight lines
// Do NOT draw or undraw
//
void Polygon::AddContourForPadClearance( int type, int x, int y, int w, 
						int l, int r, int angle, int fill_clearance,
						int hole_w, int hole_clearance, bool bThermal, int spoke_w )
{
	int dx = l/2;
	int dy = w/2;
	if( angle%180 == 90 )
	{
		dx = w/2;
		dy = l/2;
	}
	int x_clearance = max( fill_clearance, hole_clearance+hole_w/2-dx);		
	int y_clearance = max( fill_clearance, hole_clearance+hole_w/2-dy);
	dx += x_clearance;
	dy += y_clearance;
	if( !bThermal )
	{
		// normal clearance
		if( type == PAD_ROUND || (type == PAD_NONE && hole_w > 0) )
		{
			AppendCorner( x-dx, y, ARC_CW, 0 );
			AppendCorner( x, y+dy, ARC_CW, 0 );
			AppendCorner( x+dx, y, ARC_CW, 0 );
			AppendCorner( x, y-dy, ARC_CW, 0 );
			Close( ARC_CW ); 
		}
		else if( type == PAD_SQUARE || type == PAD_RECT 
			|| type == PAD_RRECT || type == PAD_OVAL )
		{
			AppendCorner( x-dx, y-dy, STRAIGHT, 0 );
			AppendCorner( x+dx, y-dy, STRAIGHT, 0 );
			AppendCorner( x+dx, y+dy, STRAIGHT, 0 );
			AppendCorner( x-dx, y+dy, STRAIGHT, 0 );
			Close( STRAIGHT ); 
		}
	}
	else
	{
		// thermal relief
		if( type == PAD_ROUND || (type == PAD_NONE && hole_w > 0) )
		{
			// draw 4 "wedges"
			double r = max(w/2 + fill_clearance, hole_w/2 + hole_clearance);
			double start_angle = asin( spoke_w/(2.0*r) );
			double th1, th2, corner_x, corner_y;
			for( int i=0; i<4; i++ )
			{
				if( i == 0 )
				{
					corner_x = spoke_w/2;
					corner_y = spoke_w/2;
					th1 = start_angle;
					th2 = pi/2.0 - start_angle;
				}
				else if( i == 1 )
				{
					corner_x = -spoke_w/2;
					corner_y = spoke_w/2;
					th1 = pi/2.0 + start_angle;
					th2 = pi - start_angle;
				}
				else if( i == 2 )
				{
					corner_x = -spoke_w/2;
					corner_y = -spoke_w/2;
					th1 = -pi + start_angle;
					th2 = -pi/2.0 - start_angle;
				}
				else if( i == 3 )
				{
					corner_x = spoke_w/2;
					corner_y = -spoke_w/2;
					th1 = -pi/2.0 + start_angle;
					th2 = -start_angle;
				}
				AppendCorner( x+corner_x, y+corner_y, STRAIGHT, 0 );
				AppendCorner( x+r*cos(th1), y+r*sin(th1),  STRAIGHT, 0 );
				AppendCorner( x+r*cos(th2), y+r*sin(th2),  ARC_CCW, 0 );
				Close( STRAIGHT );
			}
		}
		else if( type == PAD_SQUARE || type == PAD_RECT 
			|| type == PAD_RRECT || type == PAD_OVAL )
		{
			// draw 4 rectangles
			int xL = x - dx;
			int xR = x - spoke_w/2;
			int yB = y - dy;
			int yT = y - spoke_w/2;
			AppendCorner( xL, yB, STRAIGHT, 0 );
			AppendCorner( xR, yB, STRAIGHT, 0 );
			AppendCorner( xR, yT, STRAIGHT, 0 );
			AppendCorner( xL, yT, STRAIGHT, 0 );
			Close( STRAIGHT ); 
			xL = x + spoke_w/2;
			xR = x + dx;
			AppendCorner( xL, yB, STRAIGHT, 0 );
			AppendCorner( xR, yB, STRAIGHT, 0 );
			AppendCorner( xR, yT, STRAIGHT, 0 );
			AppendCorner( xL, yT, STRAIGHT, 0 );
			Close( STRAIGHT ); 
			xL = x - dx;
			xR = x - spoke_w/2;
			yB = y + spoke_w/2;
			yT = y + dy;
			AppendCorner( xL, yB, STRAIGHT, 0 );
			AppendCorner( xR, yB, STRAIGHT, 0 );
			AppendCorner( xR, yT, STRAIGHT, 0 );
			AppendCorner( xL, yT, STRAIGHT, 0 );
			Close( STRAIGHT ); 
			xL = x + spoke_w/2;
			xR = x + dx;
			AppendCorner( xL, yB, STRAIGHT, 0 );
			AppendCorner( xR, yB, STRAIGHT, 0 );
			AppendCorner( xR, yT, STRAIGHT, 0 );
			AppendCorner( xL, yT, STRAIGHT, 0 );
			Close( STRAIGHT ); 
		}
	}
}

// Functions migrated from netlist


#if 0
// XXX TODO this needs to be moved to the CArea class or something
/// Process an area that has been modified, by clipping its polygon against
/// itself and the polygons for any other areas on the same net.
/// This may change the number and order of copper areas in the net.
/// \returns
///	-1 if arcs intersect other sides, so polygon can't be clipped
///	 0 if no intersecting sides
///	 1 if intersecting sides, polygon clipped
int PolyLine::OnModified()
{
	// clip polygon against itself
	int test = Clip();
	if( test == -1 )
		return test;
	// now see if we need to clip against other areas
	bool bCheckAllAreas = false;
	if( test == 1 )
		bCheckAllAreas = true;
	else
		bCheckAllAreas = TestAreaIntersections( net, iarea );
	if( bCheckAllAreas )
		CombineAllAreasInNet( net, bMessageBoxInt, true );
	SetAreaConnections( net );
	return test;
}
#endif

#if 0
// XXX TODO move this to area class

// Checks all copper areas in net for intersections, combining them if found
// If bUseUtility == true, don't check areas if both utility flags are 0
// Sets utility flag = 1 for any areas modified
// If an area has self-intersecting arcs, doesn't try to combine it
//
int CNetList::CombineAllAreasInNet( cnet * net, bool bMessageBox, bool bUseUtility )
{
	if( net->nareas > 1 )
	{
		// start by testing all area polygons to set utility2 flags
		for( int ia=0; ia<net->nareas; ia++ )
			TestAreaPolygon( net, ia );
		// now loop through all combinations
		bool message_shown = false;
		for( int ia1=0; ia1<net->nareas-1; ia1++ )
		{
			// legal polygon
			CRect b1 = net->area[ia1].poly->GetCornerBounds();
			bool mod_ia1 = false;
			for( int ia2=net->nareas-1; ia2 > ia1; ia2-- )
			{
				if( net->area[ia1].poly->GetLayer() == net->area[ia2].poly->GetLayer()
					&& net->area[ia1].utility2 != -1 && net->area[ia2].utility2 != -1 )
				{
					CRect b2 = net->area[ia2].poly->GetCornerBounds();
					if( !( b1.left > b2.right || b1.right < b2.left
						|| b1.bottom > b2.top || b1.top < b2.bottom ) )
					{
						// check ia2 against 1a1
						if( net->area[ia1].utility || net->area[ia2].utility || bUseUtility == false )
						{
							int ret = TestAreaIntersection( net, ia1, ia2 );
							if( ret == 1 )
								ret = CombineAreas( net, ia1, ia2 );
							if( ret == 1 )
							{
								if( bMessageBox && bDontShowIntersectionWarning == false )
								{
									CString str;
									str.Format( "Areas %d and %d of net \"%s\" intersect and will be combined.\n",
										ia1+1, ia2+1, net->name );
									str += "If they are complex, this may take a few seconds.";
									CDlgMyMessageBox dlg;
									dlg.Initialize( str );
									dlg.DoModal();
									bDontShowIntersectionWarning = dlg.bDontShowBoxState;
								}
								mod_ia1 = true;
							}
							else if( ret == 2 )
							{
								if( bMessageBox && bDontShowIntersectionArcsWarning == false )
								{
									CString str;
									str.Format( "Areas %d and %d of net \"%s\" intersect, but some of the intersecting sides are arcs.\n",
										ia1+1, ia2+1, net->name );
									str += "Therefore, these areas can't be combined.";
									CDlgMyMessageBox dlg;
									dlg.Initialize( str );
									dlg.DoModal();
									bDontShowIntersectionArcsWarning = dlg.bDontShowBoxState;
								}
							}
						}
					}
				}
			}
			if( mod_ia1 )
				ia1--;		// if modified, we need to check it again
		}
	}
	return 0;
}
#endif




#if 0
/// Combines this polygon with the supplied one, if possible.
/// returns: 0 if no intersection
///			1 if intersection
int PolyLine::Combine( const PolyLine &other )
{
	// save arcs when making GPC poly
	QList<CArc> arc_array1;
	QList<CArc> arc_array2;

	MakeGpcPoly( -1, &arc_array1 );
	other.MakeGpcPoly( -1, &arc_array2 );
	int n_ext_cont1 = 0;
	for( int ic=0; ic<GetGpcPoly()->num_contours; ic++ )
		if( !((GetGpcPoly()->hole)[ic]) )
			n_ext_cont1++;
	int n_ext_cont2 = 0;
	for( int ic=0; ic<other.GetGpcPoly()->num_contours; ic++ )
		if( !((other.GetGpcPoly()->hole)[ic]) )
			n_ext_cont2++;

	gpc_polygon * union_gpc = new gpc_polygon;
	gpc_polygon_clip( GPC_UNION, poly1->GetGpcPoly(), poly2->GetGpcPoly(), union_gpc );

	// get number of outside contours
	int n_union_ext_cont = 0;
	for( int ic=0; ic<union_gpc->num_contours; ic++ )
		if( !((union_gpc->hole)[ic]) )
			n_union_ext_cont++;

	// if no intersection, free new gpc and return
	if( n_union_ext_cont == n_ext_cont1 + n_ext_cont2 )
	{
		gpc_free_polygon( union_gpc );
		delete union_gpc;
		return 0;
	}

	// create area with external contour
	for( int ic=0; ic<union_gpc->num_contours; ic++ )
	{
		if( !(union_gpc->hole)[ic] )
		{
			// external contour, replace this poly
			mCorners.clear();
			mSides.clear();

			for( int i=0; i<union_gpc->contour[ic].num_vertices; i++ )
			{
				int x = ((union_gpc->contour)[ic].vertex)[i].x;
				int y = ((union_gpc->contour)[ic].vertex)[i].y;
				if( i==0 )
				{
					mCorners.append(CPolyPt(x, y));
				}
				else
					AppendCorner( x, y, PolyLine::STRAIGHT );
			}

			Close( PolyLine::STRAIGHT );
		}
	}
	// add holes
	for( int ic=0; ic<union_gpc->num_contours; ic++ )
	{
		if( (union_gpc->hole)[ic] )
		{
			// hole
			for( int i=0; i<union_gpc->contour[ic].num_vertices; i++ )
			{
				int x = ((union_gpc->contour)[ic].vertex)[i].x;
				int y = ((union_gpc->contour)[ic].vertex)[i].y;
				AppendCorner(x, y, PolyLine::STRAIGHT );
			}
			Close( PolyLine::STRAIGHT );
		}
	}
	RestoreArcs( &arc_array1 );
	RestoreArcs( &arc_array2 );
	gpc_free_polygon( union_gpc );
	delete union_gpc;
	return 1;
}
#endif



