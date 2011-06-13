#include "Part.h"
#include "PCBDoc.h"
#include "Log.h"
#include "smfontutil.h"

///////////////////// PART PIN /////////////////////

PartPin::~PartPin()
{
	if (mNet)
		mNet->removePin(this);
}

Pad PartPin::getPadOnLayer(const Layer& layer) const
{
	return mPin->getPadOnLayer(mapLayer(layer));
}

Layer PartPin::mapLayer(const Layer& layer) const
{
	if ((!layer.isCopper() && layer.type() != Layer::LAY_HOLE))
		return Layer();

	Part::SIDE side = mPart->side();
	if ((layer == Layer::LAY_TOP_COPPER && side == Part::SIDE_TOP) ||
		(layer == Layer::LAY_BOTTOM_COPPER && side == Part::SIDE_BOTTOM))
		return Layer::LAY_START;
	else if ((layer == Layer::LAY_TOP_COPPER && side == Part::SIDE_BOTTOM) ||
		(layer == Layer::LAY_BOTTOM_COPPER && side == Part::SIDE_TOP))
		return Layer::LAY_END;
	else if (layer.isCopper())
		return Layer::LAY_INNER;
	else
		return Layer::LAY_HOLE;
}

bool PartPin::testHit( const QPoint& pt, const Layer& layer) const
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

void PartPin::draw(QPainter *painter, const Layer& layer) const
{
	if (layer.type() == Layer::LAY_SELECTION)
		painter->drawRect(bbox());
	if (!layer.isCopper() && layer.type() != Layer::LAY_HOLE) return;
	mPin->draw(painter, mapLayer(layer));
}

void PartPin::setNet(Net *newnet)
{
	if (mNet)
		mNet->removePin(this);
	mNet = newnet;
	if (newnet)
		mNet->addPin(this);
}

PCBObjState PartPin::getState() const
{
	return PCBObjState(new PartPinState(*this));
}

bool PartPin::loadState(PCBObjState &state)
{
	// convert to part state
	QSharedPointer<PartPinState> s = state.ptr().dynamicCast<PartPinState>();
	if (s.isNull()) return false;
	mPin = s->pin;
	mPart = s->part;
	mNet = s->net;
	mVertex = s->vertex;
	return true;
}

///////////////////// PART /////////////////////
Part::Part(PCBDoc *doc)
	: mAngle(0), mSide(SIDE_TOP), mLocked(false), mRefdes(NULL), mRefVisible(false), mValue(NULL),
	mValueVisible(false), mFp(NULL), mDoc(doc)
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
	pp->mRefVisible = true;
	// value
	if (attr.hasAttribute("value"))
	{
		pp->mValue->setText(attr.value("value").toString());
		pp->mValueVisible = true;
	}


	// position/rotation
	pp->mPos = QPoint(attr.value("x").toString().toInt(),
					  attr.value("y").toString().toInt());
	pp->mAngle = attr.value("rot").toString().toInt();

	// side
	pp->mSide = (attr.value("side") == "top") ? SIDE_TOP : SIDE_BOTTOM;

	// locked?
	if (attr.hasAttribute("locked"))
	pp->mLocked = (attr.value("locked") == "1");

	pp->updateTransform();

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
			if (attr.hasAttribute("visible"))
				pp->mRefVisible = attr.value("visible") == "1";
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
			if (attr.hasAttribute("visible"))
				pp->mValueVisible = attr.value("visible") == "1";
			do
					reader.readNext();
			while(!reader.isEndElement());
		}
	}
	pp->mRefdes->setLayer(pp->mSide == SIDE_TOP ? Layer::LAY_SILK_TOP : Layer::LAY_SILK_BOTTOM);
	pp->mValue->setLayer(pp->mSide == SIDE_TOP ? Layer::LAY_SILK_TOP : Layer::LAY_SILK_BOTTOM);
	pp->mRefdes->setParent(pp);
	pp->mValue->setParent(pp);
	return pp;
}

