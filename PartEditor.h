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
	PartEditor(Controller *ctrl, Part *part);
	virtual ~PartEditor();

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
	virtual QList<CtrlAction> actions() const;
	virtual void action(int key);

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);

private:
	void actionEdit();
	void actionDelete();
	void actionMove();
	void actionRotate(bool cw = true);
	void actionChangeSide();
	void newPart();
	enum State {SELECTED, MOVE, ADD_MOVE, EDIT_MOVE};
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent *event);
	void startMove();
	void finishEdit();
	void finishNew();

	State mState;
	Part* mPart;
	EditPartDialog *mDialog;
	QPoint mPos;
	QRect mBox;
	int mAngle;
	PCBSIDE mSide;
};

class PartState
{
public:
	PartState()
		: angle(0), side(SIDE_TOP), refVisible(false), valueVisible(false), fp(NULL)
	{}
	PartState(Part* p)
		: pos(p->pos()), angle(p->angle()), side(p->side()), refdes(p->refdes()), refVisible(p->refVisible()),
		value(p->value()), valueVisible(p->valueVisible()), fp(p->footprint())
	{}

	void applyTo(Part* p)
	{
		p->setPos(pos);
		p->setAngle(angle);
		p->setSide(side);
		p->refdesText()->setText(refdes);
		p->setRefVisible(refVisible);
		p->valueText()->setText(value);
		p->setValueVisible(valueVisible);
		//p->setFootprint(fp);
	}

	QPoint pos;
	int angle;
	PCBSIDE side;
	QString refdes;
	bool refVisible;
	QString value;
	bool valueVisible;
	Footprint * fp;
};

class PartMoveCmd : public QUndoCommand
{
public:
	PartMoveCmd(QUndoCommand *parent, Part* obj, QPoint newPos, int newAngle, PCBSIDE newSide);

	virtual void undo();
	virtual void redo();

private:
	Part* mPart;
	QPoint mNewPos;
	QPoint mPrevPos;
	int mNewAngle;
	int mPrevAngle;
	PCBSIDE mNewSide;
	PCBSIDE mPrevSide;
};

class PartEditCmd : public QUndoCommand
{
public:
	PartEditCmd(QUndoCommand *parent, Part* p, PartState& newState)
		: mPart(p), mPrevState(PartState(p)), mNewState(newState) {}

	virtual void undo();
	virtual void redo();

private:
	Part* mPart;

	PartState mPrevState;
	PartState mNewState;
};

#endif // PARTEDITOR_H
