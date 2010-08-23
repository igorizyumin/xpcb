// Shape.cpp : implementation of CShape class
//
#include "smfontutil.h"
#include "Shape.h"
#include <math.h> 
#include "Log.h"
#include "Line.h"

/////////////////////// PAD /////////////////////////
// class pad
Pad::Pad() :
		mShape(PAD_NONE),
		mWidth(0),
		mHeight(0),
		mConnFlag(PAD_CONNECT_DEFAULT)
{
}

bool Pad::operator==(const Pad & p) const
{ 
	return( mShape==p.mShape
			&& mWidth==p.mWidth
			&& mHeight==p.mHeight
			); 
}

Pad Pad::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "pad");
	Pad p;
	QXmlStreamAttributes attr(reader.attributes());
	QStringRef t = attr.value("shape");
	if (t == "round")
	{
		p.mShape = PAD_ROUND;
		p.mWidth = attr.value("width").toString().toInt();
	}
	else if (t == "octagon")
	{
		p.mShape = PAD_OCTAGON;
		p.mWidth = attr.value("width").toString().toInt();
	}
	else if (t == "square")
	{
		p.mShape = PAD_SQUARE;
		p.mWidth = attr.value("width").toString().toInt();
	}
	else if (t == "rect")
	{
		p.mShape = PAD_RECT;
		p.mWidth = attr.value("width").toString().toInt();
		p.mHeight = attr.value("height").toString().toInt();
	}
	else if (t == "obround")
	{
		p.mShape = PAD_OBROUND;
		p.mWidth = attr.value("width").toString().toInt();
		p.mHeight = attr.value("height").toString().toInt();
	}
	return p;
}

bool Pad::testHit( const QPoint& pt )
{
	double dist = sqrt(pt.x()*pt.x() + pt.y()*pt.y());

	// check if we hit the pad
	switch( mShape )
	{
	case PAD_NONE:
		break;
	case PAD_ROUND:
	case PAD_OCTAGON:
		if( dist < (mWidth/2) )
			return true;
		break;
	case PAD_SQUARE:
	case PAD_RECT:
	case PAD_OBROUND:
		if( abs(pt.x()) < (mWidth/2) && abs(pt.y()) < (mHeight/2) )
			return true;
		break;
	}

	// did not hit anything
	return false;
}

QRect Pad::bbox() const
{
	switch( mShape )
	{
	case PAD_NONE:
	default:
		return QRect();
	case PAD_ROUND:
	case PAD_OCTAGON:
		return QRect(-mWidth/2, -mWidth/2, mWidth, mWidth);
	case PAD_SQUARE:
	case PAD_RECT:
	case PAD_OBROUND:
		return QRect(-mWidth/2, -mHeight/2, mWidth, mHeight);
	}
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
			&& start==p.start
			&& start_mask==p.start_mask
			&& start_paste==p.start_paste
			&& end==p.end
			&& end_mask==p.end_mask
			&& end_paste==p.end_paste
			&& inner==p.inner				
			); 
}

Padstack* Padstack::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "padstack");
	Padstack *p = new Padstack();
	if (reader.attributes().hasAttribute("name"))
		p->name = reader.attributes().value("name").toString();
	p->hole_size = reader.attributes().value("holesize").toString().toInt();
	while(reader.readNextStartElement())
	{
		QStringRef padtype = reader.name();
		reader.readNext();
		if (reader.isEndElement())
			continue; // no pad
		// we have a pad.  create it
		if (padtype == "startpad")
			p->start = Pad::newFromXML(reader);
		else if (padtype == "innerpad")
			p->inner = Pad::newFromXML(reader);
		else if (padtype == "endpad")
			p->end = Pad::newFromXML(reader);
		else if (padtype == "startmask")
			p->start_mask = Pad::newFromXML(reader);
		else if (padtype == "endmask")
			p->end_mask = Pad::newFromXML(reader);
		else if (padtype == "startpaste")
			p->start_paste = Pad::newFromXML(reader);
		else if (padtype == "endpaste")
			p->end_paste = Pad::newFromXML(reader);
	}
	return p;
}

QRect Padstack::bbox() const
{
	QRect ret;
	ret |= start.bbox();
	ret |= start_mask.bbox();
	ret |= start_paste.bbox();
	ret |= inner.bbox();
	ret |= end.bbox();
	ret |= end_mask.bbox();
	ret |= end_paste.bbox();
	return ret;
}

/////////////////////// PIN /////////////////////////

Pin Pin::newFromXML(QXmlStreamReader &reader, const QHash<int, Padstack*> & padstacks, Footprint *fp)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "pin");
	QXmlStreamAttributes attr = reader.attributes();

	Pin p(fp);
	p.mName = attr.value("name").toString();
	p.mPos = QPoint(attr.value("x").toString().toInt(), attr.value("y").toString().toInt());
	p.mAngle = attr.value("rot").toString().toInt();
	p.updateTransform();
	int psind = attr.value("padstack").toString().toInt();
	if (padstacks.contains(psind))
		p.mPadstack = padstacks.value(psind);
	else
	{
		Log::instance().error("Error creating pin: missing padstack");
	}
	return p;
}

