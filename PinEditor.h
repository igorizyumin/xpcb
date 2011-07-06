/*
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

#ifndef PINEDITOR_H
#define PINEDITOR_H

#include "Editor.h"
#include "Controller.h"
#include "Footprint.h"
#include "EditPinDialog.h"

class PinEditor : public AbstractEditor
{
	Q_OBJECT
public:
	PinEditor(FPController* ctrl, QSharedPointer<Pin> pin = QSharedPointer<Pin>());

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
	void actionMove();
	void actionDelete();
	void actionRotate();

private:
	void newPin();
	void startMove();
	void finishEdit();
	void finishNew(bool setPos = false);

	void clearPins();

	enum State { SELECTED, MOVE, ADD_MOVE, EDIT_MOVE };

	State mState;
	QList<QSharedPointer<Pin> > mPins;
	QSharedPointer<EditPinDialog> mDialog;
	QPoint mPos;
	int mAngle;
	QPoint mStartPos;
	int mStartAngle;

	CtrlAction mRotateAction;
	CtrlAction mEditAction;
	CtrlAction mMoveAction;
	CtrlAction mDelAction;
};

class NewPinCmd : public QUndoCommand
{
public:
	NewPinCmd(QUndoCommand *parent, FPDoc* doc, QList<QSharedPointer<Pin> > pins);

	virtual void undo();
	virtual void redo();
private:
	QList<QSharedPointer<Pin> > mPins;
	FPDoc* mDoc;
};

#endif // PINEDITOR_H
