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


PartEditor::PartEditor(PCBController *ctrl, QSharedPointer<Part> part)
	: AbstractEditor(ctrl), mState(NEW), mPart(part), mDialog(NULL),
	mChangeSideAction(1, "Change Side"),
	mRotateCWAction(2, "Rotate CW"),
	mRotateCCWAction(3, "Rotate CCW"),
	mEditAction(0, "Edit Part"),
	mEditFPAction(1, "Edit Footprint"),
	mLockAction(2, (part && part->locked()) ? "Unlock Part" : "Lock Part"),
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
	if (!mPart)
		newPart();
	else
	{
		mState = SELECTED;
		ctrl()->hideObj(mPart);
		ctrl()->hideObj(mPart->refdesText());
		ctrl()->hideObj(mPart->valueText());
	}
	emit actionsChanged();
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
	case NEW:
		break;
	}
	return out;
}

void PartEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == MOVE || mState == EDIT_MOVE || mState == ADD_MOVE)
	{
		mPart->setPos(ctrl()->snapToPlaceGrid(ctrl()->view()->transform().inverted().map(event->pos())));
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
	if (mState == MOVE || mState == EDIT_MOVE || mState == ADD_MOVE)
	{
		ctrl()->unhideObj(mPart);
		ctrl()->unhideObj(mPart->refdesText());
		ctrl()->unhideObj(mPart->valueText());
		event->accept();
	}
	if (mState == MOVE || mState == EDIT_MOVE)
	{
		mState = SELECTED;
		finishEdit();
		emit actionsChanged();
		emit overlayChanged();
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
	if (event->key() == Qt::Key_Escape && (mState == MOVE || mState == EDIT_MOVE || mState == ADD_MOVE))
	{
		if (mState == ADD_MOVE)
		{
			mPart.clear();
			emit editorFinished();
		}
		else
		{
			mState = SELECTED;
			mPart->loadState(mPrevPartState);
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
	mPrevPartState = mPart->getState();
	startMove();
	mState = MOVE;
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

	QCursor::setPos(ctrl()->view()->mapToGlobal(ctrl()->view()->transform().map(mPart->pos())));
}

void PartEditor::actionDelete()
{
	PartDeleteCmd* cmd = new PartDeleteCmd(NULL, mPart, dynamic_cast<PCBDoc*>(ctrl()->doc()));
	ctrl()->doc()->doCommand(cmd);
	emit editorFinished();
}

void PartEditor::actionRotate(bool cw)
{
	if (mPart->side() == Part::SIDE_BOTTOM)
		cw = !cw;
	mPart->setAngle((mPart->angle() + (cw ? 270 : 90)) % 360);
	emit overlayChanged();
}

void PartEditor::actionChangeSide()
{
	mPart->setSide((mPart->side() == Part::SIDE_TOP) ? Part::SIDE_BOTTOM : Part::SIDE_TOP);
	emit overlayChanged();
}

void PartEditor::newPart()
{
	if (!mDialog)
		mDialog = QSharedPointer<EditPartDialog>(new EditPartDialog(ctrl()->view(), dynamic_cast<PCBDoc*>(ctrl()->doc())));
	mDialog->init();
	if (mDialog->exec() == QDialog::Rejected)
	{
		emit editorFinished();
		return;
	}
	mPart = QSharedPointer<Part>(new Part(dynamic_cast<PCBDoc*>(ctrl()->doc())));
	mPart->setFootprint(mDialog->footprint());
	mPart->refdesText()->setText(mDialog->ref());
	mPart->valueText()->setText(mDialog->value());
	mPart->setRefVisible(mDialog->refVisible());
	mPart->setValueVisible(mDialog->valueVisible());
	if (mDialog->isPosSet())
	{
		mPart->setPos(mDialog->pos());
		mPart->setAngle(mDialog->angle());
		mPart->setSide(mDialog->side());
		finishNew();
	}
	else
	{
		startMove();
		mState = ADD_MOVE;
		emit actionsChanged();
	}
}

void PartEditor::actionEdit()
{
	if (!mDialog)
		mDialog = QSharedPointer<EditPartDialog>(new EditPartDialog(ctrl()->view(), dynamic_cast<PCBDoc*>(ctrl()->doc())));
	mPrevPartState = mPart->getState();
	mDialog->init(mPart);
	if (mDialog->exec() == QDialog::Accepted)
	{
		mPart->setFootprint(mDialog->footprint());
		mPart->refdesText()->setText(mDialog->ref());
		mPart->valueText()->setText(mDialog->value());
		mPart->setRefVisible(mDialog->refVisible());
		mPart->setValueVisible(mDialog->valueVisible());
		if (mDialog->isPosSet())
		{
			mPart->setPos(mDialog->pos());
			mPart->setAngle(mDialog->angle());
			mPart->setSide(mDialog->side());
			finishEdit();
		}
		else // drag to position
		{
			startMove();
			mState = EDIT_MOVE;
			mPart->setSide(mDialog->side());
			emit actionsChanged();
			emit overlayChanged();
		}
	}
}

void PartEditor::finishNew()
{
	PartNewCmd *cmd = new PartNewCmd(NULL, mPart, dynamic_cast<PCBDoc*>(ctrl()->doc()));
	ctrl()->doc()->doCommand(cmd);
	ctrl()->selectObj(mPart);
	ctrl()->hideObj(mPart);
	ctrl()->hideObj(mPart->refdesText());
	ctrl()->hideObj(mPart->valueText());
	mState = SELECTED;
	emit overlayChanged();
}

void PartEditor::finishEdit()
{
	PCBObjEditCmd *cmd = new PCBObjEditCmd(NULL, mPart, mPrevPartState);
	ctrl()->doc()->doCommand(cmd);
	emit overlayChanged();
}

void PartEditor::drawOverlay(QPainter *painter)
{
	if (!mPart) return;
	mPart->draw(painter, Layer::LAY_SELECTION);
	if (mPart->refVisible())
		mPart->refdesText()->draw(painter, Layer::LAY_SELECTION);
	if (mPart->valueVisible())
		mPart->valueText()->draw(painter, Layer::LAY_SELECTION);
	painter->save();
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->drawRect(mPart->bbox());
	if (mPart->refVisible())
		painter->drawRect(mPart->refdesText()->bbox());
	if (mPart->valueVisible())
		painter->drawRect(mPart->valueText()->bbox());

	if (mState == MOVE || mState == EDIT_MOVE || mState == ADD_MOVE)
	{
		painter->translate(mPart->pos());
		painter->drawLine(QPoint(0, -INT_MAX), QPoint(0, INT_MAX));
		painter->drawLine(QPoint(-INT_MAX, 0), QPoint(INT_MAX, 0));
	}
	painter->restore();
}

PartNewCmd::PartNewCmd(QUndoCommand *parent, QSharedPointer<Part> obj, PCBDoc *doc)
	: QUndoCommand(parent), mPart(obj), mDoc(doc)
{
}

void PartNewCmd::undo()
{
	mDoc->removePart(mPart);
}

void PartNewCmd::redo()
{
	mDoc->addPart(mPart);
}

PartDeleteCmd::PartDeleteCmd(QUndoCommand *parent, QSharedPointer<Part> obj, PCBDoc *doc)
	: QUndoCommand(parent), mPart(obj), mDoc(doc)
{
}

void PartDeleteCmd::undo()
{
	mDoc->addPart(mPart);
}

void PartDeleteCmd::redo()
{
	mDoc->removePart(mPart);
}
