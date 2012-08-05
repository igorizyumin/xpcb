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

#include <QPoint>
#include "TraceEditor.h"
#include "Document.h"
#include "SegmentLayerDialog.h"
#include "SegmentWidthDialog.h"

using namespace XPcb;

// utility functions
inline bool segParallel(QSharedPointer<Segment> seg1,
						QSharedPointer<Segment> seg2)
{
	return isParallel(seg1->v1()->pos() - seg1->v2()->pos(),
					  seg2->v1()->pos() - seg2->v2()->pos());
}

inline bool segZeroLength(QSharedPointer<Segment> seg)
{
	return seg->v1()->pos() == seg->v2()->pos();
}

// XXX TODO move this into the controller
void drawCrosshair45(QPainter* painter, QPoint pos)
{
	// draw 45 degree crosshair
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setBrush(Qt::NoBrush);
	QPen p = painter->pen();
	p.setStyle(Qt::DashLine);
	p.setColor(QColor(128, 128, 128));
	painter->setPen(p);
	painter->translate(pos);
	painter->drawLine(QPoint(0, -INT_MAX), QPoint(0, INT_MAX));
	painter->drawLine(QPoint(-INT_MAX, -INT_MAX), QPoint(INT_MAX, INT_MAX));
	painter->drawLine(QPoint(-INT_MAX, 0), QPoint(INT_MAX, 0));
	painter->drawLine(QPoint(-INT_MAX, INT_MAX), QPoint(INT_MAX, -INT_MAX));
	painter->restore();
}

///////////////////////////////////////////////////////////////////////////////

NewTraceEditor::NewTraceEditor(Controller *ctrl)
	: AbstractEditor(ctrl), mCtrl(ctrl), mState(PICK_START),
	  mMode(ModeStraight45), mWidth(XPcb::milToPcb(10)),
	  mLayer(Layer::LAY_TOP_COPPER)
{

}

void NewTraceEditor::drawOverlay(QPainter* painter)
{
	drawCrosshair45(painter, mPos);

	if (mState == PICK_END)
	{
		mSeg1->draw(painter, mLayer);
		mSeg2->draw(painter, mLayer);
		mVtxStart->draw(painter, mLayer);
		mVtxMid->draw(painter, mLayer);
		mVtxEnd->draw(painter, mLayer);
	}

}

void NewTraceEditor::init()
{
	emit actionsChanged();
}

QList<const CtrlAction*> NewTraceEditor::actions() const
{
	return QList<const CtrlAction*>();
}

void NewTraceEditor::mouseMoveEvent(QMouseEvent *event)
{
	mPos = mCtrl->snapToRouteGrid(mCtrl->view()->transform().inverted()
								  .map(event->pos()));
	if (mState == PICK_END)
	{
		mVtxEnd->setPos(mPos);
		updateDogleg();
	}

	emit overlayChanged();
}

void NewTraceEditor::mousePressEvent(QMouseEvent *event)
{
	event->accept();
}

void NewTraceEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (mState == PICK_START)
	{
		// TODO check if a segment or vertex is already present
		// create all the pieces
		mVtxStart = QSharedPointer<Vertex>(new Vertex(mPos));
		mVtxMid = QSharedPointer<Vertex>(new Vertex(mPos));
		mVtxEnd = QSharedPointer<Vertex>(new Vertex(mPos));
		mSeg1 = QSharedPointer<Segment>(new Segment(mLayer, mWidth));
		mSeg2 = QSharedPointer<Segment>(new Segment(mLayer, mWidth));
		mSeg1->setV1(mVtxStart);
		mSeg1->setV2(mVtxMid);
		mSeg2->setV1(mVtxMid);
		mSeg2->setV2(mVtxEnd);
		mState = PICK_END;
	}
	else if (mState == PICK_END)
	{
		// check for null segment
		if (mVtxStart->pos() == mVtxEnd->pos())
		{
			event->accept();
			return;
		}
		QUndoCommand* cmd = dynamic_cast<PCBDoc*>(mCtrl->doc())->traceList()
				->addSegmentCmd(mSeg1, mVtxStart, mVtxMid);
		mCtrl->doc()->doCommand(cmd);
		mSeg1 = mSeg2;
		mVtxStart = mVtxMid;
		mVtxMid = mVtxEnd;
		mVtxEnd = QSharedPointer<Vertex>(new Vertex(mPos));
		mSeg2 = QSharedPointer<Segment>(new Segment(mLayer, mWidth));
		mSeg2->setV1(mVtxMid);
		mSeg2->setV2(mVtxEnd);
		toggleMode();
	}
	event->accept();
}

void NewTraceEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
		emit editorFinished();
}

void NewTraceEditor::updateDogleg()
{
	QPoint start = mVtxStart->pos();
	QPoint end = mVtxEnd->pos();
	QPoint d = end - start;
	if (mMode == ModeStraight45)
	{
		if (abs(d.x()) > abs(d.y()))
			mVtxMid->setPos(QPoint(end.x() - sign(d.x())*abs(d.y()),
								   start.y()));
		else
			mVtxMid->setPos(QPoint(start.x(),
								   end.y() - sign(d.y())*abs(d.x())));
	}
	else if (mMode == Mode45Straight)
	{
		if (abs(d.x()) > abs(d.y()))
			mVtxMid->setPos(QPoint(start.x() + sign(d.x()) * abs(d.y()),
								   end.y()));
		else
			mVtxMid->setPos(QPoint(end.x(),
								   start.y() + sign(d.y()) * abs(d.x())));
	}
}

void NewTraceEditor::toggleMode()
{
	mMode = (mMode == Mode45Straight) ? ModeStraight45 : Mode45Straight;
}

///////////////////////////////////////////////////////////////////////////////

SegmentEditor::SegmentEditor(Controller *ctrl,
							 QSharedPointer<Segment> segment)
	: AbstractEditor(ctrl), mState(SELECTED), mCtrl(ctrl), mSegment(segment),
	  mSlideAction(3, "Slide Segment"), mAddVtxAction(2, "Add Vertex"),
	  mDelAction(7, "Ripup Segment"), mSetLayerAction(1, "Change Layer"),
	  mSetWidthAction(0, "Change Width")
{
	connect(&mSlideAction, SIGNAL(execFired()),
			this, SLOT(startSlideSegment()));
	connect(&mAddVtxAction, SIGNAL(execFired()),
			this, SLOT(startInsertVtx()));
	connect(&mDelAction, SIGNAL(execFired()),
			this, SLOT(deleteSegment()));
	connect(&mSetLayerAction, SIGNAL(execFired()),
			this, SLOT(setLayer()));
	connect(&mSetWidthAction, SIGNAL(execFired()),
			this, SLOT(setWidth()));
	ctrl->hideObj(mSegment);
}

void SegmentEditor::drawOverlay(QPainter* painter)
{
	if (mState == ADD_VTX || mState == SLIDE)
		drawCrosshair45(painter, mPos);

	if (mState == SELECTED)
	{
		mSegment->draw(painter, Layer::LAY_SELECTION);
	}
	else if (mState == SLIDE)
	{
		if (mSeg1)
			painter->drawLine(mFixedPt1, mPt1);
		painter->drawLine(mPt1, mPt2);
		if (mSeg2)
			painter->drawLine(mFixedPt2, mPt2);
	}
	else if (mState == ADD_VTX)
	{
		painter->drawLine(mPt1, mPos);
		painter->drawLine(mPos, mPt2);
	}
}

void SegmentEditor::init()
{
	emit actionsChanged();
}

QList<const CtrlAction*> SegmentEditor::actions() const
{
	QList<const CtrlAction*> out;
	if (mState == SELECTED)
		out << &mSlideAction << &mAddVtxAction
			<< &mDelAction << &mSetLayerAction
			<< &mSetWidthAction;
	return out;
}

void SegmentEditor::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pos = mCtrl->snapToRouteGrid(mCtrl->view()->transform().inverted()
								  .map(event->pos()));
	if (mState == SLIDE)
	{
		mPos = pos;
		updateSlide();
		emit overlayChanged();
	}
	else if (mState == ADD_VTX)
	{
		mPos = pos;
		emit overlayChanged();
	}
}

void SegmentEditor::mousePressEvent(QMouseEvent *event)
{
	if (mState == SLIDE || mState == ADD_VTX)
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void SegmentEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (mState == SLIDE)
	{
		event->accept();
		finishSlide();
	}
	else if (mState == ADD_VTX)
	{
		event->accept();
		finishAddVtx();
	}
	else
	{
		event->ignore();
	}
}

void SegmentEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
	{
		if (mState == SLIDE)
			abortSlide();
		else
			mState = SELECTED;
		emit actionsChanged();
		emit overlayChanged();
	}
}

void SegmentEditor::deleteSegment()
{
	QUndoCommand *cmd = dynamic_cast<PCBDoc*>(mCtrl->doc())->traceList()->removeSegmentCmd(mSegment);
	// exec the command
	mCtrl->doc()->doCommand(cmd);
	emit editorFinished();
}

