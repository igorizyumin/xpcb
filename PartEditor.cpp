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

#include "PartEditor.h"
#include "Controller.h"
#include "Part.h"
#include "PCBView.h"
#include "EditPartDialog.h"
#include <QMessageBox>

////////////////////////// PART EDITOR //////////////////////////////////////


PartEditor::PartEditor(Controller *ctrl, Part *part)
	: AbstractEditor(ctrl), mState(SELECTED), mPart(part), mDialog(NULL), mAngle(part->angle()),
	mChangeSideAction(1, "Change Side"),
	mRotateCWAction(2, "Rotate CW"),
	mRotateCCWAction(3, "Rotate CCW"),
	mEditAction(0, "Edit Part"),
	mEditFPAction(1, "Edit Footprint"),
	mLockAction(2, part->locked() ? "Unlock Part" : "Lock Part"),
	mMoveAction(3, "Move Part"),
	mDelAction(7, "Delete Part")
{
	connect(&mChangeSideAction, SIGNAL(execFired()), SLOT(actionChangeSide()));
	connect(&mRotateCWAction, SIGNAL(execFired()), SLOT(actionRotate()));
	connect(&mRotateCCWAction, SIGNAL(execFired()), SLOT(actionRotateCCW()));
	connect(&mEditAction, SIGNAL(execFired()), SLOT(actionEdit()));
//	connect(&mEditFPAction, SIGNAL(execFired()), SLOT(actionEdit()));
	connect(&mLockAction, SIGNAL(execFired()), SLOT(actionLock()));
	connect(&mMoveAction, SIGNAL(execFired()), SLOT(actionMove()));
	connect(&mDelAction, SIGNAL(execFired()), SLOT(actionDelete()));
}

void PartEditor::init()
{
	emit actionsChanged();
	if (!mPart)
		newPart();
}

PartEditor::~PartEditor()
{
	if (mDialog)
		delete mDialog;
}

QList<const CtrlAction*> PartEditor::actions() const
{
	QList<const CtrlAction*> out;

	switch(mState)
	{
	case MOVE:
	case ADD_MOVE:
	case EDIT_MOVE:
		out.append(&mChangeSideAction);
		out.append(&mRotateCWAction);
		out.append(&mRotateCCWAction);
		break;
	case SELECTED:
		mLockAction.setText(mPart->locked() ? "Unlock Part" : "Lock Part");
		out.append(&mEditAction);
		out.append(&mEditFPAction);
		out.append(&mLockAction);
		out.append(&mMoveAction);
		out.append(&mDelAction);
		break;
	}
	return out;
}

void PartEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == MOVE || mState == EDIT_MOVE || mState == ADD_MOVE)
	{
		mPos = ctrl()->snapToPlaceGrid(ctrl()->view()->transform().inverted().map(event->pos()));
		emit overlayChanged();
	}

	// we don't want to eat mouse events
	event->ignore();
}

void PartEditor::mousePressEvent(QMouseEvent *event)
{
	if (mState != SELECTED)
		event->accept();
	else
		event->ignore();
}

void PartEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (mState != SELECTED)
	{
		ctrl()->unhideObj(mPart);
		ctrl()->unhideObj(mPart->refdesText());
		ctrl()->unhideObj(mPart->valueText());
		event->accept();

	}
	if (mState == MOVE)
	{
		mState = SELECTED;
		PartMoveCmd* cmd = new PartMoveCmd(NULL, mPart, mPos, mAngle, mSide);
		ctrl()->doc()->doCommand(cmd);
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
		mState = SELECTED;
		finishNew();
		emit actionsChanged();
	}
	else
		event->ignore();
}

void PartEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && mState != SELECTED)
	{
		if (mState == ADD_MOVE)
		{
			delete mPart;
			mPart = NULL;
			emit editorFinished();
		}
		else
		{
			ctrl()->unhideObj(mPart);
			ctrl()->unhideObj(mPart->refdesText());
			ctrl()->unhideObj(mPart->valueText());
			mState = SELECTED;
			emit actionsChanged();
			emit overlayChanged();
		}
		event->accept();
	}
	else
		event->ignore();
}

void PartEditor::actionMove()
{

	if (mPart->locked())
	{
		QMessageBox b(QMessageBox::Question, "Part locked",
					  "The part is locked; do you want to unlock it?",
					  QMessageBox::Yes | QMessageBox::No, this->ctrl()->view());
		b.setDefaultButton(QMessageBox::Yes);
		int result = b.exec();
		if (result == QMessageBox::No) return;
		else mPart->setLocked(false);
	}
	mState = MOVE;
	startMove();
	QCursor::setPos(ctrl()->view()->mapToGlobal(ctrl()->view()->transform().map(mPos)));
	emit actionsChanged();
	emit overlayChanged();
}

void PartEditor::actionLock()
{
	mPart->setLocked(!mPart->locked());
	emit actionsChanged();
}

void PartEditor::startMove()
{
	mPos = mPart->pos();
	mAngle = mPart->angle();
	mSide = mPart->side();
	mBox = mPart->transform().inverted().mapRect(mPart->bbox());
	ctrl()->hideObj(mPart);
	ctrl()->hideObj(mPart->refdesText());
	ctrl()->hideObj(mPart->valueText());
}

