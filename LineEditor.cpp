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

#include "LineEditor.h"
#include "Controller.h"
#include "PCBObject.h"
#include "Document.h"


class LineNewCmd : public QUndoCommand
{
public:
	LineNewCmd(QUndoCommand *parent,
			   QSharedPointer<Line> obj,
			   FPDoc* doc)
		: QUndoCommand(parent), mLine(obj),
		  mDoc(doc) {}

	virtual void undo() { mDoc->removeLine(mLine); }
	virtual void redo() { mDoc->addLine(mLine); }

private:
	QSharedPointer<Line> mLine;
	FPDoc* mDoc;
};

class LineDelCmd : public QUndoCommand
{
public:
	LineDelCmd(QUndoCommand *parent,
			   QSharedPointer<Line> obj,
			   FPDoc* doc)
		: QUndoCommand(parent), mLine(obj),
		  mDoc(doc) {}

	virtual void undo() { mDoc->addLine(mLine); }
	virtual void redo() { mDoc->removeLine(mLine); }

private:
	QSharedPointer<Line> mLine;
	FPDoc* mDoc;
};


LineEditor::LineEditor(FPController *ctrl, QSharedPointer<Line> line)
	: AbstractEditor(ctrl),
	  mState(SELECTED),
	  mLine(line),
	  mWidth(0),
	  mLayer(Layer::LAY_SILK_TOP),
	  mLineType(Line::LINE),
	  mLineAction(4, "Straight Line"),
	  mArcCWAction(5, "Arc (CW)"),
	  mArcCCWAction(6, "Arc (CCW)"),
	  mEditAction(0, "Edit Segment"),
	  mMoveAction(3, "Move Segment"),
	  mDelAction(7, "Delete Segment"),
	  mMoveVtxAction(3, "Move Vertex")
{
	connect(&mLineAction, SIGNAL(execFired()), SLOT(actionLine()));
	connect(&mArcCWAction, SIGNAL(execFired()), SLOT(actionArcCW()));
	connect(&mArcCCWAction, SIGNAL(execFired()), SLOT(actionArcCCW()));
	connect(&mEditAction, SIGNAL(execFired()), SLOT(actionEdit()));
	connect(&mMoveAction, SIGNAL(execFired()), SLOT(actionMove()));
	connect(&mDelAction, SIGNAL(execFired()), SLOT(actionDel()));
	connect(&mMoveVtxAction, SIGNAL(execFired()), SLOT(actionMoveVtx()));
}

void LineEditor::init()
{
	if (mLine)
	{
		ctrl()->hideObj(mLine);
	}
	else
	{
		if (!mDialog)
			mDialog = QSharedPointer<EditLineDialog>(new EditLineDialog(ctrl()->view()));
		if (mDialog->exec() == QDialog::Rejected)
		{
			emit editorFinished();
			return;
		}
		mWidth = mDialog->width();
		mLayer = mDialog->layer();
		newLine();
		mState = LINE_NEW_FIRST;
	}
	emit actionsChanged();
}

QList<const CtrlAction*> LineEditor::actions() const
{
	QList<const CtrlAction*> out;
	switch(mState)
	{
	case SELECTED:
		out << &mEditAction << &mMoveAction << &mDelAction;
		// no break
	case LINE_NEW_SECOND:
		out << &mLineAction << &mArcCWAction << &mArcCCWAction;
		break;
	case VTX_SEL_START:
	case VTX_SEL_END:
		out << &mMoveVtxAction;
		break;
	default:
		break;
	}
	return out;
}

