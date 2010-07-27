#include "Part.h"
#include "utility.h"
#include "Netlist.h"


PCBLAYER PartPin::getLayer()
{
	Padstack ps = part->getFootprint()->getPadstack(this->name);
	if( ps.hole_size )
		return LAY_PAD_THRU;
	else if( part->GetSide() == SIDE_TOP && ps.top.shape != PAD_NONE
		|| part->GetSide() == SIDE_BOTTOM && ps.bottom.shape != PAD_NONE )
		return LAY_TOP_COPPER;
	else
		return LAY_BOTTOM_COPPER;
}

// Get max pin width, for drawing thermal symbol
int PartPin::getWidth( )
{
	Padstack ps = part->getFootprint()->getPadstack(this->name);
	return max(ps.top.size_h, max(ps.bottom.size_h, ps.hole_size));
}

// Test for hit on pad
//
bool PartPin::TestHit( QPoint pt, PCBLAYER layer )
{
	Padstack ps = part->getFootprint()->getPadstack(this->name);
	Pad p;
	if( ps.hole_size == 0 )
	{
		// SMT pad
		if( layer == LAY_TOP_COPPER && part->GetSide() == SIDE_TOP ||
			layer == LAY_BOTTOM_COPPER && part->GetSide() == SIDE_BOTTOM)
			p = ps.top;
		else
			return false;
	}
	else
	{
		// TH pad
		if( layer == LAY_TOP_COPPER && part->GetSide() == SIDE_TOP ||
			layer == LAY_BOTTOM_COPPER && part->GetSide() == SIDE_BOTTOM )
			p = ps.top;
		else if( layer == LAY_TOP_COPPER && part->GetSide() == SIDE_BOTTOM ||
				 layer == LAY_BOTTOM_COPPER && part->GetSide() == SIDE_TOP )
			p = ps.bottom;
		else
			p = ps.inner;
	}
	QPoint delta( pt - this->getPos() );
	double dist = sqrt( delta.x()*delta.x() + delta.y()*delta.y() );
	if( dist < ps.hole_size/2 )
		return true;
	switch( p.shape )
	{
	case PAD_NONE:
		break;
	case PAD_ROUND:
		if( dist < (p.size_h/2) )
			return true;
		break;
	case PAD_SQUARE:
		if( delta.x() < (p.size_h/2) && delta.y() < (p.size_h/2) )
			return true;
		break;
	case PAD_RECT:
	case PAD_RRECT:
	case PAD_OVAL:
		int pad_angle = this->angle + ps.angle;
		if( pad_angle > 270 )
			pad_angle -= 360;
		if( pad_angle == 0 || pad_angle == 180 )
		{
			if( delta.x() < (p.size_l) && delta.y() < (p.size_h/2) )
				return true;
		}
		else
		{
			if( delta.x() < (p.size_h/2) && delta.y() < (p.size_l) )
				return true;
		}
		break;
	}
	return false;
}

Part::Part(SMFontUtil * fontutil)
{
	// zero out pointers
	shape = 0;
	m_fontutil = fontutil;
}

Part::~Part()
{
}



// Set part data, draw part if m_dlist != NULL
//
int Part::SetPartData( Footprint * shape, QString ref_des, QString package,
							int x, int y, int side, int angle, int visible, int glued )
{
	// now copy data into part
	this->m_id = id( ID_PART );
	this->visible = visible;
	this->ref_des = ref_des;
	if( package )
		part->package = *package;
	else
		part->package = "";
	this->x = x;
	this->y = y;
	this->side = side;
	this->angle = angle;
	this->glued = glued;

	if( !shape )
	{
		this->shape = NULL;
		this->pin.clear();
		this->m_ref_xi = 0;
		this->m_ref_yi = 0;
		this->m_ref_angle = 0;
		this->m_ref_size = 0;
		this->m_ref_w = 0;
		this->m_value_xi = 0;
		this->m_value_yi = 0;
		this->m_value_angle = 0;
		this->m_value_size = 0;
		this->m_value_w = 0;
	}
	else
	{
		this->shape = shape;
		//this->pin.( shape->m_padstack.GetSize() );
		Move( x, y, angle, side );	// force setting pin positions
		this->m_ref_xi = shape->m_ref_xi;
		this->m_ref_yi = shape->m_ref_yi;
		this->m_ref_angle = shape->m_ref_angle;
		this->m_ref_size = shape->m_ref_size;
		this->m_ref_w = shape->m_ref_w;
		this->m_value_xi = shape->m_value_xi;
		this->m_value_yi = shape->m_value_yi;
		this->m_value_angle = shape->m_value_angle;
		this->m_value_size = shape->m_value_size;
		this->m_value_w = shape->m_value_w;
	}
	this->m_outline_stroke.clear();
	this->ref_text_stroke.clear();
	this->value_stroke.clear();
	m_size++; // ???

	return 0;
}


// Highlight part
//
int Part::HighlightPart( Part * part )
{
	// highlight it by making its selection rectangle visible
	m_dlist->HighLight( DL_HOLLOW_RECT,
				m_dlist->Get_x( part->dl_sel) ,
				m_dlist->Get_y( part->dl_sel),
				m_dlist->Get_xf(part->dl_sel),
				m_dlist->Get_yf(part->dl_sel), 1 );
	return 0;
}

// Highlight part, value and ref text
//
int Part::SelectPart( Part * part )
{
	// highlight part, value and ref text
	HighlightPart( part );
	SelectRefText( part );
	SelectValueText( part );
	return 0;
}

// Highlight part ref text
//
int Part::SelectRefText( Part * part )
{
	// highlight it by making its selection rectangle visible
	if( part->dl_ref_sel )
	{
		m_dlist->HighLight( DL_HOLLOW_RECT,
			m_dlist->Get_x(part->dl_ref_sel),
			m_dlist->Get_y(part->dl_ref_sel),
			m_dlist->Get_xf(part->dl_ref_sel),
			m_dlist->Get_yf(part->dl_ref_sel), 1 );
	}
	return 0;
}

// Highlight part value
//
int Part::SelectValueText( Part * part )
{
	// highlight it by making its selection rectangle visible
	if( part->dl_value_sel )
	{
		m_dlist->HighLight( DL_HOLLOW_RECT,
			m_dlist->Get_x(part->dl_value_sel),
			m_dlist->Get_y(part->dl_value_sel),
			m_dlist->Get_xf(part->dl_value_sel),
			m_dlist->Get_yf(part->dl_value_sel), 1 );
	}
	return 0;
}


// Select part pad
//
int Part::SelectPad( Part * part, int i )
{
	// select it by making its selection rectangle visible
	if( part->pin[i].dl_sel )
	{
		m_dlist->HighLight( DL_RECT_X,
			m_dlist->Get_x(part->pin[i].dl_sel),
			m_dlist->Get_y(part->pin[i].dl_sel),
			m_dlist->Get_xf(part->pin[i].dl_sel),
			m_dlist->Get_yf(part->pin[i].dl_sel),
			1, GetPinLayer( part, i ) );
	}
	return 0;
}



// Move part to new position, angle and side
// x and y are in world coords
// Does not adjust connections to pins
//
int Part::Move( int x, int y, int angle, int side )
{
	// move part
	this->x = x;
	this->y = y;
	this->angle = angle % 360;
	this->side = side;
	// calculate new pin positions
	if( this->shape )
	{
		for( int ip=0; ip<part->pin.size(); ip++ )
		{
			QPoint pt = GetPinPoint( part, ip );
			this->pin[ip].x = pt.x;
			this->pin[ip].y = pt.y;
		}
	}

	return PL_NOERR;
}

// Move ref text with given id to new position and angle
// x and y are in absolute world coords
// angle is relative to part angle
//
int Part::MoveRefText( int x, int y, int angle, int size, int w )
{
	// get position of new text box origin relative to part
	QPoint tb_org(x - this->x, y - this->y);

	// correct for rotation of part
	RotatePoint( tb_org, 360-this->angle, zero );

	// correct for part on bottom of board (reverse relative x-axis)
	if( part->side == 1 )
		tb_org.setX(-tb_org.x());

	// reset ref text position
	this->m_ref_xi = tb_org.x();
	this->m_ref_yi = tb_org.y();
	this->m_ref_angle = angle % 360;
	this->m_ref_size = size;
	this->m_ref_w = w;

	return PL_NOERR;
}

// Resize ref text for part
//
void Part::ResizeRefText( int size, int width, bool vis )
{
	if( this->shape )
	{
		// change ref text size
		this->m_ref_size = size;
		this->m_ref_w = width;
		this->m_ref_vis = vis;
	}
}

// Move value text to new position and angle
// x and y are in absolute world coords
// angle is relative to part angle
//
int Part::MoveValueText( int x, int y, int angle, int size, int w )
{

	// get position of new text box origin relative to part
	QPoint tb_org(x - this->x, y - this->y);

	// correct for rotation of part
	RotatePoint( tb_org, 360-this->angle, zero );

	// correct for part on bottom of board (reverse relative x-axis)
	if( this->side == 1 )
		tb_org.setX( -tb_org.x());

	// reset value text position
	this->m_value_xi = tb_org.x();
	this->m_value_yi = tb_org.y();
	this->m_value_angle = angle % 360;
	this->m_value_size = size;
	this->m_value_w = w;

	return PL_NOERR;
}

