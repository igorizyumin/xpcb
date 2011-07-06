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

#include "Controller.h"
#include "PCBDoc.h"
#include "PCBView.h"
#include "Editor.h"
#include "LayerWidget.h"
#include "Log.h"
#include "ActionBar.h"
#include "FPPropDialog.h"

Controller::Controller(QObject *parent) :
	QObject(parent), mView(NULL), mLayerWidget(NULL), mActionBar(NULL),
	mPlaceGrid(XPcb::IN2PCB(0.05)), mRouteGrid(XPcb::IN2PCB(0.001))
{
}

void Controller::registerView(PCBView* view)
{
	if (!mView)
	{
		mView = view;
		view->installEventFilter(this);
		view->setCtrl(this);
	}
}

void Controller::registerActionBar(ActionBar* bar)
{
	mActionBar = bar;
	updateActions();
}

void Controller::registerLayerWidget(LayerWidget *widget)
{
	mLayerWidget = widget;
	if (doc())
		mLayerWidget->layersChanged(doc()->layerList());
	connect(mLayerWidget, SIGNAL(layerVisibilityChanged()), this, SLOT(onDocumentChanged()));
	connect(mLayerWidget, SIGNAL(currLayerChanged(const Layer&)), this, SLOT(onDocumentChanged()));

}

void Controller::draw(QPainter* painter, QRect &rect, const Layer& layer)
{
	if (!mView || !doc()) return;


	if (layer == Layer::LAY_SELECTION)
	{
		if (mEditor)
			mEditor->drawOverlay(painter);
	}
	else
	{
		QList<QSharedPointer<PCBObject> > objs = doc()->findObjs(rect);
		foreach(QSharedPointer<PCBObject> obj, objs)
		{
			if (!mLayerWidget->isLayerVisible(Layer::LAY_SELECTION) ||
				!mHiddenObjs.contains(obj))
				obj->draw(painter, layer);
		}
	}
}

bool Controller::eventFilter(QObject *, QEvent *event)
{
	event->accept();
	if (event->type() == QEvent::MouseMove)
	{
		mouseMoveEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::MouseButtonPress)
	{
		mousePressEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		mouseReleaseEvent(static_cast<QMouseEvent*>(event));
	}
	else
	{
		event->ignore();
		return false;
	}
	return event->isAccepted();
}

void Controller::mouseMoveEvent(QMouseEvent *event)
{
	event->ignore();
}

void Controller::mousePressEvent(QMouseEvent *event)
{
	event->ignore();
}

void Controller::mouseReleaseEvent(QMouseEvent *event)
{
	if (!doc() || !mView)
	{
		event->ignore();
		return;
	}
	QPoint pos = mView->transform().inverted().map(event->pos());
	QList<QSharedPointer<PCBObject> > objs = doc()->findObjs(pos);
	QMutableListIterator<QSharedPointer<PCBObject> > i(objs);
	while(i.hasNext())
	{
		QSharedPointer<PCBObject> obj = i.next();
		bool hit = false;
		// traverse list of layers in reverse draw order (topmost layer first)
		QList<Layer> layerList = doc()->layerList(PCBDoc::DrawPriorityOrder);
		QListIterator<Layer> j(layerList);
		j.toBack();
		while(j.hasPrevious())
		{
			const Layer& l = j.previous();
			if (!mLayerWidget->isLayerVisible(l)) continue;
			if (obj->testHit(pos, l))
			{
				hit = true;
				break;
			}
		}
		if (!hit)
			i.remove();
	}

	if (objs.size() == 0)
	{
		mSelectedObjs.clear();
		updateEditor();
		mView->update();
		return;
	}
	if (mSelectedObjs.size() != 1)
	{
		mSelectedObjs.clear();
		mSelectedObjs.append(objs.first());
		updateEditor();
		mView->update();
	}
	else // one obj currently selected
	{
		QSharedPointer<PCBObject> currObj = mSelectedObjs.first();
		mSelectedObjs.clear();
		// if only one object is under cursor, switch to the new one (if it's another one)
		if (objs.size() == 1)
		{
			if (currObj != objs.first())
				mSelectedObjs.append(objs.first());
		}
		// multiple objects under cursor, pick the next one if the current one is among them, otherwise just choose the first one
		else
		{
			int i = 0;
			if (objs.contains(currObj))
				i = (objs.indexOf(currObj) + 1) % objs.size();
			mSelectedObjs.append(objs.at(i));
		}
		updateEditor();
		mView->update();
	}

	event->accept();
}

void Controller::updateEditor()
{
	mEditor.clear();
	mHiddenObjs.clear();
	if (mSelectedObjs.size() == 1)
	{
		installEditor(EditorFactory::instance().newEditor(mSelectedObjs[0], this));
	}
	else
		updateActions();
}

void Controller::installEditor(QSharedPointer<AbstractEditor> editor)
{
	Q_ASSERT(!mEditor);
	if (!editor) return;

	mEditor = editor;

	mView->installEventFilter(mEditor.data());
	connect(mEditor.data(), SIGNAL(overlayChanged()), this, SLOT(onEditorOverlayChanged()));
	connect(mEditor.data(), SIGNAL(editorFinished()), this, SLOT(onEditorFinished()));
	connect(mEditor.data(), SIGNAL(actionsChanged()), this, SLOT(onEditorActionsChanged()));
	updateActions();
	mEditor->init();
}

void Controller::selectObj(QSharedPointer<PCBObject> obj)
{
	mSelectedObjs.append(obj);
	mView->update();
}

void Controller::hideObj(QSharedPointer<PCBObject> obj)
{
	mHiddenObjs.append(obj);
}

void Controller::unhideObj(QSharedPointer<PCBObject> obj)
{
	mHiddenObjs.removeAll(obj);
}