void LineEditor::drawOverlay(QPainter* painter)
{
	if (mState == LINE_NEW_FIRST
			|| mState == LINE_NEW_SECOND
			|| mState == VTX_MOVE_START
			|| mState == VTX_MOVE_END
			|| mState == PICK_REF
			|| mState == LINE_MOVE)
	{
		painter->save();
		QPen pen = painter->pen();
		pen.setWidth(0);
		painter->setPen(pen);
		painter->setBrush(Qt::NoBrush);
		painter->translate(mPos);
		painter->drawLine(QPoint(0, -INT_MAX), QPoint(0, INT_MAX));
		painter->drawLine(QPoint(-INT_MAX, 0), QPoint(INT_MAX, 0));
		painter->restore();
	}
	if (!mLine) return;
	switch(mState)
	{
	case SELECTED:
	case LINE_NEW_SECOND:
	case VTX_MOVE_START:
	case VTX_MOVE_END:
	case PICK_REF:
		mLine->draw(painter, Layer::LAY_SELECTION);
		break;
	case LINE_MOVE:
		painter->save();
		painter->translate(mPos - mRefPt);
		mLine->draw(painter, Layer::LAY_SELECTION);
		painter->restore();
		break;
	case VTX_SEL_START:
	case VTX_SEL_END:
	{
		mLine->draw(painter, Layer::LAY_SELECTION);
		double handSize = 10.0 / ctrl()->view()->transform().m11();
		QPoint handPos;
		if (mState == VTX_SEL_START)
			handPos = mLine->start();
		else
			handPos = mLine->end();
		painter->save();
		painter->setBrush(QColor(255,0,0));
		QPen pen = painter->pen();
		pen.setWidth(0);
		pen.setColor(QColor(255,0,0));
		painter->setPen(pen);
		painter->drawRect(handPos.x() - handSize/2,
						  handPos.y() - handSize/2,
						  handSize, handSize);
		painter->restore();
	}
		break;
	default:
		break;
	}
}

void LineEditor::actionMove()
{
	mPrevState = mLine->getState();
	mState = PICK_REF;
	mRefPt = mPos;
	emit actionsChanged();
}

void LineEditor::actionMoveVtx()
{
	if (mState == VTX_SEL_START)
	{
		mState = VTX_MOVE_START;
		mPos = mLine->start();
	}
	else
	{
		mState = VTX_MOVE_END;
		mPos = mLine->end();
	}
	mPrevState = mLine->getState();
}

void LineEditor::newLine()
{
	mLine = QSharedPointer<Line>(new Line());
	mLine->setLayer(mLayer);
	mLine->setWidth(mWidth);
	mLine->setType(mLineType);
}

void LineEditor::setLineType(Line::LineType type)
{
	if (mState == SELECTED)
	{
		PCBObjState prev = mLine->getState();
		mLine->setType(type);
		QUndoCommand* cmd = new PCBObjEditCmd(NULL, mLine, prev);
		ctrl()->doc()->doCommand(cmd);
	}
	else
	{
		mLine->setType(type);
	}
	emit overlayChanged();
}

void LineEditor::actionEdit()
{
	PCBObjState prev = mLine->getState();
	if (!mDialog)
		mDialog = QSharedPointer<EditLineDialog>(new EditLineDialog(ctrl()->view()));
	mDialog->init(mLine);
	if (mDialog->exec() == QDialog::Rejected)
		return;
	mWidth = mDialog->width();
	mLayer = mDialog->layer();
	mLine->setWidth(mWidth);
	mLine->setLayer(mLayer);
	QUndoCommand* cmd = new PCBObjEditCmd(NULL, mLine, prev);
	ctrl()->doc()->doCommand(cmd);
	emit overlayChanged();
}

void LineEditor::actionDel()
{
	QUndoCommand* cmd = new LineDelCmd(NULL, mLine, dynamic_cast<FPDoc*>(ctrl()->doc()));
	ctrl()->doc()->doCommand(cmd);
	emit editorFinished();
}

void LineEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == LINE_NEW_FIRST
			|| mState == LINE_NEW_SECOND
			|| mState == VTX_MOVE_START
			|| mState == VTX_MOVE_END
			|| mState == PICK_REF
			|| mState == LINE_MOVE)
	{
		mPos = ctrl()->snapToPlaceGrid(
					ctrl()->view()->transform().inverted()
					.map(event->pos()));
		switch(mState)
		{
		case VTX_MOVE_START:
			mLine->setStart(mPos);
			break;
		case LINE_NEW_SECOND:
		case VTX_MOVE_END:
			mLine->setEnd(mPos);
			break;
		default:
			break;
		}
		emit overlayChanged();
	}
}