// Resize value text for part
//
void Part::ResizeValueText(int size, int width, bool vis )
{
	if( this->shape )
	{
		// change ref text size
		this->m_value_size = size;
		this->m_value_w = width;
		this->m_value_vis = vis;
	}
}

// Set part value
//
void Part::SetValue( QString value,
						 int x, int y, int angle, int size, int w, bool vis )
{
	this->value = value;
	this->m_value_xi = x;
	this->m_value_yi = y;
	this->m_value_angle = angle;
	this->m_value_size = size;
	this->m_value_w = w;
	this->m_value_vis = vis;
}


// Get side of part
//
int Part::GetSide()
{
	return this->side;
}

// Get angle of part
//
int Part::GetAngle( )
{
	return this->angle;
}

// Get angle of ref text for part
//
int Part::GetRefAngle( )
{
	return this->m_ref_angle;
}

// Get angle of value for part
//
int Part::GetValueAngle( )
{
	return this->m_value_angle;
}

// get actual position of ref text
//
QPoint Part::GetRefPoint( )
{
	// move origin of text box to position relative to part
	QPoint ref_pt(m_ref_xi, m_ref_yi);

	// flip if part on bottom
	if( this->side )
	{
		ref_pt.rx() *= -1;
	}
	// rotate with part about part origin
	RotatePoint( ref_pt, this->angle, zero );
	ref_pt.rx() += this->x;
	ref_pt.ry() += part->y;
	return ref_pt;
}

// get actual position of value text
//
QPoint Part::GetValuePoint( )
{
	// move origin of text box to position relative to part
	QPoint value_pt(m_value_xi, m_value_yi);

	// flip if part on bottom
	if( this->side )
	{
		value_pt.rx() *= -1;
	}
	// rotate with part about part origin
	RotatePoint( value_pt, this->angle, zero );
	value_pt.rx() += this->x;
	value_pt.ry() += this->y;
	return value_pt;
}


// generate an array of strokes for a string that is attached to a part
// enter with:
//  str = text string
//	strokes = reference to QList of strokes to receive data
//	br = reference to QRect to receive bounding rectangle
//	dlist = pointer to display list to use for drawing (not used)
//	sm = pointer to SMFontUtil for character data
// returns number of strokes generated
//
int Part::GenerateStrokesForString( QString str, QList<Stroke> & strokes, QRect & br)
{
	//strokes->SetSize( 10000 );
	double x_scale = (double)m_size/22.0;
	double y_scale = (double)m_size/22.0;
	double y_offset = 9.0*y_scale;
	int i = 0;
	double xc = 0.0;
	QPoint si, sf;
	int xmin = INT_MAX;
	int xmax = INT_MIN;
	int ymin = INT_MAX;
	int ymax = INT_MIN;
	strokes.clear();
	for( int ic=0; ic<str.size(); ic++ )
	{
		// get stroke info for character
		int xi, yi, xf, yf;
		double coord[64][4];
		double min_x, min_y, max_x, max_y;
		int nstrokes = m_fontutil->GetCharStrokes( str.at(ic), SIMPLEX,
			&min_x, &min_y, &max_x, &max_y, coord, 64 );
		for( int is=0; is<nstrokes; is++ )
		{
			xi = (coord[is][0] - min_x)*x_scale + xc;
			yi = coord[is][1]*y_scale + y_offset;
			xf = (coord[is][2] - min_x)*x_scale + xc;
			yf = coord[is][3]*y_scale + y_offset;
			xmax = max( xi, xmax );
			xmax = max( xf, xmax );
			xmin = min( xi, xmin );
			xmin = min( xf, xmin );
			ymax = max( yi, ymax );
			ymax = max( yf, ymax );
			ymin = min( yi, ymin );
			ymin = min( yf, ymin );
			// get stroke relative to text box
			if( yi > yf )
			{
				si = QPoint(xi, yi);
				sf = QPoint(xf, yf);
			}
			else
			{
				si = QPoint(xf, yf);
				sf = QPoint(xi, yi);
			}
			// rotate about text box origin
			RotatePoint( si, rel_angle, zero );
			RotatePoint( sf, rel_angle, zero );
			// move origin of text box to position relative to part
			si += QPoint(rel_xi, rel_yi);
			sf += QPoint(rel_xi, rel_yi);
			// flip if part on bottom
			if( side )
			{
				si.rx() *= -1;
				sf.rx() *= -1;
			}
			// rotate with part about part origin
			RotatePoint( si, angle, zero );
			RotatePoint( sf, angle, zero );

			// add x, y to part origin and draw
			Stroke st;
			st.type = DL_LINE;
			st.w = this->w;
			st.xi = this->x + si.x();
			st.yi = this->y + si.y();
			st.xf = this->x + sf.x();
			st.yf = this->y + sf.y();
			strokes.append(st);
			i++;
		}
		xc += (max_x - min_x + 8.0)*x_scale;
	}
	br = QRect(xmin-w/2, ymin-w/2, xmax-xmin+w, ymax-ymin+w);
	return i;
}


// get bounding rect of value text relative to part origin
// works even if part isn't drawn
//
QRect Part::GetValueRect( )
{
	QList<Stroke> m_stroke;
	QRect br;
	GenerateStrokesForString( this->value, m_stroke, br );
	return br;
}

