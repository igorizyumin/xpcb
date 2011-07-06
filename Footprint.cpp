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

#include "smfontutil.h"
#include "Footprint.h"
#include <math.h> 
#include "Log.h"
#include "Line.h"
#include "Document.h"
#include <QDir>

/////////////////////// PAD /////////////////////////
// class pad
Pad::Pad(PADSHAPE shape, int width, int height, int radius, PADCONNTYPE connType) :
		mShape(shape),
		mWidth(width),
		mLength(height),
		mRadius(radius),
		mConnFlag(connType)
{
}

bool Pad::operator==(const Pad & p) const
{ 
	return( mShape==p.mShape
			&& mWidth==p.mWidth
			&& mLength==p.mLength
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
		p.mLength = attr.value("height").toString().toInt();
	}
	else if (t == "obround")
	{
		p.mShape = PAD_OBROUND;
		p.mWidth = attr.value("width").toString().toInt();
		p.mLength = attr.value("height").toString().toInt();
	}
	else if (t == "default")
	{
		p.mShape = PAD_DEFAULT;
	}
	// read until the end of this element
	do
			reader.readNext();
	while(!reader.isEndElement());
	return p;
}

void Pad::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("pad");
	switch(mShape)
	{
	case PAD_ROUND:
		writer.writeAttribute("shape", "round");
		writer.writeAttribute("width", QString::number(mWidth));
		break;
	case PAD_OCTAGON:
		writer.writeAttribute("shape", "octagon");
		writer.writeAttribute("width", QString::number(mWidth));
		break;
	case PAD_SQUARE:
		writer.writeAttribute("shape", "square");
		writer.writeAttribute("width", QString::number(mWidth));
		break;
	case PAD_RECT:
		writer.writeAttribute("shape", "rect");
		writer.writeAttribute("width", QString::number(mWidth));
		writer.writeAttribute("height", QString::number(mLength));
		break;
	case PAD_OBROUND:
		writer.writeAttribute("shape", "obround");
		writer.writeAttribute("width", QString::number(mWidth));
		writer.writeAttribute("height", QString::number(mLength));
		break;
	case PAD_DEFAULT:
		writer.writeAttribute("shape", "default");
	default:
		break;
	}
	writer.writeEndElement();
}

bool Pad::testHit( const QPoint& pt )
{
	double dist = sqrt(pow(pt.x(), 2) + pow(pt.y(), 2));

	// check if we hit the pad
	switch( mShape )
	{
	case PAD_NONE:
	case PAD_DEFAULT:
		break;
	case PAD_ROUND:
	case PAD_OCTAGON:
	case PAD_SQUARE:
		if( dist < (mWidth/2) )
			return true;
		break;
	case PAD_RECT:
	case PAD_RRECT:
	case PAD_OBROUND:
		if( abs(pt.x()) < (mWidth/2) && abs(pt.y()) < (mLength/2) )
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
	case PAD_SQUARE:
		return QRect(-mWidth/2, -mWidth/2, mWidth, mWidth);
	case PAD_RECT:
	case PAD_OBROUND:
		return QRect(-mWidth/2, -mLength/2, mWidth, mLength);
	}
}

void Pad::draw(QPainter *painter) const
{
	switch(mShape)
	{
	case PAD_ROUND:
		painter->drawEllipse(QRect(-mWidth/2, -mWidth/2, mWidth, mWidth));
		break;
	case PAD_SQUARE:
		painter->drawRect(QRect(-mWidth/2, -mWidth/2, mWidth, mWidth));
		break;
	case PAD_RECT:
		painter->drawRect(QRect(-mWidth/2, -mLength/2, mWidth, mLength));
		break;
	case PAD_OCTAGON:
		{
			int x = mWidth * 0.2071 + 0.5;
			int w = mWidth * 0.5 + 0.5;
			QPoint pts[8] = {QPoint(x, w), QPoint(w, x), QPoint(w, -x), QPoint(x, -w),
							 QPoint(-x, -w), QPoint(-w, -x), QPoint(-w, x), QPoint(-x, w)};
			painter->drawConvexPolygon(pts, 8);
		}
		break;
	case PAD_OBROUND:
		if (mWidth > mLength)
		{
			// draw horizontal oval
			int wr = 0.5* (mWidth - mLength);
			painter->drawRect(-wr, -mLength/2, 2*wr, mLength);
			painter->drawPie(-mWidth/2, -mLength/2, mLength, mLength, -1440, -2880);
			painter->drawPie(wr-mLength/2, -mLength/2, mLength, mLength, 1440, -2880);
		}
		else
		{
			// draw vertical oval
			int wr = 0.5* (mLength - mWidth);
			painter->drawRect(-mWidth/2, -wr, mWidth, 2*wr);
			painter->drawPie(-mWidth/2, -mLength/2, mWidth, mWidth, 0, 2880);
			painter->drawPie(-mWidth/2, wr-mWidth/2, mWidth, mWidth, 0, -2880);
		}
		break;
	default:
		break;
	}
}
/////////////////////// PADSTACK /////////////////////////

