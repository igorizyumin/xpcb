// PolyLine.cpp ... implementation of CPolyLine class

#include "math.h"
#include "PolyLine.h"
#include "utility.h"
#include "gpc.h"


#define pi  3.14159265359

PolyLine::PolyLine()
	: m_ncorners(0), mHatch(0)
{ 
}

// destructor, removes display elements
//
PolyLine::~PolyLine()
{
}



// Restore arcs to a polygon where they were replaced with steps
// If pa != NULL, also use polygons in pa array
//
int PolyLine::RestoreArcs( QList<CArc> * arc_array)
{
	// get poly info
	int n_polys = 1;
	PolyLine * poly;

	// clear utility flag for all corners
	for( int ip=0; ip<n_polys; ip++ )
	{
		if( ip == 0 )
			poly = this;
		else
			poly = (*pa)[ip-1];
		for( int ic=0; ic<poly->GetNumCorners(); ic++ )
			poly->SetUtility( ic, 0 );	// clear utility flag
	}

	// find arcs and replace them
	bool bFound;
	int arc_start;
	int arc_end;
	for( int iarc=0; iarc<arc_array->length(); iarc++ )
	{
		CArc arc = arc_array->at(iarc);
		int arc_xi = arc.xi;
		int arc_yi = arc.yi;
		int arc_xf = arc.xf;
		int arc_yf = arc.yf;
		int n_steps = arc.n_steps;
		int style = arc.style;
		bFound = false;
		// loop through polys
		for( int ip=0; ip<n_polys; ip++ )
		{
			if( ip == 0 )
				poly = this;
			else
				poly = (*pa)[ip-1];
			for( int icont=0; icont<poly->GetNumContours(); icont++ )
			{
				int ic_start = poly->GetContourStart(icont);
				int ic_end = poly->GetContourEnd(icont);
				if( (ic_end-ic_start) > n_steps )
				{
					for( int ic=ic_start; ic<=ic_end; ic++ )
					{
						int ic_next = ic+1;
						if( ic_next > ic_end )
							ic_next = ic_start;
						int xi = poly->GetX(ic);
						int yi = poly->GetY(ic);
						if( xi == arc_xi && yi == arc_yi )
						{
							// test for forward arc
							int ic2 = ic + n_steps;
							if( ic2 > ic_end )
								ic2 = ic2 - ic_end + ic_start - 1;
							int xf = poly->GetX(ic2);
							int yf = poly->GetY(ic2);
							if( xf == arc_xf && yf == arc_yf )
							{
								// arc from ic to ic2
								bFound = true;
								arc_start = ic;
								arc_end = ic2;
							}
							else
							{
								// try reverse arc
								ic2 = ic - n_steps;
								if( ic2 < ic_start )
									ic2 = ic2 - ic_start + ic_end + 1;
								xf = poly->GetX(ic2);
								yf = poly->GetY(ic2);
								if( xf == arc_xf && yf == arc_yf )
								{
									// arc from ic2 to ic
									bFound = true;
									arc_start = ic2;
									arc_end = ic;
									style = 3 - style;
								}
							}
							if( bFound )
							{
								poly->mSides[arc_start] = style;
								// mark corners for deletion from arc_start+1 to arc_end-1
								for( int i=arc_start+1; i!=arc_end; )
								{
									if( i > ic_end )
										i = ic_start;
									poly->SetUtility( i, 1 );
									if( i == ic_end )
										i = ic_start;
									else
										i++;
								}
								break;
							}
						}
						if( bFound )
							break;
					}
				}
				if( bFound )
					break;
			}
		}
		if( bFound )
			(*arc_array)[iarc].bFound = true;
	}

	// now delete all marked corners
	for( int ip=0; ip<n_polys; ip++ )
	{
		if( ip == 0 )
			poly = this;
		else
			poly = (*pa)[ip-1];
		for( int ic=poly->GetNumCorners()-1; ic>=0; ic-- )
		{
			if( poly->GetUtility(ic) )
				poly->DeleteCorner( ic, false );
		}
	}
	return 0;
}