void Part::toXML(QXmlStreamWriter &writer) const
{
	QPoint refdesPos = transform().inverted().map(mRefdes->pos());
	QPoint valuePos = transform().inverted().map(mValue->pos());

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
	writer.writeAttribute("x", QString::number(refdesPos.x()));
	writer.writeAttribute("y", QString::number(refdesPos.y()));
	writer.writeAttribute("rot", QString::number(mRefdes->angle()));
	writer.writeAttribute("textSize", QString::number(mRefdes->fontSize()));
	writer.writeAttribute("lineWidth", QString::number(mRefdes->strokeWidth()));
	writer.writeAttribute("visible", mRefVisible ? "1" : "0");
	writer.writeEndElement();
	writer.writeStartElement("valueText");
	writer.writeAttribute("x", QString::number(valuePos.x()));
	writer.writeAttribute("y", QString::number(valuePos.y()));
	writer.writeAttribute("rot", QString::number(mValue->angle()));
	writer.writeAttribute("textSize", QString::number(mValue->fontSize()));
	writer.writeAttribute("lineWidth", QString::number(mValue->strokeWidth()));
	writer.writeAttribute("visible", mValueVisible ? "1" : "0");
	writer.writeEndElement();
	writer.writeEndElement();
}

void Part::setSide(SIDE side)
{
	mSide = side;
	mRefdes->setLayer(mSide == SIDE_TOP ? Layer::LAY_SILK_TOP : Layer::LAY_SILK_BOTTOM);
	mValue->setLayer(mSide == SIDE_TOP ? Layer::LAY_SILK_TOP : Layer::LAY_SILK_BOTTOM);
	updateTransform();
}

void Part::updateTransform()
{
	mTransform.reset();
	mTransform.translate(mPos.x(), mPos.y());
	if (mSide == SIDE_BOTTOM)
		mTransform.scale(-1, 1);
	mTransform.rotate(mAngle);
	this->mRefdes->parentChanged();
	this->mValue->parentChanged();
}

void Part::draw(QPainter *painter, const Layer& layer) const
{
	if (layer.type() == Layer::LAY_SELECTION)
	{
		painter->drawRect(bbox());
		if (refVisible())
			painter->drawRect(refdesText()->bbox());
		if (valueVisible())
			painter->drawRect(valueText()->bbox());
		return;
	}

	painter->save();
	painter->setTransform(mTransform, true);


	// draw footprint
	if ((layer.type() == Layer::LAY_SILK_TOP && mSide == SIDE_TOP) ||
		(layer.type() == Layer::LAY_SILK_BOTTOM && mSide == SIDE_BOTTOM))
		mFp->draw(painter, Footprint::LAY_START);
	else if ((layer.type() == Layer::LAY_SILK_TOP && mSide == SIDE_BOTTOM) ||
		(layer.type() == Layer::LAY_SILK_BOTTOM && mSide == SIDE_TOP))
		mFp->draw(painter, Footprint::LAY_END);

	// draw pins
	foreach(PartPin* p, mPins)
	{
		p->draw(painter, layer);
	}

	painter->restore();
}

QRect Part::bbox() const
{
	return mTransform.mapRect(mFp->bbox());
}

PartPin* Part::getPin(const QString &name)
{
	foreach(PartPin* p, mPins)
	{
		if (p->name() == name)
			return p;
	}
	return NULL;
}

PCBObjState Part::getState() const
{
	return PCBObjState(new PartState(*this));
}

bool Part::loadState(PCBObjState &state)
{
	// convert to part state
	QSharedPointer<PartState> s = state.ptr().dynamicCast<PartState>();
	if (s.isNull()) return false;
	mTransform = s->transform;
	mPos = s->pos;
	mAngle = s->angle;
	mSide = s->side;
	mLocked = s->locked;
	mRefVisible = s->refVis;
	mValueVisible = s->valVis;
	mFp = s->fp;
	mPins = s->pins;
	mDoc = s->doc;
	return true;
}
