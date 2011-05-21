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

#include "PinEditor.h"
#include "Shape.h"
#include "EditPinDialog.h"
#include "PCBView.h"
#include "Controller.h"
#include "PCBDoc.h"
#include "Log.h"

PinEditor::PinEditor(FPController* ctrl, Pin* pin)
	: AbstractEditor(ctrl), mState(SELECTED), mDialog(NULL),
	mAngle(0)
{
	if (pin)
		mPins.append(pin);
}

PinEditor::~PinEditor()
{
	if (mDialog)
		delete mDialog;
}

void PinEditor::init()
{
	emit actionsChanged();
	if (mPins.size() == 0)
		newPin();
}

QList<CtrlAction> PinEditor::actions() const
{
	QList<CtrlAction> out;

	switch(mState)
	{
	case MOVE:
	case ADD_MOVE:
	case EDIT_MOVE:
		out.append(CtrlAction(2, "Rotate"));
		break;
	case SELECTED:
		out.append(CtrlAction(0, "Edit Pin"));
		out.append(CtrlAction(3, "Move Pin"));
		out.append(CtrlAction(7, "Delete Pin"));
		break;
	}
	return out;
}

void PinEditor::action(int key)
{
	switch(mState)
	{
	case MOVE:
	case ADD_MOVE:
	case EDIT_MOVE:
		if (key == 2)
			actionRotate();
		break;
	case SELECTED:
		if (key == 0)
			actionEdit();
		else if (key == 3)
			actionMove();
		else if (key == 7)
			actionDelete();
		break;
	}
}

void PinEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == MOVE || mState == EDIT_MOVE || mState == ADD_MOVE)
	{
		mPos = mCtrl->snapToPlaceGrid(mCtrl->view()->transform().inverted().map(event->pos()));
		emit overlayChanged();
	}

	// we don't want to eat mouse events
	event->ignore();
}

void PinEditor::mousePressEvent(QMouseEvent *event)
{
	if (mState != SELECTED)
		event->accept();
	else
		event->ignore();
}

void PinEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (mState != SELECTED)
	{
		event->accept();
	}
	if (mState == MOVE)
	{
		mState = SELECTED;
		PinMoveCmd* cmd = new PinMoveCmd(NULL, dynamic_cast<FPDoc*>(mCtrl->doc()), mPins[0], mPos, mAngle);
		mCtrl->doc()->doCommand(cmd);
		foreach(Pin* p, mPins)
			mCtrl->unhideObj(p);
		emit actionsChanged();
		emit overlayChanged();
	}
	else if (mState == EDIT_MOVE)
	{
		mState = SELECTED;
		finishEdit();
		emit actionsChanged();
	}
	else if (mState == ADD_MOVE)
	{
		finishNew(true);
	}
	else
		event->ignore();
}

void PinEditor::clearPins()
{
	foreach(Pin* p, mPins)
		delete p;
	mPins.clear();
}

void PinEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && mState != SELECTED)
	{
		foreach(Pin* p, mPins)
			mCtrl->unhideObj(p);
		if (mState == ADD_MOVE)
		{
			clearPins();
			emit editorFinished();
		}
		else
		{
			mState = SELECTED;
			emit actionsChanged();
			emit overlayChanged();
		}
		event->accept();
	}
	else
		event->ignore();
}

void PinEditor::actionMove()
{
	mState = MOVE;
	startMove();
	QCursor::setPos(mCtrl->view()->mapToGlobal(mCtrl->view()->transform().map(mPos)));
	emit actionsChanged();
	emit overlayChanged();
}

void PinEditor::startMove()
{
	Q_ASSERT(mPins.size() > 0);
	mPos = mStartPos = mPins[0]->pos();
	mAngle = mStartAngle = mPins[0]->angle();
	foreach(Pin* p, mPins)
		mCtrl->hideObj(p);
}

void PinEditor::actionDelete()
{
//	TextDeleteCmd* cmd = new TextDeleteCmd(NULL, mText, mCtrl->doc());
//	mCtrl->doc()->doCommand(cmd);
//	emit editorFinished();
}

void PinEditor::actionRotate()
{
	mAngle += 90;
	mAngle %= 360;
	emit overlayChanged();
}

void PinEditor::newPin()
{
	if (!mDialog)
		mDialog = new EditPinDialog(mCtrl->view(), mCtrl->doc());

//	mDialog->init();

	if (mDialog->exec() == QDialog::Rejected)
	{
		emit editorFinished();
		return;
	}

	mPins = mDialog->makePins(dynamic_cast<FPDoc*>(mCtrl->doc())->footprint());

	if (mDialog->dragToPos())
	{
		mState = ADD_MOVE;
		startMove();
		emit actionsChanged();
	}
	else
	{
		finishNew();
	}
}