#if 0
// Draw part into display list
//
int Part::DrawPart( Part * part )
{
	int i;

	if( !m_dlist )
		return PL_NO_DLIST;
	if( !part->shape )
		return PL_NO_FOOTPRINT;
	if( part->drawn )
		UndrawPart( part );		// ideally, should be undrawn when changes made, not now

	// this part
	CShape * shape = part->shape;
	int x = part->x;
	int y = part->y;
	int angle = part->angle;
	id id = part->m_id;

	// draw selection rectangle (layer = top or bottom copper, depending on side)
	CRect sel;
	int sel_layer;
	if( !part->side )
	{
		// part on top
		sel.left = shape->m_sel_xi;
		sel.right = shape->m_sel_xf;
		sel.bottom = shape->m_sel_yi;
		sel.top = shape->m_sel_yf;
		sel_layer = LAY_SELECTION;
	}
	else
	{
		// part on bottom
		sel.right = - shape->m_sel_xi;
		sel.left = - shape->m_sel_xf;
		sel.bottom = shape->m_sel_yi;
		sel.top = shape->m_sel_yf;
		sel_layer = LAY_SELECTION;
	}
	if( angle > 0 )
		RotateRect( &sel, angle, zero );
	id.st = ID_SEL_RECT;
	part->dl_sel = m_dlist->AddSelector( id, part, sel_layer, DL_HOLLOW_RECT, 1,
		0, 0, x + sel.left, y + sel.bottom, x + sel.right, y + sel.top, x, y );
	m_dlist->Set_sel_vert( part->dl_sel, 0 );
	if( angle == 90 || angle ==  270 )
		m_dlist->Set_sel_vert( part->dl_sel, 1 );

	CArray<stroke> m_stroke;	// used for text
	CRect br;
	CPoint si, sf;

	int silk_lay = LAY_SILK_TOP;
	if( part->side )
		silk_lay = LAY_SILK_BOTTOM;

	// draw ref designator text
	part->dl_ref_sel = NULL;
	if( part->m_ref_vis && part->m_ref_size )
	{
		int nstrokes = ::GenerateStrokesForPartString( &part->ref_des, part->m_ref_size,
			part->m_ref_angle, part->m_ref_xi, part->m_ref_yi, part->m_ref_w,
			part->x, part->y, part->angle, part->side,
			&m_stroke, &br, m_dlist, m_fontutil );
		if( nstrokes )
		{
			int xmin = br.left;
			int xmax = br.right;
			int ymin = br.bottom;
			int ymax = br.top;
			id.type = ID_PART;
			id.st = ID_REF_TXT;
			id.sst = ID_STROKE;
			part->ref_text_stroke.SetSize( nstrokes );
			for( int is=0; is<nstrokes; is++ )
			{
				id.ii = is;
				m_stroke[is].dl_el = m_dlist->Add( id, this,
					silk_lay, DL_LINE, 1, m_stroke[is].w, 0,
					m_stroke[is].xi, m_stroke[is].yi,
					m_stroke[is].xf, m_stroke[is].yf, 0, 0 );
				part->ref_text_stroke[is] = m_stroke[is];
			}
			// draw selection rectangle for ref text
			// get text box relative to ref text origin, angle = 0
			si.x = xmin;
			sf.x = xmax;
			si.y = ymin;
			sf.y = ymax;
			// rotate to ref text angle
			RotatePoint( &si, part->m_ref_angle, zero );
			RotatePoint( &sf, part->m_ref_angle, zero );
			// move to position relative to part
			si.x += part->m_ref_xi;
			sf.x += part->m_ref_xi;
			si.y += part->m_ref_yi;
			sf.y += part->m_ref_yi;
			// flip if part on bottom
			if( part->side )
			{
				si.x = -si.x;
				sf.x = -sf.x;
			}
			// rotate to part angle
			RotatePoint( &si, angle, zero );
			RotatePoint( &sf, angle, zero );
			id.st = ID_SEL_REF_TXT;
			// move to part position and draw
			part->dl_ref_sel = m_dlist->AddSelector( id, part, silk_lay, DL_HOLLOW_RECT, 1,
				0, 0, x + si.x, y + si.y, x + sf.x, y + sf.y, x + si.x, y + si.y );
		}
	}
	else
	{
		for( int is=0; is<part->ref_text_stroke.GetSize(); is++ )
			m_dlist->Remove( part->ref_text_stroke[is].dl_el );
		part->ref_text_stroke.SetSize(0);
	}

	// draw value text
	part->dl_value_sel = NULL;
	if( part->m_value_vis && part->m_value_size )
	{
		int nstrokes = ::GenerateStrokesForPartString( &part->value, part->m_value_size,
			part->m_value_angle, part->m_value_xi, part->m_value_yi, part->m_value_w,
			part->x, part->y, part->angle, part->side,
			&m_stroke, &br, m_dlist, m_fontutil );

		if( nstrokes )
		{
			int xmin = br.left;
			int xmax = br.right;
			int ymin = br.bottom;
			int ymax = br.top;
			id.type = ID_PART;
			id.st = ID_VALUE_TXT;
			id.sst = ID_STROKE;
			part->value_stroke.SetSize( nstrokes );
			for( int is=0; is<nstrokes; is++ )
			{
				id.ii = is;
				m_stroke[is].dl_el = m_dlist->Add( id, this,
					silk_lay, DL_LINE, 1, m_stroke[is].w, 0,
					m_stroke[is].xi, m_stroke[is].yi,
					m_stroke[is].xf, m_stroke[is].yf, 0, 0 );
				part->value_stroke[is] = m_stroke[is];
			}

			// draw selection rectangle for value
			// get text box relative to value origin, angle = 0
			si.x = xmin;
			sf.x = xmax;
			si.y = ymin;
			sf.y = ymax;
			// rotate to value angle
			RotatePoint( &si, part->m_value_angle, zero );
			RotatePoint( &sf, part->m_value_angle, zero );
			// move to position relative to part
			si.x += part->m_value_xi;
			sf.x += part->m_value_xi;
			si.y += part->m_value_yi;
			sf.y += part->m_value_yi;
			// flip if part on bottom
			if( part->side )
			{
				si.x = -si.x;
				sf.x = -sf.x;
			}
			// rotate to part angle
			RotatePoint( &si, angle, zero );
			RotatePoint( &sf, angle, zero );
			id.st = ID_SEL_VALUE_TXT;
			// move to part position and draw
			part->dl_value_sel = m_dlist->AddSelector( id, part, silk_lay, DL_HOLLOW_RECT, 1,
				0, 0, x + si.x, y + si.y, x + sf.x, y + sf.y, x + si.x, y + si.y );
		}
	}
	else
	{
		for( int is=0; is<part->value_stroke.GetSize(); is++ )
			m_dlist->Remove( part->value_stroke[is].dl_el );
		part->value_stroke.SetSize(0);
	}

	// draw part outline
	part->m_outline_stroke.SetSize(0);
	for( int ip=0; ip<shape->m_outline_poly.GetSize(); ip++ )
	{
		int pos = part->m_outline_stroke.GetSize();
		int nsides;
		if( shape->m_outline_poly[ip].GetClosed() )
			nsides = shape->m_outline_poly[ip].GetNumCorners();
		else
			nsides = shape->m_outline_poly[ip].GetNumCorners() - 1;
		part->m_outline_stroke.SetSize( pos + nsides );
		int w = shape->m_outline_poly[ip].GetW();
		for( i=0; i<nsides; i++ )
		{
			int g_type;
			if( shape->m_outline_poly[ip].GetSideStyle( i ) == CPolyLine::STRAIGHT )
				g_type = DL_LINE;
			else if( shape->m_outline_poly[ip].GetSideStyle( i ) == CPolyLine::ARC_CW )
				g_type = DL_ARC_CW;
			else if( shape->m_outline_poly[ip].GetSideStyle( i ) == CPolyLine::ARC_CCW )
				g_type = DL_ARC_CCW;
			si.x = shape->m_outline_poly[ip].GetX( i );
			si.y = shape->m_outline_poly[ip].GetY( i );
			if( i == (nsides-1) && shape->m_outline_poly[ip].GetClosed() )
			{
				sf.x = shape->m_outline_poly[ip].GetX( 0 );
				sf.y = shape->m_outline_poly[ip].GetY( 0 );
			}
			else
			{
				sf.x = shape->m_outline_poly[ip].GetX( i+1 );
				sf.y = shape->m_outline_poly[ip].GetY( i+1 );
			}
			// flip if part on bottom
			if( part->side )
			{
				si.x = -si.x;
				sf.x = -sf.x;
				if( g_type == DL_ARC_CW )
					g_type = DL_ARC_CCW;
				else if( g_type == DL_ARC_CCW )
					g_type = DL_ARC_CW;
			}
			// rotate with part and draw
			RotatePoint( &si, angle, zero );
			RotatePoint( &sf, angle, zero );
			part->m_outline_stroke[i+pos].xi = x+si.x;
			part->m_outline_stroke[i+pos].xf = x+sf.x;
			part->m_outline_stroke[i+pos].yi = y+si.y;
			part->m_outline_stroke[i+pos].yf = y+sf.y;
			part->m_outline_stroke[i+pos].type = g_type;
			part->m_outline_stroke[i+pos].w = w;
			part->m_outline_stroke[i+pos].dl_el = m_dlist->Add( part->m_id, part, silk_lay,
				g_type, 1, w, 0, x+si.x, y+si.y, x+sf.x, y+sf.y, 0, 0 );
		}
	}

	// draw text
	for( int it=0; it<part->shape->m_tl->text_ptr.GetSize(); it++ )
	{
		CText * t = part->shape->m_tl->text_ptr[it];
		int nstrokes = 0;
		CArray<stroke> m_stroke;
		m_stroke.SetSize( 1000 );
		id.st = ID_STROKE;

		double x_scale = (double)t->m_font_size/22.0;
		double y_scale = (double)t->m_font_size/22.0;
		double y_offset = 9.0*y_scale;
		i = 0;
		double xc = 0.0;
		CPoint si, sf;
		int w = t->m_stroke_width;
		int xmin = INT_MAX;
		int xmax = INT_MIN;
		int ymin = INT_MAX;
		int ymax = INT_MIN;

		nstrokes = ::GenerateStrokesForPartString( &t->m_str, t->m_font_size,
			t->m_angle, t->m_x, t->m_y, t->m_stroke_width,
			part->x, part->y, part->angle, part->side,
			&m_stroke, &br, m_dlist, m_fontutil );

		xmin = min( xmin, br.left );
		xmax = max( xmax, br.right );
		ymin = min( ymin, br.bottom );
		ymax = max( ymax, br.top );
		id.type = ID_PART;
		id.st = ID_FP_TXT;
		id.i = it;
		id.sst = ID_STROKE;
		for( int is=0; is<nstrokes; is++ )
		{
			id.ii = is;
			m_stroke[is].dl_el = m_dlist->Add( id, this,
				silk_lay, DL_LINE, 1, m_stroke[is].w, 0,
				m_stroke[is].xi, m_stroke[is].yi,
				m_stroke[is].xf, m_stroke[is].yf, 0, 0 );
			part->m_outline_stroke.Add( m_stroke[is] );
		}


	}

	// draw padstacks and save absolute position of pins
	CPoint pin_pt;
	CPoint pad_pi;
	CPoint pad_pf;
	for( i=0; i<shape->GetNumPins(); i++ )
	{
		// set layer for pads
		padstack * ps = &shape->m_padstack[i];
		part_pin * pin = &part->pin[i];
		pin->dl_els.SetSize(m_layers);
		pad * p;
		int pad_layer;
		// iterate through all copper layers
		pad * any_pad = NULL;
		for( int il=0; il<m_layers; il++ )
		{
			pin_pt.x = ps->x_rel;
			pin_pt.y = ps->y_rel;
			pad_layer = il + LAY_TOP_COPPER;
			pin->dl_els[il] = NULL;
			// get appropriate pad
			padstack * ps = &shape->m_padstack[i];
			pad * p = NULL;
			if( pad_layer == LAY_TOP_COPPER && part->side == 0 )
				p = &ps->top;
			else if( pad_layer == LAY_TOP_COPPER && part->side == 1 )
				p = &ps->bottom;
			else if( pad_layer == LAY_BOTTOM_COPPER && part->side == 0 )
				p = &ps->bottom;
			else if( pad_layer == LAY_BOTTOM_COPPER && part->side == 1 )
				p = &ps->top;
			else if( ps->hole_size )
				p = &ps->inner;
			int sel_layer = pad_layer;
			if( ps->hole_size )
				sel_layer = LAY_SELECTION;
			if( p )
			{
				if( p->shape != PAD_NONE )
					any_pad = p;

				// draw pad
				dl_element * pad_el = NULL;
				if( p->shape == PAD_NONE )
				{
				}
				else if( p->shape == PAD_ROUND )
				{
					// flip if part on bottom
					if( part->side )
						pin_pt.x = -pin_pt.x;
					// rotate
					if( angle > 0 )
						RotatePoint( &pin_pt, angle, zero );
					// add to display list
					id.st = ID_PAD;
					id.i = i;
					pin->x = x + pin_pt.x;
					pin->y = y + pin_pt.y;
					pad_el = m_dlist->Add( id, part, pad_layer,
						DL_CIRC, 1,
						p->size_h,
						0,
						x + pin_pt.x, y + pin_pt.y, 0, 0, 0, 0 );
					if( !pin->dl_sel )
					{
						id.st = ID_SEL_PAD;
						pin->dl_sel = m_dlist->AddSelector( id, part, sel_layer,
							DL_HOLLOW_RECT, 1, 1, 0,
							pin->x-p->size_h/2,
							pin->y-p->size_h/2,
							pin->x+p->size_h/2,
							pin->y+p->size_h/2, 0, 0 );
					}
				}
				else if( p->shape == PAD_SQUARE )
				{
					// flip if part on bottom
					if( part->side )
					{
						pin_pt.x = -pin_pt.x;
					}
					// rotate
					if( angle > 0 )
						RotatePoint( &pin_pt, angle, zero );
					id.st = ID_PAD;
					id.i = i;
					pin->x = x + pin_pt.x;
					pin->y = y + pin_pt.y;
					pad_el = m_dlist->Add( part->m_id, part, pad_layer,
						DL_SQUARE, 1,
						p->size_h,
						0,
						pin->x, pin->y,
						0, 0,
						0, 0 );
					if( !pin->dl_sel )
					{
						id.st = ID_SEL_PAD;
						pin->dl_sel = m_dlist->AddSelector( id, part, sel_layer,
							DL_HOLLOW_RECT, 1, 1, 0,
							pin->x-p->size_h/2,
							pin->y-p->size_h/2,
							pin->x+p->size_h/2,
							pin->y+p->size_h/2, 0, 0 );
					}
				}
				else if( p->shape == PAD_RECT
					|| p->shape == PAD_RRECT
					|| p->shape == PAD_OVAL )
				{
					int gtype;
					if( p->shape == PAD_RECT )
						gtype = DL_RECT;
					else if( p->shape == PAD_RRECT )
						gtype = DL_RRECT;
					else
						gtype = DL_OVAL;
					pad_pi.x = pin_pt.x - p->size_l;
					pad_pi.y = pin_pt.y - p->size_h/2;
					pad_pf.x = pin_pt.x + p->size_r;
					pad_pf.y = pin_pt.y + p->size_h/2;
					// rotate pad about pin if necessary
					if( shape->m_padstack[i].angle > 0 )
					{
						RotatePoint( &pad_pi, ps->angle, pin_pt );
						RotatePoint( &pad_pf, ps->angle, pin_pt );
					}

					// flip if part on bottom
					if( part->side )
					{
						pin_pt.x = -pin_pt.x;
						pad_pi.x = -pad_pi.x;
						pad_pf.x = -pad_pf.x;
					}
					// rotate part about
					if( angle > 0 )
					{
						RotatePoint( &pin_pt, angle, zero );
						RotatePoint( &pad_pi, angle, zero );
						RotatePoint( &pad_pf, angle, zero );
					}
					id.st = ID_PAD;
					id.i = i;
					int radius = p->radius;
					pin->x = x + pin_pt.x;
					pin->y = y + pin_pt.y;
					pad_el = m_dlist->Add( part->m_id, part, pad_layer,
						gtype, 1,
						0,
						0,
						x + pad_pi.x, y + pad_pi.y,
						x + pad_pf.x, y + pad_pf.y,
						x + pin_pt.x, y + pin_pt.y,
						p->radius );
					if( !pin->dl_sel )
					{
						id.st = ID_SEL_PAD;
						pin->dl_sel = m_dlist->AddSelector( id, part, sel_layer,
							DL_HOLLOW_RECT, 1, 1, 0,
							x + pad_pi.x, y + pad_pi.y,
							x + pad_pf.x, y + pad_pf.y,
							0, 0 );
					}
				}
				else if( p->shape == PAD_OCTAGON )
				{
					// flip if part on bottom
					if( part->side )
					{
						pin_pt.x = -pin_pt.x;
					}
					// rotate
					if( angle > 0 )
						RotatePoint( &pin_pt, angle, zero );
					id.st = ID_PAD;
					id.i = i;
					pin->x = x + pin_pt.x;
					pin->y = y + pin_pt.y;
					pad_el = m_dlist->Add( part->m_id, part, pad_layer,
						DL_OCTAGON, 1,
						p->size_h,
						0,
						pin->x, pin->y,
						0, 0,
						0, 0 );
					if( !pin->dl_sel )
					{
						id.st = ID_SEL_PAD;
						pin->dl_sel = m_dlist->AddSelector( id, part, sel_layer,
							DL_HOLLOW_RECT, 1, 1, 0,
							pin->x-p->size_h/2,
							pin->y-p->size_h/2,
							pin->x+p->size_h/2,
							pin->y+p->size_h/2, 0, 0 );
					}
				}
				pin->dl_els[il] = pad_el;
				pin->dl_hole = pad_el;
			}
		}
		// if through-hole pad, just draw hole and set pin_dl_el;
		if( ps->hole_size )
		{
			pin_pt.x = ps->x_rel;
			pin_pt.y = ps->y_rel;
			// flip if part on bottom
			if( part->side )
			{
				pin_pt.x = -pin_pt.x;
			}
			// rotate
			if( angle > 0 )
				RotatePoint( &pin_pt, angle, zero );
			// add to display list
			id.st = ID_PAD;
			id.i = i;
			pin->x = x + pin_pt.x;
			pin->y = y + pin_pt.y;
			pin->dl_hole = m_dlist->Add( id, part, LAY_PAD_THRU,
								DL_HOLE, 1,
								ps->hole_size,
								0,
								pin->x, pin->y, 0, 0, 0, 0 );
			if( !pin->dl_sel )
			{
				// make selector for pin with hole only
				id.st = ID_SEL_PAD;
				pin->dl_sel = m_dlist->AddSelector( id, part, sel_layer,
					DL_HOLLOW_RECT, 1, 1, 0,
					pin->x-ps->hole_size/2,
					pin->y-ps->hole_size/2,
					pin->x+ps->hole_size/2,
					pin->y+ps->hole_size/2,
					0, 0 );
			}
		}
		else
			pin->dl_hole = NULL;
	}
	part->drawn = true;
	return PL_NOERR;
}