// initialize new polyline
// set layer, width, selection box size, starting point, id and pointer
//
// if sel_box = 0, don't create selection elements at all
//
// if polyline is board outline, enter with:
//	id.type = ID_BOARD
//	id.st = ID_BOARD_OUTLINE
//	id.i = 0
//	ptr = NULL
//
// if polyline is copper area, enter with:
//	id.type = ID_NET;
//	id.st = ID_AREA
//	id.i = index to area
//	ptr = pointer to net
//
void PolyLine::Start( int layer, int w, int sel_box, int x, int y, 
					  int hatch )
{
	mLayer = layer;
	mWidth = w;
	mHatch = hatch;

	mCorners.append(CPolyPt(x,y));
}

// add a corner to unclosed polyline
//
void PolyLine::AppendCorner( int x, int y, int style)
{
	// add entries for new corner and side
	if( !mCorners.last().end_contour )
	{
		mSides.pop_back();
		mSides.push_back(style);
	}

	mCorners.append(CPolyPt(x,y));

	ASSERT( style == PolyLine::STRAIGHT ||
			style == PolyLine::ARC_CW ||
			style == PolyLine::ARC_CCW );

}

// close last polyline contour
//
void PolyLine::Close( int style )
{
	ASSERT( !GetClosed() );

	mSides.pop_back();
	mSides.push_back(style);
	mCorners.last().end_contour = true;
}

// move corner of polyline
//
void PolyLine::MoveCorner( int ic, int x, int y )
{
	mCorners[ic].x = x;
	mCorners[ic].y = y;
}

// delete corner and adjust arrays
//
void PolyLine::DeleteCorner( int ic )
{
	int icont = GetContour( ic );
	int istart = GetContourStart( icont );
	int iend = GetContourEnd( icont );
	bool bClosed = icont < GetNumContours()-1 || GetClosed();

	if( !bClosed )
	{
		// open contour, must be last contour
		mCorners.removeAt( ic );
		if( ic != istart )
			mSides.removeAt( ic-1 );
	}
	else
	{
		// closed contour
		mCorners.removeAt( ic );
		mSides.removeAt( ic );
		if( ic == iend )
			mCorners[ic-1].end_contour = true;
	}
	if( bClosed && GetContourSize(icont) < 3 )
	{
		// delete the entire contour
		RemoveContour( icont );
	}
}

void PolyLine::RemoveContour( int icont )
{
	int istart = GetContourStart( icont );
	int iend = GetContourEnd( icont );
	int numc = GetNumContours();

	// remove the only contour
	ASSERT(!( numc == 1 ) );

	if (numc > 1)
	{
		// remove closed contour
		for( int ic=iend; ic>=istart; ic-- )
		{
			mCorners.removeAt( ic );
			mSides.removeAt( ic );
		}
	}
}

// insert a new corner between two existing corners
//
void PolyLine::InsertCorner( int ic, int x, int y )
{
	mCorners.insert( ic, CPolyPt(x,y) );
	mSides.insert( ic, STRAIGHT );
	if( ic )
	{
		if( mCorners[ic-1].end_contour )
		{
			mCorners[ic].end_contour = true;
			mCorners[ic-1].end_contour = false;
		}
	}
}