void PinEditor::actionEdit()
{
	Q_ASSERT(mPins.size() == 1);
	if (!mDialog)
		mDialog = new EditPinDialog(mCtrl->view(), mCtrl->doc());
	mDialog->init(mPins[0]);
	if (mDialog->exec() == QDialog::Accepted)
	{
		if (!mDialog->dragToPos())
		{	// position is set
			mPos = mDialog->pos();
			mAngle = mDialog->angle();
			finishEdit();
		}
		else // drag to position
		{
			mState = EDIT_MOVE;
			startMove();
			emit actionsChanged();
			emit overlayChanged();
		}
	}
}

void PinEditor::finishNew(bool setPos)
{
	if (setPos)
	{
		QTransform tf;
		tf.translate(mPos.x(), mPos.y());
		tf.rotate(mAngle);
		foreach(Pin* p, mPins)
		{
			p->setPos(tf.map(p->pos()));
			p->setAngle(mAngle);
		}
	}
	NewPinCmd *cmd = new NewPinCmd(NULL, dynamic_cast<FPDoc*>(mCtrl->doc()), mPins);
	mCtrl->doc()->doCommand(cmd);
	foreach(Pin* p, mPins)
		mCtrl->unhideObj(p);
	mPins.clear(); // pins now owned by cmd
	emit editorFinished();
}

void PinEditor::finishEdit()
{
	PinEditCmd *cmd = new PinEditCmd(NULL, mPins[0], mDialog->name(), mDialog->padstack(),
									 mPos, mAngle);
	mCtrl->doc()->doCommand(cmd);
	foreach(Pin* p, mPins)
		mCtrl->unhideObj(p);
	emit overlayChanged();
}

void PinEditor::drawOverlay(QPainter *painter)
{
	if (mState == SELECTED)
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, false);
		painter->setBrush(Qt::NoBrush);
		foreach(Pin* p, mPins)
		{
			p->draw(painter, Layer::LAY_SELECTION);
		}
		painter->restore();
	}
	else // move / add_move / edit_move
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, false);
		painter->setBrush(Qt::NoBrush);
		painter->translate(mPos);
		painter->drawLine(QPoint(0, -INT_MAX), QPoint(0, INT_MAX));
		painter->drawLine(QPoint(-INT_MAX, 0), QPoint(INT_MAX, 0));
		painter->translate(-mStartPos);
		painter->rotate(mAngle - mStartAngle);
		foreach(Pin* p, mPins)
		{
			p->draw(painter, Layer::LAY_SELECTION);
		}
		painter->restore();
	}
}

/////////////////////////////// UNDO COMMANDS ////////////////////////////


NewPinCmd::NewPinCmd(QUndoCommand *parent, FPDoc *doc, QList<Pin *> pins)
	: QUndoCommand(parent), mPins(pins), mDoc(doc), mInDoc(false)
{
}

NewPinCmd::~NewPinCmd()
{
	if (!mInDoc)
	{
		foreach(Pin* p, mPins)
			delete p;
	}
}

void NewPinCmd::redo()
{
	foreach(Pin* p, mPins)
		mDoc->addPin(p);
	mInDoc = true;
}

void NewPinCmd::undo()
{
	foreach(Pin* p, mPins)
		mDoc->removePin(p);
	mInDoc = false;
}

PinMoveCmd::PinMoveCmd(QUndoCommand *parent, FPDoc *doc, Pin *pin, QPoint pos, int angle)
	: QUndoCommand(parent), mPin(pin), mDoc(doc), mNewPos(pos), mNewAngle(angle),
	mOldPos(pin->pos()), mOldAngle(pin->angle())
{
}

void PinMoveCmd::undo()
{
	mPin->setPos(mOldPos);
	mPin->setAngle(mOldAngle);
}

void PinMoveCmd::redo()
{
	mPin->setPos(mNewPos);
	mPin->setAngle(mNewAngle);
}

PinEditCmd::PinEditCmd(QUndoCommand *parent, Pin *pin,
					   QString name, Padstack *ps, QPoint pos, int angle)
	: QUndoCommand(parent), mPin(pin), mNewName(name), mPrevName(pin->name()), mNewPS(ps),
	mPrevPS(pin->padstack()), mNewPos(pos), mPrevPos(pin->pos()),
	mNewAngle(angle), mPrevAngle(pin->angle())
{

}

void PinEditCmd::undo()
{
	mPin->setName(mPrevName);
	mPin->setPadstack(mPrevPS);
	mPin->setPos(mPrevPos);
	mPin->setAngle(mPrevAngle);
}

void PinEditCmd::redo()
{
	mPin->setName(mNewName);
	mPin->setPadstack(mNewPS);
	mPin->setPos(mNewPos);
	mPin->setAngle(mNewAngle);
}
