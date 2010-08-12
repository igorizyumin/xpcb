#include "Part.h"
#include "utility.h"
#include "Netlist.h"
#include "PCBDoc.h"
#include "Log.h"

PCBLAYER PartPin::getLayer()
{
	Padstack *ps = mPin->getPadstack();
	if( ps->getHole() )
		return LAY_PAD_THRU;
	else if( part->getSide() == SIDE_TOP && !ps->getStartPad().isNull()
		|| part->getSide() == SIDE_BOTTOM && !ps->getEndPad().isNull() )
		return LAY_TOP_COPPER;
	else
		return LAY_BOTTOM_COPPER;
}

bool PartPin::getPadOnLayer(PCBLAYER layer, Pad &pad)
{
	if (layer < LAY_PAD_THRU)
		return false; // non-copper layer

	Padstack *ps = mPin->getPadstack();
	PCBSIDE side = mPart->getSide();

	if( ps->getHole() == 0 )
	{
		// SMT pad
		if( layer == LAY_TOP_COPPER && side == SIDE_TOP ||
			layer == LAY_BOTTOM_COPPER && side == SIDE_BOTTOM)
			pad = ps->getStartPad();
		else
			return false;
	}
	else
	{
		// TH pad
		if( layer == LAY_TOP_COPPER && side == SIDE_TOP ||
			layer == LAY_BOTTOM_COPPER && side == SIDE_BOTTOM )
			pad = ps->getStartPad();
		else if( layer == LAY_TOP_COPPER && side == SIDE_BOTTOM ||
				 layer == LAY_BOTTOM_COPPER && side == SIDE_TOP )
			pad = ps->getEndPad();
		else
			pad = ps->getInnerPad();
	}
	return true;
}

// Test for hit on pad
//
bool PartPin::TestHit( QPoint pt, PCBLAYER layer )
{
	Padstack *ps = mPin->getPadstack();
	// check if we hit a thru-hole
	QPoint delta( pt - this->getPos() );
	double dist = sqrt( delta.x()*delta.x() + delta.y()*delta.y() );
	if( dist < ps.hole_size/2 )
		return true;

	// otherwise, check if there is a pad on this layer
	Pad p;
	if (!getPadOnLayer(layer, p))
		return false;

	// check if we hit the pad
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

	// did not hit anything
	return false;
}

Part::Part(PCBDoc *doc)
	: mAngle(0), mSide(SIDE_TOP), mLocked(false), mRefdes(NULL), mValue(NULL), mFp(NULL), mDoc(doc)
{

}

Part::~Part()
{
	resetFp();
}

void Part::resetFp()
{
	delete mRefdes;
	mRefdes = NULL;
	delete mValue;
	mValue = NULL;
	foreach(PartPin* pin, mPins)
	{
		delete pin;
	}
	mPins.clear();
}

void Part::setFootprint(Footprint *fp)
{
	resetFp();

	// set pointer
	mFp = fp;

	// create new pins
	for(int i = 0; i < fp->numPins(); i++)
	{
		Pin *p = fp->getPin(i);
		mPins.append(new PartPin(this, p));
	}

	// create texts
	mRefdes = new Text(fp->getRefText());
	mValue = new Text(fp->getValueText());
}

Part* Part::newFromXML(QXmlStreamReader &reader, PCBDoc *doc)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "part");


	QXmlStreamAttributes attr = reader.attributes();

	Part *pp = new Part(mDoc);


	// find footprint
	Footprint *fp = doc->getFootprint(attr.value("footprint"));
	if (!fp)
	{
		Log::instance().error("Error when creating part: footprint not found");
		delete pp;
		return NULL;
	}
	else
		pp->setFootprint(fp);

	// reference designator
	pp->mRefdes->setText(attr.value("refdes"));
	// value
	if (attr.hasAttribute("value"))
		pp->mValue->setText(attr.value("value"));


	// position/rotation
	pp->mPos = QPoint(attr.value("x").toString().toInt(),
					  attr.value("y").toString().toInt());
	pp->mAngle = attr.value("rot").toString().toInt();

	// side
	pp->mSide = (attr.value("side") == "top") ? SIDE_TOP : SIDE_BOTTOM;

	// locked?
	if (attr.hasAttribute("locked"))
	pp->mLocked = (attr.value("locked") == "1");

	// set text properties from part def, if they exist
	while(reader.readNextStartElement())
	{
		switch(reader.name())
		{
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