// class padstack
Padstack::Padstack() :
		hole_size(0), mID(PCBObject::getNextID())
{
}

bool Padstack::operator==(const Padstack &p) const
{ 
	return( mName == p.mName
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

QSharedPointer<Padstack> Padstack::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "padstack");
	QSharedPointer<Padstack> p(new Padstack());
	if (reader.attributes().hasAttribute("name"))
		p->mName = reader.attributes().value("name").toString();
	p->hole_size = reader.attributes().value("holesize").toString().toInt();
	reader.readNextStartElement();
	do
	{
		// we are at the pad type element
		QStringRef padtype = reader.name();
		if (!reader.readNextStartElement())
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
		else
			Q_ASSERT(false);
		// read until the end of the pad type element
		do
				reader.readNext();
		while(!reader.isEndElement());
	}
	while(reader.readNextStartElement());
	return p;
}

void Padstack::draw(QPainter *painter, const Layer& layer) const
{
	switch(layer.type())
	{
	case Layer::LAY_START:
		start.draw(painter);
		break;
	case Layer::LAY_INNER:
		inner.draw(painter);
		break;
	case Layer::LAY_END:
		end.draw(painter);
		break;
	case Layer::LAY_HOLE:
		if (hole_size)
		{
			painter->drawEllipse(QRect(QPoint(-hole_size/2, -hole_size/2),
									   QPoint(hole_size/2, hole_size/2)));
		}
		break;
	case Layer::LAY_SELECTION:
		painter->drawRect(bbox());
	default:
		break;
	}
}

void Padstack::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("padstack");
	writer.writeAttribute("name", mName);
	writer.writeAttribute("id", QString::number(getid()));
	writer.writeAttribute("holesize", QString::number(hole_size));

	writer.writeStartElement("startpad");
	if (!start.isNull())
		start.toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("innerpad");
	if (!inner.isNull())
		inner.toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("endpad");
	if (!end.isNull())
		end.toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("startmask");
	if (!start_mask.isNull())
		start_mask.toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("endmask");
	if (!end_mask.isNull())
		end_mask.toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("startpaste");
	if (!start_paste.isNull())
		start_paste.toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("endpaste");
	if (!end_paste.isNull())
		end_paste.toXML(writer);
	writer.writeEndElement();

	writer.writeEndElement();
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

QSharedPointer<Pin> Pin::newFromXML(QXmlStreamReader &reader, const QHash<int, QSharedPointer<Padstack> > & padstacks, Footprint *fp)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "pin");
	QXmlStreamAttributes attr = reader.attributes();

	QSharedPointer<Pin> p(new Pin(fp));
	p->mName = attr.value("name").toString();
	p->mPos = QPoint(attr.value("x").toString().toInt(), attr.value("y").toString().toInt());
	p->mAngle = attr.value("rot").toString().toInt();
	p->updateTransform();
	int psind = attr.value("padstack").toString().toInt();
	if (padstacks.contains(psind))
		p->mPadstack = padstacks.value(psind);
	else
	{
		Log::instance().error("Error creating pin: missing padstack");
	}
	do
			reader.readNext();
	while(!reader.isEndElement());
	return p;
}

void Pin::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("pin");
	writer.writeAttribute("name", mName);
	writer.writeAttribute("x", QString::number(mPos.x()));
	writer.writeAttribute("y", QString::number(mPos.y()));
	writer.writeAttribute("rot", QString::number(mAngle));
	writer.writeAttribute("padstack", QString::number(mPadstack->getid()));
	writer.writeEndElement();
}

bool Pin::testHit(QPoint pt, const Layer& layer) const
{
	// check if we hit a thru-hole
	QPoint delta( pt - pos() );
	double dist = sqrt( delta.x()*delta.x() + delta.y()*delta.y() );
	if( dist < padstack()->holeSize()/2 )
		return true;

	Pad pad = getPadOnLayer(layer);
	if (!pad.isNull())
		return pad.testHit(delta);

	return false;
}

