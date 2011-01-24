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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "global.h"
#include <QPainter>
#include <QMouseEvent>
#include <QUndoCommand>

class PCBView;
class PCBDoc;
class PCBObject;
class AbstractEditor;

/// The controller mediates all interaction between the model and view.  It
/// also manages selection.  Other control tasks are handled by delegates.
class Controller : public QObject
{
    Q_OBJECT
public:
	explicit Controller(QObject *parent = 0);

	void registerDoc(PCBDoc* doc);
	void registerView(PCBView* view);
	void registerActions(QList<QAction*> actions);

	void draw(QPainter* painter, QRect &rect, PCBLAYER layer);

	bool docIsOpen() {return mDoc != NULL;}

	void hideObj(PCBObject* obj);

	PCBDoc* doc() { return mDoc; }
	PCBView* view() {return mView; }

	QPoint snapToPlaceGrid(QPoint p);
	QPoint snapToRouteGrid(QPoint p);
signals:
	void selectionChanged();
	void documentChanged();

public slots:
	void onPlaceGridChanged(int grid) { mPlaceGrid = grid; }
	void onRouteGridChanged(int grid) { mRouteGrid = grid; }
private slots:
	void onDocChanged();
	void onEditorOverlayChanged();

private:
	virtual bool eventFilter(QObject *watched, QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void updateEditor();

	PCBView* mView;
	PCBDoc* mDoc;
	AbstractEditor* mEditor;

	/// Selected objects
	QList<PCBObject*> mSelectedObjs;

	/// Hidden objects
	QList<PCBObject*> mHiddenObjs;

	/// Actions
	QList<QAction*> mActions;

	int mPlaceGrid;
	int mRouteGrid;
};

#endif // CONTROLLER_H