#endif

// normal completion of any dragging operation
//
int Part::StopDragging()
{
	m_dlist->StopDragging();
	return 0;
}

// Make part visible or invisible, including thermal reliefs
//
void Part::MakePartVisible( Part * part, bool bVisible )
{
	// make part elements invisible, including copper area connections
	// outline strokes
	for( int i=0; i<part->m_outline_stroke.GetSize(); i++ )
	{
		dl_element * el = part->m_outline_stroke[i].dl_el;
		el->visible = bVisible;
	}
	// pins
	for( int ip=0; ip<part->shape->m_padstack.GetSize(); ip++ )
	{
		// pin pads
		dl_element * el = part->pin[ip].dl_hole;
		if( el )
			el->visible = bVisible;
		for( int i=0; i<part->pin[ip].dl_els.GetSize(); i++ )
		{
			if( part->pin[ip].dl_els[i] )
				part->pin[ip].dl_els[i]->visible = bVisible;
		}
		// pin copper area connections
		cnet * net = (cnet*)part->pin[ip].net;
		if( net )
		{
			for( int ia=0; ia<net->nareas; ia++ )
			{
				for( int i=0; i<net->area[ia].npins; i++ )
				{
					if( net->pin[net->area[ia].pin[i]].part == part )
					{
						m_dlist->Set_visible( net->area[ia].dl_thermal[i], bVisible );
					}
				}
			}
		}
	}
	// ref text strokes
	for( int is=0; is<part->ref_text_stroke.GetSize(); is++ )
	{
		void * ptr = part->ref_text_stroke[is].dl_el;
		dl_element * el = (dl_element*)ptr;
		el->visible = bVisible;
	}
	// value strokes
	for( int is=0; is<part->value_stroke.GetSize(); is++ )
	{
		void * ptr = part->value_stroke[is].dl_el;
		dl_element * el = (dl_element*)ptr;
		el->visible = bVisible;
	}
}