void Controller::onEditorOverlayChanged()
{
	mView->update();
}

void Controller::onEditorFinished()
{
	mSelectedObjs.clear();
	updateEditor();
	mView->update();
}

void Controller::onEditorActionsChanged()
{
	updateActions();
}

void Controller::onDocumentChanged()
{
	mView->update();
}

QPoint Controller::snapToPlaceGrid(const QPoint &p) const
{
	return QPoint(((p.x() + mPlaceGrid/2) / mPlaceGrid) * mPlaceGrid,
				  ((p.y() + mPlaceGrid/2) / mPlaceGrid) * mPlaceGrid);
}

QPoint Controller::snapToRouteGrid(const QPoint &p) const
{
	return QPoint(((p.x() + mRouteGrid/2) / mRouteGrid) * mRouteGrid,
				  ((p.y() + mRouteGrid/2) / mRouteGrid) * mRouteGrid);
}

bool Controller::isLayerVisible(const Layer& l) const
{
	if (!mLayerWidget) return false;
	else return mLayerWidget->isLayerVisible(l);
}

const Layer& Controller::activeLayer() const
{
	return mLayerWidget->activeLayer();
}

void Controller::updateActions()
{
	if (!mActionBar) return;

	if (!doc())
	{
		mActionBar->clearActions();
		return;
	}

	if (!mEditor)
	{
		mActionBar->setActions(mActions);
	}
	else
	{
		mActionBar->setActions(mEditor->actions());
	}
}


//////////////////////////// PCBCONTROLLER /////////////////////////////////

PCBController::PCBController(QObject *parent)
		: Controller(parent), mDoc(NULL), mAddTextAction(2, "Add Text"), mAddPartAction(3, "Add Part")
{
	connect(&mAddTextAction, SIGNAL(execFired()), this, SLOT(onAddTextAction()));
	connect(&mAddPartAction, SIGNAL(execFired()), this, SLOT(onAddPartAction()));
	registerAction(&mAddTextAction);
	registerAction(&mAddPartAction);
}

void PCBController::registerDoc(PCBDoc* doc)
{
	if (!mDoc && doc)
	{
		mDoc = doc;
		if (mLayerWidget)
			mLayerWidget->layersChanged(mDoc->layerList());
		connect(mDoc, SIGNAL(changed()), this, SLOT(onDocumentChanged()));
		onDocumentChanged();
	}
	else if (mDoc && !doc)
	{
		disconnect(mDoc, SIGNAL(changed()), this, SIGNAL(onDocumentChanged()));
		mDoc = NULL;
		mSelectedObjs.clear();
		mHiddenObjs.clear();
		mEditor.clear();
		onDocumentChanged();
	}
	updateEditor();
}

Document* PCBController::doc()
{
	return mDoc;
}

void PCBController::onAddTextAction()
{
	Q_ASSERT(mEditor == NULL && mSelectedObjs.size() == 0);

	installEditor(EditorFactory::instance().newTextEditor(this));
}

void PCBController::onAddPartAction()
{
	Q_ASSERT(mEditor == NULL && mSelectedObjs.size() == 0);

	installEditor(EditorFactory::instance().newPartEditor(this));
}


//////////////////////////// FPCONTROLLER /////////////////////////////////

FPController::FPController(QObject *parent)
		: Controller(parent), mDoc(NULL),
		  mAddLineAction(2, "Add Line"),
		  mAddPinAction(3, "Add Pin"),
		  mAddTextAction(1, "Add Text"),
		  mEditPropsAction(0, "Edit Properties")
{
	connect(&mAddLineAction, SIGNAL(execFired()), SLOT(onAddLineAction()));
	connect(&mAddPinAction, SIGNAL(execFired()), SLOT(onAddPinAction()));
	connect(&mAddTextAction, SIGNAL(execFired()), SLOT(onAddTextAction()));
	connect(&mEditPropsAction, SIGNAL(execFired()), this, SLOT(onEditPropsAction()));
	registerAction(&mAddLineAction);
	registerAction(&mEditPropsAction);
	registerAction(&mAddPinAction);
	registerAction(&mAddTextAction);
}

void FPController::registerDoc(FPDoc* doc)
{
	if (!mDoc && doc)
	{
		mDoc = doc;
		if (mLayerWidget)
			mLayerWidget->layersChanged(mDoc->layerList());
		connect(mDoc, SIGNAL(changed()), this, SLOT(onDocumentChanged()));
		onDocumentChanged();
	}
	else if (mDoc && !doc)
	{
		disconnect(mDoc, SIGNAL(changed()), this, SIGNAL(onDocumentChanged()));
		mDoc = NULL;
		mSelectedObjs.clear();
		mHiddenObjs.clear();
		mEditor.clear();
		onDocumentChanged();
	}
	updateEditor();
}

Document* FPController::doc()
{
	return mDoc;
}

void FPController::onAddPinAction()
{
	Q_ASSERT(mEditor == NULL && mSelectedObjs.size() == 0);

	installEditor(EditorFactory::instance().newPinEditor(this));
}

void FPController::onAddTextAction()
{
	Q_ASSERT(mEditor == NULL && mSelectedObjs.size() == 0);

	installEditor(EditorFactory::instance().newTextEditor(this));
}

void FPController::onAddLineAction()
{
	Q_ASSERT(mEditor == NULL && mSelectedObjs.size() == 0);

	installEditor(EditorFactory::instance().newLineEditor(this));
}

void FPController::onEditPropsAction()
{
	FPPropDialog d(this->view());
	d.init(mDoc->footprint().data());
	int result = d.exec();
	if (result == QDialog::Accepted)
	{
		d.updateFp(mDoc->footprint().data());
	}
}