bool Pin::testHit(const QPoint &pt, PINLAYER layer) const
{
	Padstack *ps = padstack();
	// check if we hit a thru-hole
	QPoint delta( pt - pos() );
	double dist = sqrt( delta.x()*delta.x() + delta.y()*delta.y() );
	if( dist < ps->getHole()/2 )
		return true;

	Pad pad = getPadOnLayer(layer);
	if (!pad.isNull())
		return pad.testHit(pt);

	return false;
}

Pad Pin::getPadOnLayer(PINLAYER layer) const
{
	Padstack *ps = padstack();

	switch(layer)
	{
	case LAY_START:
		return ps->getStartPad();
	case LAY_END:
		return ps->getEndPad();
	case LAY_INNER:
		return ps->getInnerPad();
	case LAY_UNKNOWN:
		return Pad();
	}
}

QRect Pin::bbox() const
{
	return mPartTransform.mapRect(padstack()->bbox());
}

void Pin::updateTransform()
{
	mPartTransform.reset();
	mPartTransform.translate(mPos.x(), mPos.y());
	mPartTransform.rotate(mAngle);
}



/////////////////////// FOOTPRINT /////////////////////////

Footprint::Footprint()
	: mName("EMPTY_SHAPE"), mUnits(MIL), mCustomCentroid(false)
{
} 

// destructor
//
Footprint::~Footprint()
{
}

void Footprint::draw(QPainter *painter, PCBLAYER layer)
{
}

Footprint* Footprint::newFromXML(QXmlStreamReader &reader, const QHash<int, Padstack*> &padstacks)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "footprint");

	Footprint *fp = new Footprint();
	while(reader.readNextStartElement())
	{
		QStringRef el = reader.name();
		if (el == "name")
		{
			fp->mName = reader.readElementText();
		}
		else if (el == "units")
		{
			if (reader.readElementText() == "mm")
				fp->mUnits = MM;
			else
				fp->mUnits = MIL;
		}
		else if (el == "author")
		{
			fp->mAuthor = reader.readElementText();
		}
		else if (el == "desc")
		{
			fp->mDesc = reader.readElementText();
		}
		else if (el == "centroid")
		{
			QXmlStreamAttributes attr = reader.attributes();
			fp->mCentroid = QPoint(attr.value("x").toString().toInt(),
								   attr.value("y").toString().toInt());
			if (attr.hasAttribute("custom"))
				fp->mCustomCentroid = (attr.value("custom") == "1");
		}
		else if (el == "line")
		{
			fp->mOutlineLines.append(Line::newFromXml(reader));
		}
		else if (el == "arc")
		{
			fp->mOutlineArcs.append(Arc::newFromXml(reader));
		}
		else if (el == "pins")
		{
			while(reader.readNextStartElement())
			{
				Pin pin = Pin::newFromXML(reader, padstacks, fp);
				fp->mPins.append(pin);
			}
		}
		else if (el == "text")
		{
			fp->mTexts.append(Text::newFromXML(reader));
		}
		else if (el == "refText")
		{
			QXmlStreamAttributes attr = reader.attributes();
			fp->mRefText.setPos(QPoint(attr.value("x").toString().toInt(),
									   attr.value("y").toString().toInt()));
			fp->mRefText.setAngle(attr.value("rot").toString().toInt());
			fp->mRefText.setFontSize(attr.value("textSize").toString().toInt());
			fp->mRefText.setStrokeWidth(attr.value("lineWidth").toString().toInt());
		}
		else if (el == "valueText")
		{
			QXmlStreamAttributes attr = reader.attributes();
			fp->mValueText.setPos(QPoint(attr.value("x").toString().toInt(),
									   attr.value("y").toString().toInt()));
			fp->mValueText.setAngle(attr.value("rot").toString().toInt());
			fp->mValueText.setFontSize(attr.value("textSize").toString().toInt());
			fp->mValueText.setStrokeWidth(attr.value("lineWidth").toString().toInt());
		}
	}
	return fp;
}

int Footprint::numPins() const
{
	return mPins.size();
}

const Pin* Footprint::getPin( const QString & name ) const
{	
	foreach(const Pin& p, mPins)
	{
		if (p.name() == name)
			return &p;
	}
	return NULL;
}

// Get default centroid
// if no pads, returns (0,0)
QPoint Footprint::getDefaultCentroid()
{
	if( numPins() == 0 )
		return QPoint(0,0);
	return getPinBounds().center();
}

// Get bounding rectangle of all pads
// if no pads, returns null rectangle
QRect Footprint::getPinBounds() const
{
	QRect r;
	foreach(const Pin& p, mPins)
	{
		r |= p.bbox();
	}
	return r;
}

// move to pin
#if 0
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
#endif

// Get bounding rectangle of footprint
//
QRect Footprint::bbox() const
{
	QRect r = getPinBounds();
	foreach(const Line& l, mOutlineLines)
	{
		r |= l.bbox();
	}
	foreach(const Arc& a, mOutlineArcs)
	{
		r |= a.bbox();
	}
	foreach(const Text& t, mTexts)
	{
		r |= t.bbox();
	}

	// should this be included?
	//r |= mRefText.bbox();
	//r |= mValueText.bbox();

	return r;
}

