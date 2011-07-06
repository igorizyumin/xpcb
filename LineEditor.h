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

#ifndef LINEEDITOR_H
#define LINEEDITOR_H

#include <QSharedPointer>
#include "Editor.h"
#include "Controller.h"
#include "EditLineDialog.h"

class LineEditor : public AbstractEditor
{
    Q_OBJECT

public:
	explicit LineEditor(FPController *ctrl, QSharedPointer<Line> part = QSharedPointer<Line>());

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
	virtual QList<const CtrlAction*> actions() const;

protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void actionLine() { setLineType(Line::LINE); }
	void actionArcCW() { setLineType(Line::ARC_CW); }
	void actionArcCCW() { setLineType(Line::ARC_CCW); }
	void actionEdit();
	void actionMove();
	void actionDel();
	void actionMoveVtx();

private:
	void newLine();
	void setLineType(Line::LineType type);

	enum State { SELECTED, VTX_SEL_START, VTX_SEL_END, VTX_MOVE_START,
				 VTX_MOVE_END, PICK_REF, LINE_MOVE, LINE_NEW_FIRST,
				 LINE_NEW_SECOND };

	State mState;

	QSharedPointer<Line> mLine;
	QSharedPointer<EditLineDialog> mDialog;

	int mWidth;
	Layer mLayer;
	QPoint mPos;
	QPoint mRefPt;
	Line::LineType mLineType;
	PCBObjState mPrevState;

	CtrlAction mLineAction;
	CtrlAction mArcCWAction;
	CtrlAction mArcCCWAction;
	CtrlAction mEditAction;
	CtrlAction mMoveAction;
	CtrlAction mDelAction;
	CtrlAction mMoveVtxAction;
};

#endif // LINEEDITOR_H
