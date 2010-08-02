#ifndef TEXT_H
#define TEXT_H

#include "PCBObject.h"

class Text : public PCBObject
{
public:
	Text( const QPoint &pos, int angle,
		bool mirror, bool negative, PCBLAYER layer, int font_size,
		int stroke_width, const QString &text );
	~Text();

	// overrides
	virtual void draw(QPainter *painter, PCBLAYER layer);
	virtual const QRect & bbox();

	// getters / setters
	const QPoint& pos() {return mPos;}
	void setPos(const QPoint &newpos) : mPos(newpos), mIsDirty(true) {}

	const QString & text() {return mText;}
	void setText(const QString &text) : mText(text), mIsDirty(true) {}

	int angle() {return mAngle;}
	void setAngle(int angle) : mAngle(angle), mIsDirty(true) {}

	int fontSize() {return mFontSize;}
	void setFontSize(int size) : mFontSize(size), mIsDirty(true) {}

	int strokeWidth() {return mStrokeWidth;}
	void setStrokeWidth(int w) : mStrokeWidth(w), mIsDirty(true) {}

	bool isMirrored() {return mIsMirrored;}
	void setMirrored(bool b) : mIsMirrored(b), mIsDirty(true) {}

	bool isNegative() {return mIsNegative;}
	void setNegative(bool b) : mIsNegative(b) {}

	PCBLAYER layer() {return mLayer;}
	void setMirrored(PCBLAYER l) : mLayer(l) {}

protected:
	void initText();
	void initTransform();

private:
	// member variables
	QPoint mPos;
	PCBLAYER mLayer;
	int mAngle;
	bool mIsMirrored;
	bool mIsNegative;
	int mFontSize;
	int mStrokeWidth;
	QString mText;

	/// mStrokes stores the untransformed font strokes.
	/// Rotation and scaling are applied during the draw operation.
	QList<QPainterPath*> mStrokes;
	/// Untransformed stroke bounding box
	QRect mStrokeBBox;

	/// Indicates that the stroke array has not been rebuilt
	/// after a change.
	bool mIsDirty;

	/// Coordinate transformation to use
	QTransform mTransform;
};

#endif // TEXT_H