// Start dragging part by setting up display list
// if bRatlines == false, no rubber-band ratlines
// else if bBelowPinCount == true, only use ratlines for nets with less than pin_count pins
//
int Part::StartDraggingPart( CDC * pDC, Part * part, bool bRatlines,
								 bool bBelowPinCount, int pin_count )
{
	// make part invisible
	MakePartVisible( part, false );
	m_dlist->CancelHighLight();

	// create drag lines
	CPoint zero(0,0);
	m_dlist->MakeDragLineArray( 2*part->shape->m_padstack.GetSize() + 4 );
	CArray<CPoint> pin_points;
	pin_points.SetSize( part->shape->m_padstack.GetSize() );
	int xi = part->shape->m_sel_xi;
	int xf = part->shape->m_sel_xf;
	if( part->side )
	{
		xi = -xi;
		xf = -xf;
	}
	int yi = part->shape->m_sel_yi;
	int yf = part->shape->m_sel_yf;
	CPoint p1( xi, yi );
	CPoint p2( xf, yi );
	CPoint p3( xf, yf );
	CPoint p4( xi, yf );
	RotatePoint( &p1, part->angle, zero );
	RotatePoint( &p2, part->angle, zero );
	RotatePoint( &p3, part->angle, zero );
	RotatePoint( &p4, part->angle, zero );
	m_dlist->AddDragLine( p1, p2 );
	m_dlist->AddDragLine( p2, p3 );
	m_dlist->AddDragLine( p3, p4 );
	m_dlist->AddDragLine( p4, p1 );
	for( int ip=0; ip<part->shape->m_padstack.GetSize(); ip++ )
	{
		// make X for each pin
		int d = part->shape->m_padstack[ip].top.size_h/2;
		CPoint p(part->shape->m_padstack[ip].x_rel,part->shape->m_padstack[ip].y_rel);
		int xi = p.x-d;
		int yi = p.y-d;
		int xf = p.x+d;
		int yf = p.y+d;
		// reverse if on other side of board
		if( part->side )
		{
			xi = -xi;
			xf = -xf;
			p.x = -p.x;
		}
		CPoint p1, p2, p3, p4;
		p1.x = xi;
		p1.y = yi;
		p2.x = xf;
		p2.y = yi;
		p3.x = xf;
		p3.y = yf;
		p4.x = xi;
		p4.y = yf;
		// rotate by part.angle
		RotatePoint( &p1, part->angle, zero );
		RotatePoint( &p2, part->angle, zero );
		RotatePoint( &p3, part->angle, zero );
		RotatePoint( &p4, part->angle, zero );
		RotatePoint( &p, part->angle, zero );
		// save pin position
		pin_points[ip].x = p.x;
		pin_points[ip].y = p.y;
		// draw X
		m_dlist->AddDragLine( p1, p3 );
		m_dlist->AddDragLine( p2, p4 );
	}
	// create drag lines for ratlines connected to pins
	if( bRatlines )
	{
		m_dlist->MakeDragRatlineArray( 2*part->shape->m_padstack.GetSize(), 1 );
		// zero utility flags for all nets
		cnet * n = m_nlist->GetFirstNet();
		while( n )
		{
			n->utility = 0;
			n = m_nlist->GetNextNet();
		}

		// now loop through all pins in part to find nets that connect
		for( int ipp=0; ipp<part->shape->m_padstack.GetSize(); ipp++ )
		{
			n = (cnet*)part->pin[ipp].net;
			if( n )
			{
				// only look at visible nets, only look at each net once
				if( n->visible && n->utility == 0 )
				{
					// zero utility flags for all connections
					for( int ic=0; ic<n->nconnects; ic++ )
					{
						n->connect[ic].utility = 0;
					}
					for( int ic=0; ic<n->nconnects; ic++ )
					{
						cconnect * c = &n->connect[ic];
						if( c->utility )
							continue;	// skip this connection

						// check for connection to part
						int pin1 = n->connect[ic].start_pin;
						int pin2 = n->connect[ic].end_pin;
						Part * pin1_part = n->pin[pin1].part;
						Part * pin2_part = NULL;
						if( pin2 != cconnect::NO_END )
							pin2_part = n->pin[pin2].part;
						if( pin1_part != part && pin2_part != part )
							continue;	// no

						// OK, this connection is attached to our part
						if( pin1_part == part )
						{
							int ip = pin1_part->shape->GetPinIndexByName( n->pin[pin1].pin_name );
							if( ip != -1 )
							{
								// ip is the start pin for the connection
								// hide segment
								m_dlist->Set_visible( c->seg[0].dl_el, 0 );
								for( int i=0; i<c->vtx[1].dl_el.GetSize(); i++ )
									m_dlist->Set_visible( c->vtx[1].dl_el[i], 0 );
								if( c->vtx[1].dl_hole )
									m_dlist->Set_visible( c->vtx[1].dl_hole, 0 );
								// add ratline
//**								if( !bBelowPinCount || n->npins <= pin_count )
								{
									bool bDraw = false;
									if( pin2_part == part )
									{
										// connection starts and ends on this part,
										// only drag if 3 or more segments
										if( c->nsegs > 2 )
											bDraw = true;
									}
									else if( pin2_part == NULL )
									{
										// stub trace starts on this part,
										// drag if more than 1 segment or next vertex is a tee
										if( c->nsegs > 1 || c->vtx[1].tee_ID )
											bDraw = true;
									}
									else if( pin2_part )
									{
										// connection ends on another part
										bDraw = true;
									}
									if( bDraw )
									{
										// add dragline from pin to second vertex
										CPoint vx( c->vtx[1].x, c->vtx[1].y );
										m_dlist->AddDragRatline( vx, pin_points[ip] );
									}
								}
							}
						}
						if( pin2_part == part )
						{
							int ip = -1;
							if( pin2 != cconnect::NO_END )
								ip = pin2_part->shape->GetPinIndexByName( n->pin[pin2].pin_name );
							if( ip != -1 )
							{
								// ip is the end pin for the connection
								m_dlist->Set_visible( n->connect[ic].seg[c->nsegs-1].dl_el, 0 );
								if( c->vtx[c->nsegs-1].dl_el.GetSize() )
									for( int i=0; i<c->vtx[c->nsegs-1].dl_el.GetSize(); i++ )
										m_dlist->Set_visible( c->vtx[c->nsegs-1].dl_el[i], 0 );
								if( c->vtx[c->nsegs-1].dl_hole )
									m_dlist->Set_visible( c->vtx[c->nsegs-1].dl_hole, 0 );
								// OK, get prev vertex, add ratline and hide segment
//**								if( !bBelowPinCount || n->npins <= pin_count )
								{
									bool bDraw = false;
									if( pin1_part == part )
									{
										// starts and ends on part
										if( c->nsegs > 2 )
											bDraw = true;
									}
									else
										bDraw = true;
									if( bDraw )
									{
										CPoint vx( n->connect[ic].vtx[c->nsegs-1].x, n->connect[ic].vtx[c->nsegs-1].y );
										m_dlist->AddDragRatline( vx, pin_points[ip] );
									}
								}
							}
						}
						c->utility = 1;	// this connection has been checked
					}
				}
				n->utility = 1;	// all connections for this net have been checked
			}
		}
	}
	int vert = 0;
	if( part->angle == 90 || part->angle == 270 )
		vert = 1;
	m_dlist->StartDraggingArray( pDC, part->x, part->y, vert, LAY_SELECTION );
	return 0;
}

// start dragging ref text
//
int Part::StartDraggingRefText( CDC * pDC, Part * part )
{
	// make ref text elements invisible
	for( int is=0; is<part->ref_text_stroke.GetSize(); is++ )
	{
		void * ptr = part->ref_text_stroke[is].dl_el;
		dl_element * el = (dl_element*)ptr;
		el->visible = 0;
	}
	// cancel selection
	m_dlist->CancelHighLight();
	// drag
	m_dlist->StartDraggingRectangle( pDC,
						m_dlist->Get_x(part->dl_ref_sel),
						m_dlist->Get_y(part->dl_ref_sel),
						m_dlist->Get_x(part->dl_ref_sel) - m_dlist->Get_x_org(part->dl_ref_sel),
						m_dlist->Get_y(part->dl_ref_sel) - m_dlist->Get_y_org(part->dl_ref_sel),
						m_dlist->Get_xf(part->dl_ref_sel) - m_dlist->Get_x_org(part->dl_ref_sel),
						m_dlist->Get_yf(part->dl_ref_sel) - m_dlist->Get_y_org(part->dl_ref_sel),
						0, LAY_SELECTION );
	return 0;
}

// start dragging value
//
int Part::StartDraggingValue( CDC * pDC, Part * part )
{
	// make value text elements invisible
	for( int is=0; is<part->value_stroke.GetSize(); is++ )
	{
		void * ptr = part->value_stroke[is].dl_el;
		dl_element * el = (dl_element*)ptr;
		el->visible = 0;
	}
	// cancel selection
	m_dlist->CancelHighLight();
	// drag
	m_dlist->StartDraggingRectangle( pDC,
						m_dlist->Get_x(part->dl_value_sel),
						m_dlist->Get_y(part->dl_value_sel),
						m_dlist->Get_x(part->dl_value_sel) - m_dlist->Get_x_org(part->dl_value_sel),
						m_dlist->Get_y(part->dl_value_sel) - m_dlist->Get_y_org(part->dl_value_sel),
						m_dlist->Get_xf(part->dl_value_sel) - m_dlist->Get_x_org(part->dl_value_sel),
						m_dlist->Get_yf(part->dl_value_sel) - m_dlist->Get_y_org(part->dl_value_sel),
						0, LAY_SELECTION );
	return 0;
}

