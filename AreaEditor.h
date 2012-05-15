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

#ifndef AREAEDITOR_H
#define AREAEDITOR_H

#include "Editor.h"
#include "Polygon.h"

class AreaEditor : public AbstractEditor
{
	Q_OBJECT
public:
	explicit AreaEditor(PCBController *ctrl);

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
	virtual QList<const CtrlAction*> actions() const;

protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private:
	enum State { PICK_FIRST, PICK_NEXT };

	void drawSeg(QPainter *painter, QPoint start, QPoint end, PolyContour::Segment::SegType type);
	void updatePos(QPoint pos);
	void finishPolygon();

	QPoint mPos;

	PCBController* mCtrl;

	State mState;
	PolyContour::Segment::SegType mCurrSegType;
	Layer mLayer;

	QList<PolyContour::Segment> mSegments;
};

#endif // AREAEDITOR_H