Pad Pin::getPadOnLayer(const Layer& layer) const
{
	if (layer == Layer::LAY_START)
		return padstack()->startPad();
	else if (layer == Layer::LAY_END)
		return padstack()->endPad();
	else if (layer == Layer::LAY_INNER)
		return padstack()->innerPad();
	return Pad();
}

QRect Pin::bbox() const
{
	return mFpTransform.mapRect(padstack()->bbox());
}

void Pin::updateTransform() const
{
	mFpTransform.reset();
	mFpTransform.translate(mPos.x(), mPos.y());
	mFpTransform.rotate(mAngle);
}

void Pin::draw(QPainter *painter, const Layer& layer) const
{
	if (mIsDirty)
		updateTransform();
	painter->save();
	painter->setTransform(mFpTransform, true);
	mPadstack->draw(painter, layer);
	painter->restore();
}

PCBObjState Pin::getState() const
{
	return PCBObjState(new PinState(*this));
}

bool Pin::loadState(PCBObjState &state)
{
	// convert to part state
	QSharedPointer<PinState> s = state.ptr().dynamicCast<PinState>();
	if (s.isNull()) return false;
	mName = s->name;
	mPos = s->pos;
	mAngle = s->angle;
	mPadstack = s->ps;
	return true;
}

/////////////////////// FOOTPRINT /////////////////////////

Footprint::Footprint()
	: mName("EMPTY_SHAPE"), mUnits(XPcb::MIL),
	  mRefText(QSharedPointer<Text>(new Text())),
	  mValueText(QSharedPointer<Text>(new Text())),
	  mCustomCentroid(false), mUuid(QUuid::createUuid())
{
	mValueText->setText("VALUE");
	mValueText->setLayer(Layer::LAY_SILK_TOP);
	mValueText->setPos(QPoint(0, XPcb::MIL2PCB(-120)));
	mRefText->setText("REF");
	mRefText->setLayer(Layer::LAY_SILK_TOP);
	Log::instance().message("FP constructed");
}

Footprint::Footprint(const Footprint &other)
	: mName(other.mName), mAuthor(other.mAuthor),
	  mSource(other.mSource), mDesc(other.mDesc),
	  mUnits(other.mUnits),
	  mRefText(QSharedPointer<Text>(new Text(*other.mRefText))),
	  mValueText(QSharedPointer<Text>(new Text(*other.mValueText))),
	  mCentroid(other.mCentroid),
	  mCustomCentroid(other.mCustomCentroid),
	  mUuid(other.mUuid)
{
	QHash<QSharedPointer<Padstack>, QSharedPointer<Padstack> > psmap;
	foreach(QSharedPointer<Padstack> ps, other.mPadstacks)
	{
		QSharedPointer<Padstack> newps(new Padstack(*ps));
		psmap.insert(ps, newps);
		mPadstacks.append(newps);
	}
	foreach(QSharedPointer<Pin> p, other.mPins)
	{
		QSharedPointer<Pin> np(new Pin(this));
		np->setPos(p->pos());
		np->setName(p->name());
		np->setAngle(p->angle());
		np->setPadstack(psmap.value(p->padstack()));
		mPins.append(np);
	}
	foreach(QSharedPointer<Line> p, other.mOutlineLines)
	{
		mOutlineLines.append(QSharedPointer<Line>(new Line(*p)));
	}
	foreach(QSharedPointer<Text> p, other.mTexts)
	{
		mTexts.append(QSharedPointer<Text>(new Text(*p)));
	}
	Log::instance().message("FP copy constructor");

}

// destructor
//
Footprint::~Footprint()
{
	Log::instance().message("FP destroyed");
}

void Footprint::draw(QPainter *painter, FP_DRAW_LAYER layer) const
{
	// draw silkscreen
	// pins are handled separately by PartPin, so this will only be called to draw the silkscreen (for now)
	Layer pcblayer = (layer == Footprint::LAY_START) ? Layer(Layer::LAY_SILK_TOP) : Layer(Layer::LAY_SILK_BOTTOM);
	foreach(QSharedPointer<Line> l, mOutlineLines)
		l->draw(painter, pcblayer);
	foreach(QSharedPointer<Text> t, mTexts)
		t->draw(painter, pcblayer);
}