// cancel dragging, return to pre-dragging state
//
int Part::CancelDraggingPart( Part * part )
{
	// make part visible again
	MakePartVisible( part, true );

	// get any connecting segments and make visible
	for( int ip=0; ip<part->shape->m_padstack.GetSize(); ip++ )
	{
		cnet * net = (cnet*)part->pin[ip].net;
		if( net )
		{
			if( net->visible )
			{
				for( int ic=0; ic<net->nconnects; ic++ )
				{
					cconnect * c = &net->connect[ic];
					int pin1 = c->start_pin;
					int pin2 = c->end_pin;
					int nsegs = c->nsegs;
					if( net->pin[pin1].part == part )
					{
						// start pin
						m_dlist->Set_visible( c->seg[0].dl_el, 1 );
						for( int i=0; i<c->vtx[1].dl_el.GetSize(); i++ )
							m_dlist->Set_visible( c->vtx[1].dl_el[i], 1 );
						if( c->vtx[1].dl_hole )
							m_dlist->Set_visible( c->vtx[1].dl_hole, 1 );
					}
					if( pin2 != cconnect::NO_END )
					{
						if( net->pin[pin2].part == part )
						{
							// end pin
							m_dlist->Set_visible( c->seg[nsegs-1].dl_el, 1 );
							if( c->vtx[c->nsegs-1].dl_el.GetSize() )
								for( int i=0; i<c->vtx[c->nsegs-1].dl_el.GetSize(); i++ )
									m_dlist->Set_visible( c->vtx[c->nsegs-1].dl_el[i], 1 );
							if( c->vtx[c->nsegs-1].dl_hole )
								m_dlist->Set_visible( c->vtx[c->nsegs-1].dl_hole, 1 );
						}
					}
				}
			}
		}
	}
	m_dlist->StopDragging();
	return 0;
}

// cancel dragging of ref text, return to pre-dragging state
int Part::CancelDraggingRefText( Part * part )
{
	// make ref text elements invisible
	for( int is=0; is<part->ref_text_stroke.GetSize(); is++ )
	{
		void * ptr = part->ref_text_stroke[is].dl_el;
		dl_element * el = (dl_element*)ptr;
		el->visible = 1;
	}
	m_dlist->StopDragging();
	return 0;
}

// cancel dragging value, return to pre-dragging state
int Part::CancelDraggingValue( Part * part )
{
	// make ref text elements invisible
	for( int is=0; is<part->value_stroke.GetSize(); is++ )
	{
		void * ptr = part->value_stroke[is].dl_el;
		dl_element * el = (dl_element*)ptr;
		el->visible = 1;
	}
	m_dlist->StopDragging();
	return 0;
}

// the footprint was changed for a particular part
// note that this function also updates the netlist
//
void Part::PartFootprintChanged( Footprint * new_shape )
{
	// new footprint
	this->shape = new_shape;
	part->pin.clear();
	// calculate new pin positions
	if( part->shape )
	{
		for( int ip=0; ip<part->pin.GetSize(); ip++ )
		{
			QPoint pt = GetPinPoint( ip );
			this->pin[ip].x = pt.x();
			this->pin[ip].y = pt.y();
		}
	}
	// XXX IGOR XXX
//	m_nlist->PartFootprintChanged( this );
}



// Undraw part from display list
//
int Part::UndrawPart( Part * part )
{
	int i;

	if( !m_dlist )
		return 0;

	if( part->drawn == false )
		return 0;

	Footprint * shape = part->shape;
	if( shape )
	{
		// undraw selection rectangle
		m_dlist->Remove( part->dl_sel );
		part->dl_sel = 0;

		// undraw selection rectangle for ref text
		m_dlist->Remove( part->dl_ref_sel );
		part->dl_ref_sel = 0;

		// undraw ref designator text
		int nstrokes = part->ref_text_stroke.GetSize();
		for( i=0; i<nstrokes; i++ )
		{
			m_dlist->Remove( part->ref_text_stroke[i].dl_el );
			part->ref_text_stroke[i].dl_el = 0;
		}

		// undraw selection rectangle for value
		m_dlist->Remove( part->dl_value_sel );
		part->dl_value_sel = 0;

		// undraw  value text
		nstrokes = part->value_stroke.GetSize();
		for( i=0; i<nstrokes; i++ )
		{
			m_dlist->Remove( part->value_stroke[i].dl_el );
			part->value_stroke[i].dl_el = 0;
		}

		// undraw part outline (this also includes footprint free text)
		for( i=0; i<part->m_outline_stroke.GetSize(); i++ )
		{
			m_dlist->Remove( (dl_element*)part->m_outline_stroke[i].dl_el );
			part->m_outline_stroke[i].dl_el = 0;
		}

		// undraw padstacks
		for( i=0; i<part->pin.GetSize(); i++ )
		{
			part_pin * pin = &part->pin[i];
			if( pin->dl_els.GetSize()>0 )
			{
				for( int il=0; il<pin->dl_els.GetSize(); il++ )
				{
					if( pin->dl_els[il] != pin->dl_hole )
						m_dlist->Remove( pin->dl_els[il] );
				}
				pin->dl_els.RemoveAll();
			}
			m_dlist->Remove( pin->dl_hole );
			m_dlist->Remove( pin->dl_sel );
			pin->dl_hole = NULL;
			pin->dl_sel = NULL;
		}
	}
	part->drawn = false;
	return PL_NOERR;
}

// Get bounding rect for all pins
// Currently, just uses selection rect
// returns 1 if successful
//
int Part::GetPartBoundingRect( CRect * part_r )
{
	CRect r;

	if( !part->shape )
		return 0;
	if( part->dl_sel )
	{
		r.left = min( m_dlist->Get_x( part->dl_sel ), m_dlist->Get_xf( part->dl_sel ) );
		r.right = max( m_dlist->Get_x( part->dl_sel ), m_dlist->Get_xf( part->dl_sel ) );
		r.bottom = min( m_dlist->Get_y( part->dl_sel ), m_dlist->Get_yf( part->dl_sel ) );
		r.top = max( m_dlist->Get_y( part->dl_sel ), m_dlist->Get_yf( part->dl_sel ) );
		*part_r = r;
		return 1;
	}
	return 0;
}


// Get pin info from part
//
QPoint Part::GetPinPoint( QString pin_name )
{
	// get pin coords relative to part origin
	int pin_index = this->shape->GetPinIndexByName( pin_name );
	if( pin_index == -1 )
		ASSERT(0);
	return GetPinPoint( pin_index );
}

QPoint Part::partToWorld(QPoint pt)
{
	// flip if part on bottom
	if( this->side )
	{
		pt.rx() *= -1;
	}
	// rotate if necess.
	int angle = this->angle;
	if( angle > 0 )
	{
		RotatePoint( pp, angle, zero );
	}
	// add coords of part origin
	pt += QPoint(this->x, this->y);
	return pt;
}

// Get pin info from part
//
QPoint Part::GetPinPoint( int pin_index )
{
	if( !this->shape )
		ASSERT(0);

	if( pin_index < 0 )
		ASSERT(0);

	return partToWorld(QPoint(this->shape->m_padstack[pin_index].x_rel,
			  this->shape->m_padstack[pin_index].y_rel));
}

// Get centroid info from part
//
QPoint Part::GetCentroidPoint()
{
	if( this->shape == NULL )
		ASSERT(0);

	// get coords relative to part origin
	return partToWorld(QPoint(this->shape->m_centroid_x,
			  this->shape->m_centroid_y));
}

// Get glue spot info from part
//
QPoint Part::GetGluePoint( int iglue )
{
	if( part->shape == NULL )
		ASSERT(0);
	if( iglue >= part->shape->m_glue.GetSize() )
		ASSERT(0);
	// get coords relative to part origin
	return partToWorld(QPoint(this->shape->m_glue[iglue].x_rel,
							  this->shape->m_glue[iglue].y_rel));
}

// Get pin layer
// returns LAY_TOP_COPPER, LAY_BOTTOM_COPPER or LAY_PAD_THRU
//
int Part::GetPinLayer( QString pin_name )
{
	int pin_index = this->shape->GetPinIndexByName( pin_name );
	return GetPinLayer( pin_index );
}

// Get pin layer
// returns LAY_TOP_COPPER, LAY_BOTTOM_COPPER or LAY_PAD_THRU
//
int Part::GetPinLayer( int pin_index )
{
	if( this->shape->m_padstack[pin_index].hole_size )
		return LAY_PAD_THRU;
	else if( this->side == 0 && this->shape->m_padstack[pin_index].top.shape != PAD_NONE
		|| this->side == 1 && this->shape->m_padstack[pin_index].bottom.shape != PAD_NONE )
		return LAY_TOP_COPPER;
	else
		return LAY_BOTTOM_COPPER;
}

// Get pin net
//
Net * Part::GetPinNet(QString pin_name )
{
	int pin_index = this->shape->GetPinIndexByName( pin_name );
	if( pin_index == -1 )
		return NULL;
	return this->pin[pin_index].net;
}

// Get pin net
//
Net * Part::GetPinNet( int pin_index )
{
	return this->pin[pin_index].net;
}



