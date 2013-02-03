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

#include "Area.h"
#include "Controller.h"
#include "AreaEditor.h"
#include "Document.h"
#include "Line.h"

class AreaNewCmd : public QUndoCommand
{
public:
	AreaNewCmd(QUndoCommand *parent,
			   QSharedPointer<Area> obj,
			   PCBDoc* doc)
		: QUndoCommand(parent), mArea(obj),
		  mDoc(doc) {}

	virtual void undo() { mDoc->removeArea(mArea); }
	virtual void redo() { mDoc->addArea(mArea); }

private:
	QSharedPointer<Area> mArea;
	PCBDoc* mDoc;
};

class AreaDelCmd : public QUndoCommand
{
public:
    AreaDelCmd(QUndoCommand *parent,
               QSharedPointer<Area> obj,
               PCBDoc* doc)
        : QUndoCommand(parent), mArea(obj),
          mDoc(doc) {}

    virtual void undo() { mDoc->addArea(mArea); }
    virtual void redo() { mDoc->removeArea(mArea); }

private:
    QSharedPointer<Area> mArea;
    PCBDoc* mDoc;
};

NewAreaEditor::NewAreaEditor(Controller *ctrl) :
	AbstractEditor(ctrl), mCtrl(ctrl), mState(PICK_FIRST),
	mCurrSegType(PolyContour::Segment::LINE), mLayer(Layer::LAY_TOP_COPPER)
{
}

void NewAreaEditor::drawSeg(QPainter *painter, QPoint start, QPoint end,
						 PolyContour::Segment::SegType type)
{
	switch(type)
	{
	case PolyContour::Segment::START:
		break;
	case PolyContour::Segment::LINE:
		painter->drawLine(start, end);
		break;
	case PolyContour::Segment::ARC_CW:
		XPcb::drawArc(painter, start, end, true);
		break;
	case PolyContour::Segment::ARC_CCW:
		XPcb::drawArc(painter, start, end, false);
		break;
	}
}

void NewAreaEditor::drawOverlay(QPainter *painter)
{
	QPoint prev;
	if (mSegments.isEmpty())
		return;
	foreach(PolyContour::Segment s, mSegments)
	{
		drawSeg(painter, prev, s.end, s.type);
		prev = s.end;
	}
	drawSeg(painter, prev, mPos, mCurrSegType);
}

void NewAreaEditor::init()
{
	emit actionsChanged();
}

QList<const CtrlAction*> NewAreaEditor::actions() const
{
	return QList<const CtrlAction*>();
}

void NewAreaEditor::mouseMoveEvent(QMouseEvent *event)
{
	updatePos(mCtrl->snapToRouteGrid(mCtrl->view()->transform().inverted()
								  .map(event->pos())));

	emit overlayChanged();
}

void NewAreaEditor::updatePos(QPoint pos)
{
	// XXX TODO snap to 45 degrees or whatever
	mPos = pos;
}

void NewAreaEditor::mousePressEvent(QMouseEvent *event)
{
	event->accept();
}

void NewAreaEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (mState == PICK_FIRST)
		{
			mSegments.append(PolyContour::Segment(PolyContour::Segment::START, mPos));
			mState = PICK_NEXT;
		}
		else if (mState == PICK_NEXT)
		{
			// check for null segment
			if (mSegments.last().end == mPos)
			{
				event->accept();
				return;
			}

			// XXX TODO check for self intersection

			// add segment
			mSegments.append(PolyContour::Segment(mCurrSegType, mPos));

			// check if the polygon has been closed
			if (mSegments.first().end == mSegments.last().end)
				finishPolygon();
		}
		event->accept();
	}
	else if (event->button() == Qt::RightButton)
	{
		if (mState == PICK_NEXT)
		{
			mSegments.append(PolyContour::Segment(mCurrSegType, mSegments.first().end));
			finishPolygon();
		}
		else
			emit editorFinished();
		event->accept();
	}
}

void NewAreaEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
		emit editorFinished();
}

void NewAreaEditor::finishPolygon()
{
	QSharedPointer<Area> a(new Area(dynamic_cast<PCBDoc*>(mCtrl->doc())));
	a->setLayer(mLayer);
	foreach(PolyContour::Segment s, mSegments)
	{
		a->poly().outline()->appendSegment(s);
	}
	if (!a->poly().isVoid())
	{
		QUndoCommand *cmd = new AreaNewCmd(NULL, a, dynamic_cast<PCBDoc*>(mCtrl->doc()));
		mCtrl->doc()->doCommand(cmd);
	}
	emit editorFinished();
}

///////////////////////////////////////////////////////////////////////////////

AreaEditor::AreaEditor(Controller *ctrl, QSharedPointer<Area> a)
    : AbstractEditor(ctrl), mState(SELECTED), mCtrl(ctrl), mArea(a),
      mMoveAction(3, "Move Area"), mDelAction(7, "Delete Area")
{
    connect(&mMoveAction, SIGNAL(execFired()),
            this, SLOT(startMoveArea()));
    connect(&mDelAction, SIGNAL(execFired()),
            this, SLOT(deleteArea()));
}

void AreaEditor::drawOverlay(QPainter* painter)
{
    painter->save();
    painter->setBrush(Layer::color(Layer::LAY_SELECTION));
    QPen pen = painter->pen();
    pen.setWidth(0);
    painter->setPen(pen);

    if (mState == MOVE || mState == PICK_REF)
    {
        Controller::drawCrosshair45(painter, mPos);
    }

    mArea->poly().outline()->draw(painter);

    painter->restore();
}

void AreaEditor::init()
{
    emit actionsChanged();
}

QList<const CtrlAction*> AreaEditor::actions() const
{
    QList<const CtrlAction*> out;
    if (mState == SELECTED)
    {
        out << &mMoveAction;
        out << &mDelAction;
    }
    return out;
}

void AreaEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (mState == MOVE || mState == PICK_REF)
    {
        mPos = mCtrl->snapToRouteGrid(mCtrl->view()->transform().inverted()
                                      .map(event->pos()));
        if (mState == MOVE)
            updateMove();

        emit overlayChanged();
    }
}

void AreaEditor::mousePressEvent(QMouseEvent *event)
{
    if (mState == MOVE || mState == PICK_REF)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void AreaEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (mState == MOVE)
    {
        event->accept();
        finishMove();
    }
    else if (mState == PICK_REF)
    {
        event->accept();
        mPrevPt = mPos;
        mState = MOVE;
        emit actionsChanged();
        emit overlayChanged();
    }
    else
    {
        event->ignore();
    }
}

void AreaEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        if (mState == MOVE)
            abortMove();
        if (mState == PICK_REF)
        {
            mState = SELECTED;
            emit actionsChanged();
            emit overlayChanged();
        }
    }
}

void AreaEditor::startMoveArea()
{
    mPrevState = mArea->getState();
    mState = PICK_REF;
    emit actionsChanged();
    emit overlayChanged();
}

void AreaEditor::updateMove()
{
    mArea->poly().translate(mPos - mPrevPt);
    mPrevPt = mPos;
}

void AreaEditor::finishMove()
{
    PCBObjEditCmd* cmd = new PCBObjEditCmd(NULL, mArea, mPrevState);
    mCtrl->doc()->doCommand(cmd);
    mState = SELECTED;
    emit actionsChanged();
    emit overlayChanged();
}

void AreaEditor::abortMove()
{
    mArea->loadState(mPrevState);
    mState = SELECTED;
    emit actionsChanged();
    emit overlayChanged();
}

void AreaEditor::deleteArea()
{
    QUndoCommand *cmd = new AreaDelCmd(NULL, mArea, dynamic_cast<PCBDoc*>(mCtrl->doc()));
    mCtrl->doc()->doCommand(cmd);
    emit editorFinished();
}