QSharedPointer<Footprint> Footprint::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "footprint");

	QHash<int, QSharedPointer<Padstack> > psHash;

	QSharedPointer<Footprint> fp(new Footprint());
	while(reader.readNextStartElement())
	{
		QStringRef el = reader.name();
		if (el == "name")
		{
			fp->mName = reader.readElementText();
		}
		else if (el == "uuid")
		{
			fp->mUuid = QUuid(reader.readElementText());
		}
		else if (el == "units")
		{
			if (reader.readElementText() == "mm")
				fp->mUnits = XPcb::MM;
			else
				fp->mUnits = XPcb::MIL;
		}
		else if (el == "author")
		{
			fp->mAuthor = reader.readElementText();
		}
		else if (el == "source")
		{
			fp->mSource = reader.readElementText();
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
			do
					reader.readNext();
			while(!reader.isEndElement());
		}
		else if (el == "line" || el == "arc")
		{
			fp->mOutlineLines.append(Line::newFromXml(reader));
		}
		else if (el == "text")
		{
			fp->mTexts.append(Text::newFromXML(reader));
		}
		else if (el == "padstacks")
		{
			while(reader.readNextStartElement())
			{
				int psid = reader.attributes().value("id").toString().toInt();
				QSharedPointer<Padstack> ps = Padstack::newFromXML(reader);
				psHash.insert(psid, ps);
			}
		}
		else if (el == "pins")
		{
			while(reader.readNextStartElement())
			{
				QSharedPointer<Pin> pin = Pin::newFromXML(reader, psHash, fp.data());
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
			fp->mRefText->setPos(QPoint(attr.value("x").toString().toInt(),
									   attr.value("y").toString().toInt()));
			fp->mRefText->setAngle(attr.value("rot").toString().toInt());
			fp->mRefText->setFontSize(attr.value("textSize").toString().toInt());
			fp->mRefText->setStrokeWidth(attr.value("lineWidth").toString().toInt());
			do
					reader.readNext();
			while(!reader.isEndElement());
		}
		else if (el == "valueText")
		{
			QXmlStreamAttributes attr = reader.attributes();
			fp->mValueText->setPos(QPoint(attr.value("x").toString().toInt(),
									   attr.value("y").toString().toInt()));
			fp->mValueText->setAngle(attr.value("rot").toString().toInt());
			fp->mValueText->setFontSize(attr.value("textSize").toString().toInt());
			fp->mValueText->setStrokeWidth(attr.value("lineWidth").toString().toInt());
			do
					reader.readNext();
			while(!reader.isEndElement());
		}
	}
	fp->mPadstacks = psHash.values();
	return fp;
}

void Footprint::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("footprint");

	writer.writeTextElement("name", mName);
	writer.writeTextElement("uuid", mUuid.toString());
	writer.writeTextElement("units", mUnits == XPcb::MM ? "mm" : "mils");
	writer.writeTextElement("author", mAuthor);
	writer.writeTextElement("source", mSource);
	writer.writeTextElement("desc", mDesc);

	writer.writeStartElement("centroid");
	writer.writeAttribute("x", QString::number(mCentroid.x()));
	writer.writeAttribute("y", QString::number(mCentroid.y()));
	writer.writeAttribute("custom", mCustomCentroid ? "1" : "0");
	writer.writeEndElement();

	foreach(QSharedPointer<Line> l, mOutlineLines)
		l->toXML(writer);
	foreach(QSharedPointer<Text> t, mTexts)
		t->toXML(writer);

	writer.writeStartElement("padstacks");
	foreach(QSharedPointer<Padstack> ps, mPadstacks)
		ps->toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("pins");
	foreach(QSharedPointer<Pin> p, mPins)
		p->toXML(writer);
	writer.writeEndElement();

	writer.writeStartElement("refText");
	writer.writeAttribute("x", QString::number(mRefText->pos().x()));
	writer.writeAttribute("y", QString::number(mRefText->pos().y()));
	writer.writeAttribute("rot", QString::number(mRefText->angle()));
	writer.writeAttribute("textSize", QString::number(mRefText->fontSize()));
	writer.writeAttribute("lineWidth", QString::number(mRefText->strokeWidth()));
	writer.writeEndElement();

	writer.writeStartElement("valueText");
	writer.writeAttribute("x", QString::number(mValueText->pos().x()));
	writer.writeAttribute("y", QString::number(mValueText->pos().y()));
	writer.writeAttribute("rot", QString::number(mValueText->angle()));
	writer.writeAttribute("textSize", QString::number(mValueText->fontSize()));
	writer.writeAttribute("lineWidth", QString::number(mValueText->strokeWidth()));
	writer.writeEndElement();

	writer.writeEndElement(); // footprint
}

