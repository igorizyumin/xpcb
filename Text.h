#ifndef TEXT_H
#define TEXT_H

#include <QUndoCommand>
#include "PCBObject.h"
#include "Editor.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class Text : public PCBObject
{
public:
	Text();
	Text(const QPoint &pos, int angle,
		bool mirror, bool negative, PCBLAYER layer, int font_size,
		int stroke_width, const QString &text );
	~Text();

	// overrides
	virtual void draw(QPainter *painter, PCBLAYER layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }

	// getters / setters
	const QPoint& pos() const {return mPos;}
	void setPos(const QPoint &newpos) { mPos = newpos; changed(); }

	const QString & text() const {return mText;}
	void setText(const QString &text) { mText = text; changed(); }

	int angle() const {return mAngle;}
	void setAngle(int angle) { mAngle = angle; changed();}

	int fontSize() const {return mFontSize;}
	void setFontSize(int size) {mFontSize = size; changed();}

	int strokeWidth() const {return mStrokeWidth;}
	void setStrokeWidth(int w) { mStrokeWidth = w; changed();}

	bool isMirrored() const {return mIsMirrored;}
	void setMirrored(bool b) { mIsMirrored = b;  changed(); }

	bool isNegative() const {return mIsNegative;}
	void setNegative(bool b) {mIsNegative=b; changed();}

	PCBLAYER layer() const {return mLayer;}
	void setLayer(PCBLAYER l) {mLayer = l; changed();}

	static Text newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

protected:
	void changed() { mIsDirty = true; }
	void initText() const;
	void initTransform() const;

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
	mutable QList<QPainterPath> mStrokes;
	/// Untransformed stroke bounding box
	mutable QRect mStrokeBBox;

	/// Indicates that the stroke array has not been rebuilt
	/// after a change.
	mutable bool mIsDirty;

	/// Coordinate transformation to use
	mutable QTransform mTransform;
};

class TextEditor : public AbstractEditor
{
	Q_OBJECT
public:
	TextEditor(Controller *ctrl, QList<QAction*> actions, Text *text);

	virtual void drawOverlay(QPainter* painter);

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);

private slots:
	void actionEdit();
	void actionDelete();
	void actionMove();
	void actionRotate();

private:
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent *event);
	void updateActions();

	bool mIsMoving;
	Text* mText;
	QPoint mPos;
	QRect mBox;
	int mAngleDelta;
};

class TextMoveCmd : public QUndoCommand
{
public:
	TextMoveCmd(QUndoCommand *parent, Text* obj, QPoint newPos, int angleDelta);

	virtual void undo();
	virtual void redo();

private:
	Text* mText;
	QPoint mNewPos;
	QPoint mPrevPos;
	int mNewAngle;
	int mPrevAngle;
};

#endif // TEXT_H
