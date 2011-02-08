#ifndef TEXT_H
#define TEXT_H

#include <QUndoCommand>
#include "PCBObject.h"
#include "Editor.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class EditTextDialog;

class Text : public PCBObject
{
public:
	Text();
	Text(const QPoint &pos, int angle,
		bool mirror, bool negative, PCBLAYER layer, int font_size,
		int stroke_width, const QString &text );

	/// Sets the object's parent.  When the object has a parent, its internal
	/// coordinates are in the parent's coordinate system.  All getters and setters
	/// still use world coordinates, and the parent's transform is used to map
	/// them to the parent's coordinate system.
	void setParent(PCBObject *parent) { mParent = parent; changed(); }

	// overrides
	virtual void draw(QPainter *painter, PCBLAYER layer) const;
	virtual QRect bbox() const;
	virtual void accept(PCBObjectVisitor *v) { v->visit(this); }
	virtual bool testHit(QPoint pt, PCBLAYER l) const { return bbox().contains(pt) && mLayer == l; }

	// i/o
	static Text* newFromXML(QXmlStreamReader &reader);
	void toXML(QXmlStreamWriter &writer) const;

	// getters / setters
	QPoint pos() const;
	void setPos(const QPoint &newpos);

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


	virtual void parentChanged() { changed(); }

protected:
	void changed() { mIsDirty = true; }
	void rebuild() const;

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

	PCBObject* mParent;

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
	TextEditor(Controller *ctrl, Text *text);
	virtual ~TextEditor();

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
	virtual QList<CtrlAction> actions() const;
	virtual void action(int key);

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);

private slots:
	void actionEdit();
	void actionDelete();
	void actionMove();
	void actionRotate();

private:
	void newText();
	enum State {SELECTED, MOVE, ADD_MOVE, EDIT_MOVE};
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent *event);
	void startMove();
	void finishEdit();
	void finishNew();

	State mState;
	Text* mText;
	EditTextDialog *mDialog;
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

class TextNewCmd : public QUndoCommand
{
public:
	TextNewCmd(QUndoCommand *parent, Text* obj, PCBDoc* doc);
	virtual ~TextNewCmd();

	virtual void undo();
	virtual void redo();

private:
	Text* mText;
	PCBDoc* mDoc;
	bool mInDoc;
};

class TextDeleteCmd : public QUndoCommand
{
public:
	TextDeleteCmd(QUndoCommand *parent, Text* obj, PCBDoc* doc);
	virtual ~TextDeleteCmd();

	virtual void undo();
	virtual void redo();

private:
	Text* mText;
	PCBDoc* mDoc;
	bool mInDoc;
};

class TextEditCmd : public QUndoCommand
{
public:
	TextEditCmd(QUndoCommand *parent, Text* obj, QPoint newPos, PCBLAYER newLayer,
				int newAngle, bool isMirrored, bool isNegative, int newSize,
				int newWidth, QString newText );

	virtual void undo();
	virtual void redo();

private:
	Text* mText;
	// old
	QPoint mOldPos;
	PCBLAYER mOldLayer;
	int mOldAngle;
	bool mOldIsMirrored;
	bool mOldIsNegative;
	int mOldFontSize;
	int mOldStrokeWidth;
	QString mOldText;
	// new
	QPoint mNewPos;
	PCBLAYER mNewLayer;
	int mNewAngle;
	bool mNewIsMirrored;
	bool mNewIsNegative;
	int mNewFontSize;
	int mNewStrokeWidth;
	QString mNewText;
};


#endif // TEXT_H
