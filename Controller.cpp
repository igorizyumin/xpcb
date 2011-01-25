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

Controller::Controller(QObject *parent) :
	QObject(parent), mView(NULL), mDoc(NULL), mEditor(NULL), mPlaceGrid(IN2PCB(0.05)), mRouteGrid(IN2PCB(0.001))
{
}

void Controller::registerDoc(PCBDoc* doc)
{
	if (!mDoc && doc)
	{
		mDoc = doc;
		connect(mDoc, SIGNAL(changed()), this, SLOT(onDocChanged()));
		emit documentChanged();
	}
	else if (!doc)
	{
		disconnect(mDoc, SIGNAL(changed()), this, SLOT(onDocChanged()));
		mDoc = NULL;
		emit documentChanged();
	}
}

void Controller::onDocChanged()
{
	emit documentChanged();
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

void Controller::registerActions(QList<QAction *> actions)
{
	mActions = actions;
}

void Controller::draw(QPainter* painter, QRect &rect, PCBLAYER layer)
{
	if (!mView || !mDoc) return;
	QList<PCBObject*> objs = mDoc->findObjs(rect);
	foreach(PCBObject* obj, objs)
	{
		if (!mHiddenObjs.contains(obj))
			obj->draw(painter, layer);
	}

	if (layer == LAY_SELECTION)
	{
		foreach(PCBObject* o, mSelectedObjs)
		{
			if (!mHiddenObjs.contains(o))
				painter->drawRect(o->bbox());
		}
		if (mEditor)
			mEditor->drawOverlay(painter);
	}
}

bool Controller::eventFilter(QObject *watched, QEvent *event)
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
	if (!mDoc || !mView)
	{
		event->ignore();
		return;
	}
	QPoint pos = mView->transform().inverted().map(event->pos());
	QList<PCBObject*> objs = mDoc->findObjs(pos);
	if (objs.size() == 0)
	{
		mSelectedObjs.clear();
		updateEditor();
		emit selectionChanged();
		return;
	}
	if (mSelectedObjs.size() != 1)
	{
		mSelectedObjs.clear();
		mSelectedObjs.append(objs.first());
		updateEditor();
		emit selectionChanged();
	}
	else // one obj currently selected
	{
		PCBObject* currObj = mSelectedObjs.first();
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
		emit selectionChanged();
	}

	event->accept();
}

void Controller::updateEditor()
{
	if (mEditor)
	{
		delete mEditor;
		mEditor = NULL;
	}
	mHiddenObjs.clear();
	if (mSelectedObjs.size() == 1)
	{
		mEditor = EditorFactory::instance().newEditor(mSelectedObjs[0], this, mActions);
		mView->installEventFilter(mEditor);
		connect(mEditor, SIGNAL(overlayChanged()), this, SLOT(onEditorOverlayChanged()));
		connect(mEditor, SIGNAL(editorFinished()), this, SLOT(onEditorFinished()));
	}
}

void Controller::hideObj(PCBObject *obj)
{
	mHiddenObjs.append(obj);
}

void Controller::onEditorOverlayChanged()
{
	emit selectionChanged();
}

void Controller::onEditorFinished()
{
	mSelectedObjs.clear();
	updateEditor();
}

QPoint Controller::snapToPlaceGrid(QPoint p)
{
	return QPoint(((p.x() + mPlaceGrid/2) / mPlaceGrid) * mPlaceGrid,
				  ((p.y() + mPlaceGrid/2) / mPlaceGrid) * mPlaceGrid);
}

QPoint Controller::snapToRouteGrid(QPoint p)
{
	return QPoint(((p.x() + mRouteGrid/2) / mRouteGrid) * mRouteGrid,
				  ((p.y() + mRouteGrid/2) / mRouteGrid) * mRouteGrid);
}