// returns description of part
//
QString Part::GetDescription()
{
	QString line = QString("part: %1\n").arg(part->ref_des );

	if( part->shape )
		line.append( QString("  ref_text: %1 %2 %3 %4 %5 %6\n").arg(part->m_ref_size)
					 .arg(part->m_ref_w).arg(part->m_ref_angle%360)
					 .arg(part->m_ref_xi).arg(part->m_ref_yi).arg(part->m_ref_vis ));
	else
		line.append( "  ref_text: \n" );

	line.append(QString("  package: \"%1\"\n").arg(part->package) );

	if( part->value != "" )
	{
		if( part->shape )
			line.append( QString("  value: \"%1\" %2 %3 %4 %5 %6 %7\n")
						 .arg(part->value).arg(part->m_value_size)
						 .arg(part->m_value_w).arg(part->m_value_angle%360)
						 .arg(part->m_value_xi).arg(part->m_value_yi)
						 .arg(part->m_value_vis) );
		else
			line.append( QString("  value: \"%1\"\n").arg(part->value) );
	}
	if( part->shape )
		line.append( QString("  shape: \"%1\"\n").arg(part->shape->m_name) );
	else
		line.append( "  shape: \n" );

	line.append( QString("  pos: %d %d %d %d %d\n\n").arg(part->x).arg(part->y).arg(part->side).arg(part->angle%360).arg(part->glued) );

	return line;
}

#if 0
// create record describing part for use by CUndoList
// if part == NULL, just set m_plist and new_ref_des
//
undo_part * Part::CreatePartUndoRecord( Part * part, CString * new_ref_des )
{
	int size = sizeof( undo_part );
	if( part )
		size = sizeof( undo_part ) + part->shape->GetNumPins()*(CShape::MAX_PIN_NAME_SIZE+1);
	undo_part * upart = (undo_part*)malloc( size );
	upart->size = size;
	upart->m_plist = this;
	if( part )
	{
		char * chptr = (char*)upart;
		chptr += sizeof(undo_part);
		upart->m_id = part->m_id;
		upart->visible = part->visible;
		upart->x = part->x;
		upart->y = part->y;
		upart->side = part->side;
		upart->angle = part->angle;
		upart->glued = part->glued;
		upart->m_ref_vis = part->m_ref_vis;
		upart->m_ref_xi = part->m_ref_xi;
		upart->m_ref_yi = part->m_ref_yi;
		upart->m_ref_angle = part->m_ref_angle;
		upart->m_ref_size = part->m_ref_size;
		upart->m_ref_w = part->m_ref_w;
		upart->m_value_vis = part->m_value_vis;
		upart->m_value_xi = part->m_value_xi;
		upart->m_value_yi = part->m_value_yi;
		upart->m_value_angle = part->m_value_angle;
		upart->m_value_size = part->m_value_size;
		upart->m_value_w = part->m_value_w;
		strcpy( upart->ref_des, part->ref_des );
		strcpy( upart->package , part->package );
		strcpy( upart->shape_name, part->shape->m_name );
		strcpy( upart->value, part->value );
		upart->shape = part->shape;
		if( part->shape )
		{
			// save names of nets attached to each pin
			for( int ip=0; ip<part->shape->GetNumPins(); ip++ )
			{
				if( cnet * net = part->pin[ip].net )
					strcpy( chptr, net->name );
				else
					*chptr = 0;
				chptr += CShape::MAX_PIN_NAME_SIZE + 1;
			}
		}
	}
	if( new_ref_des )
		strcpy( upart->new_ref_des, *new_ref_des );
	else
		strcpy( upart->new_ref_des, part->ref_des );
	return upart;
}
#endif

#if 0
// create special record for use by CUndoList
//
void * Part::CreatePartUndoRecordForRename( Part * part, CString * old_ref_des )
{
	int size = sizeof( undo_part );
	undo_part * upart = (undo_part*)malloc( size );
	upart->m_plist = this;
	strcpy( upart->ref_des, part->ref_des );
	strcpy( upart->package, *old_ref_des );
	return (void*)upart;
}
#endif

#if 0
// undo an operation on a part
// note that this is a static function, for use as a callback
//
void Part::PartUndoCallback( int type, void * ptr, bool undo )
{
	undo_part * upart = (undo_part*)ptr;

	if( undo )
	{
		// perform undo
		CString new_ref_des = upart->new_ref_des;
		CString old_ref_des = upart->ref_des;
		CPartList * pl = upart->m_plist;
		CDisplayList * old_dlist = pl->m_dlist;
		Part * part = pl->GetPart( new_ref_des );
		if( type == UNDO_PART_ADD )
		{
			// part was added, just delete it
			pl->m_nlist->PartDeleted( part );
			pl->Remove( part );
		}
		else if( type == UNDO_PART_DELETE )
		{
			// part was deleted, lookup shape in cache and add part
			pl->m_dlist = NULL;		// prevent drawing
			CShape * s;
			void * s_ptr;
			int err = pl->m_footprint_cache_map->Lookup( upart->shape_name, s_ptr );
			if( err )
			{
				// found in cache
				s = (CShape*)s_ptr;
			}
			else
				ASSERT(0);	// shape not found
			CString ref_des = upart->ref_des;
			CString package = upart->package;
			part = pl->Add( s, &ref_des, &package, upart->x, upart->y,
				upart->side, upart->angle, upart->visible, upart->glued );
			part->m_ref_vis = upart->m_ref_vis;
			part->m_ref_xi = upart->m_ref_xi;
			part->m_ref_yi = upart->m_ref_yi;
			part->m_ref_angle = upart->m_ref_angle;
			part->m_ref_size = upart->m_ref_size;
			part->m_ref_w = upart->m_ref_w;
			part->value = upart->value;
			part->m_value_vis = upart->m_value_vis;
			part->m_value_xi = upart->m_value_xi;
			part->m_value_yi = upart->m_value_yi;
			part->m_value_angle = upart->m_value_angle;
			part->m_value_size = upart->m_value_size;
			part->m_value_w = upart->m_value_w;
			pl->m_dlist = old_dlist;	// turn drawing back on;
			pl->DrawPart( part );
			pl->m_nlist->PartAdded( part );
		}
		else if( type == UNDO_PART_MODIFY )
		{
			// part was moved or modified
			pl->UndrawPart( part );
			pl->m_dlist = NULL;		// prevent further drawing
			if( upart->shape != part->shape )
			{
				// footprint was changed
				pl->PartFootprintChanged( part, upart->shape );
				pl->m_nlist->PartFootprintChanged( part );
			}
			if( upart->x != part->x
				|| upart->y != part->y
				|| upart->angle != part->angle
				|| upart->side != part->side )
			{
				pl->Move( part, upart->x, upart->y, upart->angle, upart->side );
				pl->m_nlist->PartMoved( part );
			}
			part->glued = upart->glued;
			part->m_ref_vis = upart->m_ref_vis;
			part->m_ref_xi = upart->m_ref_xi;
			part->m_ref_yi = upart->m_ref_yi;
			part->m_ref_angle = upart->m_ref_angle;
			part->m_ref_size = upart->m_ref_size;
			part->m_ref_w = upart->m_ref_w;
			part->value = upart->value;
			part->m_value_vis = upart->m_value_vis;
			part->m_value_xi = upart->m_value_xi;
			part->m_value_yi = upart->m_value_yi;
			part->m_value_angle = upart->m_value_angle;
			part->m_value_size = upart->m_value_size;
			part->m_value_w = upart->m_value_w;
			char * chptr = (char*)ptr + sizeof( undo_part );
			for( int ip=0; ip<part->shape->GetNumPins(); ip++ )
			{
				if( *chptr != 0 )
				{
					CString net_name = chptr;
					cnet * net = pl->m_nlist->GetNetPtrByName( &net_name );
					part->pin[ip].net = net;
				}
				else
					part->pin[ip].net = NULL;
				chptr += MAX_NET_NAME_SIZE + 1;
			}
			// if part was renamed
			if( new_ref_des != old_ref_des )
			{
				pl->m_nlist->PartRefChanged( &new_ref_des, &old_ref_des );
				part->ref_des = old_ref_des;
			}
			pl->m_dlist = old_dlist;	// turn drawing back on
			pl->DrawPart( part );
		}
		else
			ASSERT(0);
	}
	free(ptr);	// delete the undo record
}
#endif


// checks to see if a pin is connected to a trace or a copper area on a
// particular layer
//
// returns: ON_NET | TRACE_CONNECT | AREA_CONNECT
// where:
//		ON_NET = 1 if pin is on a net
//		TRACE_CONNECT = 2 if pin connects to a trace
//		AREA_CONNECT = 4 if pin connects to copper area
//
int Part::GetPinConnectionStatus( QString pin_name, int layer )
{
	int pin_index = this->shape->GetPinIndexByName( pin_name );
	Net * net = this->pin[pin_index].net;
	if( !net )
		return NOT_CONNECTED;

	int status = ON_NET;

	// now check for traces
	for( int ic=0; ic<net->nconnects; ic++ )
	{
		int nsegs = net->connect[ic].nsegs;
		int p1 = net->connect[ic].start_pin;
		int p2 = net->connect[ic].end_pin;
		if( net->pin[p1].part == part &&
			net->pin[p1].pin_name == pin_name &&
			net->connect[ic].seg[0].layer == layer )
		{
			// first segment connects to pin on this layer
			status |= TRACE_CONNECT;
		}
		else if( p2 == Connection::NO_END )
		{
			// stub trace, ignore end pin
		}
		else if( net->pin[p2].part == part &&
			net->pin[p2].pin_name == *pin_name &&
			net->connect[ic].seg[nsegs-1].layer == layer )
		{
			// last segment connects to pin on this layer
			status |= TRACE_CONNECT;
			break;
		}
	}
	// now check for connection to copper area
	for( int ia=0; ia<net->nareas; ia++ )
	{
		Area * a = &net->area[ia];
		for( int ip=0; ip<a->npins; ip++ )
		{
			Pin * pin = &net->pin[a->pin[ip]];
			if( pin->part == this
				&& pin->pin_name == *pin_name
				&& a->poly->GetLayer() == layer )
			{
				status |= AREA_CONNECT;
				break;
			}
		}
	}
	return status;
}