void Footprint::removePadstack(QSharedPointer<Padstack> ps)
{
	// check if it is in use
	foreach(QSharedPointer<Pin> p, mPins)
	{
		if (p->padstack() == ps)
			return;
	}
	// not in use, delete
	mPadstacks.removeOne(ps);
}

int Footprint::numPins() const
{
	return mPins.size();
}

QSharedPointer<Pin> Footprint::pin( const QString & name ) const
{	
	foreach(QSharedPointer<Pin> p, mPins)
	{
		if (p->name() == name)
			return p;
	}
	return QSharedPointer<Pin>();
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
	foreach(QSharedPointer<Pin> p, mPins)
	{
		r |= p->bbox();
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
	foreach(QSharedPointer<Line> l, mOutlineLines)
	{
		r |= l->bbox();
	}
	foreach(QSharedPointer<Text> t, mTexts)
	{
		r |= t->bbox();
	}

	// should this be included?
	//r |= mRefText.bbox();
	//r |= mValueText.bbox();

	return r;
}

//////////////////////////////////////////////////////////////////////

FPDatabase* FPDatabase::mInst = NULL;

FPDatabase::FPDatabase()
{
	// create root folders
	QSharedPointer<FPDBFolder> f = createFolder("footprints", true);
	if (f)
		mRootFolders.append(f);
}

FPDatabase& FPDatabase::instance()
{
	if (!mInst)
		mInst = new FPDatabase();
	return *mInst;
}

QSharedPointer<FPDBFolder> FPDatabase::createFolder(QString path, bool fullName)
{
	QDir dir(path);
	if (!dir.exists() || !dir.count() || !dir.isReadable())
		return QSharedPointer<FPDBFolder>();
	QList<QSharedPointer<FPDBFolder> > subfolders;
	// find all subdirs
	QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot
										| QDir::Readable);
	// recursively process all subdirs
	foreach(QString s, subdirs)
	{
		QSharedPointer<FPDBFolder> f = createFolder(dir.filePath(s));
		if (f)
			subfolders.append(f);
	}
	QList<QSharedPointer<FPDBFile> > files;
	// find any files in this dir
	QString pattern = "*.xfp";
	QStringList flist = dir.entryList(QStringList(pattern),
									  QDir::Files | QDir::Readable);
	foreach(QString s, flist)
	{
		QSharedPointer<FPDBFile> f = createFile(dir.filePath(s));
		if (f)
			files.append(f);
	}
	// if we didn't find any footprints, stop
	if (subfolders.length() == 0 && files.length() == 0)
		return QSharedPointer<FPDBFolder>();
	// otherwise, create a folder entry
	return QSharedPointer<FPDBFolder>(
				new FPDBFolder(
					fullName ? path : dir.dirName(),
					subfolders, files));
}

QSharedPointer<FPDBFile> FPDatabase::createFile(QString path)
{
	FPDoc doc;
	if(!doc.loadFromFile(path))
		return QSharedPointer<FPDBFile>();
	QSharedPointer<FPDBFile> f = QSharedPointer<FPDBFile>(
				new FPDBFile(path, doc.footprint()->name(),
							   doc.footprint()->author(),
							   doc.footprint()->source(),
							   doc.footprint()->desc(),
							   doc.footprint()->uuid()));
	mUuidHash.insert(doc.footprint()->uuid(), f);
	return f;
}

FPDBFile::FPDBFile(QString path, QString name, QString author, QString source, QString desc, QUuid uuid)
	: mFpPath(path), mName(name), mAuthor(author), mSource(source), mDesc(desc), mUuid(uuid), mParent(NULL)
{
}

QSharedPointer<Footprint> FPDBFile::loadFootprint() const
{
	FPDoc doc;
	if(!static_cast<Document*>(&doc)->loadFromFile(path()))
		return QSharedPointer<Footprint>();
	return QSharedPointer<Footprint>(new Footprint(*doc.footprint()));
}

FPDBFolder::FPDBFolder(QString name, QList<QSharedPointer<FPDBFolder> > folders,
					   QList<QSharedPointer<FPDBFile> > files)
	: mName(name), mFolders(folders), mFiles(files), mParent(NULL)
{
	// set the parent of all the children
	foreach(QSharedPointer<FPDBFolder> f, mFolders)
		f->setParent(this);
	foreach(QSharedPointer<FPDBFile> f, mFiles)
		f->setParent(this);
}
