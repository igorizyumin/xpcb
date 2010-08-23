#include <QXmlStreamReader>
#include "Text.h"
#include "smfontutil.h"

Text::Text()
	: mLayer(LAY_UNKNOWN), mAngle(0), mIsMirrored(false), mIsNegative(false),
	mFontSize(0), mStrokeWidth(0), mIsDirty(true)
{
}

Text::Text(  const QPoint &pos, int angle, bool mirror,
			bool negative, PCBLAYER layer, int font_size, int stroke_width,
			const QString &str ) :
mPos(pos), mLayer(layer), mAngle(angle), mIsMirrored(mirror), mIsNegative(negative),
mFontSize(font_size), mStrokeWidth(stroke_width), mText(str), mIsDirty(true)
{

}

Text::~Text()
{
}

void Text::initText() const
{
	double scale = (double)mFontSize/22.0;
	QRect newBbox;
	SMFontUtil &fu = SMFontUtil::instance();

	mStrokes.clear();

	// get new strokes for all characters
	for (int i = 0; i < mText.size(); i++)
	{
		fu.GetCharPath(mText[i].toAscii(), SIMPLEX, QPoint(newBbox.right()+1, 0), scale, newBbox, mStrokes);
	}
	mStrokeBBox = newBbox;
}

void Text::initTransform() const
{
	double scale = (double)mFontSize/22.0;
	double yoffset = 9.0*scale;
	mTransform.reset();
	mTransform.scale(scale, scale);
	mTransform.translate(0, yoffset);
	mTransform.translate(mPos.x(), mPos.y());
	mTransform.rotate(mAngle);
}

QRect Text::bbox() const
{
	if (mIsDirty)
	{
		initText();
		initTransform();
		mIsDirty = false;
	}
	return mTransform.mapRect(this->mStrokeBBox);
}

void Text::draw(QPainter *painter, PCBLAYER layer)
{
	if (layer != mLayer)
		return;

	// reload strokes if text has changed
	if (mIsDirty)
	{
		initText();
		initTransform();
		mIsDirty = false;
	}

	// save the painter state
	painter->save();

	// set up the transformation
	painter->setTransform(mTransform, true);

	// set up pen/brush
	QPen pen = painter->pen();
	pen.setWidth(mStrokeWidth);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);

	// now draw strokes
	foreach(const QPainterPath& path, this->mStrokes)
	{
		painter->drawPath(path);
	}

	// restore painter state
	painter->restore();
}

Text Text::newFromXML(QXmlStreamReader &reader)
{
	Q_ASSERT(reader.isStartElement() && reader.name() == "text");

	QXmlStreamAttributes attr = reader.attributes();
	Text t;
	t.mLayer = (PCBLAYER) attr.value("layer").toString().toInt();
	t.mPos = QPoint(
			attr.value("x").toString().toInt(),
			attr.value("y").toString().toInt());
	t.mAngle = attr.value("rot").toString().toInt();
	t.mStrokeWidth = attr.value("lineWidth").toString().toInt();
	t.mFontSize = attr.value("textSize").toString().toInt();
	t.mText = reader.readElementText();
	t.mIsDirty = true;

	return t;

}