// draw polyline
// if side style is ARC_CW or ARC_CCW but endpoints are not angled,
// convert to STRAIGHT
//
void PolyLine::Draw(  QPainter * painter )
{
#if 0
	int i_start_contour = 0;

	if( m_dlist )
	{
		// now draw elements
		for( int ic=0; ic<m_ncorners; ic++ )
		{
			m_id.ii = ic;
			int xi = corner[ic].x;
			int yi = corner[ic].y;
			int xf, yf;
			if( corner[ic].end_contour == false && ic < m_ncorners-1 )
			{
				xf = corner[ic+1].x;
				yf = corner[ic+1].y;
			}
			else
			{
				xf = corner[i_start_contour].x;
				yf = corner[i_start_contour].y;
				i_start_contour = ic+1;
			}
			// draw
			if( m_sel_box )
			{
				m_id.sst = ID_SEL_CORNER;
				dl_corner_sel[ic] = m_dlist->AddSelector( m_id, m_ptr, m_layer, DL_HOLLOW_RECT, 
					1, 0, 0, xi-m_sel_box, yi-m_sel_box, 
					xi+m_sel_box, yi+m_sel_box, 0, 0 );
			}
			if( ic<(m_ncorners-1) || corner[ic].end_contour )
			{
				// draw side
				if( xi == xf || yi == yf )
				{
					// if endpoints not angled, make side STRAIGHT
					side_style[ic] = STRAIGHT;
				}
				int g_type = DL_LINE;
				if( side_style[ic] == STRAIGHT )
					g_type = DL_LINE;
				else if( side_style[ic] == ARC_CW )
					g_type = DL_ARC_CW;
				else if( side_style[ic] == ARC_CCW )
					g_type = DL_ARC_CCW;
				m_id.sst = ID_SIDE;
				dl_side[ic] = m_dlist->Add( m_id, m_ptr, m_layer, g_type, 
					1, m_w, 0, xi, yi, xf, yf, 0, 0 );
				if( m_sel_box )
				{
					m_id.sst = ID_SEL_SIDE;
					dl_side_sel[ic] = m_dlist->AddSelector( m_id, m_ptr, m_layer, g_type, 
						1, m_w, 0, xi, yi, xf, yf, 0, 0 );
				}
			}
		}
		if( m_hatch )
			Hatch();
	}
	bDrawn = true;
#endif
}

QRect PolyLine::GetBounds()
{
	QRect r = GetCornerBounds();
	// XXX IGOR XXX
	// check this
	r.adjust(-mWidth/2, mWidth/2, mWidth/2, -mWidth/2 );
	return r;
}

QRect PolyLine::GetCornerBounds()
{
	int left, bottom, right, top;
	left = bottom = INT_MAX;
	right = top = INT_MIN;
	for( int i=0; i<m_ncorners; i++ )
	{
		left = min( left, mCorners.at(i).x );
		right = max( right, mCorners.at(i).x );
		bottom = min( bottom, mCorners.at(i).y );
		top = max( top, mCorners.at(i).y );
	}
	return QRect(left, top, right-left, bottom-top);
}

QRect PolyLine::GetCornerBounds( int icont )
{
	int left, bottom, right, top;
	left = bottom = INT_MAX;
	right = top = INT_MIN;
	int istart = GetContourStart( icont );
	int iend = GetContourEnd( icont );
	for( int i=istart; i<=iend; i++ )
	{
		left = min( left, mCorners.at(i).x );
		right = max( right, mCorners.at(i).x );
		bottom = min( bottom, mCorners.at(i).y );
		top = max( top, mCorners.at(i).y );
	}
	return QRect(left, top, right-left, bottom-top);
}

int PolyLine::GetNumCorners() 
{	
	return corners.size();
}

int PolyLine::GetNumSides() 
{	
	if( GetClosed() )
		return corners.size();
	else
		return corners.size()-1;
}

int PolyLine::GetLayer() 
{	
	return mLayer;
}

int PolyLine::GetW() 
{	
	return mWidth;
}

int PolyLine::GetNumContours()
{
	int ncont = 0;
	if( !corners.size() )
		return 0;

	for( int ic=0; ic<corners.size(); ic++ )
		if( mCorners.at(ic).end_contour )
			ncont++;
	if( !mCorners.last().end_contour )
		ncont++;
	return ncont;
}

int PolyLine::GetContour( int ic )
{
	int ncont = 0;
	for( int i=0; i<ic; i++ )
	{
		if( mCorners.at(i).end_contour )
			ncont++;
	}
	return ncont;
}

