// Shape.cpp : implementation of CShape class
//
#include "smfontutil.h"
#include "Shape.h"
#include <math.h> 
#include "utility.h"
#include "TextList.h"
#include "Log.h"

/////////////////////// PAD /////////////////////////
// class pad
Pad::Pad() :
		shape(PAD_NONE),
		width(0),
		height(0)
{
}

bool Pad::operator==(Pad p)
{ 
	return( shape==p.shape 
			&& width==p.width
			&& height==p.height
			); 
}

Pad* Pad::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "pad");
	Pad p;
	QXmlStreamAttributes attr(reader.attributes());
	switch(attr.value("shape"))
	{
	case "round":
		p.shape = PAD_ROUND;
		p.width = attr.value("width").toString().toInt();
		break;
	case "octagon":
		p.shape = PAD_OCTAGON;
		p.width = attr.value("width").toString().toInt();
		break;
	case "square":
		p.shape = PAD_SQUARE;
		p.width = attr.value("width").toString().toInt();
		break;
	case "rect":
		p.shape = PAD_RECT;
		p.width = attr.value("width").toString().toInt();
		p.height = attr.value("height").toString().toInt();
		break;
	case "obround":
		p.shape = PAD_OBROUND;
		p.width = attr.value("width").toString().toInt();
		p.height = attr.value("height").toString().toInt();
		break;
	}
	return p;
}




/////////////////////// PADSTACK /////////////////////////

// class padstack
Padstack::Padstack() :
		hole_size(0)
{
}

bool Padstack::operator==(const Padstack &p) const
{ 
	return( name == p.name
			&& hole_size==p.hole_size 
			&& top==p.top
			&& top_mask==p.top_mask
			&& top_paste==p.top_paste
			&& bottom==p.bottom
			&& bottom_mask==p.bottom_mask
			&& bottom_paste==p.bottom_paste
			&& inner==p.inner				
			); 
}

Padstack* Padstack::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "padstack");
	Padstack *p = new Padstack();
	if (reader.attributes().hasAttribute("name"))
		p->name = reader.attributes().value("name");
	p->hole_size = reader.attributes().value("holesize").toString().toInt();
	while(reader.readNextStartElement())
	{
		QString padtype = reader.name();
		reader.readNext();
		if (reader.isEndElement())
			continue; // no pad
		// we have a pad.  create it
		switch(padtype)
		{
		case "startpad":
			p->start = Pad::newFromXML(reader);
			break;
		case "innerpad":
			p->inner = Pad::newFromXML(reader);
			break;
		case "endpad":
			p->end = Pad::newFromXML(reader);
			break;
		case "startmask":
			p->start_mask = Pad::newFromXML(reader);
			break;
		case "endmask":
			p->end_mask = Pad::newFromXML(reader);
			break;
		case "startpaste":
			p->start_paste = Pad::newFromXML(reader);
			break;
		case "endpaste":
			p->end_paste = Pad::newFromXML(reader);
			break;
		}
	}
	return p;
}

/////////////////////// PIN /////////////////////////

static Pin Pin::newFromXML(QXmlStreamReader &reader, QHash<int, Padstack*> & padstacks, Footprint *fp)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "pin");
	QXmlStreamAttributes attr = reader.attributes();

	Pin p(fp);
	p.mName = attr.value("name");
	p.mPos = QPoint(attr.value("x").toString().toInt(), attr.value("y").toString().toInt());
	p.mAngle = QPoint(attr.value("rot").toString().toInt());
	int psind = attr.value("padstack").toString().toInt();
	if (padstacks.contains(psind))
		p.mPadstack = padstacks.value(psind);
	else
	{
		Log::instance().error("Error creating pin: missing padstack");
	}
	return p;
}


/////////////////////// FOOTPRINT /////////////////////////

Footprint::Footprint()
{
	m_tl = new CTextList;	
	Clear();
} 

// destructor
//
Footprint::~Footprint()
{
	Clear();
	delete m_tl;
}

