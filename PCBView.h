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

#ifndef QPCBVIEW_H
#define QPCBVIEW_H

#include <QWidget>
#include <QTransform>
#include "global.h"

class PCBDoc;
class PCBObject;
class Controller;

class PCBView : public QWidget
{
	Q_OBJECT

public:
        PCBView(QWidget *parent);
        ~PCBView();


	virtual QSize sizeHint() const;

	/// This gets called by the controller when the view is registered.
	void setCtrl(Controller* ctrl);

	const QTransform& transform() const { return mTransform; }

signals:
	void mouseMoved(QPoint pt);

public slots:
	void visGridChanged(int grid);
	//	void layerVisChanged();

protected:
	virtual void paintEvent(QPaintEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);
	virtual void wheelEvent(QWheelEvent *);

private:
	void drawOrigin(QPainter *painter);
	void drawGrid(QPainter *painter);
	void recenter(QPoint pt, bool world=false);
	void zoom(double factor, QPoint pos);

	/// Controller
	Controller* mCtrl;

	/// Transform from world coordinates to window coordinates
	QTransform mTransform;
	/// Visible grid size in PCB units
	int mVisibleGrid;
	/// Mouse position (world coordinates)
	QPoint mMousePos;
	/// Accumulated mouse wheel angle
	int mWheelAngle;
};

#endif // QPCBVIEW_H
