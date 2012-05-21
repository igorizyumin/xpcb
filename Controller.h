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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "global.h"
#include <QPointer>
#include <QPainter>
#include <QMouseEvent>
#include <QUndoCommand>
#include "PCBView.h"
#include "PCBObject.h"
#include "Editor.h"

class Document;
class PCBDoc;
class FPDoc;
class LayerWidget;
class SelFilterWidget;
class ActionBar;
class Layer;
class Controller;
class CtrlAction;

/// The controller mediates all interaction between the model and view.  It
/// also manages selection.  Other control tasks are handled by delegates.
class Controller : public QObject
{
    Q_OBJECT
public:

	enum SelectionMaskT { SM_PARTS, SM_REFDES, SM_VALUE, SM_PINS, SM_TRACES,
						  SM_VERTICES, SM_AREAS, SM_TEXT, SM_CUTOUTS, SM_OUTLINE, SM_DRC };

	explicit Controller(QObject *parent = 0);

	void registerView(PCBView* view);
	void registerActionBar(ActionBar* bar);
	void registerLayerWidget(LayerWidget* widget);
	void registerDoc(Document* doc);

	void draw(QPainter* painter, QRect &rect, const Layer &layer);

	bool docIsOpen() {return doc() != NULL;}

	void selectObj(QSharedPointer<PCBObject> obj);
	void hideObj(QSharedPointer<PCBObject> obj);
	void unhideObj(QSharedPointer<PCBObject> obj);

	Document* doc() {return mDoc; }
	PCBView* view() {return mView; }

	QPoint snapToPlaceGrid(const QPoint &p) const;
	QPoint snapToRouteGrid(const QPoint &p) const;
	// returns the object hit radius in pcb units
	int hitRadius() const {return 10*mView->transform().inverted().m11(); }

	bool isLayerVisible(const Layer& l) const;
	const Layer& activeLayer() const;

	void registerAction(const CtrlAction* action) { mActions.append(action); updateActions(); }
	void installEditor(QSharedPointer<AbstractEditor> editor);

	void clearSelection() { mSelectedObjs.clear(); updateEditor(); }
public slots:
	void onPlaceGridChanged(int grid) { mPlaceGrid = grid; }
	void onRouteGridChanged(int grid) { mRouteGrid = grid; }

protected slots:
	void onEditorOverlayChanged();
	void onEditorFinished();
	void onEditorActionsChanged();
	void onDocumentChanged();

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void updateEditor();
	void updateActions();

	PCBView* mView;
	Document* mDoc;
	QSharedPointer<AbstractEditor> mEditor;
	LayerWidget* mLayerWidget;
	ActionBar* mActionBar;

	/// Selected objects
	QList<QSharedPointer<PCBObject> > mSelectedObjs;

	/// Hidden objects
	QList<QSharedPointer<PCBObject> > mHiddenObjs;

	/// List of registered actions
	QList<const CtrlAction* > mActions;

	int mPlaceGrid;
	int mRouteGrid;
};

#endif // CONTROLLER_H
