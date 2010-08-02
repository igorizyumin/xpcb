#include "Text.h"
#include "smfontutil.h"

Text::Text(  const QPoint &pos, int angle, bool mirror,
			bool negative, PCBLAYER layer, int font_size, int stroke_width,
			const QString &str ) :
mPos(pos), mAngle(angle), mIsMirrored(mirror), mIsNegative(negative), mLayer(layer),
mFontSize(font_size), mStrokeWidth(stroke_width), mText(str), mIsDirty(true)
{

}

Text::~Text()
{
	QPainterPath* path;
	foreach(path, this->mStrokes)
	{
		delete path;
		path = NULL;
	}
}

void Text::initText()
{
	double scale = (double)m_font_size/22.0;
	QRect bbox = QRect();
	myStrokes.clear();
	SMFontUtil &fu = SMFontUtil::instance();

	// deallocate old strokes
	QPainterPath* path;
	foreach(path, this->mStrokes)
	{
		delete path;
		path = NULL;
	}
	mStrokes.clear();

	// get new strokes for all characters
	for (int i = 0; i < myText.size(); i++)
	{
		fu.GetCharPath(myText[i], SIMPLEX, QPoint(bbox.right()+1, 0), scale, bbox, myStrokes);
	}
	myStrokeBBox = bbox;
}

void Text::initTransform()
{
	double scale = (double)mFontSize/22.0;
	double yoffset = 9.0*scale;
	mTransform.reset();
	mTransform.scale(scale, scale);
	mTransform.translate(0, yoffset);
	mTransform.translate(mPos);
	mTransform.rotate(mAngle);
}

const QRect & Text::bbox()
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
	QPen &pen = painter->pen();
	pen.setWidth(mStrokeWidth);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);

	// now draw strokes
	QPainterPath* path;
	foreach(path, this->mStrokes)
	{
		painter->drawPath(*path);
	}

	// restore painter state
	painter->restore();
}