void LineEditor::mousePressEvent(QMouseEvent *event)
{
	if (mState == SELECTED
			|| mState == VTX_SEL_START
			|| mState == VTX_SEL_END)
	{
		// check if a vertex was hit
		QPoint start = ctrl()->view()->transform().map(mLine->start());
		QPoint end = ctrl()->view()->transform().map(mLine->end());
		if ((start - event->pos()).manhattanLength() <= 20 ||
			   (end - event->pos()).manhattanLength() <= 20 )
			event->accept();
		else if (mState != SELECTED)
		{
			QPoint pos = ctrl()->view()->transform().inverted().map(event->pos());
			if (mLine->testHit(pos, ctrl()->hitRadius(), mLine->layer()))
				event->accept();
			else
				event->ignore();
		}
		else
			event->ignore();
	}
	else
		event->ignore();
}

void LineEditor::mouseReleaseEvent(QMouseEvent *event)
{
	switch(mState)
	{
	case SELECTED:
	case VTX_SEL_START:
	case VTX_SEL_END:
	{
		// check if a vertex was hit
		QPoint start = ctrl()->view()->transform().map(mLine->start());
		QPoint end = ctrl()->view()->transform().map(mLine->end());
		if ((start - event->pos()).manhattanLength() <= 20)
		{
			mState = VTX_SEL_START;
			event->accept();
			emit actionsChanged();
			emit overlayChanged();
		}
		else if ((end - event->pos()).manhattanLength() <= 20 )
		{
			mState = VTX_SEL_END;
			event->accept();
			emit actionsChanged();
			emit overlayChanged();
		}

		else if (mState != SELECTED)
		{
			QPoint pos = ctrl()->view()->transform().inverted().map(event->pos());
			if (mLine->testHit(pos, ctrl()->hitRadius(), mLine->layer()))
			{
				mState = SELECTED;
				event->accept();
				emit actionsChanged();
				emit overlayChanged();
			}
			else
				event->ignore();
		}
		else
			event->ignore();
	}
		break;
	case LINE_NEW_FIRST:
	{
		mLine->setStart(mPos);
		mLine->setEnd(mPos);
		mState = LINE_NEW_SECOND;
		emit actionsChanged();
		emit overlayChanged();
	}
		break;

	case LINE_NEW_SECOND:
	{
		mLine->setEnd(mPos);
		if (mLine->start() != mLine->end())
		{
			QUndoCommand* cmd = new LineNewCmd(NULL, mLine, dynamic_cast<FPDoc*>(ctrl()->doc()));
			ctrl()->doc()->doCommand(cmd);
		}
		newLine();
		mLine->setStart(mPos);
		mLine->setEnd(mPos);
		emit overlayChanged();
	}
		break;
	case VTX_MOVE_START:
	case VTX_MOVE_END:
	{
		QUndoCommand* cmd = new PCBObjEditCmd(NULL, mLine, mPrevState);
		ctrl()->doc()->doCommand(cmd);
		mState = (mState == VTX_MOVE_START) ? VTX_SEL_START : VTX_SEL_END;
		emit overlayChanged();
		emit actionsChanged();
	}
		break;
	case PICK_REF:
	{
		mRefPt = mPos;
		mState = LINE_MOVE;
		emit overlayChanged();
		emit actionsChanged();
	}
		break;
	case LINE_MOVE:
	{
		mLine->setStart(mLine->start() + mPos - mRefPt);
		mLine->setEnd(mLine->end() + mPos - mRefPt);
		QUndoCommand* cmd = new PCBObjEditCmd(NULL, mLine, mPrevState);
		ctrl()->doc()->doCommand(cmd);
		mState = SELECTED;
		emit overlayChanged();
		emit actionsChanged();
	}
		break;
	default:
		break;
	}
}

void LineEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && mState != SELECTED)
	{
		if (mState == VTX_MOVE_START || mState == VTX_MOVE_END)
		{
			mLine->loadState(mPrevState);
		}
		emit editorFinished();
	}
}
