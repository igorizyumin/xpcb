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

// utility functions
// signum function
inline short sign(double x)
{
	return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

// computes perpendicular vector
inline QPoint perp(const QPoint& v)
{
	return QPoint(-v.y(), v.x());
}

// dot (scalar) product
inline int dotProd(const QPoint &pt1, const QPoint &pt2)
{
	return pt1.x() * pt2.x() + pt1.y() * pt2.y();
}

inline bool isParallel(const QPoint &dir1, const QPoint &dir2)
{
	return dotProd(dir1, perp(dir2)) == 0;
}

// finds the line intersection of the two lines given by (pt1, dir1)
// and (pt2, dir2)
// returns scale factor for dir1 vector (intersect. pt = pt1 + retVal * dir1
// check for parallel-ness before using this
inline double lineIntersect(const QPoint &pt1, const QPoint &dir1,
					 const QPoint &pt2, const QPoint &dir2)
{
	QPoint w = pt1 - pt2;
	return (double(dir2.y())*w.x() - double(dir2.x())*w.y()) /
			(double(dir2.x())*dir1.y() - double(dir2.y())*dir1.x());
}

// returns line intersection point
// check for lines being parallel first!
inline QPoint lineIntersectPt(const QPoint &pt1, const QPoint &dir1,
							  const QPoint &pt2, const QPoint &dir2)
{
	return pt1 + dir1 * lineIntersect(pt1, dir1, pt2, dir2);
}

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

NewTraceEditor::NewTraceEditor(PCBController *ctrl)
	: AbstractEditor(ctrl), mCtrl(ctrl), mState(PICK_START),
	  mMode(ModeStraight45), mWidth(XPcb::MIL2PCB(10)),
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
		QUndoCommand* cmd = mCtrl->pcbDoc()->traceList()
				.addSegmentCmd(mSeg1, mVtxStart, mVtxMid);
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

SegmentEditor::SegmentEditor(PCBController *ctrl,
							 QSharedPointer<Segment> segment)
	: AbstractEditor(ctrl), mState(SELECTED), mCtrl(ctrl), mSegment(segment),
	  mSlideAction(3, "Slide Segment")
{
	connect(&mSlideAction, SIGNAL(execFired()),
			this, SLOT(startSlideSegment()));
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
}

void SegmentEditor::init()
{
	emit actionsChanged();
}

QList<const CtrlAction*> SegmentEditor::actions() const
{
	QList<const CtrlAction*> out;
	if (mState == SELECTED)
		out << &mSlideAction;
	return out;
}

void SegmentEditor::mouseMoveEvent(QMouseEvent *event)
{
	if (mState == SLIDE)
	{
		mPos = mCtrl->snapToRouteGrid(mCtrl->view()->transform().inverted()
									  .map(event->pos()));
		updateSlide();
		emit overlayChanged();
	}
}

void SegmentEditor::mousePressEvent(QMouseEvent *event)
{
	if (mState == SLIDE)
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
	else
	{
		event->ignore();
	}
}

void SegmentEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
		emit editorFinished();
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
	TraceList* tl = &mCtrl->pcbDoc()->traceList();

	// debug: print the list
//	Log::instance().message("#\tid\tv1\tv2");
//	int i = 0;
//	foreach(QSharedPointer<Segment> s, segments)
//	{
//		Log::instance().message(QString("%1\t%2\t%3\t%4").arg(i)
//								.arg(s->getid()).arg(s->v1()->getid())
//								.arg(s->v2()->getid()));
//		i++;
//	}

	// list of nonzero length segments and their start vertices
	QList<SegListEntry> nzList;

	// start vertex for current segment
	QSharedPointer<Vertex> startVtx =
			segments[0]->otherVertex(segments[0]->commonVertex(segments[1]));
	foreach(QSharedPointer<Segment> s, segments)
	{
//		Log::instance().message(QString("curr is %1: %2\t%3").arg(s->getid()).arg(s->v1()->getid()).arg(s->v2()->getid()));
		// remove all zero length segments
		if (segZeroLength(s))
		{
			// remove segment
			tl->removeSegmentCmd(s, parent);
//			Log::instance().message("removing zero length segment");
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
//				Log::instance().message(QString("swap on %1 : %2 -> %3").arg(s->getid()).arg(startVtx->getid()).arg(prevVtx->getid()));
//				Log::instance().message(QString("delete seg %1").arg(nzList.last().seg->getid()));
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

void SegmentEditor::removeSegAndJoin(QSharedPointer<Segment> seg, QUndoCommand *parent)
{
	TraceList* tl = &mCtrl->pcbDoc()->traceList();
	QSet<QSharedPointer<Segment> > v1s = seg->v1()->segments();
	v1s.remove(seg);
	QSet<QSharedPointer<Segment> > v2s = seg->v2()->segments();
	v2s.remove(seg);
	// check for collinearity
	if (!v1s.isEmpty() && !v2s.isEmpty()
			&& segParallel(v1s.values().first(), v2s.values().first()))
	{
		// remove the extra segment
		tl->swapVtxCmd(v1s.values().first(), seg->v1(),
					   v2s.values().first()->otherVertex(seg->v2()), parent);
		tl->removeSegmentCmd(v2s.values().first(), parent);
	}
	else if (!v1s.isEmpty())
	{
		tl->swapVtxCmd(v1s.values().first(), seg->v1(), seg->v2(), parent);
	}
	else if (!v2s.isEmpty())
	{
		tl->swapVtxCmd(v2s.values().first(), seg->v2(), seg->v1(), parent);
	}
	tl->removeSegmentCmd(seg, parent);
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
	if (!mCtrl->pcbDoc()->traceList().segments().contains(mSegment))
	{
		emit editorFinished();
		return;
	}
	mState = SELECTED;
	emit actionsChanged();
	emit overlayChanged();
}