void SegmentEditor::setLayer()
{
	SegmentLayerDialog dlg(mCtrl->view(), mCtrl);
	dlg.init(mSegment.data());
	PCBObjState prev = mSegment->getState();
	if (dlg.exec() == QDialog::Accepted)
	{
		// XXX TODO apply to connected segments if needed
		mSegment->setLayer(dlg.layer());
		PCBObjEditCmd* cmd = new PCBObjEditCmd(0, mSegment, prev);
		mCtrl->doc()->doCommand(cmd);
	}
}

void SegmentEditor::setWidth()
{
	SegmentWidthDialog dlg(mCtrl->view());
	dlg.init(mSegment.data());
	PCBObjState prev = mSegment->getState();
	if (dlg.exec() == QDialog::Accepted)
	{
		// XXX TODO apply to connected segments if needed
		mSegment->setWidth(dlg.width().toPcb());
		PCBObjEditCmd* cmd = new PCBObjEditCmd(0, mSegment, prev);
		mCtrl->doc()->doCommand(cmd);
	}
}

void SegmentEditor::startSlideSegment()
{
	// get the neighboring segments
	// first, get the neighboring segment vertices
	QSharedPointer<Vertex> v1 = mSegment->v1();
	QSharedPointer<Vertex> v2 = mSegment->v2();
	// find the other segments
	QSet<QSharedPointer<Segment> > v1s = v1->segments();
	v1s.remove(mSegment);
	QSet<QSharedPointer<Segment> > v2s = v2->segments();
	v2s.remove(mSegment);
	if (v1s.size() > 1 || v2s.size() > 1)
	{
		// more than 1 neighbor, cannot move (yet)
		return;
	}

	mPt1 = v1->pos();
	mPt2 = v2->pos();
	mSecantVec = mPt2 - mPt1;

	if (v1s.size() == 0)
	{
		mSeg1.clear();
		mVector1 = perp(mSecantVec);
		mFixedPt1 = mPt1;
	}
	else
	{
		mSeg1 = v1s.values().first();
		mCtrl->hideObj(mSeg1);
		mFixedPt1 = mSeg1->otherVertex(v1)->pos();
		mVector1 = mPt1 - mFixedPt1;
	}

	if (v2s.size() == 0)
	{
		// if there are no neighboring segments,
		// make fake lines perpendicular to the secant
		mSeg2.clear();
		mVector2 = perp(mSecantVec);
		mFixedPt2 = mPt2;
	}
	else
	{
		mSeg2 = v2s.values().first();
		mCtrl->hideObj(mSeg2);
		mFixedPt2 = mSeg2->otherVertex(v2)->pos();
		mVector2 = mPt2 - mFixedPt2;
	}

	mSignX = sign(mSecantVec.x());
	mSignY = sign(mSecantVec.y());
	mState = SLIDE;
	emit actionsChanged();
}

void SegmentEditor::startInsertVtx()
{
	mPt1 = mSegment->v1()->pos();
	mPt2 = mSegment->v2()->pos();
	mState = ADD_VTX;
}

void SegmentEditor::updateSlide()
{
	QPoint pos = mPos;

	forever
	{
		double sc = lineIntersect(mFixedPt1, mVector1, pos, mSecantVec);
		QPoint newpt1(mFixedPt1 + mVector1 * sc);
		// make sure neighboring segment length is >= 0
		if (sc < 0 && mSeg1)
		{
			// constrain the position and try again
			pos = mFixedPt1;
			continue;
		}

		// repeat this exercise for segment 2
		sc = lineIntersect(mFixedPt2, mVector2, pos, mSecantVec);
		QPoint newpt2(mFixedPt2 + mVector2 * sc);
		if (sc < 0 && mSeg2)
		{
			pos = mFixedPt2;
			continue;
		}

		// ensure this segment length is >=0
		short sx = sign((newpt2 - newpt1).x());
		short sy = sign((newpt2 - newpt1).y());
		if ( (sx && mSignX != sx) ||
				(sy && mSignY != sy))
		{
			// limit to intersection of neighboring segments
			pos = lineIntersectPt(mFixedPt1, mVector1, mFixedPt2, mVector2);
			continue;
		}

		// everything is ok, update the points and don't loop again
		mPt1 = newpt1;
		mPt2 = newpt2;
		break;
	}
}

