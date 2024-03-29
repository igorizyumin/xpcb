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

#ifndef TEXT_H
#define TEXT_H

#include <QUndoCommand>
#include "PCBObject.h"
#include "Editor.h"
#include "Controller.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class EditTextDialog;

class Text : public PCBObject
{
	Q_OBJECT

public:
	Text(QObject* parent = NULL);
	Text(const QPoint &pos, int angle,
		bool mirror, bool negative, const Layer& layer, int font_size,
		int stroke_width, const QString &text, QObject *parent = NULL);

	/// Sets the object's parent.  When the object has a parent, its internal
	/// coordinates are in the parent's coordinate system.  All getters and setters
	/// still use world coordinates, and the parent's transform is used to map
	/// them to the parent's coordinate system.
//	void setParent(PCBObject *parent) { mParent = parent; changed(); }

	// overrides
	virtual void draw(QPainter *painter, const Layer& layer) const;
	virtual QRect bbox() const;
	virtual bool testHit(QPoint pt, int dist, const Layer& l) const
	{ return bbox().adjusted(-dist/2, -dist/2, dist/2, dist/2)
				.contains(pt) && mLayer == l; }
	virtual PCBObjState getState() const;
	virtual bool loadState(PCBObjState &state);

	// i/o
	static QSharedPointer<Text> newFromXML(QXmlStreamReader &reader);
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

	const Layer& layer() const {return mLayer;}
	void setLayer(const Layer& l) {mLayer = l; changed();}

	QSharedPointer<Text> clone() const;

protected slots:
	virtual void onParentTransformChanged(QTransform) { changed(); }

protected:
	void changed() { mIsDirty = true; }
	void rebuild() const;

private:
	class TextState : public PCBObjStateInternal
	{
	public:
		virtual ~TextState() {}
	private:
		friend class Text;
		TextState(const Text &t)
			: pos(t.mPos), layer(t.mLayer), angle(t.mAngle),
			ismirrored(t.mIsMirrored), isnegative(t.mIsNegative),
			fontsize(t.mFontSize), width(t.mStrokeWidth),
			text(t.mText)
		{}

		QPoint pos;
		Layer layer;
		int angle;
		bool ismirrored, isnegative;
		int fontsize, width;
		QString text;
	};
	friend class TextState;
	// member variables
	QPoint mPos;
	Layer mLayer;
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

	/// Indicates that the stroke list has not been rebuilt
	/// after a change.
	mutable bool mIsDirty;

	/// Coordinate transformation to use
	mutable QTransform mTransform;
};

class TextEditor : public AbstractEditor
{
	Q_OBJECT
public:
	TextEditor(Controller *ctrl, QSharedPointer<Text> text = QSharedPointer<Text>());
	virtual ~TextEditor();

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
        virtual QList<const CtrlAction*> actions() const;

protected:
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void actionEdit();
	void actionDelete();
	void actionMove();
	void actionRotate();

private:
	enum State {SELECTED, MOVE, ADD_MOVE, EDIT_MOVE};

	void newText();
	void startMove();
	void finishEdit();
	void finishNew();

	State mState;
	QSharedPointer<Text> mText;
    EditTextDialog* mDialog;
	QPoint mPos;
	QRect mBox;
	int mAngleDelta;

	CtrlAction mRotateAction;
	CtrlAction mEditAction;
	CtrlAction mMoveAction;
	CtrlAction mDeleteAction;
};

class TextNewCmd : public QUndoCommand
{
public:
	TextNewCmd(QUndoCommand *parent, QSharedPointer<Text> obj, Document* doc);

	virtual void undo();
	virtual void redo();

private:
	QSharedPointer<Text> mText;
	Document* mDoc;
	bool mInDoc;
};

class TextDeleteCmd : public QUndoCommand
{
public:
	TextDeleteCmd(QUndoCommand *parent, QSharedPointer<Text> obj, Document* doc);

	virtual void undo();
	virtual void redo();

private:
	QSharedPointer<Text> mText;
	Document* mDoc;
	bool mInDoc;
};

#endif // TEXT_H