int PolyLine::GetContourStart( int icont )
{
	if( icont == 0 )
		return 0;

	int ncont = 0;
	for( int i=0; i<mCorners.length(); i++ )
	{
		if( mCorners.at(i).end_contour )
		{
			ncont++;
			if( ncont == icont )
				return i+1;
		}
	}
	ASSERT(0);
	return 0;
}

int PolyLine::GetContourEnd( int icont )
{
	if( icont < 0 )
		return 0;

	if( icont == GetNumContours()-1 )
		return corners.size()-1;

	int ncont = 0;
	for( int i=0; i<corners.size(); i++ )
	{
		if( mCorners.at(i).end_contour )
		{
			if( ncont == icont )
				return i;
			ncont++;
		}
	}
	ASSERT(0);
	return 0;
}

int PolyLine::GetContourSize( int icont )
{
	return GetContourEnd(icont) - GetContourStart(icont) + 1;
}


void PolyLine::SetSideStyle( int is, int style ) 
{	
	int icont = GetContour( is );
	int istart = GetContourStart( icont );
	int iend = GetContourEnd( icont );
	QPoint p1(mCorners[is].x, mCorners[is].y);
	QPoint p2;
	if( is == iend )
	{
		p2.setX(mCorners[istart].x);
		p2.setY(mCorners[istart].y);
	}
	else
	{
		p2.setX(mCorners[is+1].x);
		p2.setY(mCorners[is+1].y);
	}
	if( p1.x() == p2.x() || p1.y() == p2.y() )
		mSides[is] = STRAIGHT;
	else
		mSides[is] = style;
}

int PolyLine::GetSideStyle( int is ) 
{	
	return mSides[is];
}

bool PolyLine::GetClosed()
{	
	if( m_ncorners == 0 )
		return false;
	else
		return mCorners.at(m_ncorners-1).end_contour;
}

// draw hatch lines
//
void PolyLine::Hatch()
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