// Function to provide info to draw pad in Gerber file (also used by DRC routines)
// On return:
//	if no footprint for part, or no pad and no hole on this layer, returns 0
//	else returns 1 with:
//		*type = pad shape
//		*x = pin x,
//		*y = pin y,
//		*w = pad width,
//		*l = pad length,
//		*r = pad corner radius,
//		*hole = pin hole diameter,
//		*angle = pad angle,
//		**net = pin net,
//		*connection_status = ON_NET | TRACE_CONNECT | AREA_CONNECT, where:
//			ON_NET = 1 if pin is on a net
//			TRACE_CONNECT = 2 if pin connects to a trace on this layer
//			AREA_CONNECT = 4 if pin connects to copper area on this layer
//		*pad_connect_flag =
//			PAD_CONNECT_DEFAULT if pad uses default from project
//			PAD_CONNECT_NEVER if pad never connects to copper area
//			PAD_CONNECT_THERMAL if pad connects to copper area with thermal
//			PAD_CONNECT_NOTHERMAL if pad connects to copper area without thermal
//		*clearance_type =
//			CLEAR_NORMAL if clearance from copper area required
//			CLEAR_THERMAL if thermal connection to copper area
//			CLEAR_NONE if no clearance from copper area
// For copper layers:
//	if no pad, uses annular ring if connected
//	Uses GetPinConnectionStatus() to determine connections, this uses the area
//	connection info from the netlist
//
int Part::GetPadDrawInfo( int ipin, int layer,
							  bool bUse_TH_thermals, bool bUse_SMT_thermals,
							  int solder_mask_swell, int paste_mask_shrink,
							  int * type, int * x, int * y, int * w, int * l, int * r, int * hole,
							  int * angle, Net ** net,
							  int * connection_status, int * pad_connect_flag,
							  int * clearance_type )
{
	// get footprint
	Footprint * s = this->shape;
	if( !s )
		return 0;

	// get pin and padstack info
	Padstack * ps = &s->m_padstack[ipin];
	bool bUseDefault = false; // if true, use copper pad for mask
	QString pin_name = s->GetPinNameByIndex( ipin );
	int connect_status = GetPinConnectionStatus( pin_name, layer );
	// set default return values for no pad and no hole
	int ret_code = 0;
	int ttype = PAD_NONE;
	int xx = this->pin[ipin].x;
	int yy = this->pin[ipin].y;
	int ww = 0;
	int ll = 0;
	int rr = 0;
	int aangle = s->m_padstack[ipin].angle + this->angle;
	aangle = aangle%180;
	int hole_size = s->m_padstack[ipin].hole_size;
	Net * nnet = this->pin[ipin].net;
	int clear_type = CLEAR_NORMAL;
	int connect_flag = PAD_CONNECT_DEFAULT;

	// get pad info
	Pad * p = NULL;
	if( (layer == LAY_TOP_COPPER && this->side == 0 )
		|| (layer == LAY_BOTTOM_COPPER && this->side == 1 ) )
	{
		// top copper pad is on this layer
		p = &ps->top;
	}
	else if( (layer == LAY_MASK_TOP && this->side == 0 )
		|| (layer == LAY_MASK_BOTTOM && this->side == 1 ) )
	{
		// top mask pad is on this layer
		if( ps->top_mask.shape != PAD_DEFAULT )
			p = &ps->top_mask;
		else
		{
			bUseDefault = true;		// get mask pad from copper pad
			p = &ps->top;
		}
	}
	else if( (layer == LAY_PASTE_TOP && this->side == 0 )
		|| (layer == LAY_PASTE_BOTTOM && this->side == 1 ) )
	{
		// top paste pad is on this layer
		if( ps->top_paste.shape != PAD_DEFAULT )
			p = &ps->top_paste;
		else
		{
			bUseDefault = true;
			p = &ps->top;
		}
	}
	else if( (layer == LAY_TOP_COPPER && this->side == 1 )
			|| (layer == LAY_BOTTOM_COPPER && this->side == 0 ) )
	{
		// bottom copper pad is on this layer
		p = &ps->bottom;
	}
	else if( (layer == LAY_MASK_TOP && this->side == 1 )
		|| (layer == LAY_MASK_BOTTOM && this->side == 0 ) )
	{
		// bottom mask pad is on this layer
		if( ps->bottom_mask.shape != PAD_DEFAULT )
			p = &ps->bottom_mask;
		else
		{
			bUseDefault = true;
			p = &ps->bottom;
		}
	}
	else if( (layer == LAY_PASTE_TOP && this->side == 1 )
		|| (layer == LAY_PASTE_BOTTOM && this->side == 0 ) )
	{
		// bottom paste pad is on this layer
		if( ps->bottom_paste.shape != PAD_DEFAULT )
			p = &ps->bottom_paste;
		else
		{
			bUseDefault = true;
			p = &ps->bottom;
		}
	}
	else if( layer > LAY_BOTTOM_COPPER && ps->hole_size > 0 )
	{
		// inner pad is on this layer
		p = &ps->inner;
	}

	// now set parameters for return
	if( p )
		connect_flag = p->connect_flag;
	if( p == NULL )
	{
		// no pad definition, return defaults
	}
	else if( p->shape == PAD_NONE && ps->hole_size == 0 )
	{
		// no hole, no pad, return defaults
	}
	else if( p->shape == PAD_NONE )
	{
		// hole, no pad
		ret_code = 1;
		if( connect_status > ON_NET )
		{
			// connected to copper area or trace
			// make annular ring
			ret_code = 1;
			ttype = PAD_ROUND;
			ww = 2*m_annular_ring + hole_size;
		}
		else if( ( layer == LAY_MASK_TOP || layer == LAY_MASK_BOTTOM ) && bUseDefault )
		{
			// if solder mask layer and no mask pad defined, treat hole as pad to get clearance
			ret_code = 1;
			ttype = PAD_ROUND;
			ww = hole_size;
		}
	}
	else if( p->shape != PAD_NONE )
	{
		// normal pad
		ret_code = 1;
		ttype = p->shape;
		ww = p->size_h;
		ll = 2*p->size_l;
		rr = p->radius;
	}
	else
		ASSERT(0);	// error

	// adjust mask and paste pads if necessary
	if( (layer == LAY_MASK_TOP || layer == LAY_MASK_BOTTOM) && bUseDefault )
	{
		ww += 2*solder_mask_swell;
		ll += 2*solder_mask_swell;
		rr += solder_mask_swell;
	}
	else if( (layer == LAY_PASTE_TOP || layer == LAY_PASTE_BOTTOM) && bUseDefault )
	{
		if( ps->hole_size == 0 )
		{
			ww -= 2*paste_mask_shrink;
			ll -= 2*paste_mask_shrink;
			rr -= paste_mask_shrink;
			if( rr < 0 )
				rr = 0;
		}
		else
		{
			ww = ll = 0;	// no paste for through-hole pins
		}
	}

	// if copper layer connection, decide on thermal
	if( layer >= LAY_TOP_COPPER && (connect_status & AREA_CONNECT) )
	{
		// copper area connection, thermal or not?
		if( p->connect_flag == PAD_CONNECT_NEVER )
			ASSERT(0);	// shouldn't happen, this is an error by GetPinConnectionStatus(...)
		else if( p->connect_flag == PAD_CONNECT_NOTHERMAL )
			clear_type = CLEAR_NONE;
		else if( p->connect_flag == PAD_CONNECT_THERMAL )
			clear_type = CLEAR_THERMAL;
		else if( p->connect_flag == PAD_CONNECT_DEFAULT )
		{
			if( bUse_TH_thermals && ps->hole_size )
				clear_type = CLEAR_THERMAL;
			else if( bUse_SMT_thermals && !ps->hole_size )
				clear_type = CLEAR_THERMAL;
			else
				clear_type = CLEAR_NONE;
		}
		else
			ASSERT(0);
	}
	if( x )
		*x = xx;
	if( y )
		*y = yy;
	if( type )
		*type = ttype;
	if( w )
		*w = ww;
	if( l )
		*l = ll;
	if( r )
		*r = rr;
	if( hole )
		*hole = hole_size;
	if( angle )
		*angle = aangle;
	if( connection_status )
		*connection_status = connect_status;
	if( net )
		*net = nnet;
	if( pad_connect_flag )
		*pad_connect_flag = connect_flag;
	if( clearance_type )
		*clearance_type = clear_type;
	return ret_code;
}