Footprint* Footprint::newFromXML(QXmlStreamReader &reader, QHash<int, Padstack*> &padstacks)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "footprint");

	Footprint *fp = new Footprint();
	while(reader.readNextStartElement())
	{
		switch(reader.name())
		{
		case "name":
			fp->mName = reader.readElementText();
			break;
		case "units":
			if (reader.readElementText() == "mm")
				fp->mUnits = MM;
			else
				fp->mUnits = MIL;
			break;
		case "author":
			fp->mAuthor = reader.readElementText();
			break;
		case "desc":
			fp->mDesc = reader.readElementText();
			break;
		case "centroid":
			QXmlStreamAttributes attr = reader.attributes();
			fp->mCentroid = QPoint(attr.value("x").toString().toInt(),
								   attr.value("y").toString().toInt());
			if (attr.hasAttribute("custom"))
				fp->mCustomCentroid = (attr.value("custom") == "1");
			break;
		case "polyline":
			PolyLine *p = PolyLine::newFromXML(reader);
			fp->mOutline.append(p);
			break;
		case "pins":
			while(reader.readNextStartElement())
			{
				Pin pin = Pin::newFromXML(reader, padstacks, fp);
				fp->mPins.append(pin);
			}
			break;
		case "refText":
			QXmlStreamAttributes attr = reader.attributes();
			fp->mRefText.setPos(QPoint(attr.value("x").toString().toInt(),
									   attr.value("y").toString().toInt()));
			fp->mRefText.setAngle(attr.value("rot").toString().toInt());
			fp->mRefText.setFontSize(attr.value("textSize").toString().toInt());
			fp->mRefText.setStrokeWidth(attr.value("lineWidth").toString().toInt());
			break;
		case "valueText":
			QXmlStreamAttributes attr = reader.attributes();
			fp->mValueText.setPos(QPoint(attr.value("x").toString().toInt(),
									   attr.value("y").toString().toInt()));
			fp->mValueText.setAngle(attr.value("rot").toString().toInt());
			fp->mValueText.setFontSize(attr.value("textSize").toString().toInt());
			fp->mValueText.setStrokeWidth(attr.value("lineWidth").toString().toInt());
			break;
		}
	}
	return fp;
}

void Footprint::Clear()
{
	m_name = "EMPTY_SHAPE";
	m_author = "";
	m_source = "";
	m_desc = "";
	m_units = MIL;
	m_sel_xi = m_sel_yi = 0;
	m_sel_xf = m_sel_yf = 500*NM_PER_MIL;
	m_ref_size = 100*NM_PER_MIL;
	m_ref_xi = 100*NM_PER_MIL;
	m_ref_yi = 200*NM_PER_MIL;
	m_ref_angle = 0;
	m_ref_w = 10*NM_PER_MIL;
	m_value_size = 100*NM_PER_MIL;		
	m_value_xi = 100*NM_PER_MIL;
	m_value_yi = 0;
	m_value_angle = 0;
	m_value_w = 10*NM_PER_MIL;
	m_centroid_type = CENTROID_DEFAULT;
	m_centroid_x = 0;
	m_centroid_y = 0;
	m_centroid_angle = 0;
	m_padstack.clear();
	m_outline_poly.clear();
	m_tl->RemoveAllTexts();
	m_glue.clear();
}

// copy another shape into this shape
//
int Footprint::Copy( Footprint * shape )
{
	// description
	m_name = shape->m_name;
	m_author = shape->m_author;
	m_source = shape->m_source;
	m_desc = shape->m_desc;
	m_units = shape->m_units;
	// selection box
	m_sel_xi = shape->m_sel_xi;
	m_sel_yi = shape->m_sel_yi;
	m_sel_xf = shape->m_sel_xf;
	m_sel_yf = shape->m_sel_yf;
	// reference designator text
	m_ref_size = shape->m_ref_size;
	m_ref_w = shape->m_ref_w;
	m_ref_xi = shape->m_ref_xi;
	m_ref_yi = shape->m_ref_yi;
	m_ref_angle = shape->m_ref_angle;
	// value text
	m_value_size = shape->m_value_size;
	m_value_w = shape->m_value_w;
	m_value_xi = shape->m_value_xi;
	m_value_yi = shape->m_value_yi;
	m_value_angle = shape->m_value_angle;
	// centroid
	m_centroid_type = shape->m_centroid_type;
	m_centroid_x = shape->m_centroid_x;
	m_centroid_y = shape->m_centroid_y;
	m_centroid_angle = shape->m_centroid_angle;
	// padstacks
	m_padstack.RemoveAll();
	int np = shape->m_padstack.GetSize();
	m_padstack.SetSize( np );
	for( int i=0; i<np; i++ )
		m_padstack[i] = shape->m_padstack[i];
	// outline polys
	m_outline_poly.RemoveAll();
	np = shape->m_outline_poly.GetSize();
	m_outline_poly.SetSize(np);
	for( int ip=0; ip<np; ip++ )
		m_outline_poly[ip].Copy( &shape->m_outline_poly[ip] );
	// text
	m_tl->RemoveAllTexts();
	for( int it=0; it<shape->m_tl->text_ptr.GetSize(); it++ )
	{
		Text * t = shape->m_tl->text_ptr[it];
		m_tl->AddText( t->m_x, t->m_y, t->m_angle, t->m_mirror, t->m_bNegative, 
			LAY_FP_SILK_TOP, t->m_font_size, t->m_stroke_width, &t->m_str, false );
	}
	// glue spots
	int nd = shape->m_glue.GetSize();
	m_glue.SetSize( nd );
	for( int id=0; id<nd; id++ )
		m_glue[id] = shape->m_glue[id];
	return PART_NOERR;
}