void PartEditor::actionDelete()
{
//	TextDeleteCmd* cmd = new TextDeleteCmd(NULL, mText, ctrl()->doc());
//	ctrl()->doc()->doCommand(cmd);
//	emit editorFinished();
}

void PartEditor::actionRotate(bool cw)
{
	if (mSide == Part::SIDE_BOTTOM)
		cw = !cw;
	mAngle += cw ? 270 : 90;
	mAngle %= 360;
	emit overlayChanged();
}

void PartEditor::actionChangeSide()
{
	mSide = (mSide == Part::SIDE_TOP) ? Part::SIDE_BOTTOM : Part::SIDE_TOP;
	emit overlayChanged();
}

void PartEditor::newPart()
{
	if (!mDialog)
		mDialog = new EditPartDialog();
	mDialog->init();
	if (mDialog->exec() == QDialog::Rejected)
	{
		emit editorFinished();
		return;
	}
	emit editorFinished();
//	mText = new Text(mDialog->pos(), mDialog->angle(), mDialog->isMirrored(), mDialog->isNegative(),
//					 mDialog->layer(), mDialog->textHeight(), mDialog->textWidth(), mDialog->text());
//	if (!mDialog->isPosSet())
//	{
//		mState = ADD_MOVE;
//		startMove();
//		emit actionsChanged();
//	}
//	else
//	{
//		mPos = mDialog->pos();
//		mAngleDelta = 0;
//		finishNew();
//	}
}

void PartEditor::actionEdit()
{
	if (!mDialog)
		mDialog = new EditPartDialog(ctrl()->view());
	mDialog->init(mPart);
	if (mDialog->exec() == QDialog::Accepted)
	{
		if (mDialog->isPosSet())
		{
			mPos = mDialog->pos();
			mAngle = mDialog->angle();
			mSide = mDialog->side();
			finishEdit();
		}
		else // drag to position
		{
			mState = EDIT_MOVE;
			startMove();
			mSide = mDialog->side();
			emit actionsChanged();
			emit overlayChanged();
		}
	}
}

void PartEditor::finishNew()
{
//	mText->setPos(mPos);
//	mText->setAngle(mText->angle() + mAngleDelta);
//	TextNewCmd *cmd = new TextNewCmd(NULL, mText, ctrl()->doc());
//	ctrl()->doc()->doCommand(cmd);
//	ctrl()->selectObj(mText);
//	ctrl()->hideObj(mText);
//	mState = SELECTED;
	emit overlayChanged();
}

void PartEditor::finishEdit()
{
	PartState psnew(mPart);
	psnew.angle = mAngle;
	psnew.fp = mDialog->footprint();
	psnew.pos = mPos;
	psnew.refdes = mDialog->ref();
	psnew.refVisible = mDialog->refVisible();
	psnew.value = mDialog->value();
	psnew.valueVisible = mDialog->valueVisible();
	psnew.side = mSide;
	PartEditCmd *cmd = new PartEditCmd(NULL, mPart, psnew);
	ctrl()->doc()->doCommand(cmd);
	emit overlayChanged();
}

void PartEditor::drawOverlay(QPainter *painter)
{
	if (!mPart) return;
	if (mState == SELECTED)
	{
		painter->save();
		painter->setBrush(Qt::NoBrush);
		painter->setRenderHint(QPainter::Antialiasing, false);
		mPart->draw(painter, Layer::LAY_SELECTION);
		painter->restore();
	}
	else
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, false);
		painter->setBrush(Qt::NoBrush);
		painter->translate(mPos);
		painter->drawLine(QPoint(0, -INT_MAX), QPoint(0, INT_MAX));
		painter->drawLine(QPoint(-INT_MAX, 0), QPoint(INT_MAX, 0));
		if (mSide == Part::SIDE_BOTTOM)
			painter->scale(-1, 1);
		painter->rotate(mAngle);
		painter->drawRect(mBox);
		painter->restore();
	}
}


/////////////////////////////////// UNDO COMMANDS ///////////////////////////////////////////////


PartMoveCmd::PartMoveCmd(QUndoCommand *parent, Part *obj, QPoint newPos, int newAngle, Part::SIDE newSide)
	: QUndoCommand(parent), mPart(obj), mNewPos(newPos), mPrevPos(obj->pos()), mNewAngle(newAngle),
	mPrevAngle(obj->angle()), mNewSide(newSide), mPrevSide(obj->side())
{
}

void PartMoveCmd::redo()
{
	mPart->setPos(mNewPos);
	mPart->setAngle(mNewAngle);
	mPart->setSide(mNewSide);
}

void PartMoveCmd::undo()
{
	mPart->setPos(mPrevPos);
	mPart->setAngle(mPrevAngle);
	mPart->setSide(mPrevSide);
}

void PartEditCmd::undo()
{
	mPrevState.applyTo(mPart);
}

void PartEditCmd::redo()
{
	mNewState.applyTo(mPart);
}
