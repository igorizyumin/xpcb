#include "Part.h"
#include "PCBDoc.h"
#include "Log.h"


///////////////////// PART PIN /////////////////////

PartPin::~PartPin()
{
	if (mNet)
		mNet->removePin(this);
}

Pad PartPin::getPadOnLayer(PCBLAYER layer) const
{
	return mPin->getPadOnLayer(mapLayer(layer));
}

Pin::PINLAYER PartPin::mapLayer(PCBLAYER layer) const
{
	if (layer <= LAY_PAD_THRU || layer == LAY_UNKNOWN)
		return Pin::LAY_UNKNOWN;

	PCBSIDE side = mPart->side();
	if ((layer == LAY_TOP_COPPER && side == SIDE_TOP) ||
		(layer == LAY_BOTTOM_COPPER && side == SIDE_BOTTOM))
		return Pin::LAY_START;
	else if ((layer == LAY_TOP_COPPER && side == SIDE_BOTTOM) ||
		(layer == LAY_BOTTOM_COPPER && side == SIDE_TOP))
		return Pin::LAY_END;
	else
		return Pin::LAY_INNER;
}

bool PartPin::testHit( const QPoint& pt, PCBLAYER layer ) const
{
	return mPin->testHit(mPart->transform().inverted().map(pt), mapLayer(layer));
}

QPoint PartPin::pos() const
{
	return mPart->transform().map(mPin->pos());
}

bool PartPin::isSmt() const
{
	return mPin->padstack()->isSmt();
}

QRect PartPin::bbox() const
{
	return mPart->transform().mapRect(mPin->bbox());
}

void PartPin::draw(QPainter *painter, PCBLAYER layer) const
{

}

void PartPin::setNet(Net *newnet)
{
	if (mNet)
		mNet->removePin(this);
	mNet = newnet;
	if (newnet)
		mNet->addPin(this);
}


///////////////////// PART /////////////////////
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
		const Pin *p = fp->getPin(i);
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

	Part *pp = new Part(doc);


	// find footprint
	Footprint *fp = doc->getFootprint(attr.value("footprint").toString());
	if (!fp)
	{
		Log::instance().error("Error when creating part: footprint not found");
		delete pp;
		return NULL;
	}
	else
		pp->setFootprint(fp);

	// reference designator
	pp->mRefdes->setText(attr.value("refdes").toString());
	// value
	if (attr.hasAttribute("value"))
		pp->mValue->setText(attr.value("value").toString());


	// position/rotation
	pp->mPos = QPoint(attr.value("x").toString().toInt(),
					  attr.value("y").toString().toInt());
	pp->mAngle = attr.value("rot").toString().toInt();
	pp->updateTransform();

	// side
	pp->mSide = (attr.value("side") == "top") ? SIDE_TOP : SIDE_BOTTOM;

	// locked?
	if (attr.hasAttribute("locked"))
	pp->mLocked = (attr.value("locked") == "1");

	// set text properties from part def, if they exist
	while(reader.readNextStartElement())
	{
		QStringRef t = reader.name();
		if (t == "refText")
		{
			QXmlStreamAttributes attr = reader.attributes();
			pp->mRefdes->setPos(QPoint(attr.value("x").toString().toInt(),
									   attr.value("y").toString().toInt()));
			pp->mRefdes->setAngle(attr.value("rot").toString().toInt());
			pp->mRefdes->setFontSize(attr.value("textSize").toString().toInt());
			pp->mRefdes->setStrokeWidth(attr.value("lineWidth").toString().toInt());
			do
					reader.readNext();
			while(!reader.isEndElement());
		}
		else if (t == "valueText")
		{
			QXmlStreamAttributes attr = reader.attributes();
			pp->mValue->setPos(QPoint(attr.value("x").toString().toInt(),
									   attr.value("y").toString().toInt()));
			pp->mValue->setAngle(attr.value("rot").toString().toInt());
			pp->mValue->setFontSize(attr.value("textSize").toString().toInt());
			pp->mValue->setStrokeWidth(attr.value("lineWidth").toString().toInt());
			do
					reader.readNext();
			while(!reader.isEndElement());
		}
	}
	return pp;
}

void Part::toXML(QXmlStreamWriter &writer) const
{
	writer.writeStartElement("part");
	writer.writeAttribute("footprint", mFp->name());
	writer.writeAttribute("refdes", this->mRefdes->text());
	writer.writeAttribute("value", this->mValue->text());
	writer.writeAttribute("x", QString::number(mPos.x()));
	writer.writeAttribute("y", QString::number(mPos.y()));
	writer.writeAttribute("rot", QString::number(mAngle));
	writer.writeAttribute("side", mSide == SIDE_TOP ? "top" : "bot");
	writer.writeAttribute("locked", mLocked ? "1" : "0");
	writer.writeStartElement("refText");
	writer.writeAttribute("x", QString::number(mRefdes->pos().x()));
	writer.writeAttribute("y", QString::number(mRefdes->pos().y()));
	writer.writeAttribute("rot", QString::number(mRefdes->angle()));
	writer.writeAttribute("textSize", QString::number(mRefdes->fontSize()));
	writer.writeAttribute("lineWidth", QString::number(mRefdes->strokeWidth()));
	writer.writeEndElement();
	writer.writeStartElement("valueText");
	writer.writeAttribute("x", QString::number(mValue->pos().x()));
	writer.writeAttribute("y", QString::number(mValue->pos().y()));
	writer.writeAttribute("rot", QString::number(mValue->angle()));
	writer.writeAttribute("textSize", QString::number(mValue->fontSize()));
	writer.writeAttribute("lineWidth", QString::number(mValue->strokeWidth()));
	writer.writeEndElement();
	writer.writeEndElement();
}

void Part::updateTransform()
{
	mTransform.reset();
	mTransform.translate(mPos.x(), mPos.y());
	mTransform.rotate(mAngle);
	// XXX TODO handle mirroring for bottom side
}

void Part::draw(QPainter *painter, PCBLAYER layer) const
{
}

QRect Part::bbox() const
{
	return mTransform.mapRect(mFp->bbox());
}

PartPin* Part::getPin(const QString &name)
{
	foreach(PartPin* p, mPins)
	{
		if (p->getName() == name)
			return p;
	}
	return NULL;
}