bool Footprint::Compare( Footprint * shape )
{
	// parameters
	if( m_name != shape->m_name 
		|| m_author != shape->m_author 
		|| m_source != shape->m_source 
		|| m_desc != shape->m_desc 
		|| m_sel_xi != shape->m_sel_xi 
		|| m_sel_yi != shape->m_sel_yi 
		|| m_sel_xf != shape->m_sel_xf 
		|| m_sel_yf != shape->m_sel_yf 
		|| m_ref_size != shape->m_ref_size 
		|| m_ref_w != shape->m_ref_w 
		|| m_ref_xi != shape->m_ref_xi 
		|| m_ref_yi != shape->m_ref_yi 
		|| m_ref_angle != shape->m_ref_angle 
		|| m_value_size != shape->m_value_size 
		|| m_value_w != shape->m_value_w 
		|| m_value_xi != shape->m_value_xi 
		|| m_value_yi != shape->m_value_yi 
		|| m_value_angle != shape->m_value_angle )
			return false;

	// padstacks
	int np = m_padstack.size();
	if( np != shape->m_padstack.size() )
		return false;
	for( int i=0; i<np; i++ )
	{
		if(  !(m_padstack[i] == shape->m_padstack[i]) )
			return false;
	}
	// outline polys
	np = m_outline_poly.size();
	if( np != shape->m_outline_poly.size() )
		return false;
	for( int ip=0; ip<np; ip++ )
	{
		if (m_outline_poly[ip].GetLayer() != shape->m_outline_poly[ip].GetLayer() ) return false;
		if (m_outline_poly[ip].GetClosed() != shape->m_outline_poly[ip].GetClosed() ) return false;
		if (m_outline_poly[ip].GetHatch() != shape->m_outline_poly[ip].GetHatch() ) return false;
		if (m_outline_poly[ip].GetW() != shape->m_outline_poly[ip].GetW() ) return false;
//		if (m_outline_poly[ip].GetSelBoxSize() != shape->m_outline_poly[ip].GetSelBoxSize() ) return false;
		if (m_outline_poly[ip].GetNumCorners() != shape->m_outline_poly[ip].GetNumCorners() ) return false;
		for( int ic=0; ic<m_outline_poly[ip].GetNumCorners(); ic++ )
		{
			if (m_outline_poly[ip].GetX(ic) != shape->m_outline_poly[ip].GetX(ic) ) return false;
			if (m_outline_poly[ip].GetY(ic) != shape->m_outline_poly[ip].GetY(ic) ) return false;
			if( ic<(m_outline_poly[ip].GetNumCorners()-1) || m_outline_poly[ip].GetClosed() )
				if (m_outline_poly[ip].GetSideStyle(ic) != shape->m_outline_poly[ip].GetSideStyle(ic) ) return false;
		}
	}
	// text
	int nt = m_tl->text_ptr.GetSize();
	if( nt != shape->m_tl->text_ptr.GetSize() )
		return false;
	for( int it=0; it<m_tl->text_ptr.GetSize(); it++ )
	{
		Text * t = m_tl->text_ptr[it];
		Text * st = shape->m_tl->text_ptr[it];
		if( t->m_x != st->m_x
			|| t->m_y != st->m_y
			|| t->m_layer != st->m_layer
			|| t->m_angle != st->m_angle
			|| t->m_mirror != st->m_mirror
			|| t->m_font_size != st->m_font_size
			|| t->m_stroke_width != st->m_stroke_width
			|| t->m_str != st->m_str )
			return false;
	}
	return true;
}

int Footprint::GetNumPins()
{
	return m_padstack.size();
}

int Footprint::GetPinIndexByName( QString name )
{	
	for( int ip=0; ip<m_padstack.size(); ip++ )
	{
		if( m_padstack[ip].name == name )
			return ip;
	}
	return -1;		// error
}

