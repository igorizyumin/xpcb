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

#ifndef PARTEDITOR_H
#define PARTEDITOR_H

#include "Editor.h"
#include "PCBDoc.h"

class Part;
class EditPartDialog;
class Controller;

class PartEditor : public AbstractEditor
{
	Q_OBJECT
public:
	PartEditor(PCBController *ctrl, Part *part = NULL);
	virtual ~PartEditor();

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
	void actionRotate(bool cw = true);
	void actionRotateCCW() { actionRotate(false); }
	void actionLock();
	void actionChangeSide();

private:
	void newPart();
	enum State {NEW, SELECTED, MOVE, ADD_MOVE, EDIT_MOVE};

	void startMove();
	void finishEdit();
	void finishNew();

	PCBObjState mPrevPartState;
	State mState;
	Part* mPart;
	EditPartDialog *mDialog;
	CtrlAction mChangeSideAction;
	CtrlAction mRotateCWAction;
	CtrlAction mRotateCCWAction;
	CtrlAction mEditAction;
	CtrlAction mEditFPAction;
	mutable CtrlAction mLockAction;
	CtrlAction mMoveAction;
	CtrlAction mDelAction;
};

class PartNewCmd : public QUndoCommand
{
public:
	PartNewCmd(QUndoCommand *parent, Part* obj, PCBDoc* doc);
	virtual ~PartNewCmd();

	virtual void undo();
	virtual void redo();

private:
	Part* mPart;
	PCBDoc* mDoc;
	bool mInDoc;
};

class PartDeleteCmd : public QUndoCommand
{
public:
	PartDeleteCmd(QUndoCommand *parent, Part* obj, PCBDoc* doc);
	virtual ~PartDeleteCmd();

	virtual void undo();
	virtual void redo();

private:
	Part* mPart;
	PCBDoc* mDoc;
	bool mInDoc;
};

#endif // PARTEDITOR_H