struct SegListEntry
{
	SegListEntry(QSharedPointer<Segment> s,
				 QSharedPointer<Vertex> vs,
				 QSharedPointer<Vertex> ve)
		: seg(s), start(vs), end(ve) {}

	QSharedPointer<Segment> seg;
	QSharedPointer<Vertex> start;
	QSharedPointer<Vertex> end;
};

// cleans up the trace (removes collinear and zero-length segments)
void SegmentEditor::cleanUpTrace(QList<QSharedPointer<Segment> > segments,
								 QUndoCommand* parent)
{
	if (segments.length() < 2) return;
	QSharedPointer<TraceList> tl = dynamic_cast<PCBDoc*>(mCtrl->doc())->traceList();

	// list of nonzero length segments and their start vertices
	QList<SegListEntry> nzList;

	// start vertex for current segment
	QSharedPointer<Vertex> startVtx =
			segments[0]->otherVertex(segments[0]->commonVertex(segments[1]));
	foreach(QSharedPointer<Segment> s, segments)
	{
		// remove all zero length segments
		if (segZeroLength(s))
		{
			// remove segment
			tl->removeSegmentCmd(s, parent);
		}
		else
		{
			if (!nzList.isEmpty() && segParallel(nzList.last().seg, s))
			{
				// Segment is collinear with previous nonzero segment.
				// Remove the previous segment, change the start vertex of this
				// one to the start of the previous one, pop it off the list,
				// and replace the entry with this segment.
				QSharedPointer<Vertex> prevVtx = nzList.last().start;
				tl->swapVtxCmd(s, startVtx, prevVtx, parent);
				tl->removeSegmentCmd(nzList.last().seg, parent);
				nzList.removeLast();
				nzList.append(SegListEntry(s, prevVtx,
										   s->otherVertex(startVtx)));
			}
			else
			{
				if (!nzList.isEmpty() && nzList.last().end != startVtx)
				{
					// There is a zero length segment that was removed.
					// Bypass the missing vertex.
					tl->swapVtxCmd(s, startVtx, nzList.last().end, parent);
					nzList.append(SegListEntry(s, nzList.last().end,
											   s->otherVertex(startVtx)));
				}
				else
					// Everything squares up.  Append the entry for this segment.
					nzList.append(SegListEntry(s, startVtx,
											   s->otherVertex(startVtx)));
			}
		}
		// Traverse the list of vertices.
		startVtx = s->otherVertex(startVtx);
	}
}

void SegmentEditor::finishSlide()
{
	QUndoCommand *parent = new QUndoCommand("slide segment");
	// move the points and create undo commands
	QSharedPointer<Vertex> v1 = mSegment->v1();
	QSharedPointer<Vertex> v2 = mSegment->v2();
	PCBObjState st1 = v1->getState();
	PCBObjState st2 = v2->getState();
	v1->setPos(mPt1);
	v2->setPos(mPt2);
	new PCBObjEditCmd(parent, v1, st1);
	new PCBObjEditCmd(parent, v2, st2);
	// make a list for the cleanup function
	QList<QSharedPointer<Segment> > l;
	if (mSeg1)
	{
		// get the adjoining segment to seg1
		QSet<QSharedPointer<Segment> > sl = mSeg1->otherVertex(
					mSeg1->commonVertex(mSegment))->segments();
		sl.remove(mSeg1);
		if (!sl.isEmpty())
			l.append(sl.values().first());
		l.append(mSeg1);
	}

	l.append(mSegment);

	if (mSeg2)
	{
		l.append(mSeg2);
		// get the adjoining segment to seg2
		QSet<QSharedPointer<Segment> > sl = mSeg2->otherVertex(
					mSeg2->commonVertex(mSegment))->segments();
		sl.remove(mSeg2);
		if (!sl.isEmpty())
			l.append(sl.values().first());
	}

	cleanUpTrace(l, parent);

	// unhide objs
	if (mSeg1)
		mCtrl->unhideObj(mSeg1);
	if (mSeg2)
		mCtrl->unhideObj(mSeg2);
	// execute command and reset state
	mCtrl->doc()->doCommand(parent);
	if (!dynamic_cast<PCBDoc*>(mCtrl->doc())->traceList()->segments().contains(mSegment))
	{
		emit editorFinished();
		return;
	}
	mState = SELECTED;
	emit actionsChanged();
	emit overlayChanged();
}

void SegmentEditor::abortSlide()
{
	if (mSeg1)
		mCtrl->unhideObj(mSeg1);
	if (mSeg2)
		mCtrl->unhideObj(mSeg2);
	mSeg1.clear();
	mSeg2.clear();
	mState = SELECTED;
}