QString Footprint::GetPinNameByIndex( int ip )
{
	return m_padstack[ip].name;
}

// Get default centroid
// if no pads, returns (0,0)
QPoint Footprint::GetDefaultCentroid()
{
	if( m_padstack.size() == 0 )
		return CPoint(0,0);
	QRect r = GetAllPadBounds();
	return r.center();
}

// Get bounding rectangle of all pads
// if no pads, returns with rect.left = INT_MAX:
QRect Footprint::GetAllPadBounds()
{
	QRect r;
	int left, right, bottom, top;
	left = bottom = INT_MAX;
	right = top = INT_MIN;
	for( int ip=0; ip<m_padstack.size(); ip++ )
	{
		QRect pad_r = GetPadBounds( ip );
		left = min( left, pad_r.left() );
		bottom = min( bottom, pad_r.bottom()+1 );
		right = max( right, pad_r.right()+1 );
		top = max( top, pad_r.top() );
	}
	return r;
}

// Get bounding rectangle of pad
//
QRect Footprint::GetPadBounds( int i )
{
	int dx=0, dy=0;
	Padstack * ps = &m_padstack[i];
	Pad * p = &ps->top;
	if( ps->top.shape == PAD_NONE && ps->bottom.shape != PAD_NONE )
		p = &ps->bottom;
	if( p->shape == PAD_NONE )
	{
		{
			dx = ps->hole_size/2;
			dy = dx;
		}
	}
	else if( p->shape == PAD_SQUARE || p->shape == PAD_ROUND || p->shape == PAD_OCTAGON )
	{
		dx = p->size_h/2;
		dy = dx;
	}
	else if( p->shape == PAD_RECT || p->shape == PAD_RRECT || p->shape == PAD_OVAL )
	{
		if( ps->angle == 0 || ps->angle == 180 )
		{
			dx = p->size_l;
			dy = p->size_h/2;
		}
		else if( ps->angle == 90 || ps->angle == 270 )
		{
			dx = p->size_h/2;
			dy = p->size_l;
		}
		else
			ASSERT(0);	// illegal angle
	}
	return QRect(ps->x_rel-dx, ps->y_rel-dy, 2*dx, 2*dy);

}

// Get bounding rectangle of row of pads
//
QRect Footprint::GetPadRowBounds( int i, int num )
{
	QRect r;
	int left, right, bottom, top;
	left = bottom = INT_MAX;
	right = top = INT_MIN;
	for( int ip=i; ip<(i+num); ip++ )
	{
		QRect pad_r = GetPadBounds( ip );
		left = min( left, pad_r.left() );
		bottom = min( bottom, pad_r.bottom()+1 );
		right = max( right, pad_r.right()+1 );
		top = max( top, pad_r.top() );
	}
	return r;
}

// Get bounding rectangle of footprint
//
QRect Footprint::GetBounds( bool bIncludeLineWidths )
{
	int left, right, bottom, top;
	left = bottom = INT_MAX;
	right = top = INT_MIN;
	for( int ip=0; ip<GetNumPins(); ip++ )
	{
		QRect r = GetPadBounds( ip );
		left = min( left, r.left() );
		bottom = min( bottom, r.bottom()+1);
		right = max( right, r.right()+1);
		top = max( top, r.top());
	}
	for( int ip=0; ip<m_outline_poly.GetSize(); ip++ )
	{
		QRect r;
		if( bIncludeLineWidths )
			r = m_outline_poly[ip].GetBounds();
		else
			r = m_outline_poly[ip].GetCornerBounds();
		left = min( left, r.left() );
		bottom = min( bottom, r.bottom()+1);
		right = max( right, r.right()+1);
		top = max( top, r.top());
	}
	QRect tr;
	bool bText = m_tl->GetTextBoundaries( tr );
	if( bText )
	{
		left = min( left, tr.left() );
		bottom = min( bottom, tr.bottom()+1);
		right = max( right, tr.right()+1);
		top = max( top, tr.top());
	}
	if(	left == INT_MAX || bottom == INT_MAX || right == INT_MIN || top == INT_MIN )
	{
		// no elements, make it a 100 mil square
		left = 0;
		right = 100*NM_PER_MIL;
		bottom = 0;
		top = 100*NM_PER_MIL;
	}
	return QRect(left, bottom, right-left, top-bottom);
}

// Get bounding rectangle of footprint, not including polyline widths
//
QRect Footprint::GetCornerBounds()
{
	return GetBounds( false );
}
