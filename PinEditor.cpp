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

PinEditor::PinEditor(FPController* ctrl, QSharedPointer<Pin> pin)
	: AbstractEditor(ctrl), mState(SELECTED), mDialog(NULL),
	mAngle(0),
	mRotateAction(2, "Rotate"),
	mEditAction(0, "Edit Pin"),
	mMoveAction(3, "Move Pin"),
	mDelAction(7, "Delete Pin")
{
	connect(&mRotateAction, SIGNAL(execFired()), SLOT(actionRotate()));
	connect(&mEditAction, SIGNAL(execFired()), SLOT(actionEdit()));
	connect(&mMoveAction, SIGNAL(execFired()), SLOT(actionMove()));
	connect(&mDelAction, SIGNAL(execFired()), SLOT(actionDelete()));

	if (pin)
		mPins.append(pin);
}

void PinEditor::init()
{
	emit actionsChanged();
	if (mPins.size() == 0)
		newPin();
}

QList<const CtrlAction*> PinEditor::actions() const
{
	QList<const CtrlAction*> out;

	switch(mState)
	{
	case MOVE:
	case ADD_MOVE:
	case EDIT_MOVE:
		out.append(&mRotateAction);
		break;
	case SELECTED:
		out.append(&mEditAction);
		out.append(&mMoveAction);
		out.append(&mDelAction);
		break;
	}
	return out;
}

void PinEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == MOVE || mState == EDIT_MOVE || mState == ADD_MOVE)
	{
		mPos = ctrl()->snapToPlaceGrid(ctrl()->view()->transform().inverted().map(event->pos()));
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
		PCBObjState s = mPins[0]->getState();
		mPins[0]->setPos(mPos);
		mPins[0]->setAngle(mAngle);
		PCBObjEditCmd* cmd = new PCBObjEditCmd(NULL, mPins[0], s);
		ctrl()->doc()->doCommand(cmd);
		foreach(QSharedPointer<Pin> p, mPins)
			ctrl()->unhideObj(p);
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
	mPins.clear();
}

void PinEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && mState != SELECTED)
	{
		foreach(QSharedPointer<Pin> p, mPins)
			ctrl()->unhideObj(p);
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
	QCursor::setPos(ctrl()->view()->mapToGlobal(ctrl()->view()->transform().map(mPos)));
	emit actionsChanged();
	emit overlayChanged();
}

void PinEditor::startMove()
{
	Q_ASSERT(mPins.size() > 0);
	mPos = mStartPos = mPins[0]->pos();
	mAngle = mStartAngle = mPins[0]->angle();
	foreach(QSharedPointer<Pin> p, mPins)
		ctrl()->hideObj(p);
}

void PinEditor::actionDelete()
{
//	TextDeleteCmd* cmd = new TextDeleteCmd(NULL, mText, ctrl()->doc());
//	ctrl()->doc()->doCommand(cmd);
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
		mDialog = QSharedPointer<EditPinDialog>(new EditPinDialog(ctrl()->view(), ctrl()->doc()));

//	mDialog->init();

	if (mDialog->exec() == QDialog::Rejected)
	{
		emit editorFinished();
		return;
	}

	mPins = mDialog->makePins(dynamic_cast<FPDoc*>(ctrl()->doc())->footprint());

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
		mDialog = QSharedPointer<EditPinDialog>(new EditPinDialog(ctrl()->view(), ctrl()->doc()));
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
		foreach(QSharedPointer<Pin> p, mPins)
		{
			p->setPos(tf.map(p->pos()));
			p->setAngle(mAngle);
		}
	}
	NewPinCmd *cmd = new NewPinCmd(NULL, dynamic_cast<FPDoc*>(ctrl()->doc()), mPins);
	ctrl()->doc()->doCommand(cmd);
	foreach(QSharedPointer<Pin> p, mPins)
		ctrl()->unhideObj(p);
	mPins.clear(); // pins now owned by cmd
	emit editorFinished();
}

void PinEditor::finishEdit()
{
	PCBObjState s = mPins[0]->getState();
	mPins[0]->setName(mDialog->name());
	mPins[0]->setPadstack(mDialog->padstack());
	mPins[0]->setPos(mPos);
	mPins[0]->setAngle(mAngle);
	PCBObjEditCmd* cmd = new PCBObjEditCmd(NULL, mPins[0], s);
	ctrl()->doc()->doCommand(cmd);
	foreach(QSharedPointer<Pin> p, mPins)
		ctrl()->unhideObj(p);
	emit overlayChanged();
}

void PinEditor::drawOverlay(QPainter *painter)
{
	if (mState == SELECTED)
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, false);
		painter->setBrush(Qt::NoBrush);
		foreach(QSharedPointer<Pin> p, mPins)
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
		foreach(QSharedPointer<Pin> p, mPins)
		{
			p->draw(painter, Layer::LAY_SELECTION);
		}
		painter->restore();
	}
}

/////////////////////////////// UNDO COMMANDS ////////////////////////////


NewPinCmd::NewPinCmd(QUndoCommand *parent, FPDoc *doc, QList<QSharedPointer<Pin> > pins)
	: QUndoCommand(parent), mPins(pins), mDoc(doc)
{
}

void NewPinCmd::redo()
{
	foreach(QSharedPointer<Pin> p, mPins)
		mDoc->addPin(p);
}

void NewPinCmd::undo()
{
	foreach(QSharedPointer<Pin> p, mPins)
		mDoc->removePin(p);
}