void SegmentEditor::finishAddVtx()
{
	QUndoCommand *parent = new QUndoCommand("insert vertex");
	QSharedPointer<Vertex> v2 = mSegment->v2();
	// create a new vertex
	QSharedPointer<Vertex> vnew(new Vertex(mPos));
	// create a new segment
	QSharedPointer<Segment> snew(mSegment->clone());
	// make the modifications
	QSharedPointer<TraceList> tl = dynamic_cast<PCBDoc*>(mCtrl->doc())->traceList();
	tl->swapVtxCmd(mSegment, v2, vnew, parent);
	tl->addSegmentCmd(snew, vnew, v2, parent);
	mCtrl->doc()->doCommand(parent);
	mState = SELECTED;
	emit actionsChanged();
	emit overlayChanged();
}

///////////////////////////////////////////////////////////////////////////////

VertexEditor::VertexEditor(Controller *ctrl, QSharedPointer<Vertex> vtx)
	: AbstractEditor(ctrl), mState(SELECTED), mCtrl(ctrl), mVtx(vtx),
	  mMoveAction(3, "Move Vertex"), mDelAction(7, "Delete Vertex")
{
	connect(&mMoveAction, SIGNAL(execFired()),
			this, SLOT(startMove()));
	connect(&mDelAction, SIGNAL(execFired()),
			this, SLOT(deleteVtx()));
	mPos = mVtx->pos();
}

void VertexEditor::drawOverlay(QPainter* painter)
{
	if (mState == MOVE)
		drawCrosshair45(painter, mPos);

	if (mState == SELECTED)
	{
		double handSize = 10.0 / ctrl()->view()->transform().m11();
		painter->save();
		painter->setBrush(Layer::color(Layer::LAY_SELECTION));
		QPen pen = painter->pen();
		pen.setWidth(0);
		painter->setPen(pen);
		painter->drawRect(mPos.x() - handSize/2,
						  mPos.y() - handSize/2,
						  handSize, handSize);
		painter->restore();
	}


}

void VertexEditor::init()
{
	emit actionsChanged();
}

QList<const CtrlAction*> VertexEditor::actions() const
{
	QList<const CtrlAction*> out;
	if (mState == SELECTED)
	{
		out << &mMoveAction;
		out << &mDelAction;
	}
	return out;
}
void VertexEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == MOVE)
	{
		mPos = mCtrl->snapToRouteGrid(mCtrl->view()->transform().inverted()
									  .map(event->pos()));
		updateMove();
		emit overlayChanged();
	}
}

void VertexEditor::mousePressEvent(QMouseEvent *event)
{
	if (mState == MOVE)
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void VertexEditor::mouseReleaseEvent(QMouseEvent *event)
{
	if (mState == MOVE)
	{
		event->accept();
		finishMove();
	}
	else
	{
		event->ignore();
	}
}

void VertexEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && mState == MOVE)
	{
		abortMove();
	}
}

void VertexEditor::startMove()
{
	mPrevState = mVtx->getState();
	mState = MOVE;
	emit actionsChanged();
	emit overlayChanged();
}

void VertexEditor::updateMove()
{
	mVtx->setPos(mPos);
}

void VertexEditor::finishMove()
{
	PCBObjEditCmd* cmd = new PCBObjEditCmd(NULL, mVtx, mPrevState);
	mCtrl->doc()->doCommand(cmd);
	mState = SELECTED;
	emit actionsChanged();
	emit overlayChanged();
}

void VertexEditor::abortMove()
{
	mVtx->loadState(mPrevState);
	mState = SELECTED;
	mPos = mVtx->pos();
	emit actionsChanged();
	emit overlayChanged();
}

void VertexEditor::deleteVtx()
{
	QList<QSharedPointer<Segment> > segs = mVtx->segments().toList();
	// do not delete tees or vertices with only one segment
	if (segs.size() != 2)
		return;
	QSharedPointer<TraceList> tl = dynamic_cast<PCBDoc*>(mCtrl->doc())->traceList();
	QSharedPointer<Vertex> v2 = segs[1]->otherVertex(mVtx);
	QUndoCommand *parent = new QUndoCommand("delete vertex");
	// reattach the first segment
	tl->swapVtxCmd(segs[0], mVtx, v2, parent);
	// delete the second segment
	tl->removeSegmentCmd(segs[1], parent);
	// exec the command
	mCtrl->doc()->doCommand(parent);
	emit editorFinished();
}
