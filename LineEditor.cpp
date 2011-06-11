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

#include "LineEditor.h"
#include "Line.h"
#include "PCBView.h"

LineEditor::LineEditor(FPController *ctrl, Line *line)
	: AbstractEditor(ctrl), mLine(line)
{

}

LineEditor::~LineEditor()
{

}

QList<const CtrlAction*> LineEditor::actions() const
{
#if 0
	QList<CtrlAction> out;

	switch(mState)
	{
	case LINE_NEW_SECOND:
		out.append(CtrlAction(0, "Straight line"));
		out.append(CtrlAction(1, "Arc (CW)"));
		out.append(CtrlAction(2, "Arc (CCW)"));
		break;
	case LINE_NEW_FIRST:
	case VTX_MOVE:
	case LINE_MOVE:
		break;
	case SELECTED:
		out.append(CtrlAction(0, "Edit Segment"));
		out.append(CtrlAction(3, "Move Segment"));
		out.append(CtrlAction(7, "Delete Segment"));
		break;
	}
	return out;
#endif
}

void LineEditor::drawOverlay(QPainter *painter)
{
	Q_ASSERT(mLine != NULL);


}

void LineEditor::init()
{
	if (!mLine)
	{
		// init new line
		mLine = new Line();
		mState = LINE_NEW_FIRST;
	}
}

void LineEditor::action(int key)
{

}

void LineEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == VTX_MOVE ||
		mState == LINE_MOVE ||
		mState == LINE_NEW_FIRST ||
		mState == LINE_NEW_SECOND)
	{
		mPos = mCtrl->snapToPlaceGrid(mCtrl->view()->transform().inverted().map(event->pos()));
		emit overlayChanged();
	}

	// we don't want to eat mouse events
	event->ignore();
}

void LineEditor::mousePressEvent(QMouseEvent *event)
{
	if (mState != SELECTED)
		event->accept();
	else
		event->ignore();
}

void LineEditor::mouseReleaseEvent(QMouseEvent *event)
{

}

void LineEditor::keyPressEvent(QKeyEvent *event)
{
}
