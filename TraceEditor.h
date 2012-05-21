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

#ifndef TRACEEDITOR_H
#define TRACEEDITOR_H

#include "Editor.h"
#include "Controller.h"
#include "Trace.h"


class NewTraceEditor : public AbstractEditor
{
	Q_OBJECT

public:
	explicit NewTraceEditor(Controller *ctrl);

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
	virtual QList<const CtrlAction*> actions() const;

protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:

private:
	enum State { PICK_START, PICK_END };
	enum Mode { Mode45Straight, ModeStraight45 };

	void updateDogleg();
	void toggleMode();

	Controller* mCtrl;

	State mState;
	Mode mMode;
	QPoint mPos;
	QPoint mStartPt;
	int mWidth;
	Layer mLayer;

	QSharedPointer<Vertex> mVtxStart;
	QSharedPointer<Vertex> mVtxMid;
	QSharedPointer<Vertex> mVtxEnd;
	QSharedPointer<Segment> mSeg1;
	QSharedPointer<Segment> mSeg2;
};

class SegmentEditor : public AbstractEditor
{
	Q_OBJECT

public:
	explicit SegmentEditor(Controller *ctrl, QSharedPointer<Segment> segment);

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
	virtual QList<const CtrlAction*> actions() const;

protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void startSlideSegment();
	void startInsertVtx();
	void deleteSegment();
	void setLayer();
	void setWidth();

private:
	enum State { SELECTED, SLIDE, ADD_VTX };

	void updateSlide();
	void finishSlide();
	void abortSlide();
	void finishAddVtx();
	void cleanUpTrace(QList<QSharedPointer<Segment> > segments,
									 QUndoCommand* parent);

	State mState;

	Controller* mCtrl;
	QSharedPointer<Segment> mSegment;

	QPoint mPos;

	// stuff used by move functions
	QPoint mPt1;
	QPoint mPt2;
	QPoint mSecantVec;
	// neighboring segments
	QSharedPointer<Segment> mSeg1;
	QSharedPointer<Segment> mSeg2;
	// direction vectors for neighboring vertices
	QPoint mVector1;
	QPoint mVector2;
	// fixed points (outer vertices that stay put during a slide)
	QPoint mFixedPt1;
	QPoint mFixedPt2;
	// signs (used to check move for validity)
	short mSignX;
	short mSignY;

	CtrlAction mSlideAction;
	CtrlAction mAddVtxAction;
	CtrlAction mDelAction;
	CtrlAction mSetLayerAction;
	CtrlAction mSetWidthAction;
};

class VertexEditor : public AbstractEditor
{
	Q_OBJECT

public:
	explicit VertexEditor(Controller *ctrl, QSharedPointer<Vertex> vtx);

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
	virtual QList<const CtrlAction*> actions() const;

protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void startMove();
	void deleteVtx();

private:
	enum State { SELECTED, MOVE };

	void updateMove();
	void finishMove();
	void abortMove();

	State mState;

	Controller* mCtrl;
	QSharedPointer<Vertex> mVtx;
	PCBObjState mPrevState;

	QPoint mPos;

	CtrlAction mMoveAction;
	CtrlAction mDelAction;
};

#endif // TRACEEDITOR_H
