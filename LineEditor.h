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

#ifndef LINEEDITOR_H
#define LINEEDITOR_H

#include "Editor.h"

class LineEditor : public AbstractEditor
{
public:
	LineEditor(FPController* ctrl, Line* line = NULL);
	virtual ~LineEditor();

	virtual void drawOverlay(QPainter* painter);
	virtual void init();
	virtual QList<CtrlAction> actions() const;
	virtual void action(int key);

protected:
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void keyPressEvent(QKeyEvent *event);

private:
	enum State {
		SELECTED,		///< Line is selected
		VTX_MOVE,		///< Moving vertex
		LINE_MOVE,		///< Moving entire line
		LINE_NEW_FIRST,		///< Drawing a new line, first point
		LINE_NEW_SECOND		///< Drawing a new line, second point
	};

	State mState;
	Line* mLine;
	QPoint mPos;

};

#endif // LINEEDITOR_H
