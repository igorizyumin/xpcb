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



AreaEditor::AreaEditor(PCBController *ctrl) :
	AbstractEditor(ctrl), mCtrl(ctrl), mState(PICK_FIRST),
	mCurrSegType(PolyContour::Segment::LINE), mLayer(Layer::LAY_TOP_COPPER)
{
}

void AreaEditor::drawSeg(QPainter *painter, QPoint start, QPoint end,
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

void AreaEditor::drawOverlay(QPainter *painter)
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

void AreaEditor::init()
{
	emit actionsChanged();
}

QList<const CtrlAction*> AreaEditor::actions() const
{
	return QList<const CtrlAction*>();
}

void AreaEditor::mouseMoveEvent(QMouseEvent *event)
{
	updatePos(mCtrl->snapToRouteGrid(mCtrl->view()->transform().inverted()
								  .map(event->pos())));

	emit overlayChanged();
}

void AreaEditor::updatePos(QPoint pos)
{
	// XXX TODO snap to 45 degrees or whatever
	mPos = pos;
}

void AreaEditor::mousePressEvent(QMouseEvent *event)
{
	event->accept();
}

void AreaEditor::mouseReleaseEvent(QMouseEvent *event)
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

void AreaEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
		emit editorFinished();
}

void AreaEditor::finishPolygon()
{
	QSharedPointer<Area> a(new Area(mCtrl->pcbDoc()));
	a->setLayer(mLayer);
	foreach(PolyContour::Segment s, mSegments)
	{
		a->poly().outline()->appendSegment(s);
	}
	if (!a->poly().isVoid())
	{
		QUndoCommand *cmd = new AreaNewCmd(NULL, a, mCtrl->pcbDoc());
		mCtrl->doc()->doCommand(cmd);
	}
	emit editorFinished();
}
