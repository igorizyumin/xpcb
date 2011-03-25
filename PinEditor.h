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

class EditPinDialog;

class PinEditor : public AbstractEditor
{
public:
	PinEditor(FPController* ctrl, Pin* pin = NULL);
	virtual ~PinEditor();

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
	virtual QList<CtrlAction> actions() const;
	virtual void action(int key);

protected:
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void keyPressEvent(QKeyEvent *event);

private:
	void newPin();
	void actionEdit();
	void actionMove();
	void actionDelete();
	void actionRotate();
	void startMove();
	void finishEdit();
	void finishNew(bool setPos = false);

	void clearPins();

	enum State { SELECTED, MOVE, ADD_MOVE, EDIT_MOVE };

	State mState;
	QList<Pin*> mPins;
	EditPinDialog* mDialog;
	QPoint mPos;
	int mAngle;
	QPoint mStartPos;
	int mStartAngle;
};

class NewPinCmd : public QUndoCommand
{
public:
	NewPinCmd(QUndoCommand *parent, FPDoc* doc, QList<Pin*> pins);
	virtual ~NewPinCmd();

	virtual void undo();
	virtual void redo();
private:
	QList<Pin*> mPins;
	FPDoc* mDoc;
	bool mInDoc;
};

class PinMoveCmd : public QUndoCommand
{
public:
	PinMoveCmd(QUndoCommand *parent, FPDoc* doc, Pin* pin, QPoint pos, int angle);

	virtual void undo();
	virtual void redo();
private:
	Pin* mPin;
	FPDoc* mDoc;
	QPoint mNewPos;
	int mNewAngle;
	QPoint mOldPos;
	int mOldAngle;
};

class PinEditCmd : public QUndoCommand
{
public:
	PinEditCmd(QUndoCommand *parent, Pin* pin, QString name, Padstack* ps, QPoint pos, int angle);

	virtual void undo();
	virtual void redo();
private:
	Pin* mPin;
	QString mNewName;
	QString mPrevName;
	Padstack* mNewPS;
	Padstack* mPrevPS;
	QPoint mNewPos;
	QPoint mPrevPos;
	int mNewAngle;
	int mPrevAngle;
};

#endif // PINEDITOR_H