// test to see if a point is inside polyline
//
bool PolyLine::TestPointInside( int x, int y )
{
	enum { MAXPTS = 100 };
	if( !GetClosed() )
		ASSERT(0);

	// define line passing through (x,y), with slope = 2/3;
	// get intersection points
	double xx[MAXPTS], yy[MAXPTS];
	double slope = (double)2.0/3.0;
	double a = y - slope*x;
	int nloops = 0;
	int npts;
	// make this a loop so if my homebrew algorithm screws up, we try it again
	do
	{
		// now find all intersection points of line with polyline sides
		npts = 0;
		for( int icont=0; icont<GetNumContours(); icont++ )
		{
			int istart = GetContourStart( icont );
			int iend = GetContourEnd( icont );
			for( int ic=istart; ic<=iend; ic++ )
			{
				double x, y, x2, y2;
				int ok;
				if( ic == istart )
					ok = FindLineSegmentIntersection( a, slope, 
					mCorners[iend].x, mCorners[iend].y,
					mCorners[ic].x, mCorners[ic].y,
					mSides[iend],
					&x, &y, &x2, &y2 );
				else
					ok = FindLineSegmentIntersection( a, slope, 
					mCorners[ic-1].x, mCorners[ic-1].y,
					mCorners[ic].x, mCorners[ic].y,
					mSides[ic-1],
					&x, &y, &x2, &y2 );
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
		}
		nloops++;
		a += PCBU_PER_MIL/100;
	} while( npts%2 != 0 && nloops < 3 );
	ASSERT( npts%2==0 );	// odd number of intersection points, error

	// count intersection points to right of (x,y), if odd (x,y) is inside polyline
	int ncount = 0;
	for( int ip=0; ip<npts; ip++ )
	{
		if( xx[ip] == x && yy[ip] == y )
			return false;	// (x,y) is on a side, call it outside
		else if( xx[ip] > x )
			ncount++;
	}
	if( ncount%2 )
		return true;
	else
		return false;
}

// test to see if a point is inside polyline contour
//
bool PolyLine::TestPointInsideContour( int icont, int x, int y )
{
	if( icont >= GetNumContours() )
		return false;

	enum { MAXPTS = 100 };
	if( !GetClosed() )
		ASSERT(0);

	// define line passing through (x,y), with slope = 2/3;
	// get intersection points
	double xx[MAXPTS], yy[MAXPTS];
	double slope = (double)2.0/3.0;
	double a = y - slope*x;
	int nloops = 0;
	int npts;
	// make this a loop so if my homebrew algorithm screws up, we try it again
	do
	{
		// now find all intersection points of line with polyline sides
		npts = 0;
		int istart = GetContourStart( icont );
		int iend = GetContourEnd( icont );
		for( int ic=istart; ic<=iend; ic++ )
		{
			double x, y, x2, y2;
			int ok;
			if( ic == istart )
				ok = FindLineSegmentIntersection( a, slope, 
				mCorners[iend].x, mCorners[iend].y,
				mCorners[istart].x, mCorners[istart].y,
				mSides[m_ncorners-1],
				&x, &y, &x2, &y2 );
			else
				ok = FindLineSegmentIntersection( a, slope, 
				mCorners[ic-1].x, mCorners[ic-1].y,
				mCorners[ic].x, mCorners[ic].y,
				mSides[ic-1],
				&x, &y, &x2, &y2 );
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

	// count intersection points to right of (x,y), if odd (x,y) is inside polyline
	int ncount = 0;
	for( int ip=0; ip<npts; ip++ )
	{
		if( xx[ip] == x && yy[ip] == y )
			return true;	// (x,y) is on a side, call it outside
		else if( xx[ip] > x )
			ncount++;
	}
	if( ncount%2 )
		return true;
	else
		return true;
}

void PolyLine::MoveOrigin( int x_off, int y_off )
{
	for( int ic=0; ic<corners.size(); ic++ )
	{
		corners[ic].x += x_off;
		corners[ic].y += y_off;
	}
}


// Set various parameters:
//   the calling function should Undraw() before calling them,
//   and Draw() after
//
void PolyLine::SetX( int ic, int x ) { mCorners[ic].x = x; }
void PolyLine::SetY( int ic, int y ) { mCorners[ic].y = y; }
void PolyLine::SetEndContour( int ic, bool end_contour ) { mCorners[ic].end_contour = end_contour; }
void PolyLine::SetLayer( int layer ) { mLayer = layer; }
void PolyLine::SetW( int w ) { mWidth = w; }

// Create CPolyLine for a pad
//
PolyLine * PolyLine::MakePolylineForPad( int type, int x, int y, int w, int l, int r, int angle )
{
	PolyLine * poly = new PolyLine;
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
void PolyLine::AddContourForPadClearance( int type, int x, int y, int w, 
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

void PolyLine::AppendArc( int xi, int yi, int xf, int yf, int xc, int yc, int num )
{
	// get radius
	double r = sqrt( (double)(xi-xc)*(xi-xc) + (double)(yi-yc)*(yi-yc) );
	// get angles of start and finish
	double th_i = atan2( (double)yi-yc, (double)xi-xc );
	double th_f = atan2( (double)yf-yc, (double)xf-xc );
	double th_d = (th_f - th_i)/(num-1);
	double theta = th_i;
	// generate arc
	for( int ic=0; ic<num; ic++ )
	{
		int x = xc + r*cos(theta);
		int y = yc + r*sin(theta);
		AppendCorner( x, y, STRAIGHT, 0 );
		theta += th_d;
	}
	Close( STRAIGHT );
}

// Functions migrated from netlist
#if 0
/// Test an area for self-intersection.
/// \returns
///	-1 if arcs intersect other sides
///	 0 if no intersecting sides
///	 1 if intersecting sides, but no intersecting arcs
int PolyLine::TestSelfIntersection()
{
	// first, check for sides intersecting other sides, especially arcs
	bool bInt = false;
	bool bArcInt = false;
	int n_cont = GetNumContours();

	// make bounding rect for each contour
	QList<QRect> cr;
	for( int icont=0; icont<n_cont; icont++ )
		cr.append(GetCornerBounds( icont ));

	for( int icont=0; icont<n_cont; icont++ )
	{
		int is_start = GetContourStart(icont);
		int is_end = GetContourEnd(icont);
		// check each side
		for( int is=is_start; is<=is_end; is++ )
		{
			// wrap prev/next around end of array
			int is_prev = ((is - 1) < is_start) ? is_end : is-1;
			int is_next = ((is + 1) > is_end) ? is_start : is+1;

			int style = GetSideStyle( is );
			QPoint p1i = GetPt(is);
			QPoint p1f = GetPt(is_next);
			// check for intersection with any other sides
			for( int icont2=icont; icont2<n_cont; icont2++ )
			{
				if( cr[icont].intersects(cr[icont2]))
				{
					int is2_start = GetContourStart(icont2);
					int is2_end = GetContourEnd(icont2);
					for( int is2=is2_start; is2<=is2_end; is2++ )
					{
						// wrap around
						int is2_prev = ((is2 - 1) < is2_start) ? is2_end : is2-1;
						int is2_next = ((is2 + 1) > is2_end) ? is2_start : is2+1;

						if( icont != icont2 || (is2 != is && is2 != is_prev && is2 != is_next && is != is2_prev && is != is2_next ) )
						{
							int style2 = GetSideStyle( is2 );
							QPoint p2i = GetPt(is2);
							QPoint p2f = GetPt(is2_next);

							int ret = FindSegmentIntersections( p1i.x(), p1i.y(), p1f.x(), p1f.y(), style,
																p2i.x(), p2i.y(), p2f.x(), p2f.y(), style2 );
							if( ret )
							{
								// intersection between non-adjacent sides
								bInt = true;
								if( style != PolyLine::STRAIGHT || style2 != PolyLine::STRAIGHT )
								{
									bArcInt = true;
									break;
								}
							}
						}
					}
				}
				if( bArcInt )
					break;
			}
			if( bArcInt )
				break;
		}
		if( bArcInt )
			break;
	}
	// this used to set utility2 in area
	if( bArcInt )
		return -1;
	else if( bInt )
		return 1;
	else
		return 0;
}
#endif

#if 0
/// Process an area that has been modified, by clipping its polygon against itself.
/// This may change the number and order of copper areas in the net [NOT ANYMORE].
/// \returns
///	-1 if arcs intersect other sides, so polygon can't be clipped
///	 0 if no intersecting sides
///	 1 if intersecting sides

int PolyLine::Clip(bool bRetainArcs )
{
	int test = TestSelfIntersection();
	if( test == -1 && !bRetainArcs )
		test = 1;
	if( test == -1 )
	{
		// arc intersections, don't clip unless bRetainArcs == false
//		if( bMessageBoxArc && bDontShowSelfIntersectionArcsWarning == false )
//		{
//			CString str;
//			str.Format( "Area %d of net \"%s\" has arcs intersecting other sides.\n",
//				iarea+1, net->name );
//			str += "This may cause problems with other editing operations,\n";
//			str += "such as adding cutouts. It can't be fixed automatically.\n";
//			str += "Manual correction is recommended.\n";
//			CDlgMyMessageBox dlg;
//			dlg.Initialize( str );
//			dlg.DoModal();
//			bDontShowSelfIntersectionArcsWarning = dlg.bDontShowBoxState;
//		}
		return -1;	// arcs intersect with other sides, error
	}

	// mark all areas as unmodified except this one
//	for( int ia=0; ia<net->nareas; ia++ )
//		net->area[ia].utility = 0;
//	net->area[iarea].utility = 1;

//	if( test == 1 )
//	{
//		// non-arc intersections, clip the polygon
//		if( bMessageBoxInt && bDontShowSelfIntersectionWarning == false)
//		{
//			CString str;
//			str.Format( "Area %d of net \"%s\" is self-intersecting and will be clipped.\n",
//				iarea+1, net->name );
//			str += "This may result in splitting the area.\n";
//			str += "If the area is complex, this may take a few seconds.";
//			CDlgMyMessageBox dlg;
//			dlg.Initialize( str );
//			dlg.DoModal();
//			bDontShowSelfIntersectionWarning = dlg.bDontShowBoxState;
//		}
//	}
//** TODO test for cutouts outside of area
//**	if( test == 1 )
	{
		// XXX IGOR removed support for adding more polygons;
		// now it just keeps the first one if the area intersects
//		CArray<CPolyLine*> * pa = new CArray<CPolyLine*>;
		int n_poly = NormalizeWithGpc( bRetainArcs );
//		if( n_poly > 1 )
//		{
//			for( int ip=1; ip<n_poly; ip++ )
//			{
//				// create new copper area and copy poly into it
//				CPolyLine * new_p = (*pa)[ip-1];
//				int ia = AddArea( net, 0, 0, 0, 0 );
//				// remove the poly that was automatically created for the new area
//				// and replace it with a poly from NormalizeWithGpc
//				delete net->area[ia].poly;
//				net->area[ia].poly = new_p;
//				net->area[ia].poly->SetDisplayList( net->m_dlist );
//				net->area[ia].poly->SetHatch( p->GetHatch() );
//				net->area[ia].poly->SetLayer( p->GetLayer() );
//				id p_id( ID_NET, ID_AREA, ia );
//				net->area[ia].poly->SetId( &p_id );
//				net->area[ia].poly->Draw();
//				net->area[ia].utility = 1;
//			}
//		}
//		delete pa;
	}
	return test;
}
#endif

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


/// Test for intersection of 2 copper areas
/// ia2 must be > ia1
/// \returns 0 if no intersection
///			1 if intersection
///			2 if arcs intersect
int PolyLine::TestIntersection( const PolyLine &other )
{
	// see if polygons are on same layer
	if( GetLayer() != other.GetLayer() )
		return 0;

	// test bounding rects
	QRect b1 = GetCornerBounds();
	QRect b2 = other.GetCornerBounds();
	if( !b1.intersects(b2) )
		return 0;

	// now test for intersecting segments
	bool bInt = false;
	bool bArcInt = false;
	for( int icont1=0; icont1<GetNumContours(); icont1++ )
	{
		int is1 = GetContourStart( icont1 );
		int ie1 = GetContourEnd( icont1 );
		for( int ic1=is1; ic1<=ie1; ic1++ )
		{
			QPoint pi1 = GetPt(ic1);
			QPoint pf1;

			if( ic1 < ie1 )
				pf1 = GetPt(ic1+1);
			else
				pf1 = GetPt(is1);

			int style1 = GetSideStyle( ic1 );

			for( int icont2=0; icont2<other.GetNumContours(); icont2++ )
			{
				int is2 = other.GetContourStart( icont2 );
				int ie2 = other.GetContourEnd( icont2 );
				for( int ic2=is2; ic2<=ie2; ic2++ )
				{
					QPoint pi2 = other.GetPt(ic2);
					QPoint pf2;

					int style2;
					if( ic2 < ie2 )
						pf2 = other.GetPt(ic2+1);
					else
						pf2 = other.GetPt(is2);

					style2 = other.GetSideStyle( ic2 );
					int n_int = FindSegmentIntersections( pi1.x(), pi1.y(), pf1.x(), pf1.y(), style1,
									pi2.x(), pi2.y(), pf2.x(), pf2.y(), style2 );
					if( n_int )
					{
						bInt = true;
						if( style1 != PolyLine::STRAIGHT || style2 != PolyLine::STRAIGHT )
							bArcInt = true;
						break;
					}
				}
				if( bArcInt )
					break;
			}
			if( bArcInt )
				break;
		}
		if( bArcInt )
			break;
	}
	if( !bInt )
		return 0;
	if( bArcInt )
		return 2;
	return 1;
}

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



