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
class Document;
class PCBDoc;
class FPDoc;
class PCBObject;
class AbstractEditor;
class LayerWidget;
class SelFilterWidget;
class ActionBar;
class Layer;
class Controller;

/// Action triggered by a function key
class CtrlAction : public QObject
{
	Q_OBJECT

public:
	CtrlAction(int key, QString text) : mKey(key), mText(text) {}

	int key() const { return mKey; }
	QString text() const { return mText; }
	void setText(QString text) { mText = text; }

public slots:
	void exec() { emit execFired(); }

signals:
	void execFired();

private:
	int mKey;
	QString mText;
};

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

	void draw(QPainter* painter, QRect &rect, const Layer &layer);

	bool docIsOpen() {return doc() != NULL;}

	void selectObj(PCBObject* obj);
	void hideObj(PCBObject* obj);
	void unhideObj(PCBObject* obj);

	virtual Document* doc() = 0;
	PCBView* view() {return mView; }

	QPoint snapToPlaceGrid(const QPoint &p) const;
	QPoint snapToRouteGrid(const QPoint &p) const;

	bool isLayerVisible(const Layer& l) const;
	const Layer& activeLayer() const;

	void registerAction(CtrlAction* action) { mActions.append(action); updateActions(); }
	void installEditor(AbstractEditor* editor);

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
	AbstractEditor* mEditor;
	LayerWidget* mLayerWidget;
	ActionBar* mActionBar;

	/// Selected objects
	QList<PCBObject*> mSelectedObjs;

	/// Hidden objects
	QList<PCBObject*> mHiddenObjs;

	/// List of registered actions
	QList<const CtrlAction*> mActions;

	int mPlaceGrid;
	int mRouteGrid;
};

class PCBController : public Controller
{
	Q_OBJECT
public:
	explicit PCBController(QObject *parent = 0);

	void registerDoc(PCBDoc* doc);
	virtual Document* doc();

protected slots:
	void onAddTextAction();


protected:
	PCBDoc* mDoc;
	CtrlAction mAddTextAction;
};

class FPController : public Controller
{
	Q_OBJECT
public:
	explicit FPController(QObject *parent = 0);

	void registerDoc(FPDoc* doc);
	virtual Document* doc();
	FPDoc* fpDoc() { return mDoc; }

protected slots:
	void onAddPinAction();
	void onAddTextAction();

protected:
	FPDoc* mDoc;
	CtrlAction mAddPinAction;
	CtrlAction mAddTextAction;

};

#endif // CONTROLLER_H
